/* musivid-progress-row.h
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

#define MUSIVID_TYPE_PROGRESS_ROW (musivid_progress_row_get_type())

G_DECLARE_FINAL_TYPE (MusividProgressRow, musivid_progress_row, MUSIVID, PROGRESS_ROW, GtkListBoxRow)

MusividProgressRow *musivid_progress_row_new (const char *name);

void musivid_progress_row_set_position (MusividProgressRow *self,
                                        gint64              position);
void musivid_progress_row_set_duration (MusividProgressRow *self,
                                        gint64              duration);

void musivid_progress_row_hide (MusividProgressRow *self);

G_END_DECLS

