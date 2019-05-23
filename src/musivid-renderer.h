/* musivid-renderer.h
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

#pragma once

#include <glib-object.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "musivid-music-file.h"
#include "musivid-render-options.h"

G_BEGIN_DECLS

#define MUSIVID_TYPE_RENDERER (musivid_renderer_get_type())

G_DECLARE_FINAL_TYPE (MusividRenderer, musivid_renderer, MUSIVID, RENDERER, GObject)

MusividRenderer *musivid_renderer_new (GdkPixbuf            *image,
                                       MusividRenderOptions *render_options);

MusividMusicFile *musivid_renderer_get_file (MusividRenderer *self,
                                             int              index);

void musivid_renderer_take_file (MusividRenderer  *self,
                                 MusividMusicFile *file);

void musivid_renderer_run (MusividRenderer *self);

typedef struct MusividRenderProgress MusividRenderProgress;

struct MusividRenderProgress {
  int    index;
  gint64 position;
  gint64 duration;
};

G_END_DECLS

