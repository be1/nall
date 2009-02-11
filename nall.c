/* 
 * nall.c Copyright © 2008 by Benoît Rouits <brouits@free.fr>
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
 * see the COPYING file included in the csv2latex package or
 * http://www.gnu.org/licenses/gpl.txt for the full licence
 * 
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>
#include <glib.h>
#include <gtk/gtk.h>
#include "na.h"
#include "menu.h"

/* handler for the "Quit" menu item */
void menu_item_on_quit(GtkMenuItem* instance, gpointer app_data)
{
	na_quit(app_data);
}

/* handler for left-button click */
void tray_icon_on_click(GtkStatusIcon* instance, gpointer app_data)
{
	if (app_data)
		na_reap(app_data);
}

/* handler for right-button click */
void tray_icon_on_menu(GtkStatusIcon* instance, guint button, guint activate_time, gpointer app_data)
{
	gpointer* d = (gpointer*) app_data;
	if (app_data)
		menu_show(GTK_MENU(d[MENU]), button, activate_time);
}

/* Gtkmenu creator */
GtkMenu* menu_create(void)
{
	GtkMenu* menu;

	menu = menu_new();

	return GTK_MENU(menu);
}

/* GtkStatusIcon creator */
GtkStatusIcon* tray_icon_create(void)
{
        GtkStatusIcon* tray_icon;

        tray_icon = gtk_status_icon_new();
        gtk_status_icon_set_from_icon_name(tray_icon, GTK_STOCK_INFO);
        gtk_status_icon_set_tooltip(tray_icon, "nall");
        gtk_status_icon_set_visible(tray_icon, TRUE);

        return tray_icon;
}

/* cli usage message */
void usage(char* prog, int exitcode) {
	fprintf(stderr,"Usage: %s [-r REAP_FREQ] (defaults to %d seconds)\n",
			basename(prog), NA_FALLBACK_REAP_FREQ);
	exit(exitcode);
}

/* here we are */
int main(int argc, char **argv)
{
	GError* err = NULL;
	gchar* script_path = NULL;
	gint reap = NA_FALLBACK_REAP_FREQ;

	GtkStatusIcon* main_tray_icon = NULL;
	GtkMenu* main_menu = NULL;
	GList* main_script_list = NULL;
	gchar main_tooltip_buffer [BUFSIZ];

	/* application data for callbacks: icon, menu, list, and tip */
	gpointer app_data [4];

/*
 * scan $HOME/.nall for scripts
 */ 
	script_path = g_build_path ("/", g_get_home_dir(), ".nall", NULL);
	main_script_list = na_register_scripts(script_path);

	if (!main_script_list) {
		g_message("no script to run: aborting");
		exit(EXIT_SUCCESS);
	}
	app_data[LIST]=(void*)main_script_list;
	g_free(script_path);

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
			if (!strcmp(argv[1],"-r")) {
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
 * initialisation
 */
	app_data[TIP]=(gpointer)main_tooltip_buffer;

	/* create the main menu */
	main_menu = menu_create();

	/* and its item callbacks */
	menu_append_item(main_menu, "Schedule", NULL, NULL);
	menu_append_item(main_menu, "Rescan", NULL, NULL);
	menu_append_item(main_menu, "About", NULL, NULL);
	menu_append_item(main_menu, "Quit", G_CALLBACK(menu_item_on_quit), app_data);

	app_data[MENU]=(gpointer)main_menu;

	/* the tray icon */
	main_tray_icon = tray_icon_create();

	/* and its callbacks */
	g_signal_connect(G_OBJECT(main_tray_icon), "popup-menu",
                         G_CALLBACK(tray_icon_on_menu), app_data);
        g_signal_connect(G_OBJECT(main_tray_icon), "activate", 
                         G_CALLBACK(tray_icon_on_click), app_data);

	app_data[ICON]=(gpointer)main_tray_icon;

/* 
 * run
 */
	/* schedule the buffers reaper */
	na_init_reaper(reap, app_data);

        gtk_main();
        exit(EXIT_SUCCESS);
}

