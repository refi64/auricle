/* musivid-window.c
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

#include "musivid-config.h"
#include "musivid-window.h"
#include "musivid-image-section.h"
#include "musivid-music-file.h"
#include "musivid-music-table.h"
#include "musivid-notification.h"
#include "musivid-options-editor.h"
#include "musivid-progress-view.h"
#include "musivid-renderer.h"

struct _MusividWindow
{
  GtkApplicationWindow  parent_instance;

  GtkStack            *header_stack;
  GtkStack            *content_stack;
  GtkButton           *render_button_1;
  GtkButton           *render_button_2;
  GtkBox              *main_box;
  GtkOverlay          *notification_overlay;

  MusividImageSection  *image_section;
  MusividMusicTable    *music_table;
  MusividNotification  *notification;
  MusividOptionsEditor *options_editor;
  MusividProgressView  *progress_view;

  MusividRenderer      *renderer;
  MusividRenderOptions *render_options;
};

G_DEFINE_TYPE (MusividWindow, musivid_window, GTK_TYPE_APPLICATION_WINDOW)

MusividWindow *
musivid_window_new (GtkApplication *app)
{
  return g_object_new (MUSIVID_TYPE_WINDOW,
		                   "application", app,
		                   NULL);
}

static void musivid_window_goto (MusividWindow *self,
                                 const char    *child);

void
musivid_window_show_notification (MusividWindow *window,
                                  const char    *message)
{
  musivid_notification_show (window->notification, message);
  musivid_window_goto (window, "main");
}

static void
musivid_window_class_init (MusividWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/com/refi64/Musivid/musivid-window.ui");
  gtk_widget_class_bind_template_child (widget_class, MusividWindow, header_stack);
  gtk_widget_class_bind_template_child (widget_class, MusividWindow, content_stack);
  gtk_widget_class_bind_template_child (widget_class, MusividWindow, render_button_1);
  gtk_widget_class_bind_template_child (widget_class, MusividWindow, render_button_2);
  gtk_widget_class_bind_template_child (widget_class, MusividWindow, main_box);
  gtk_widget_class_bind_template_child (widget_class, MusividWindow, notification_overlay);
}

static void
musivid_window_update_render_button_1_state (MusividWindow *self)
{
  gboolean enabled = musivid_image_section_get_pixbuf (self->image_section) != NULL &&
                    !musivid_music_table_is_empty (self->music_table);
  gtk_widget_set_sensitive (GTK_WIDGET (self->render_button_1), enabled);
}

static void
musivid_window_update_render_button_2_state (MusividWindow *self)
{
  gboolean enabled = self->render_options != NULL && musivid_render_options_get_output_directory (self->render_options) != NULL;
  gtk_widget_set_sensitive (GTK_WIDGET (self->render_button_2), enabled);
}

static void
on_pixbuf_notify (MusividImageSection *isec,
                  GParamSpec          *pspec,
                  gpointer             udata)
{
  MusividWindow *self = MUSIVID_WINDOW (udata);

  musivid_window_update_render_button_1_state (self);
}

static void
on_files_changed (MusividMusicTable *mutab,
                  int                changes,
                  gpointer           udata)
{
  MusividWindow *self = MUSIVID_WINDOW (udata);

  musivid_window_update_render_button_1_state (self);
}

static void
on_output_directory_notify (MusividRenderOptions *render_options,
                            GParamSpec           *pspec,
                            gpointer              udata)
{
  MusividWindow *self = MUSIVID_WINDOW (udata);

  musivid_window_update_render_button_2_state (self);
}

static void
musivid_open_add_music_dialog_action (GSimpleAction *action,
                                      GVariant      *param,
                                      gpointer       udata)
{
  MusividWindow *self = MUSIVID_WINDOW (udata);

  GtkFileChooserNative *chooser = gtk_file_chooser_native_new ("Add Music", GTK_WINDOW (self), GTK_FILE_CHOOSER_ACTION_OPEN,
                                                               "_Add", "_Cancel");

  GtkFileFilter *filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, "Audio files");
  gtk_file_filter_add_mime_type (filter, "audio/*");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), filter);

  gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (chooser), TRUE);

  if (gtk_native_dialog_run (GTK_NATIVE_DIALOG (chooser)) == GTK_RESPONSE_ACCEPT)
    {
      GSList *filenames = gtk_file_chooser_get_filenames (GTK_FILE_CHOOSER (chooser));
      musivid_music_table_add_tracks (self->music_table, filenames);
      g_slist_free_full (filenames, g_free);
    }
}

static void
musivid_window_goto (MusividWindow *self,
                     const char    *child)
{
  if (strcmp (child, "render") == 0)
    {
      g_warn_if_fail (self->renderer == NULL);
      self->renderer = musivid_renderer_new (musivid_image_section_get_pixbuf (self->image_section),
                                             self->render_options);

      g_autoptr(GList) files = musivid_music_table_get_files (self->music_table);
      for (GList *l = files; l != NULL; l = l->next)
        musivid_renderer_take_file (self->renderer, g_steal_pointer (&l->data));

      musivid_progress_view_reset_renderer (self->progress_view, self->renderer);
      musivid_renderer_run (self->renderer);
    }

  gtk_stack_set_visible_child_name (self->header_stack, child);
  gtk_stack_set_visible_child_name (self->content_stack, child);

  if (strcmp (child, "render") != 0 && self->renderer)
    {
      g_clear_object (&self->renderer);
      musivid_progress_view_reset_renderer (self->progress_view, NULL);
    }
}

static void
musivid_goto_action (GSimpleAction *action,
                     GVariant      *param,
                     gpointer       udata)
{
  MusividWindow *self = MUSIVID_WINDOW (udata);

  g_return_if_fail (g_variant_is_of_type (param, G_VARIANT_TYPE_STRING));

  const char *child = g_variant_get_string (param, NULL);
  musivid_window_goto (self, child);
}

static GActionEntry action_entries[] = {
    { "open-add-music-dialog", musivid_open_add_music_dialog_action, NULL, NULL, NULL },
    { "goto", musivid_goto_action, "s", NULL, NULL },
};

static void
musivid_window_init (MusividWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  g_action_map_add_action_entries (G_ACTION_MAP (self), action_entries, G_N_ELEMENTS (action_entries), self);

  self->music_table = musivid_music_table_new ();
  gtk_container_add (GTK_CONTAINER (self->main_box), GTK_WIDGET (self->music_table));

  self->image_section = musivid_image_section_new ();
  gtk_container_add (GTK_CONTAINER (self->main_box), GTK_WIDGET (self->image_section));

  self->notification = musivid_notification_new ();
  gtk_overlay_add_overlay (self->notification_overlay, GTK_WIDGET (self->notification));

  self->render_options = musivid_render_options_new ();
  self->options_editor = musivid_options_editor_new (self->render_options);
  g_signal_connect (self->render_options, "notify::output-directory", G_CALLBACK (on_output_directory_notify), self);
  gtk_stack_add_named (self->content_stack, GTK_WIDGET (self->options_editor), "options");

  self->progress_view = musivid_progress_view_new ();
  gtk_stack_add_named (self->content_stack, GTK_WIDGET (self->progress_view), "render");

  musivid_window_update_render_button_1_state (self);
  musivid_window_update_render_button_2_state (self);

  g_signal_connect (self->image_section, "notify::image-path", G_CALLBACK (on_pixbuf_notify), self);
  g_signal_connect (self->music_table, "files-changed", G_CALLBACK (on_files_changed), self);
}
