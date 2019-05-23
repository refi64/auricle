/* musivid-options.c
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

#include "musivid-render-options.h"

struct _MusividRenderOptions
{
  GObject parent_instance;

  char *output_directory;
  guint audio_bitrate;
};

G_DEFINE_TYPE (MusividRenderOptions, musivid_render_options, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_OUTPUT_DIRECTORY,
  PROP_AUDIO_BITRATE,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

MusividRenderOptions *
musivid_render_options_new (void)
{
  return g_object_new (MUSIVID_TYPE_RENDER_OPTIONS, NULL);
}

static void
musivid_render_options_finalize (GObject *object)
{
  MusividRenderOptions *self = (MusividRenderOptions *)object;

  g_clear_pointer (&self->output_directory, g_free);

  G_OBJECT_CLASS (musivid_render_options_parent_class)->finalize (object);
}

static void
musivid_render_options_take_output_directory_notify (MusividRenderOptions *self,
                                                     char           *output_directory,
                                                     gboolean        notify)
{
  g_free (self->output_directory);
  self->output_directory = output_directory;
  if (notify)
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_OUTPUT_DIRECTORY]);
}

static void
musivid_render_options_set_audio_bitrate_notify (MusividRenderOptions *self,
                                                 guint                 bitrate,
                                                 gboolean              notify)
{
  self->audio_bitrate = bitrate;
  if (notify)
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_AUDIO_BITRATE]);
}

static void
musivid_render_options_get_property (GObject    *object,
                                     guint       prop_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
  MusividRenderOptions *self = MUSIVID_RENDER_OPTIONS (object);

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
musivid_render_options_set_property (GObject      *object,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  MusividRenderOptions *self = MUSIVID_RENDER_OPTIONS (object);

  switch (prop_id)
    {
    case PROP_OUTPUT_DIRECTORY:
      musivid_render_options_take_output_directory_notify (self, g_value_dup_string (value), FALSE);
      break;
    case PROP_AUDIO_BITRATE:
      musivid_render_options_set_audio_bitrate_notify (self, g_value_get_uint (value), FALSE);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
musivid_render_options_class_init (MusividRenderOptionsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = musivid_render_options_finalize;
  object_class->get_property = musivid_render_options_get_property;
  object_class->set_property = musivid_render_options_set_property;

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
musivid_render_options_init (MusividRenderOptions *self)
{
}

const char *
musivid_render_options_get_output_directory (MusividRenderOptions *self)
{
  return self->output_directory;
}

void
musivid_render_options_take_output_directory (MusividRenderOptions *self,
                                              char                 *output_directory)
{
  musivid_render_options_take_output_directory_notify (self, output_directory, TRUE);
}

void
musivid_render_options_set_output_directory (MusividRenderOptions *self,
                                             const char     *output_directory)
{
  musivid_render_options_take_output_directory (self, g_strdup (output_directory));
}

guint
musivid_render_options_get_audio_bitrate (MusividRenderOptions *self)
{
  return self->audio_bitrate;
}

void
musivid_render_options_set_audio_bitrate (MusividRenderOptions *self,
                                          guint                 bitrate)
{
  musivid_render_options_set_audio_bitrate_notify (self, bitrate, TRUE);
}

