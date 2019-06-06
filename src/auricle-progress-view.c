/* auricle-progress-view.c
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

#include "auricle-progress-view.h"
#include "auricle-progress-row.h"

struct _AuricleProgressView
{
  GtkBin parent_instance;

  GtkListBox *progress_list;

  AuricleRenderer *renderer;
  GPtrArray       *progress_rows;
};

G_DEFINE_TYPE (AuricleProgressView, auricle_progress_view, GTK_TYPE_BIN)

/* enum { */
/*   PROP_0, */
/*   N_PROPS */
/* }; */

/* static GParamSpec *properties [N_PROPS]; */

AuricleProgressView *
auricle_progress_view_new (void)
{
  return g_object_new (AURICLE_TYPE_PROGRESS_VIEW, NULL);
}

static void
auricle_progress_view_finalize (GObject *object)
{
  AuricleProgressView *self = (AuricleProgressView *)object;

  g_clear_object (&self->renderer);
  g_clear_pointer (&self->progress_rows, g_ptr_array_unref);

  G_OBJECT_CLASS (auricle_progress_view_parent_class)->finalize (object);
}

static void
auricle_progress_view_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  /* AuricleProgressView *self = AURICLE_PROGRESS_VIEW (object); */

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
auricle_progress_view_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  /* AuricleProgressView *self = AURICLE_PROGRESS_VIEW (object); */

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
auricle_progress_view_class_init (AuricleProgressViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = auricle_progress_view_finalize;
  object_class->get_property = auricle_progress_view_get_property;
  object_class->set_property = auricle_progress_view_set_property;

  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/com/refi64/Auricle/auricle-progress-view.ui");
  gtk_widget_class_bind_template_child (widget_class, AuricleProgressView, progress_list);
}

static void
auricle_progress_view_init (AuricleProgressView *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  self->progress_rows = g_ptr_array_new_with_free_func ((GDestroyNotify) gtk_widget_destroy);
}

static void
on_progress_updated (AuricleRenderer *renderer,
                     GList           *progress,
                     gpointer         udata)
{
  AuricleProgressView *self = AURICLE_PROGRESS_VIEW (udata);

  for (GList *l = progress; l != NULL; l = l->next)
    {
      AuricleRenderProgress *p = l->data;
      AuricleProgressRow *row = AURICLE_PROGRESS_ROW (g_ptr_array_index (self->progress_rows, p->index));

      if (p->finished)
        {
          auricle_progress_row_hide (row);
        }
      else
        {
          auricle_progress_row_set_position (row, p->position);
          auricle_progress_row_set_duration (row, p->duration);
        }
    }
}

void
auricle_progress_view_reset_renderer (AuricleProgressView *self,
                                      AuricleRenderer     *renderer)
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
          AuricleMusicFile *file = auricle_renderer_get_file (self->renderer, i);
          if (file == NULL)
            break;

          const char *name = auricle_music_file_get_result_name (file);
          g_info ("Progress row added: %s", name);
          AuricleProgressRow *row = auricle_progress_row_new (name);
          gtk_container_add (GTK_CONTAINER (self->progress_list), GTK_WIDGET (row));
          g_ptr_array_add (self->progress_rows, row);
        }
    }

  gtk_widget_show_all (GTK_WIDGET (self));
}

