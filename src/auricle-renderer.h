/* auricle-renderer.h
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
#include "auricle-music-file.h"
#include "auricle-render-options.h"

G_BEGIN_DECLS

#define AURICLE_TYPE_RENDERER (auricle_renderer_get_type())

G_DECLARE_FINAL_TYPE (AuricleRenderer, auricle_renderer, AURICLE, RENDERER, GObject)

AuricleRenderer *auricle_renderer_new (GdkPixbuf            *image,
                                       AuricleRenderOptions *render_options);

AuricleMusicFile *auricle_renderer_get_file (AuricleRenderer *self,
                                             int              index);

void auricle_renderer_take_file (AuricleRenderer  *self,
                                 AuricleMusicFile *file);

void auricle_renderer_run (AuricleRenderer *self);

typedef struct AuricleRenderProgress AuricleRenderProgress;

struct AuricleRenderProgress {
  int      index;
  gint64   position;
  gint64   duration;
  gboolean finished;
};

G_END_DECLS

