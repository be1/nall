/* 
 * nall.c Copyright © 2008-2009 by Benoît Rouits <brouits@free.fr>
 * Published under the terms of the GNU General Public License v2 (GPLv2).
 * 
 ***************************************************** 
 * nall: a non-intrusive desktop all-purpose notifer *
 *****************************************************
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, 
 * Boston, MA  02110-1301, USA.
 * 
 * see the COPYING file included in the nall package or
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt for the full licence
 * 
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <libgen.h>
#include <libintl.h>
#include <locale.h>
#include <glib.h>
#include <gtk/gtk.h>
#include "config.h"
#include "na.h"
#include "menu.h"
#include "cb.h"
#include "version.h"
#define _(string) gettext(string)

/* message dialog creator */
GtkWidget* warning_message_create(GtkWindow* parent, const gchar* message) {
	GtkWidget* window;

	window = gtk_message_dialog_new(parent, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "%s", _("Warning:"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(window), "%s", message);
	g_signal_connect_swapped (window, "response", G_CALLBACK (gtk_widget_destroy), window);
	gtk_widget_show(GTK_WIDGET(window));
	return window;
}

/* GtkStatusIcon creator */
GtkStatusIcon* tray_icon_create(void)
{
        GtkStatusIcon* tray_icon;

        tray_icon = gtk_status_icon_new();
	/* systray visible icon */
        gtk_status_icon_set_from_icon_name(tray_icon, GTK_STOCK_INFO);
	/* default wm icon (for subsequent windows) */
        gtk_window_set_default_icon_name(GTK_STOCK_INFO);
        gtk_status_icon_set_tooltip(tray_icon, "nall");
        gtk_status_icon_set_visible(tray_icon, TRUE);

        return tray_icon;
}

/* cli usage message */
void usage(char* prog, int exitcode) {
	fprintf(stderr, "%s\n", PACKAGE_VERSION);
	fprintf(stderr, "Usage: %s [-r REFERSH_FREQ] (buffer refresh: defaults to %d second(s))\n",
			basename(prog), NA_FALLBACK_REFRESH_FREQ);
	exit(exitcode);
}

/* here we are */
int main(int argc, char **argv)
{
	gint reap = NA_FALLBACK_REFRESH_FREQ;

	GtkStatusIcon* main_tray_icon = NULL;
	GtkMenu* main_menu = NULL;
	GList* main_script_list = NULL;

	/* application data for callbacks: icon, menu, list, tip... */
	app_data_t app_data;
	memset(&app_data, 0, sizeof(app_data));
	
	/*  internationalization */
	bindtextdomain ("nall", LOCALEDIR);
	textdomain ( "nall" );

/*
 * cli argument parsing
 */ 
	gtk_init(&argc, &argv);
	switch (argc) {
		case 1:
			break;
		case 2:
			usage(argv[0], EXIT_FAILURE);
		case 3:
			if (!strcmp(argv[1], "-r")) {
				if (isdigit(argv[2][0])) {
					reap = atoi(argv[2]);
					break;
				}
				usage(argv[0], EXIT_FAILURE);
			}
			usage(argv[0], EXIT_FAILURE);
		default:
			usage(argv[0], EXIT_FAILURE);
	}
/*
 * scan $HOME/.nall for scripts
 */ 
	app_data.script_path = g_build_path ("/", g_get_home_dir(), ".nall", NULL);

	app_data.script_list = na_register_scripts(app_data.script_path);
	if (!app_data.script_list) {
		warning_message_create(NULL, _("No script found in ~/.nall directory.\nPlease provide one or more scripts to schedule and rescan.\nSee examples in the documentation directory."));
	} 


/*
 * initialisation
 */
	/* create the main menu */
	main_menu = menu_new();

	/* and its item callbacks */
	menu_append_item(main_menu, _("Reschedule"), G_CALLBACK(menu_item_on_schedule), &app_data);
	menu_append_item(main_menu, _("Reload all"), G_CALLBACK(menu_item_on_rescan), &app_data);
	menu_append_image_item(main_menu, GTK_STOCK_ABOUT, G_CALLBACK(menu_item_on_about), &app_data);
	menu_append_image_item(main_menu, GTK_STOCK_QUIT, G_CALLBACK(menu_item_on_quit), &app_data);

	app_data.menu = main_menu;

	/* the tray icon */
	main_tray_icon = tray_icon_create();

	/* and its callbacks */
	g_signal_connect(G_OBJECT(main_tray_icon), "popup-menu",
                         G_CALLBACK(tray_icon_on_menu), &app_data);
        g_signal_connect(G_OBJECT(main_tray_icon), "activate", 
                         G_CALLBACK(tray_icon_on_click), &app_data);

	app_data.icon = (gpointer)main_tray_icon;

/* 
 * run
 */
	/* schedule the buffers reaper */
	na_init_reaper(reap, &app_data);

        gtk_main();
	g_free(app_data.script_path);
        exit(EXIT_SUCCESS);
}

