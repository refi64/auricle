/* auricle-options.c
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

#include "auricle-render-options.h"

struct _AuricleRenderOptions
{
  GObject parent_instance;

  char *output_directory;
  guint audio_bitrate;
};

G_DEFINE_TYPE (AuricleRenderOptions, auricle_render_options, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_OUTPUT_DIRECTORY,
  PROP_AUDIO_BITRATE,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

AuricleRenderOptions *
auricle_render_options_new (void)
{
  return g_object_new (AURICLE_TYPE_RENDER_OPTIONS, NULL);
}

static void
auricle_render_options_finalize (GObject *object)
{
  AuricleRenderOptions *self = (AuricleRenderOptions *)object;

  g_clear_pointer (&self->output_directory, g_free);

  G_OBJECT_CLASS (auricle_render_options_parent_class)->finalize (object);
}

static void
auricle_render_options_take_output_directory_notify (AuricleRenderOptions *self,
                                                     char           *output_directory,
                                                     gboolean        notify)
{
  g_free (self->output_directory);
  self->output_directory = output_directory;
  if (notify)
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_OUTPUT_DIRECTORY]);
}

static void
auricle_render_options_set_audio_bitrate_notify (AuricleRenderOptions *self,
                                                 guint                 bitrate,
                                                 gboolean              notify)
{
  self->audio_bitrate = bitrate;
  if (notify)
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_AUDIO_BITRATE]);
}

static void
auricle_render_options_get_property (GObject    *object,
                                     guint       prop_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
  AuricleRenderOptions *self = AURICLE_RENDER_OPTIONS (object);

  switch (prop_id)
    {
    case PROP_OUTPUT_DIRECTORY:
      g_value_set_string (value, self->output_directory);
      break;
    case PROP_AUDIO_BITRATE:
      g_value_set_uint (value, self->audio_bitrate);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
auricle_render_options_set_property (GObject      *object,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  AuricleRenderOptions *self = AURICLE_RENDER_OPTIONS (object);

  switch (prop_id)
    {
    case PROP_OUTPUT_DIRECTORY:
      auricle_render_options_take_output_directory_notify (self, g_value_dup_string (value), FALSE);
      break;
    case PROP_AUDIO_BITRATE:
      auricle_render_options_set_audio_bitrate_notify (self, g_value_get_uint (value), FALSE);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
auricle_render_options_class_init (AuricleRenderOptionsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = auricle_render_options_finalize;
  object_class->get_property = auricle_render_options_get_property;
  object_class->set_property = auricle_render_options_set_property;

  properties [PROP_OUTPUT_DIRECTORY] =
    g_param_spec_string ("output-directory",
                         "Output directory",
                         "Output directory",
                         NULL,
                         (G_PARAM_READWRITE |
                          G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class, PROP_OUTPUT_DIRECTORY,
                                   properties [PROP_OUTPUT_DIRECTORY]);

  properties [PROP_AUDIO_BITRATE] =
    g_param_spec_uint ("audio-bitrate",
                       "Audio bitrate",
                       "Audio bitrate",
                       128, 512, 384,
                       (G_PARAM_READWRITE |
                        G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class, PROP_AUDIO_BITRATE,
                                   properties [PROP_AUDIO_BITRATE]);
}

static void
auricle_render_options_init (AuricleRenderOptions *self)
{
}

const char *
auricle_render_options_get_output_directory (AuricleRenderOptions *self)
{
  return self->output_directory;
}

void
auricle_render_options_take_output_directory (AuricleRenderOptions *self,
                                              char                 *output_directory)
{
  auricle_render_options_take_output_directory_notify (self, output_directory, TRUE);
}

void
auricle_render_options_set_output_directory (AuricleRenderOptions *self,
                                             const char     *output_directory)
{
  auricle_render_options_take_output_directory (self, g_strdup (output_directory));
}

guint
auricle_render_options_get_audio_bitrate (AuricleRenderOptions *self)
{
  return self->audio_bitrate;
}

void
auricle_render_options_set_audio_bitrate (AuricleRenderOptions *self,
                                          guint                 bitrate)
{
  auricle_render_options_set_audio_bitrate_notify (self, bitrate, TRUE);
}

