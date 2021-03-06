/*************************************************************************/
/* Copyright (C) 2021 matias <mati86dl@gmail.com>                        */
/*                                                                       */
/* This program is free software: you can redistribute it and/or modify  */
/* it under the terms of the GNU General Public License as published by  */
/* the Free Software Foundation, either version 3 of the License, or     */
/* (at your option) any later version.                                   */
/*                                                                       */
/* This program is distributed in the hope that it will be useful,       */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of        */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         */
/* GNU General Public License for more details.                          */
/*                                                                       */
/* You should have received a copy of the GNU General Public License     */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */
/*************************************************************************/


#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <glib/gi18n.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

#ifdef HAVE_CHEESE
#include <cheese-gtk.h>
#endif

#include "cc-user-panel.h"

#include "xings-user-accounts-common.h"

static void
xings_user_accounts_application_activate (GtkApplication *application,
                                          gpointer        user_data)
{
	GtkWindow *window;

	g_assert (GTK_IS_APPLICATION (application));

	window = gtk_application_get_active_window (GTK_APPLICATION (application));

	if (window == NULL) {
		window = GTK_WINDOW (g_object_new (CC_TYPE_USER_PANEL, NULL));
		gtk_application_add_window (application, window);
		gtk_window_set_title (window, _("Users"));
		gtk_window_set_icon_name (window, "system-users");
	}

	gtk_window_present (window);

}

int
main (int    argc,
      char **argv)
{
	GtkApplication *app;
	GOptionContext *context;
	int status;

	/* Translation */

	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	/* Debug options */

	context = g_option_context_new (NULL);
	g_option_context_set_summary (context, _("User Accounts"));
	g_option_context_parse (context, &argc, &argv, NULL);
	g_option_context_free (context);

	/* Dependencies */
#ifdef HAVE_CHEESE
	cheese_gtk_init (&argc, &argv);
#endif

	/* GtkApplication */

	app = gtk_application_new (XUA_DBUS_NAME, G_APPLICATION_FLAGS_NONE);
	g_signal_connect (app, "activate",
	                  G_CALLBACK (xings_user_accounts_application_activate), NULL);

	status = g_application_run (G_APPLICATION (app), argc, argv);
	g_object_unref (app);

	return status;
}

