/* main.c
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

#include <glib/gi18n.h>
#include <gst/gst.h>

#include "musivid-config.h"
#include "musivid-window.h"

static void
on_activate (GtkApplication *app)
{
	GtkWindow *window;

	g_assert (GTK_IS_APPLICATION (app));

	window = gtk_application_get_active_window (app);
	if (window == NULL)
    {
  		window = GTK_WINDOW (musivid_window_new (app));

      g_autoptr(GtkCssProvider) provider = gtk_css_provider_new ();
      gtk_css_provider_load_from_resource (provider, "/com/refi64/Musivid/styles.css");
      gtk_style_context_add_provider_for_screen (gdk_screen_get_default (), GTK_STYLE_PROVIDER (provider),
                                                 GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }

	gtk_window_present (window);
}

int
main (int   argc,
      char *argv[])
{

  if (g_getenv ("GST_DEBUG") == NULL)
    {
      if (g_strcmp0 (g_getenv ("G_MESSAGES_DEBUG"), "all") == 0)
        g_setenv ("GST_DEBUG", "4", TRUE);
      else
        g_setenv ("GST_DEBUG", "2", TRUE);
    }

  gst_init (&argc, &argv);

	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	g_autoptr(GtkApplication) app = gtk_application_new ("com.refi64.Musivid", G_APPLICATION_FLAGS_NONE);
  g_application_set_default (G_APPLICATION (app));

	g_signal_connect (app, "activate", G_CALLBACK (on_activate), NULL);

	return g_application_run (G_APPLICATION (app), argc, argv);
}
