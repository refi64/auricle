/* musivid-imagesection.h
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

#define MUSIVID_TYPE_IMAGE_SECTION (musivid_image_section_get_type())

G_DECLARE_FINAL_TYPE (MusividImageSection, musivid_image_section, MUSIVID, IMAGE_SECTION, GtkBox)

MusividImageSection *musivid_image_section_new            ();
GdkPixbuf           *musivid_image_section_get_pixbuf     ();

G_END_DECLS

