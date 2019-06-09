/* auricle-window.c
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

#include "auricle-config.h"
#include "auricle-window.h"
#include "auricle-image-section.h"
#include "auricle-music-file.h"
#include "auricle-music-table.h"
#include "auricle-notification.h"
#include "auricle-options-editor.h"
#include "auricle-progress-view.h"
#include "auricle-renderer.h"

struct _AuricleWindow
{
  GtkApplicationWindow  parent_instance;

  GtkStack            *header_stack;
  GtkStack            *content_stack;
  GtkPopoverMenu      *menu;
  GtkButton           *render_button_1;
  GtkButton           *render_button_2;
  GtkBox              *main_box;
  GtkOverlay          *notification_overlay;

  AuricleImageSection  *image_section;
  AuricleMusicTable    *music_table;
  AuricleNotification  *notification;
  AuricleOptionsEditor *options_editor;
  AuricleProgressView  *progress_view;

  AuricleRenderer      *renderer;
  AuricleRenderOptions *render_options;
};

G_DEFINE_TYPE (AuricleWindow, auricle_window, GTK_TYPE_APPLICATION_WINDOW)

AuricleWindow *
auricle_window_new (GtkApplication *app)
{
  return g_object_new (AURICLE_TYPE_WINDOW,
		                   "application", app,
		                   NULL);
}

static void auricle_window_goto (AuricleWindow *self,
                                 const char    *child);

void
auricle_window_show_notification (AuricleWindow *window,
                                  const char    *message)
{
  auricle_notification_show (window->notification, message);
  auricle_window_goto (window, "main");
}

static void
auricle_window_class_init (AuricleWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/com/refi64/Auricle/auricle-window.ui");
  gtk_widget_class_bind_template_child (widget_class, AuricleWindow, header_stack);
  gtk_widget_class_bind_template_child (widget_class, AuricleWindow, content_stack);
  gtk_widget_class_bind_template_child (widget_class, AuricleWindow, menu);
  gtk_widget_class_bind_template_child (widget_class, AuricleWindow, render_button_1);
  gtk_widget_class_bind_template_child (widget_class, AuricleWindow, render_button_2);
  gtk_widget_class_bind_template_child (widget_class, AuricleWindow, main_box);
  gtk_widget_class_bind_template_child (widget_class, AuricleWindow, notification_overlay);
}

static void
auricle_window_update_render_button_1_state (AuricleWindow *self)
{
  gboolean enabled = auricle_image_section_get_pixbuf (self->image_section) != NULL &&
                    !auricle_music_table_is_empty (self->music_table);
  gtk_widget_set_sensitive (GTK_WIDGET (self->render_button_1), enabled);
}

static void
auricle_window_update_render_button_2_state (AuricleWindow *self)
{
  gboolean enabled = self->render_options != NULL && auricle_render_options_get_output_directory (self->render_options) != NULL;
  gtk_widget_set_sensitive (GTK_WIDGET (self->render_button_2), enabled);
}

static void
on_pixbuf_notify (AuricleImageSection *isec,
                  GParamSpec          *pspec,
                  gpointer             udata)
{
  AuricleWindow *self = AURICLE_WINDOW (udata);

  auricle_window_update_render_button_1_state (self);
}

static void
on_files_changed (AuricleMusicTable *mutab,
                  int                changes,
                  gpointer           udata)
{
  AuricleWindow *self = AURICLE_WINDOW (udata);

  auricle_window_update_render_button_1_state (self);
}

static void
on_output_directory_notify (AuricleRenderOptions *render_options,
                            GParamSpec           *pspec,
                            gpointer              udata)
{
  AuricleWindow *self = AURICLE_WINDOW (udata);

  auricle_window_update_render_button_2_state (self);
}

static void
auricle_open_add_music_dialog_action (GSimpleAction *action,
                                      GVariant      *param,
                                      gpointer       udata)
{
  AuricleWindow *self = AURICLE_WINDOW (udata);

  GtkFileChooserNative *chooser = gtk_file_chooser_native_new ("Add Music", GTK_WINDOW (self), GTK_FILE_CHOOSER_ACTION_OPEN,
                                                               "_Add", "_Cancel");

  GtkFileFilter *filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, "Audio files");
  gtk_file_filter_add_mime_type (filter, "audio/*");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), filter);

  GtkFileFilter *all_filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (all_filter, "All files");
  gtk_file_filter_add_pattern (all_filter, "*");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), all_filter);

  gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (chooser), TRUE);

  if (gtk_native_dialog_run (GTK_NATIVE_DIALOG (chooser)) == GTK_RESPONSE_ACCEPT)
    {
      GSList *filenames = gtk_file_chooser_get_filenames (GTK_FILE_CHOOSER (chooser));
      auricle_music_table_add_tracks (self->music_table, filenames);
      g_slist_free_full (filenames, g_free);
    }
}

static gboolean
goto_main_on_idle (gpointer udata)
{
  AuricleWindow *self = AURICLE_WINDOW (udata);
  auricle_window_goto (self, "main");
  return FALSE;
}

static void
on_render_complete (AuricleRenderer *renderer,
                    gpointer         udata)
{
  // Avoid destroyed the renderer while we're inside its signal.
  g_idle_add (goto_main_on_idle, udata);
}

static void
auricle_window_goto (AuricleWindow *self,
                     const char    *child)
{
  if (strcmp (child, "render") == 0)
    {
      g_warn_if_fail (self->renderer == NULL);
      self->renderer = auricle_renderer_new (auricle_image_section_get_pixbuf (self->image_section),
                                             self->render_options);

      g_signal_connect (self->renderer, "complete", G_CALLBACK (on_render_complete), self);

      g_autoptr(GList) files = auricle_music_table_get_files (self->music_table);
      for (GList *l = files; l != NULL; l = l->next)
        auricle_renderer_take_file (self->renderer, g_steal_pointer (&l->data));

      auricle_progress_view_reset_renderer (self->progress_view, self->renderer);
      auricle_renderer_run (self->renderer);
    }

  gtk_stack_set_visible_child_name (self->header_stack, child);
  gtk_stack_set_visible_child_name (self->content_stack, child);

  if (strcmp (child, "render") != 0 && self->renderer)
    {
      g_clear_object (&self->renderer);
      auricle_progress_view_reset_renderer (self->progress_view, NULL);
    }
}

static void
auricle_goto_action (GSimpleAction *action,
                     GVariant      *param,
                     gpointer       udata)
{
  AuricleWindow *self = AURICLE_WINDOW (udata);

  g_return_if_fail (g_variant_is_of_type (param, G_VARIANT_TYPE_STRING));

  const char *child = g_variant_get_string (param, NULL);
  auricle_window_goto (self, child);
}

static void
auricle_open_menu_action (GSimpleAction *action,
                          GVariant      *param,
                          gpointer       udata)
{
  AuricleWindow *self = AURICLE_WINDOW (udata);

  g_print ("open menu\n");
  gtk_popover_popup (GTK_POPOVER (self->menu));
}

static void
auricle_about_action (GSimpleAction *action,
                      GVariant      *param,
                      gpointer udata)
{
  AuricleWindow *self = AURICLE_WINDOW (udata);

  const char *authors[] = {"Ryan Gonzalez", NULL};
  gtk_show_about_dialog (GTK_WINDOW (self),
                         "title", "About Auricle",
                         "program-name", "Auricle",
                         "logo-icon-name", "com.refi64.Auricle",
                         "version", PACKAGE_VERSION,
                         "website", "https://github.com/refi64/auricle",
                         "authors", authors,
                         "license-type", GTK_LICENSE_GPL_3_0,
                         NULL);
}

static GActionEntry action_entries[] = {
    { "open-add-music-dialog", auricle_open_add_music_dialog_action, NULL, NULL, NULL },
    { "goto", auricle_goto_action, "s", NULL, NULL },
    { "open-menu", auricle_open_menu_action, NULL, NULL, NULL },
    { "about", auricle_about_action, NULL, NULL, NULL },
};

static void
auricle_window_init (AuricleWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  g_action_map_add_action_entries (G_ACTION_MAP (self), action_entries, G_N_ELEMENTS (action_entries), self);

  self->music_table = auricle_music_table_new ();
  gtk_container_add (GTK_CONTAINER (self->main_box), GTK_WIDGET (self->music_table));

  self->image_section = auricle_image_section_new ();
  gtk_container_add (GTK_CONTAINER (self->main_box), GTK_WIDGET (self->image_section));

  self->notification = auricle_notification_new ();
  gtk_overlay_add_overlay (self->notification_overlay, GTK_WIDGET (self->notification));

  self->render_options = auricle_render_options_new ();
  self->options_editor = auricle_options_editor_new (self->render_options);
  g_signal_connect (self->render_options, "notify::output-directory", G_CALLBACK (on_output_directory_notify), self);
  gtk_stack_add_named (self->content_stack, GTK_WIDGET (self->options_editor), "options");

  self->progress_view = auricle_progress_view_new ();
  gtk_stack_add_named (self->content_stack, GTK_WIDGET (self->progress_view), "render");

  auricle_window_update_render_button_1_state (self);
  auricle_window_update_render_button_2_state (self);

  g_signal_connect (self->image_section, "notify::image-path", G_CALLBACK (on_pixbuf_notify), self);
  g_signal_connect (self->music_table, "files-changed", G_CALLBACK (on_files_changed), self);
}
