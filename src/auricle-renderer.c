/* auricle-renderer.c
 *
 * Copyright 2019 Ryan Gonzalez <rymg19@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "auricle-renderer.h"
#include "auricle-utils.h"
#include <gst/gst.h>
#include <gst/app/app.h>
#include <gst/video/video.h>

typedef struct _AuricleRendererPadProbe AuricleRendererPadProbe;

struct _AuricleRendererPadProbe {
  GstPad *pad;
  guint   id;
};

typedef struct _AuricleRendererFileData AuricleRendererFileData;

struct _AuricleRendererFileData
{
  AuricleMusicFile *file;

  GstElement *src;
  GstElement *sink;
  GList      *request_pads;
  GList      *pad_probes;
  gboolean    is_eos;
  gboolean    emitted_finished_progress;
};

static void
auricle_renderer_file_data_destroy (AuricleRendererFileData *data)
{
  g_clear_object (&data->file);

  for (GList *l = data->request_pads; l != NULL; l = l->next)
    {
      g_autoptr(GstPad) pad = GST_PAD (g_steal_pointer (&l->data));
      g_autoptr(GstElement) parent = gst_pad_get_parent_element (pad);
      gst_element_release_request_pad (parent, pad);
    }

  g_list_free (data->request_pads);

  for (GList *l = data->pad_probes; l != NULL; l = l->next)
    {
      AuricleRendererPadProbe *probe = l->data;
      gst_pad_remove_probe (probe->pad, probe->id);
    }

  g_list_free_full (data->pad_probes, g_free);

  g_clear_pointer (&data->src, gst_object_unref);
  g_clear_pointer (&data->sink, gst_object_unref);
}

struct _AuricleRenderer
{
  GObject parent_instance;

  GdkPixbuf            *pixbuf;
  AuricleRenderOptions *render_options;

  GPtrArray  *file_data;
  GstElement *pipeline;
  guint       bus_watch_id;
  guint       progress_timer_id;
};

G_DEFINE_TYPE (AuricleRenderer, auricle_renderer, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_PIXBUF,
  PROP_RENDER_OPTIONS,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

enum {
  SIGNAL_0,
  PROGRESS_UPDATE,
  COMPLETE,
  N_SIGNALS
};

static guint signals [N_SIGNALS];

AuricleRenderer *
auricle_renderer_new (GdkPixbuf            *pixbuf,
                      AuricleRenderOptions *render_options)
{
  return g_object_new (AURICLE_TYPE_RENDERER,
                       "pixbuf", pixbuf,
                       "render-options", render_options,
                       NULL);
}

static void
auricle_renderer_finalize (GObject *object)
{
  AuricleRenderer *self = (AuricleRenderer *)object;

  g_clear_object (&self->pixbuf);
  g_clear_object (&self->render_options);

  if (self->bus_watch_id != 0)
    {
      g_source_remove (self->bus_watch_id);
      self->bus_watch_id = 0;
    }

  if (self->progress_timer_id != 0)
    {
      g_source_remove (self->progress_timer_id);
      self->progress_timer_id = 0;
    }

  g_clear_pointer (&self->file_data, g_ptr_array_unref);

  g_debug ("Destroying renderer");
  gst_element_set_state (self->pipeline, GST_STATE_NULL);
  g_clear_object (&self->pipeline);

  G_OBJECT_CLASS (auricle_renderer_parent_class)->finalize (object);
}

static void
auricle_renderer_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  AuricleRenderer *self = AURICLE_RENDERER (object);

  switch (prop_id)
    {
    case PROP_PIXBUF:
      g_warn_if_fail (self->pixbuf != NULL);
      g_value_set_object (value, self->pixbuf);
      break;
    case PROP_RENDER_OPTIONS:
      g_warn_if_fail (self->render_options != NULL);
      g_value_set_object (value, self->render_options);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
auricle_renderer_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  AuricleRenderer *self = AURICLE_RENDERER (object);

  switch (prop_id)
    {
    case PROP_PIXBUF:
      g_warn_if_fail (self->pixbuf == NULL);
      self->pixbuf = g_value_dup_object (value);
      break;
    case PROP_RENDER_OPTIONS:
      g_warn_if_fail (self->render_options == NULL);
      self->render_options = g_value_dup_object (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
auricle_renderer_class_init (AuricleRendererClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = auricle_renderer_finalize;
  object_class->get_property = auricle_renderer_get_property;
  object_class->set_property = auricle_renderer_set_property;

  properties [PROP_PIXBUF] =
    g_param_spec_object ("pixbuf",
                         "Image pixbuf",
                         "Image pixbuf",
                         GDK_TYPE_PIXBUF,
                         (G_PARAM_READWRITE |
                          G_PARAM_CONSTRUCT_ONLY |
                          G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class, PROP_PIXBUF,
                                   properties [PROP_PIXBUF]);

  properties [PROP_RENDER_OPTIONS] =
    g_param_spec_object ("render-options",
                         "Render options",
                         "Render options",
                         AURICLE_TYPE_RENDER_OPTIONS,
                         (G_PARAM_READWRITE |
                          G_PARAM_CONSTRUCT_ONLY |
                          G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class, PROP_RENDER_OPTIONS,
                                   properties [PROP_RENDER_OPTIONS]);

  signals [PROGRESS_UPDATE] =
    g_signal_new ("progress-update",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL,
                  NULL,
                  g_cclosure_marshal_generic,
                  G_TYPE_NONE,
                  1, G_TYPE_POINTER);

  signals [COMPLETE] =
    g_signal_new ("complete",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL,
                  NULL,
                  g_cclosure_marshal_generic,
                  G_TYPE_NONE,
                  0);
}

static void
auricle_renderer_init (AuricleRenderer *self)
{
  self->file_data = g_ptr_array_new_with_free_func ((GDestroyNotify) auricle_renderer_file_data_destroy);
}

void
auricle_renderer_take_file (AuricleRenderer  *self,
                            AuricleMusicFile *file)
{
  AuricleRendererFileData *data = g_new0 (AuricleRendererFileData, 1);
  data->file = file;
  g_ptr_array_add (self->file_data, data);
}

static gboolean
on_bus_message (GstBus     *bus,
                GstMessage *message,
                gpointer    udata)
{
  AuricleRenderer *self = AURICLE_RENDERER (udata);
  g_autoptr(GError) error = NULL;

  switch (GST_MESSAGE_TYPE (message))
    {
    case GST_MESSAGE_ERROR:
      gst_message_parse_error (message, &error, NULL);
      auricle_show_notification ("Error in render pipeline: %s", error->message);
      // Fallthrough.
    case GST_MESSAGE_EOS:
      self->bus_watch_id = 0;
      auricle_show_notification ("Render complete");
      /* g_signal_emit (self, COMPLETE, 0); */
      return FALSE;
    default:
      break;
    }

  return TRUE;
}

static void
on_dec_pad_added (GstElement *el,
                  GstPad     *pad,
                  GstElement *sink)
{
  g_autoptr(GstPad) sinkpad = gst_element_get_static_pad (sink, "sink");
  if (!gst_pad_is_linked (sinkpad))
    {
      if (gst_pad_link (pad, sinkpad) != GST_PAD_LINK_OK)
        g_error ("Failed to link pads");
    }
}

static gboolean
on_progress_timer (gpointer udata)
{
  AuricleRenderer *self = AURICLE_RENDERER (udata);

  g_autoptr(GstQuery) position_query = gst_query_new_position (GST_FORMAT_TIME);
  g_autoptr(GstQuery) duration_query = gst_query_new_duration (GST_FORMAT_TIME);

  GList *progress = NULL;

  for (int i = 0; i < self->file_data->len; i++)
    {
      AuricleRendererFileData *data = g_ptr_array_index (self->file_data, i);

      AuricleRenderProgress *p = g_new0 (AuricleRenderProgress, 1);
      p->index = i;

      if (data->is_eos)
        {
          if (data->emitted_finished_progress)
            continue;

          p->finished = TRUE;
          data->emitted_finished_progress = TRUE;
        }
      else
        {
          if (!gst_element_query (data->sink, position_query) || !gst_element_query (data->src, duration_query))
            continue;

          gst_query_parse_position (position_query, NULL, &p->position);
          gst_query_parse_duration (duration_query, NULL, &p->duration);
          if (p->position > p->duration)
            p->position = p->duration;
        }

      progress = g_list_prepend (progress, p);
    }

  g_signal_emit (self, signals[PROGRESS_UPDATE], 0, progress);
  g_list_free_full (progress, g_free);

  if (self->bus_watch_id == 0)
    {
      // Likely hit EOS.
      self->progress_timer_id = 0;
      return G_SOURCE_REMOVE;
    }

  return G_SOURCE_CONTINUE;
}

static void
on_image_src_needs_data (GstAppSrc *appsrc,
                         guint      length,
                         gpointer   udata)
{
  GstBuffer *buffer = GST_BUFFER (udata);
  gst_app_src_push_buffer (appsrc, buffer);
  gst_app_src_end_of_stream (appsrc);
}

static GstElement *
auricle_renderer_create_image_source (AuricleRenderer *self)
{
  g_warn_if_fail (gdk_pixbuf_get_colorspace (self->pixbuf) == GDK_COLORSPACE_RGB);

  GstVideoInfo info;
  gst_video_info_init (&info);
  gst_video_info_set_format (&info,
                             gdk_pixbuf_get_has_alpha (self->pixbuf) ? GST_VIDEO_FORMAT_RGBA : GST_VIDEO_FORMAT_RGB,
                             gdk_pixbuf_get_width (self->pixbuf), gdk_pixbuf_get_height (self->pixbuf));
  info.fps_n = 0;
  info.fps_d = 1;

  g_autoptr(GstCaps) caps = gst_video_info_to_caps (&info);

  gsize byte_buffer_size = gdk_pixbuf_get_byte_length (self->pixbuf);
  char *byte_buffer = g_memdup (gdk_pixbuf_read_pixels (self->pixbuf), byte_buffer_size);
  GstBuffer *buffer = gst_buffer_new_wrapped (byte_buffer, byte_buffer_size);

  GstElement *src = gst_element_factory_make ("appsrc", NULL);
  gst_app_src_set_caps (GST_APP_SRC (src), caps);
  g_signal_connect (src, "need-data", G_CALLBACK (on_image_src_needs_data), buffer);

  return src;
}

AuricleMusicFile *
auricle_renderer_get_file (AuricleRenderer *self,
                           int              index)
{
  if (index >= self->file_data->len)
    return NULL;
  return ((AuricleRendererFileData *) g_ptr_array_index (self->file_data, index))->file;
}

static GstPadProbeReturn
on_downstream_audio_pad_event (GstPad          *pad,
                               GstPadProbeInfo *info,
                               gpointer         udata)
{
  AuricleRendererFileData *data = udata;
  GstEvent *evt = GST_EVENT (info->data);

  if (GST_EVENT_TYPE (evt) == GST_EVENT_EOS)
    {
      g_info ("Forwarding an EOS event in %s", auricle_music_file_get_result_name (data->file));
      for (GList *l = data->request_pads; l != NULL; l = l->next) {
        GstPad *pad = GST_PAD (l->data);
        gst_pad_send_event (pad, gst_event_new_eos ());
      }
      return GST_PAD_PROBE_REMOVE;
    }

  return GST_PAD_PROBE_OK;
}

static GstPadProbeReturn
on_downstream_sink_pad_event (GstPad          *pad,
                              GstPadProbeInfo *info,
                              gpointer         udata)
{
  AuricleRendererFileData *data = udata;
  GstEvent *evt = GST_EVENT (info->data);

  if (GST_EVENT_TYPE (evt) == GST_EVENT_EOS)
    {
      g_info ("Received final EOS for %s", auricle_music_file_get_result_name (data->file));
      data->is_eos = TRUE;
      return GST_PAD_PROBE_REMOVE;
    }

  return GST_PAD_PROBE_OK;
}

void
add_downstream_event_probe (GstPad                  *pad,
                            GstPadProbeCallback      callback,
                            AuricleRendererFileData *data)
{
  guint id = gst_pad_add_probe (pad, GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM, callback, data, NULL);

  AuricleRendererPadProbe *probe = g_new (AuricleRendererPadProbe, 1);
  probe->pad = pad;
  probe->id = id;
  data->pad_probes = g_list_prepend (data->pad_probes, probe);
}

void
auricle_renderer_run (AuricleRenderer *self)
{
  const char *output_directory = auricle_render_options_get_output_directory (self->render_options);
  guint audio_bitrate = auricle_render_options_get_audio_bitrate (self->render_options);

  g_return_if_fail (self->pipeline == NULL);
  g_return_if_fail (self->pixbuf != NULL);
  g_return_if_fail (output_directory != NULL);
  self->pipeline = gst_pipeline_new ("render-pipeline");

  self->bus_watch_id = gst_bus_add_watch (GST_ELEMENT_BUS (self->pipeline), on_bus_message, self);

  for (int i = 0; i < self->file_data->len; i++)
    {
      AuricleRendererFileData *data = g_ptr_array_index (self->file_data, i);
      g_info ("Adding %s -> %s to pipeline", auricle_music_file_get_path (data->file),
               auricle_music_file_get_result_name (data->file));

      GstElement *image_src = auricle_renderer_create_image_source (self);
      GstElement *image_conv = gst_element_factory_make ("videoconvert", NULL);
      GstElement *image_freeze = gst_element_factory_make ("imagefreeze", NULL);
      GstElement *image_enc = gst_element_factory_make ("x264enc", NULL);

      g_autofree char *output_basename = g_strdup_printf ("%s.mp4", auricle_music_file_get_result_name (data->file));
      g_autofree char *output_path = g_build_filename (output_directory, output_basename, NULL);

      GstElement *audio_src = gst_element_factory_make ("filesrc", NULL);
      g_object_set (audio_src, "location", auricle_music_file_get_path (data->file), NULL);

      GstElement *audio_dec = gst_element_factory_make ("decodebin3", NULL);

      GstElement *audio_enc = gst_element_factory_make ("fdkaacenc", NULL);
      g_object_set (audio_enc, "bitrate", audio_bitrate * 1000, NULL);

      GstElement *mux  = gst_element_factory_make ("mp4mux", NULL);

      GstElement *sink = gst_element_factory_make ("filesink", NULL);
      g_object_set (sink, "location", output_path,
                          "sync", FALSE, NULL);

      gst_bin_add_many (GST_BIN (self->pipeline),
                        image_src, image_conv, image_freeze, image_enc,
                        audio_src, audio_dec, audio_enc,
                        mux, sink, NULL);

      gst_element_link_many (image_src, image_conv, image_freeze, image_enc, NULL);
      gst_element_link (audio_src, audio_dec);
      gst_element_link (mux, sink);

      GstPad *mux_audio_pad = gst_element_get_request_pad (mux, "audio_%u");
      GstPad *mux_video_pad = gst_element_get_request_pad (mux, "video_%u");

      g_autoptr(GstPad) audio_enc_pad = gst_element_get_static_pad (audio_enc, "src");
      g_autoptr(GstPad) image_enc_pad = gst_element_get_static_pad (image_enc, "src");

      gst_pad_link (audio_enc_pad, mux_audio_pad);
      gst_pad_link (image_enc_pad, mux_video_pad);

      g_signal_connect (audio_dec, "pad-added", G_CALLBACK (on_dec_pad_added), audio_enc);

      data->src = g_object_ref (audio_dec);
      data->sink = g_object_ref (sink);
      data->request_pads = g_list_prepend (data->request_pads, mux_video_pad);
      data->request_pads = g_list_prepend (data->request_pads, mux_audio_pad);

      add_downstream_event_probe (audio_enc_pad, on_downstream_audio_pad_event, data);
      g_autoptr(GstPad) sink_pad = gst_element_get_static_pad (sink, "sink");
      add_downstream_event_probe (sink_pad, on_downstream_sink_pad_event, data);
    }

  self->progress_timer_id = g_timeout_add (100, on_progress_timer, self);

  gst_element_set_state (self->pipeline, GST_STATE_PLAYING);
}

