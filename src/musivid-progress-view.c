/* musivid-progress-view.c
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

#include "musivid-progress-view.h"
#include "musivid-progress-row.h"

struct _MusividProgressView
{
  GtkBin parent_instance;

  GtkListBox *progress_list;

  MusividRenderer *renderer;
  GPtrArray       *progress_rows;
};

G_DEFINE_TYPE (MusividProgressView, musivid_progress_view, GTK_TYPE_BIN)

/* enum { */
/*   PROP_0, */
/*   N_PROPS */
/* }; */

/* static GParamSpec *properties [N_PROPS]; */

MusividProgressView *
musivid_progress_view_new (void)
{
  return g_object_new (MUSIVID_TYPE_PROGRESS_VIEW, NULL);
}

static void
musivid_progress_view_finalize (GObject *object)
{
  MusividProgressView *self = (MusividProgressView *)object;

  g_clear_object (&self->renderer);
  g_clear_pointer (&self->progress_rows, g_ptr_array_unref);

  G_OBJECT_CLASS (musivid_progress_view_parent_class)->finalize (object);
}

static void
musivid_progress_view_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  /* MusividProgressView *self = MUSIVID_PROGRESS_VIEW (object); */

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
musivid_progress_view_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  /* MusividProgressView *self = MUSIVID_PROGRESS_VIEW (object); */

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
musivid_progress_view_class_init (MusividProgressViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = musivid_progress_view_finalize;
  object_class->get_property = musivid_progress_view_get_property;
  object_class->set_property = musivid_progress_view_set_property;

  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/com/refi64/Musivid/musivid-progress-view.ui");
  gtk_widget_class_bind_template_child (widget_class, MusividProgressView, progress_list);
}

static void
musivid_progress_view_init (MusividProgressView *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  self->progress_rows = g_ptr_array_new_with_free_func ((GDestroyNotify) gtk_widget_destroy);
}

static void
on_progress_updated (MusividRenderer *renderer,
                     GList           *progress,
                     gpointer         udata)
{
  MusividProgressView *self = MUSIVID_PROGRESS_VIEW (udata);

  for (GList *l = progress; l != NULL; l = l->next)
    {
      MusividRenderProgress *p = l->data;
      MusividProgressRow *row = MUSIVID_PROGRESS_ROW (g_ptr_array_index (self->progress_rows, p->index));

      musivid_progress_row_set_position (row, p->position);
      musivid_progress_row_set_duration (row, p->duration);
    }
}

void
musivid_progress_view_reset_renderer (MusividProgressView *self,
                                      MusividRenderer     *renderer)
{
  // Will also destroy all rows
  g_ptr_array_remove_range (self->progress_rows, 0, self->progress_rows->len);

  g_clear_object (&self->renderer);
  self->renderer = renderer;

  if (self->renderer != NULL)
    {
      g_object_ref (self->renderer);
      g_signal_connect (self->renderer, "progress-update", G_CALLBACK (on_progress_updated), self);

      for (int i = 0; ; i++)
        {
          MusividMusicFile *file = musivid_renderer_get_file (self->renderer, i);
          if (file == NULL)
            break;

          const char *name = musivid_music_file_get_result_name (file);
          g_info ("Progress row added: %s", name);
          MusividProgressRow *row = musivid_progress_row_new (name);
          gtk_container_add (GTK_CONTAINER (self->progress_list), GTK_WIDGET (row));
          g_ptr_array_add (self->progress_rows, row);
        }
    }

  gtk_widget_show_all (GTK_WIDGET (self));
}

