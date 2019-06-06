/* auricle-options.c
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

#include "auricle-options-editor.h"

struct _AuricleOptionsEditor
{
  GtkGrid parent_instance;

  GtkFileChooserButton *options_output_directory;
  GtkComboBoxText      *options_audio_bitrate;

  AuricleRenderOptions *render_options;
};

G_DEFINE_TYPE (AuricleOptionsEditor, auricle_options_editor, GTK_TYPE_GRID)

enum {
  PROP_0,
  PROP_RENDER_OPTIONS,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

AuricleOptionsEditor *
auricle_options_editor_new (AuricleRenderOptions *render_options)
{
  return g_object_new (AURICLE_TYPE_OPTIONS_EDITOR,
                       "render-options", render_options,
                       NULL);
}

static void
auricle_options_editor_finalize (GObject *object)
{
  AuricleOptionsEditor *self = (AuricleOptionsEditor *)object;

  g_clear_object (&self->render_options);

  G_OBJECT_CLASS (auricle_options_editor_parent_class)->finalize (object);
}

static void
auricle_options_editor_get_property (GObject    *object,
                                     guint       prop_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
  AuricleOptionsEditor *self = AURICLE_OPTIONS_EDITOR (object);

  switch (prop_id)
    {
    case PROP_RENDER_OPTIONS:
      g_value_set_object (value, self->render_options);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
auricle_options_editor_set_property (GObject      *object,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  AuricleOptionsEditor *self = AURICLE_OPTIONS_EDITOR (object);

  switch (prop_id)
    {
    case PROP_RENDER_OPTIONS:
      self->render_options = g_value_dup_object (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
auricle_options_editor_class_init (AuricleOptionsEditorClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = auricle_options_editor_finalize;
  object_class->get_property = auricle_options_editor_get_property;
  object_class->set_property = auricle_options_editor_set_property;

  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/com/refi64/Auricle/auricle-options-editor.ui");
  gtk_widget_class_bind_template_child (widget_class, AuricleOptionsEditor, options_output_directory);
  gtk_widget_class_bind_template_child (widget_class, AuricleOptionsEditor, options_audio_bitrate);

  properties [PROP_RENDER_OPTIONS] =
    g_param_spec_object ("render-options",
                         "Render options",
                         "Render options",
                         AURICLE_TYPE_RENDER_OPTIONS,
                         (G_PARAM_READWRITE |
                          G_PARAM_CONSTRUCT_ONLY |
                          G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class, PROP_RENDER_OPTIONS,
                                   properties [PROP_RENDER_OPTIONS]);
}

static void
on_file_set (GtkFileChooserButton *button,
             gpointer              udata)
{
  AuricleOptionsEditor *self = AURICLE_OPTIONS_EDITOR (udata);

  char *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (self->options_output_directory));
  auricle_render_options_take_output_directory (self->render_options, filename);
}

static void
on_bitrate_changed (GtkComboBox *combo_box,
                    gpointer     udata)
{
  AuricleOptionsEditor *self = AURICLE_OPTIONS_EDITOR (udata);

  guint bitrate = atoi (gtk_combo_box_get_active_id (GTK_COMBO_BOX (self->options_audio_bitrate)));
  auricle_render_options_set_audio_bitrate (self->render_options, bitrate);
}

static void
auricle_options_editor_init (AuricleOptionsEditor *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  g_signal_connect (self->options_output_directory, "file-set", G_CALLBACK (on_file_set), self);
  g_signal_connect (self->options_audio_bitrate, "changed", G_CALLBACK (on_bitrate_changed), self);
}

