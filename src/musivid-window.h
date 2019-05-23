/* musivid-window.h
 *
 * Copyright 2019 Ryan Gonzalez
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
 */

#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define MUSIVID_TYPE_WINDOW (musivid_window_get_type())

G_DECLARE_FINAL_TYPE (MusividWindow, musivid_window, MUSIVID, WINDOW, GtkApplicationWindow)

MusividWindow *musivid_window_new (GtkApplication *app);

void musivid_window_show_notification (MusividWindow *window,
                                       const char    *message);

G_END_DECLS
