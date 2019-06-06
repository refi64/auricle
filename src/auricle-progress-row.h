/* auricle-progress-row.h
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

#define AURICLE_TYPE_PROGRESS_ROW (auricle_progress_row_get_type())

G_DECLARE_FINAL_TYPE (AuricleProgressRow, auricle_progress_row, AURICLE, PROGRESS_ROW, GtkListBoxRow)

AuricleProgressRow *auricle_progress_row_new (const char *name);

void auricle_progress_row_set_position (AuricleProgressRow *self,
                                        gint64              position);
void auricle_progress_row_set_duration (AuricleProgressRow *self,
                                        gint64              duration);

void auricle_progress_row_hide (AuricleProgressRow *self);

G_END_DECLS

