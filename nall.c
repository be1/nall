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

/* global stuff (FIXME) */
GtkStatusIcon *global_tray_icon = NULL;
GList* global_script_list = NULL;
gchar global_tooltip_buffer [BUFSIZ];

/* handler for left-button click (FIXME) */
void tray_icon_on_click(void)
{
        na_refresh_tooltip(global_tray_icon, global_script_list);
}

/* handler for right-button click (FIXME) */
void tray_icon_on_menu(void)
{
	/* FIXME: popup a "apropos"/"quit" menu entry instead */
        na_quit(global_tray_icon, global_script_list);
}

/* GtkStatusIcon creator */
GtkStatusIcon* tray_icon_new(void)
{
        GtkStatusIcon *tray_icon;

        tray_icon = gtk_status_icon_new();

	g_signal_connect(G_OBJECT(tray_icon), "popup-menu",
                         G_CALLBACK(tray_icon_on_menu), NULL);
        g_signal_connect(G_OBJECT(tray_icon), "activate", 
                         G_CALLBACK(tray_icon_on_click), NULL);
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

/* we scan $HOME/.nall for scripts */
	script_path = g_build_path ("/", g_get_home_dir(), ".nall", NULL);
	global_script_list = na_register_scripts(script_path, NULL);

	if (!global_script_list) {
		g_message("no script to run: aborting");
		exit(EXIT_SUCCESS);
	}
	g_free(script_path);

/* cli argument parsing */
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
/* initialisation */
	na_init_reaper(reap, NULL);
	global_tray_icon = tray_icon_new();

/* run */
        gtk_main();
        exit(EXIT_SUCCESS);
}

