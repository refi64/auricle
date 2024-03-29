/* auricle-imagesection.c
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

#include "auricle-image-section.h"
#include "auricle-utils.h"

struct _AuricleImageSection
{
  GtkBox parent_instance;

  GtkImage             *image;
  GtkFileChooserButton *image_chooser;

  GdkPixbuf *pixbuf;
};

G_DEFINE_TYPE (AuricleImageSection, auricle_image_section, GTK_TYPE_BOX)

enum {
  PROP_0,
  PROP_PIXBUF,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

AuricleImageSection *
auricle_image_section_new (void)
{
  return g_object_new (AURICLE_TYPE_IMAGE_SECTION, NULL);
}

static void
auricle_image_section_finalize (GObject *object)
{
  AuricleImageSection *self = (AuricleImageSection *)object;

  g_clear_object (&self->pixbuf);

  G_OBJECT_CLASS (auricle_image_section_parent_class)->finalize (object);
}

static void
auricle_image_section_take_pixbuf (AuricleImageSection *self,
                                   GdkPixbuf           *pixbuf,
                                   gboolean             notify)
{
  g_clear_object (&self->pixbuf);
  self->pixbuf = pixbuf;
  if (notify)
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_PIXBUF]);
}

static void
auricle_image_section_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  AuricleImageSection *self = AURICLE_IMAGE_SECTION (object);

  switch (prop_id)
    {
    case PROP_PIXBUF:
      g_value_set_object (value, self->pixbuf);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
auricle_image_section_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  AuricleImageSection *self = AURICLE_IMAGE_SECTION (object);

  switch (prop_id)
    {
    case PROP_PIXBUF:
      auricle_image_section_take_pixbuf (self, g_value_dup_object (value), FALSE);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
auricle_image_section_class_init (AuricleImageSectionClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = auricle_image_section_finalize;
  object_class->get_property = auricle_image_section_get_property;
  object_class->set_property = auricle_image_section_set_property;

  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/com/refi64/Auricle/auricle-image-section.ui");
  gtk_widget_class_bind_template_child (widget_class, AuricleImageSection, image);
  gtk_widget_class_bind_template_child (widget_class, AuricleImageSection, image_chooser);

  properties [PROP_PIXBUF] =
    g_param_spec_object ("pixbuf",
                         "Image pixbuf",
                         "Image pixbuf",
                         GDK_TYPE_PIXBUF,
                         (G_PARAM_READWRITE |
                          G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class, PROP_PIXBUF,
                                   properties [PROP_PIXBUF]);
}

static void
on_file_set (GtkFileChooserButton *button,
             gpointer              udata)
{
  AuricleImageSection *self = AURICLE_IMAGE_SECTION (udata);
  g_autoptr(GError) error = NULL;

  g_autofree char *image_path = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (self->image_chooser));

  g_autoptr(GdkPixbuf) pixbuf = gdk_pixbuf_new_from_file (image_path, &error);
  if (pixbuf == NULL)
    {
      g_warning ("Failed to open %s: %s", image_path, error->message);
      auricle_show_notification ("Failed to open %s: %s", image_path, error->message);
      return;
    }

  int image_width = gdk_pixbuf_get_width (pixbuf);
  int image_height = gdk_pixbuf_get_height (pixbuf);

  int width_request;
  gtk_widget_get_size_request (GTK_WIDGET (self), &width_request, NULL);
  int height_request = ((float) image_height / image_width) * width_request;
  gtk_image_set_from_pixbuf (self->image, gdk_pixbuf_scale_simple (pixbuf, width_request, height_request, GDK_INTERP_BILINEAR));

  if (image_width % 2 != 0 || image_height % 2 != 0)
    {
      // h264 requires even dimensions, so we need to scale up around a pixel on the needed sides.
      g_autoptr(GdkPixbuf) old_pixbuf = g_steal_pointer (&pixbuf);

      image_width += image_width % 2;
      image_height += image_height % 2;

      pixbuf = gdk_pixbuf_scale_simple (old_pixbuf, image_width, image_height, GDK_INTERP_BILINEAR);
    }

  auricle_image_section_take_pixbuf (self, g_steal_pointer (&pixbuf), TRUE);
}

static void
auricle_image_section_init (AuricleImageSection *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  g_signal_connect (self->image_chooser, "file-set", G_CALLBACK (on_file_set), self);

  GtkFileFilter *image_filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (image_filter, "Image files");
  gtk_file_filter_add_mime_type (image_filter, "image/*");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (self->image_chooser), image_filter);

  GtkFileFilter *all_filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (all_filter, "All files");
  gtk_file_filter_add_pattern (all_filter, "*");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (self->image_chooser), all_filter);
}

GdkPixbuf *
auricle_image_section_get_pixbuf (AuricleImageSection *self)
{
  return self->pixbuf;
}

