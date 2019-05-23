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

#include <gtk/gtk.h>
#include "musivid-render-options.h"

G_BEGIN_DECLS

#define MUSIVID_TYPE_OPTIONS_EDITOR (musivid_options_editor_get_type())

G_DECLARE_FINAL_TYPE (MusividOptionsEditor, musivid_options_editor, MUSIVID, OPTIONS_EDITOR, GtkGrid)

MusividOptionsEditor *musivid_options_editor_new (MusividRenderOptions *render_options);

MusividRenderOptions *musivid_options_editor_get_render_options (MusividOptionsEditor *self);

G_END_DECLS

