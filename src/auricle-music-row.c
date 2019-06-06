/* auricle-music-row.c
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

#include <gst/gst.h>

#include "auricle-music-row.h"
#include "auricle-utils.h"

struct _AuricleMusicRow
{
  GtkListBoxRow parent_instance;

  GtkLabel    *music_row_basename;
  GtkLabel    *music_row_result_name;
  GtkButton   *music_row_delete;
  GtkImage    *music_row_expand_icon;
  GtkRevealer *music_row_revealer;
  GtkGrid     *music_row_tag_grid;
  GtkSwitch   *music_row_override_switch;
  GtkRevealer *music_row_override_revealer;
  GtkRevealer *music_row_override_entry;

  GHashTable *template_vars;

  char       *path;
  char       *template;
  char       *template_override;
  GstElement *pipeline;
  guint       bus_watch_id;
};

G_DEFINE_TYPE (AuricleMusicRow, auricle_music_row, GTK_TYPE_LIST_BOX_ROW)

enum {
  PROP_0,
  PROP_PATH,
  PROP_TEMPLATE,
  PROP_TEMPLATE_OVERRIDE,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

enum {
  SIGNAL_0,
  DELETE_REQUESTED,
  N_SIGNALS
};

static guint signals [N_SIGNALS];

AuricleMusicRow *
auricle_music_row_new (const char *path)
{
  return g_object_new (AURICLE_TYPE_MUSIC_ROW,
                       "path", path,
                       NULL);
}

static void
auricle_music_row_finalize (GObject *object)
{
  AuricleMusicRow *self = (AuricleMusicRow *)object;

  if (self->pipeline)
    gst_element_set_state (self->pipeline, GST_STATE_NULL);

  if (self->bus_watch_id != 0)
    {
      g_source_remove (self->bus_watch_id);
      self->bus_watch_id = 0;
    }

  g_clear_pointer (&self->template_vars, g_hash_table_unref);
  g_clear_pointer (&self->path, g_free);
  g_clear_pointer (&self->template, g_free);
  g_clear_pointer (&self->template_override, g_free);
  g_clear_pointer (&self->pipeline, gst_object_unref);
  G_OBJECT_CLASS (auricle_music_row_parent_class)->finalize (object);
}

static void
on_dec_pad_added (GstElement *el,
                  GstPad     *pad,
                  GstElement *sink)
{
  g_autoptr(GstPad) sinkpad = gst_element_get_static_pad (sink, "sink");
  if (!gst_pad_is_linked (sinkpad))
    {
      if (gst_pad_link (pad, sinkpad) != GST_PAD_LINK_OK)
        g_error ("Failed to link pads");
    }
}

static char *
value_to_string (const GValue *value)
{
  if (G_VALUE_HOLDS_BOOLEAN (value))
    return g_value_get_boolean (value) ? g_strdup ("TRUE") : g_strdup ("FALSE");
  else if (G_VALUE_HOLDS_INT (value))
    return g_strdup_printf ("%i", g_value_get_int (value));
  else if (G_VALUE_HOLDS_UINT (value))
    return g_strdup_printf ("%u", g_value_get_uint (value));
  else if (G_VALUE_HOLDS_INT64 (value))
    return g_strdup_printf ("%"G_GINT64_FORMAT, g_value_get_int64 (value));
  else if (G_VALUE_HOLDS_UINT64 (value))
    return g_strdup_printf ("%"G_GUINT64_FORMAT, g_value_get_uint64 (value));
  else if (G_VALUE_HOLDS_FLOAT  (value))
    return g_strdup_printf ("%f", g_value_get_float (value));
  else if (G_VALUE_HOLDS_DOUBLE  (value))
    return g_strdup_printf ("%lf", g_value_get_double (value));
  else if (G_VALUE_HOLDS_STRING (value))
    return g_value_dup_string (value);
  else
    return NULL;
}

static void
auricle_music_row_update_result_name (AuricleMusicRow *self)
{
  g_autofree char *result = auricle_substitute (self->template, self->template_vars);

  if (gtk_switch_get_active (self->music_row_override_switch))
    {
      char **default_strv = g_new0 (char *, 2);
      default_strv[0] = g_steal_pointer (&result);
      g_hash_table_replace (self->template_vars, g_strdup ("default"), g_steal_pointer (&default_strv));

      result = auricle_substitute (self->template_override, self->template_vars);
      g_hash_table_remove (self->template_vars, "default");
    }

  gtk_label_set_label (self->music_row_result_name, result);
}

static void
auricle_music_row_add_tags (AuricleMusicRow *self,
                            GstTagList      *tags)
{
  int ntags = gst_tag_list_n_tags (tags);

  for (int i = 0; i < ntags; i++)
    {
      const char *name = gst_tag_list_nth_tag_name (tags, i);
      int nitems = gst_tag_list_get_tag_size (tags, name);

      g_autoptr(GPtrArray) values = g_ptr_array_new_with_free_func (g_free);

      for (int j = 0; j < nitems; j++)
        {
          const GValue *value = gst_tag_list_get_value_index (tags, name, j);
          g_autofree char *str = value_to_string (value);
          if (str == NULL)
            continue;

          g_ptr_array_add (values, g_steal_pointer (&str));
        }

      g_ptr_array_add (values, NULL);

      g_hash_table_replace (self->template_vars, g_strdup (name), g_strdupv ((gchar **)values->pdata));

      g_autofree char *values_str = g_strjoinv (",", (gchar **)values->pdata);

      GtkWidget *name_label = gtk_label_new (name);
      GtkWidget *values_label = gtk_label_new (values_str);

      gtk_widget_set_halign (name_label, GTK_ALIGN_END);
      gtk_widget_set_halign (values_label, GTK_ALIGN_START);

      gtk_grid_attach_next_to (self->music_row_tag_grid, name_label, NULL, GTK_POS_BOTTOM, 1, 1);
      gtk_grid_attach_next_to (self->music_row_tag_grid, values_label, name_label, GTK_POS_RIGHT, 1, 1);
    }

  gtk_widget_show_all (GTK_WIDGET (self->music_row_tag_grid));
  auricle_music_row_update_result_name (self);
}

static gboolean
on_bus_message (GstBus     *bus,
                GstMessage *message,
                gpointer    udata)
{
  AuricleMusicRow *self = AURICLE_MUSIC_ROW (udata);
  g_autofree char *basename = NULL;
  g_autoptr(GError) error = NULL;
  g_autoptr(GstTagList) tags = NULL;

  switch (GST_MESSAGE_TYPE (message))
    {
    case GST_MESSAGE_ERROR:
      gst_message_parse_error (message, &error, NULL);
      basename = g_path_get_basename (self->path);
      auricle_show_notification ("Pipeline for %s tags got error: %s", basename, error->message);
      // Fallthrough.
    case GST_MESSAGE_ASYNC_DONE:
      self->bus_watch_id = 0;
      return FALSE;
    case GST_MESSAGE_TAG:
      gst_message_parse_tag (message, &tags);
      auricle_music_row_add_tags (self, tags);
      break;
    default:
      break;
    }

  return TRUE;
}

static void
auricle_music_row_update_tags (AuricleMusicRow *self)
{
  g_warn_if_fail (self->pipeline == NULL);
  self->pipeline = gst_pipeline_new ("tag-pipeline");

  self->bus_watch_id = gst_bus_add_watch (GST_ELEMENT_BUS (self->pipeline), on_bus_message, self);

  GstElement *src = gst_element_factory_make ("filesrc", NULL);
  g_object_set (src, "location", self->path, NULL);

  GstElement *typefind = gst_element_factory_make ("typefind", NULL);
  GstElement *dec = gst_element_factory_make ("decodebin3", NULL);
  GstElement *sink = gst_element_factory_make ("fakesink", NULL);

  gst_bin_add_many (GST_BIN (self->pipeline), src, typefind, dec, sink, NULL);
  gst_element_link_many (src, typefind, dec, sink, NULL);
  g_signal_connect (dec, "pad-added", G_CALLBACK (on_dec_pad_added), sink);

  gst_element_set_state (self->pipeline, GST_STATE_PAUSED);
}

static void
auricle_music_row_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  AuricleMusicRow *self = AURICLE_MUSIC_ROW (object);

  switch (prop_id)
    {
    case PROP_PATH:
      g_warn_if_fail (self->path != NULL);
      g_value_set_string (value, self->path);
      break;
    case PROP_TEMPLATE:
      g_value_set_string (value, self->template);
      break;
    case PROP_TEMPLATE_OVERRIDE:
      g_value_set_string (value, self->template_override);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static char **
strv_new_with_value (const char *value)
{
  char **result = g_new0 (char *, 2);
  result[0] = g_strdup (value);
  return result;
}

static char *
remove_extension (const char *path)
{
  const char *p = strrchr (path, '.');
  if (p == NULL)
    return g_strdup (path);
  else
    return g_strndup (path, p - path);
}

static void
auricle_music_row_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  AuricleMusicRow *self = AURICLE_MUSIC_ROW (object);
  g_autofree char *basename = NULL;
  g_autofree char *name = NULL;

  switch (prop_id)
    {
    case PROP_PATH:
      g_warn_if_fail (self->path == NULL);
      self->path = g_value_dup_string (value);

      basename = g_path_get_basename (self->path);
      gtk_label_set_text (self->music_row_basename, basename);

      name = remove_extension (basename);

      g_hash_table_replace (self->template_vars, g_strdup ("path"), strv_new_with_value (self->path));
      g_hash_table_replace (self->template_vars, g_strdup ("basename"), strv_new_with_value (basename));
      g_hash_table_replace (self->template_vars, g_strdup ("name"), strv_new_with_value (name));

      auricle_music_row_update_tags (self);
      // Don't update the result name here, because the template prop is synced shortly after this
      break;
    case PROP_TEMPLATE:
      g_free (self->template);
      self->template = g_value_dup_string (value);

      auricle_music_row_update_result_name (self);
      break;
    case PROP_TEMPLATE_OVERRIDE:
      g_free (self->template_override);
      self->template_override = g_value_dup_string (value);

      auricle_music_row_update_result_name (self);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
auricle_music_row_class_init (AuricleMusicRowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = auricle_music_row_finalize;
  object_class->get_property = auricle_music_row_get_property;
  object_class->set_property = auricle_music_row_set_property;

  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  gtk_widget_class_set_template_from_resource (widget_class, "/com/refi64/Auricle/auricle-music-row.ui");
  gtk_widget_class_bind_template_child (widget_class, AuricleMusicRow, music_row_basename);
  gtk_widget_class_bind_template_child (widget_class, AuricleMusicRow, music_row_result_name);
  gtk_widget_class_bind_template_child (widget_class, AuricleMusicRow, music_row_delete);
  gtk_widget_class_bind_template_child (widget_class, AuricleMusicRow, music_row_expand_icon);
  gtk_widget_class_bind_template_child (widget_class, AuricleMusicRow, music_row_revealer);
  gtk_widget_class_bind_template_child (widget_class, AuricleMusicRow, music_row_tag_grid);
  gtk_widget_class_bind_template_child (widget_class, AuricleMusicRow, music_row_override_switch);
  gtk_widget_class_bind_template_child (widget_class, AuricleMusicRow, music_row_override_revealer);
  gtk_widget_class_bind_template_child (widget_class, AuricleMusicRow, music_row_override_entry);

  properties [PROP_PATH] =
    g_param_spec_string ("path",
                         "Audio file path",
                         "The path to the audio file this row contains",
                         NULL,
                         (G_PARAM_READWRITE |
                          G_PARAM_CONSTRUCT_ONLY |
                          G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class, PROP_PATH,
                                   properties [PROP_PATH]);

  properties [PROP_TEMPLATE] =
    g_param_spec_string ("template",
                         "Template",
                         "Template",
                         "",
                         (G_PARAM_READWRITE |
                          G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class, PROP_TEMPLATE,
                                   properties [PROP_TEMPLATE]);

  properties [PROP_TEMPLATE_OVERRIDE] =
    g_param_spec_string ("template-override",
                         "Template override",
                         "Template override",
                         "",
                         (G_PARAM_READWRITE |
                          G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class, PROP_TEMPLATE_OVERRIDE,
                                   properties [PROP_TEMPLATE_OVERRIDE]);

  signals [DELETE_REQUESTED] =
    g_signal_new ("delete-requested",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL,
                  NULL,
                  g_cclosure_marshal_generic,
                  G_TYPE_NONE,
                  0);
}

static void
on_delete_clicked (GtkButton *buttom,
                   gpointer   udata)
{
  AuricleMusicRow *self = AURICLE_MUSIC_ROW (udata);

  g_signal_emit (self, signals[DELETE_REQUESTED], 0);
}

static void
auricle_music_row_init (AuricleMusicRow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  self->template_vars = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

  g_signal_connect (self->music_row_delete, "clicked", G_CALLBACK (on_delete_clicked), self);
  g_object_bind_property (self->music_row_override_switch, "active", self->music_row_override_revealer, "reveal-child", 0);
  g_object_bind_property (self->music_row_override_entry, "text", self, "template-override", 0);
}

const char *
auricle_music_row_get_path (AuricleMusicRow *self)
{
  return self->path;
}

const char *
auricle_music_row_get_result_name (AuricleMusicRow *self)
{
  return gtk_label_get_text (self->music_row_result_name);
}

void
auricle_music_row_toggle (AuricleMusicRow *self)
{
  gboolean current = gtk_revealer_get_reveal_child (self->music_row_revealer);
  gtk_revealer_set_reveal_child (self->music_row_revealer, !current);

  GtkStyleContext *style_context = gtk_widget_get_style_context (GTK_WIDGET (self->music_row_expand_icon));
  if (current)
    gtk_style_context_remove_class (style_context, "rotate");
  else
    gtk_style_context_add_class (style_context, "rotate");
}

