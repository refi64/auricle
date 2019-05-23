/* musivid-options.h
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

G_BEGIN_DECLS

#define MUSIVID_TYPE_RENDER_OPTIONS (musivid_render_options_get_type())

G_DECLARE_FINAL_TYPE (MusividRenderOptions, musivid_render_options, MUSIVID, RENDER_OPTIONS, GObject)

MusividRenderOptions *musivid_render_options_new (void);

const char *musivid_render_options_get_output_directory  (MusividRenderOptions *self);
void        musivid_render_options_take_output_directory (MusividRenderOptions *self,
                                                          char                 *output_directory);
void        musivid_render_options_set_output_directory  (MusividRenderOptions *self,
                                                          const char           *output_directory);

guint musivid_render_options_get_audio_bitrate (MusividRenderOptions *self);
void  musivid_render_options_set_audio_bitrate (MusividRenderOptions *self,
                                                guint                 bitrate);



G_END_DECLS

