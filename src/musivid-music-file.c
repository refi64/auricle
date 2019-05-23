/* musivid-music-file.c
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

#include "musivid-music-file.h"

struct _MusividMusicFile
{
  GObject parent_instance;

  char *path;
  char *result_name;
};

G_DEFINE_TYPE (MusividMusicFile, musivid_music_file, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_PATH,
  PROP_RESULT_NAME,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

MusividMusicFile *
musivid_music_file_new (const char *path,
                        const char *result_name)
{
  return g_object_new (MUSIVID_TYPE_MUSIC_FILE,
                       "path", path,
                       "result-name", result_name,
                       NULL);
}

static void
musivid_music_file_finalize (GObject *object)
{
  MusividMusicFile *self = (MusividMusicFile *)object;

  g_clear_pointer (&self->path, g_free);
  g_clear_pointer (&self->result_name, g_free);

  G_OBJECT_CLASS (musivid_music_file_parent_class)->finalize (object);
}

static void
musivid_music_file_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  MusividMusicFile *self = MUSIVID_MUSIC_FILE (object);

  switch (prop_id)
    {
    case PROP_PATH:
      g_warn_if_fail (self->path != NULL);
      g_value_set_string (value, self->path);
      break;
    case PROP_RESULT_NAME:
      g_warn_if_fail (self->result_name != NULL);
      g_value_set_string (value, self->result_name);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
musivid_music_file_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  MusividMusicFile *self = MUSIVID_MUSIC_FILE (object);

  switch (prop_id)
    {
    case PROP_PATH:
      g_warn_if_fail (self->path == NULL);
      self->path = g_value_dup_string (value);
      break;
    case PROP_RESULT_NAME:
      g_warn_if_fail (self->result_name == NULL);
      self->result_name = g_value_dup_string (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
musivid_music_file_class_init (MusividMusicFileClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = musivid_music_file_finalize;
  object_class->get_property = musivid_music_file_get_property;
  object_class->set_property = musivid_music_file_set_property;

  properties [PROP_PATH] =
    g_param_spec_string ("path",
                         "Path",
                         "Path",
                         NULL,
                         (G_PARAM_READWRITE |
                          G_PARAM_CONSTRUCT_ONLY |
                          G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class, PROP_PATH,
                                   properties [PROP_PATH]);

  properties [PROP_RESULT_NAME] =
    g_param_spec_string ("result-name",
                         "Result name",
                         "Result name",
                         NULL,
                         (G_PARAM_READWRITE |
                          G_PARAM_CONSTRUCT_ONLY |
                          G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class, PROP_RESULT_NAME,
                                   properties [PROP_RESULT_NAME]);
}

static void
musivid_music_file_init (MusividMusicFile *self)
{
}

const char *
musivid_music_file_get_path (MusividMusicFile *self)
{
  return self->path;
}

const char *
musivid_music_file_get_result_name (MusividMusicFile *self)
{
  return self->result_name;
}
