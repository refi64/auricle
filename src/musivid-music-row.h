/* musivid-music-row.h
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

#define MUSIVID_TYPE_MUSIC_ROW (musivid_music_row_get_type())

G_DECLARE_FINAL_TYPE (MusividMusicRow, musivid_music_row, MUSIVID, MUSIC_ROW, GtkListBoxRow)

MusividMusicRow *musivid_music_row_new (const char *path);

const char *musivid_music_row_get_path        (MusividMusicRow *row);
const char *musivid_music_row_get_result_name (MusividMusicRow *row);

void musivid_music_row_toggle (MusividMusicRow *row);

G_END_DECLS

