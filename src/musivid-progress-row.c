/* musivid-progress-row.c
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

#include "musivid-progress-row.h"
#include <gst/gst.h>

struct _MusividProgressRow
{
  GtkListBoxRow parent_instance;

  GtkRevealer    *row_revealer;
  GtkLabel       *row_name;
  GtkLabel       *row_position;
  GtkProgressBar *row_progress;

  char  *name;
  gint64 position;
  gint64 duration;
};

G_DEFINE_TYPE (MusividProgressRow, musivid_progress_row, GTK_TYPE_LIST_BOX_ROW)

enum {
  PROP_0,
  PROP_NAME,
  PROP_POSITION,
  PROP_DURATION,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

MusividProgressRow *
musivid_progress_row_new (const char *name)
{
  return g_object_new (MUSIVID_TYPE_PROGRESS_ROW,
                       "name", name,
                       NULL);
}

static void
musivid_progress_row_finalize (GObject *object)
{
  MusividProgressRow *self = (MusividProgressRow *)object;

  g_clear_pointer (&self->name, g_free);

  G_OBJECT_CLASS (musivid_progress_row_parent_class)->finalize (object);
}

static void
musivid_progress_row_set_position_notify (MusividProgressRow *self,
                                          gint64              position,
                                          gboolean            notify)
{
  self->position = position;
  if (notify)
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_POSITION]);
}

static void
musivid_progress_row_set_duration_notify (MusividProgressRow *self,
                                          gint64              duration,
                                          gboolean            notify)
{
  self->duration = duration;
  if (notify)
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_DURATION]);
}

static void
musivid_progress_row_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  MusividProgressRow *self = MUSIVID_PROGRESS_ROW (object);

  switch (prop_id)
    {
    case PROP_NAME:
      g_warn_if_fail (self->name != NULL);
      g_value_set_string (value, self->name);
      break;
    case PROP_POSITION:
      g_value_set_int64 (value, self->position);
      break;
    case PROP_DURATION:
      g_value_set_int64 (value, self->duration);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
musivid_progress_row_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  MusividProgressRow *self = MUSIVID_PROGRESS_ROW (object);

  switch (prop_id)
    {
    case PROP_NAME:
      g_warn_if_fail (self->name == NULL);
      self->name = g_value_dup_string (value);
      break;
    case PROP_POSITION:
      musivid_progress_row_set_position_notify (self, g_value_get_int64 (value), FALSE);
      break;
    case PROP_DURATION:
      musivid_progress_row_set_duration_notify (self, g_value_get_int64 (value), FALSE);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
musivid_progress_row_class_init (MusividProgressRowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = musivid_progress_row_finalize;
  object_class->get_property = musivid_progress_row_get_property;
  object_class->set_property = musivid_progress_row_set_property;

  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/com/refi64/Musivid/musivid-progress-row.ui");
  gtk_widget_class_bind_template_child (widget_class, MusividProgressRow, row_revealer);
  gtk_widget_class_bind_template_child (widget_class, MusividProgressRow, row_name);
  gtk_widget_class_bind_template_child (widget_class, MusividProgressRow, row_position);
  gtk_widget_class_bind_template_child (widget_class, MusividProgressRow, row_progress);

  properties [PROP_NAME] =
    g_param_spec_string ("name",
                         "Name",
                         "Name",
                         NULL,
                         (G_PARAM_READWRITE |
                          G_PARAM_CONSTRUCT_ONLY |
                          G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class, PROP_NAME,
                                   properties [PROP_NAME]);

  properties [PROP_POSITION] =
    g_param_spec_int64 ("position",
                        "Position",
                        "Position",
                        0, G_MAXINT64, 0,
                        (G_PARAM_READWRITE |
                         G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class, PROP_POSITION,
                                   properties [PROP_POSITION]);

  properties [PROP_DURATION] =
    g_param_spec_int64 ("duration",
                        "Duration",
                        "Duration",
                        0, G_MAXINT64, 0,
                        (G_PARAM_READWRITE |
                         G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class, PROP_DURATION,
                                   properties [PROP_DURATION]);
}

static char *
format_nanosec_time (gint64 ns)
{
  return g_strdup_printf ("%02"G_GINT64_FORMAT":%02"G_GINT64_FORMAT, ns / (GST_SECOND * 60),
                          (ns / GST_SECOND) % 60);
}

static void
on_property_notify (MusividProgressRow  *self,
                    GParamSpec          *pspec,
                    gpointer             udata)
{
  if (self->duration != 0)
    {
      g_autofree char *position_str = format_nanosec_time (self->position);
      g_autofree char *duration_str = format_nanosec_time (self->duration);
      g_autofree char *str = g_strjoin ("/", position_str, duration_str, NULL);
      gtk_label_set_text (self->row_position, str);
      gtk_progress_bar_set_fraction (self->row_progress, (double)self->position / self->duration);
    }
  else
    {
      gtk_label_set_text (self->row_position, "--");
      gtk_progress_bar_set_fraction (self->row_progress, 0);
    }
}

static void
musivid_progress_row_init (MusividProgressRow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  g_object_bind_property (self, "name", self->row_name, "label", 0);

  g_signal_connect (self, "notify::position", G_CALLBACK (on_property_notify), NULL);
  g_signal_connect (self, "notify::duration", G_CALLBACK (on_property_notify), NULL);
}

void
musivid_progress_row_set_position (MusividProgressRow *self,
                                   gint64              position)
{
  g_debug ("Set position of %s to %"G_GINT64_FORMAT, self->name, position);
  musivid_progress_row_set_position_notify (self, position, TRUE);
}

void
musivid_progress_row_set_duration (MusividProgressRow *self,
                                   gint64              duration)
{
  musivid_progress_row_set_duration_notify (self, duration, TRUE);
}

void
musivid_progress_row_hide (MusividProgressRow *self)
{
  gtk_revealer_set_reveal_child (self->row_revealer, FALSE);

  GtkStyleContext *style_context = gtk_widget_get_style_context (GTK_WIDGET (self));
  gtk_style_context_add_class (style_context, "no-padding");
}

