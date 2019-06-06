/* auricle-utils.c
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

#include "auricle-utils.h"
#include "auricle-window.h"

void
auricle_show_notification_internal (char *message)
{
  GApplication *app = g_application_get_default ();
  g_return_if_fail (app != NULL);
  g_return_if_fail (GTK_IS_APPLICATION (app));

  g_info ("Notification: %s", message);

  GList *windows = gtk_application_get_windows (GTK_APPLICATION (app));
  for (GList *l = windows; l != NULL; l = l->data)
    {
      if (AURICLE_IS_WINDOW (l->data))
        {
          AuricleWindow *win = AURICLE_WINDOW (l->data);
          auricle_window_show_notification (win, message);
          goto done;
        }
    }

  g_warning ("auricle_show_notification failed to find main window");

done:
  g_free (message);
}

static gpointer
build_substitute (gpointer udata)
{
  return g_regex_new ("@\\{([\\w_\\-]+)(?:,([^}]+)|:(\\d+))?\\}|(?:\\\\)?(.)", 0, 0, NULL);
}

char *
auricle_substitute (const char *template,
                    GHashTable *vars)
{
  static GOnce substitute_re_once = G_ONCE_INIT;
  g_once (&substitute_re_once, build_substitute, NULL);

  GRegex *substitute_re = substitute_re_once.retval;

  g_autoptr(GString) result = g_string_new ("");
  g_autoptr(GMatchInfo) match = NULL;
  g_regex_match (substitute_re, template, 0, &match);

  while (g_match_info_matches (match))
    {
      g_autofree char *var = g_match_info_fetch (match, 1);
      if (var && *var != '\0')
        {
          char **value = g_hash_table_lookup (vars, var);
          if (value != NULL)
            {
              g_autofree char *join = g_match_info_fetch (match, 2);
              g_autofree char *idx_s = g_match_info_fetch (match, 3);

              if (idx_s != NULL && *idx_s != '\0')
                {
                  gint64 idx = g_ascii_strtoll (idx_s, NULL, 10);
                  if (idx < g_strv_length (value))
                    g_string_append (result, value[idx]);
                }
              else
                {
                  const gchar *sep = join != NULL && *join != '\0' ? join : ", ";
                  g_autofree char *joined = g_strjoinv (sep, value);
                  g_string_append (result, joined);
                }
            }
        }
      else
        {
          g_autofree char *plain = g_match_info_fetch (match, 4);
          g_string_append (result, plain);
        }

      g_match_info_next (match, NULL);
    }

  return g_string_free (g_steal_pointer (&result), FALSE);
}

