/* auricle-notification.c
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

#include "auricle-notification.h"

struct _AuricleNotification
{
  GtkBin parent_instance;

  GtkButton   *notification_close;
  GtkLabel    *notification_message;
  GtkRevealer *notification_revealer;
};

G_DEFINE_TYPE (AuricleNotification, auricle_notification, GTK_TYPE_BIN)

/* enum { */
/*   PROP_0, */
/*   N_PROPS */
/* }; */

/* static GParamSpec *properties [N_PROPS]; */

AuricleNotification *
auricle_notification_new (void)
{
  return g_object_new (AURICLE_TYPE_NOTIFICATION, NULL);
}

static void
auricle_notification_finalize (GObject *object)
{
  /* AuricleNotification *self = (AuricleNotification *)object; */

  G_OBJECT_CLASS (auricle_notification_parent_class)->finalize (object);
}

static void
auricle_notification_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  /* AuricleNotification *self = AURICLE_NOTIFICATION (object); */

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
auricle_notification_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  /* AuricleNotification *self = AURICLE_NOTIFICATION (object); */

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

void
auricle_notification_show (AuricleNotification *self,
                           const char          *message)
{
  gtk_label_set_text (self->notification_message, message);
  gtk_revealer_set_reveal_child (self->notification_revealer, TRUE);
}

static void
auricle_notification_close (GtkButton *button,
                            gpointer   udata)
{
  AuricleNotification *self = AURICLE_NOTIFICATION (udata);

  gtk_revealer_set_reveal_child (self->notification_revealer, FALSE);
}

static void
auricle_notification_class_init (AuricleNotificationClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = auricle_notification_finalize;
  object_class->get_property = auricle_notification_get_property;
  object_class->set_property = auricle_notification_set_property;

  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/com/refi64/Auricle/auricle-notification.ui");
  gtk_widget_class_bind_template_child (widget_class, AuricleNotification, notification_close);
  gtk_widget_class_bind_template_child (widget_class, AuricleNotification, notification_message);
  gtk_widget_class_bind_template_child (widget_class, AuricleNotification, notification_revealer);
}

static void
auricle_notification_init (AuricleNotification *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  g_signal_connect (self->notification_close, "clicked", G_CALLBACK (auricle_notification_close), self);
}

