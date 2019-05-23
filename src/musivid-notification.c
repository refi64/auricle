/* musivid-notification.c
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

#include "musivid-notification.h"

struct _MusividNotification
{
  GtkBin parent_instance;

  GtkButton   *notification_close;
  GtkLabel    *notification_message;
  GtkRevealer *notification_revealer;
};

G_DEFINE_TYPE (MusividNotification, musivid_notification, GTK_TYPE_BIN)

/* enum { */
/*   PROP_0, */
/*   N_PROPS */
/* }; */

/* static GParamSpec *properties [N_PROPS]; */

MusividNotification *
musivid_notification_new (void)
{
  return g_object_new (MUSIVID_TYPE_NOTIFICATION, NULL);
}

static void
musivid_notification_finalize (GObject *object)
{
  /* MusividNotification *self = (MusividNotification *)object; */

  G_OBJECT_CLASS (musivid_notification_parent_class)->finalize (object);
}

static void
musivid_notification_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  /* MusividNotification *self = MUSIVID_NOTIFICATION (object); */

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
musivid_notification_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  /* MusividNotification *self = MUSIVID_NOTIFICATION (object); */

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

void
musivid_notification_show (MusividNotification *self,
                           const char          *message)
{
  gtk_label_set_text (self->notification_message, message);
  gtk_revealer_set_reveal_child (self->notification_revealer, TRUE);
}

static void
musivid_notification_close (GtkButton *button,
                            gpointer   udata)
{
  MusividNotification *self = MUSIVID_NOTIFICATION (udata);

  gtk_revealer_set_reveal_child (self->notification_revealer, FALSE);
}

static void
musivid_notification_class_init (MusividNotificationClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = musivid_notification_finalize;
  object_class->get_property = musivid_notification_get_property;
  object_class->set_property = musivid_notification_set_property;

  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/com/refi64/Musivid/musivid-notification.ui");
  gtk_widget_class_bind_template_child (widget_class, MusividNotification, notification_close);
  gtk_widget_class_bind_template_child (widget_class, MusividNotification, notification_message);
  gtk_widget_class_bind_template_child (widget_class, MusividNotification, notification_revealer);
}

static void
musivid_notification_init (MusividNotification *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  g_signal_connect (self->notification_close, "clicked", G_CALLBACK (musivid_notification_close), self);
}

