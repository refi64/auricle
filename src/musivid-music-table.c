/* musivid-music-table.c
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

#include "musivid-music-file.h"
#include "musivid-music-table.h"
#include "musivid-music-row.h"

struct _MusividMusicTable
{
  GtkGrid parent_instance;

  GtkListBox *music_list_box;
  GtkLabel   *music_list_placeholder;
  GtkEntry   *music_list_template;
};

G_DEFINE_TYPE (MusividMusicTable, musivid_music_table, GTK_TYPE_GRID)

/* enum { */
/*   PROP_0, */
/*   N_PROPS */
/* }; */

/* static GParamSpec *properties [N_PROPS]; */

enum {
  SIGNAL_0,
  FILES_CHANGED,
  N_SIGNALS
};

static guint signals [N_SIGNALS];

MusividMusicTable *
musivid_music_table_new (void)
{
  return g_object_new (MUSIVID_TYPE_MUSIC_TABLE, NULL);
}

static void
musivid_music_table_finalize (GObject *object)
{
  /* MusividMusicTable *self = (MusividMusicTable *)object; */

  G_OBJECT_CLASS (musivid_music_table_parent_class)->finalize (object);
}

static void
musivid_music_table_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  /* MusividMusicTable *self = MUSIVID_MUSIC_TABLE (object); */

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
musivid_music_table_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  /* MusividMusicTable *self = MUSIVID_MUSIC_TABLE (object); */

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
musivid_music_table_class_init (MusividMusicTableClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = musivid_music_table_finalize;
  object_class->get_property = musivid_music_table_get_property;
  object_class->set_property = musivid_music_table_set_property;

  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/com/refi64/Musivid/musivid-music-table.ui");
  gtk_widget_class_bind_template_child (widget_class, MusividMusicTable, music_list_box);
  gtk_widget_class_bind_template_child (widget_class, MusividMusicTable, music_list_placeholder);
  gtk_widget_class_bind_template_child (widget_class, MusividMusicTable, music_list_template);

  signals [FILES_CHANGED] =
    g_signal_new ("files-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL,
                  NULL,
                  g_cclosure_marshal_generic,
                  G_TYPE_NONE,
                  1, G_TYPE_INT);
}

static void
on_row_activated (GtkListBox    *box,
                  GtkListBoxRow *row,
                  gpointer       udata)
{
  /* MusividMusicTable *self = MUSIVID_MUSIC_TABLE (udata); */

  musivid_music_row_toggle (MUSIVID_MUSIC_ROW (row));
}

static void
musivid_music_table_init (MusividMusicTable *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  gtk_list_box_set_placeholder (self->music_list_box, GTK_WIDGET (self->music_list_placeholder));
  g_signal_connect (self->music_list_box, "row-activated", G_CALLBACK (on_row_activated), self);
}

static void
on_row_delete_requested (MusividMusicRow *row,
                         gpointer         udata)
{
  MusividMusicTable *self = MUSIVID_MUSIC_TABLE (udata);

  gtk_container_remove (GTK_CONTAINER (self->music_list_box), GTK_WIDGET (row));
  g_signal_emit (self, signals[FILES_CHANGED], 0, -1);
}

void
musivid_music_table_add_tracks (MusividMusicTable *self,
                                GSList *filenames)
{
  int added = 0;

  for (GSList *l = filenames; l != NULL; l = l->next)
    {
      MusividMusicRow *row = musivid_music_row_new (l->data);
      g_object_bind_property (self->music_list_template, "text", row, "template", G_BINDING_SYNC_CREATE);
      g_signal_connect (row, "delete-requested", G_CALLBACK (on_row_delete_requested), self);
      gtk_container_add (GTK_CONTAINER (self->music_list_box), GTK_WIDGET (row));
      added++;
    }

  g_signal_emit (self, signals[FILES_CHANGED], 0, added);
}

gboolean
musivid_music_table_is_empty (MusividMusicTable *self)
{
  g_autoptr(GList) children = gtk_container_get_children (GTK_CONTAINER (self->music_list_box));
  return children == NULL;
}

GList *
musivid_music_table_get_files (MusividMusicTable *self)
{
  g_autoptr(GList) children = gtk_container_get_children (GTK_CONTAINER (self->music_list_box));
  GList *result = NULL;

  for (GList *l = children; l != NULL; l = l->next)
    {
      MusividMusicRow *row = MUSIVID_MUSIC_ROW (l->data);
      MusividMusicFile *file = musivid_music_file_new (musivid_music_row_get_path (row),
                                                       musivid_music_row_get_result_name (row));
      result = g_list_prepend (result, file);
    }

  return result;
}

