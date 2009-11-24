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
#include "manage.h"
#include "menu.h"
#include "cb.h"
#include "notify.h"
#include "script_list.h"
#include "version.h"
#define _(string) gettext(string)

nall_globals_t nall_globals;

#if 0
/* message dialog creator */
static GtkWidget* warning_message_create(GtkWindow* parent, const gchar* message) {
	GtkWidget* window;

	window = gtk_message_dialog_new(parent, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "%s", _("Warning:"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(window), "%s", message);
	g_signal_connect_swapped (window, "response", G_CALLBACK (gtk_widget_destroy), window);
	gtk_widget_show(GTK_WIDGET(window));
	return window;
}
#endif

/* GtkStatusIcon creator */
static GtkStatusIcon* tray_icon_create(void)
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

/* here we are */
int main(int argc, char **argv)
{
	GtkStatusIcon* main_tray_icon = NULL;
	GtkMenu* main_menu = NULL;

	/* application data for callbacks: icon, menu, list, tip... */
	memset(&nall_globals, 0, sizeof(nall_globals));
	
	/*  internationalization */
	bindtextdomain ("nall", LOCALEDIR);
	textdomain ( "nall" );

	gtk_init(&argc, &argv);
	nall_notify_init();

	nall_globals.script_path = g_build_path ("/", g_get_home_dir(), ".nall", NULL);
	nall_globals.script_list = script_list_load();
	na_schedule_all();

/*
 * initialisation
 */
	/* create the main menu */
	main_menu = menu_new();

	/* and its item callbacks */
	menu_append_item(main_menu, _("_Reschedule"), G_CALLBACK(menu_item_on_schedule), &nall_globals);
	menu_append_item(main_menu, _("_Manage Scripts"), G_CALLBACK(menu_item_on_manage), &nall_globals);
	menu_append_image_item(main_menu, GTK_STOCK_ABOUT, G_CALLBACK(menu_item_on_about), &nall_globals);
	menu_append_image_item(main_menu, GTK_STOCK_QUIT, G_CALLBACK(menu_item_on_quit), &nall_globals);

	nall_globals.menu = main_menu;

	/* the tray icon */
	main_tray_icon = tray_icon_create();

	/* and its callbacks */
	g_signal_connect(G_OBJECT(main_tray_icon), "popup-menu",
                         G_CALLBACK(tray_icon_on_menu), &nall_globals);
        g_signal_connect(G_OBJECT(main_tray_icon), "activate", 
                         G_CALLBACK(tray_icon_on_click), &nall_globals);

	nall_globals.icon = (gpointer)main_tray_icon;

/* 
 * run
 */
        gtk_main();
	g_free(nall_globals.script_path);
        exit(EXIT_SUCCESS);
}

