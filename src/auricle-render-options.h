/* auricle-options.h
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

#define AURICLE_TYPE_RENDER_OPTIONS (auricle_render_options_get_type())

G_DECLARE_FINAL_TYPE (AuricleRenderOptions, auricle_render_options, AURICLE, RENDER_OPTIONS, GObject)

AuricleRenderOptions *auricle_render_options_new (void);

const char *auricle_render_options_get_output_directory  (AuricleRenderOptions *self);
void        auricle_render_options_take_output_directory (AuricleRenderOptions *self,
                                                          char                 *output_directory);
void        auricle_render_options_set_output_directory  (AuricleRenderOptions *self,
                                                          const char           *output_directory);

guint auricle_render_options_get_audio_bitrate (AuricleRenderOptions *self);
void  auricle_render_options_set_audio_bitrate (AuricleRenderOptions *self,
                                                guint                 bitrate);



G_END_DECLS

