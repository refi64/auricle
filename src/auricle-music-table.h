/* auricle-music-table.h
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

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define AURICLE_TYPE_MUSIC_TABLE (auricle_music_table_get_type())

G_DECLARE_FINAL_TYPE (AuricleMusicTable, auricle_music_table, AURICLE, MUSIC_TABLE, GtkGrid)

AuricleMusicTable *auricle_music_table_new (void);

void auricle_music_table_add_tracks (AuricleMusicTable *self,
                                     GSList *filenames);

gboolean auricle_music_table_is_empty  (AuricleMusicTable *self);
GList   *auricle_music_table_get_files (AuricleMusicTable *self);

G_END_DECLS

