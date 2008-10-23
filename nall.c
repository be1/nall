/* Copyright © 2008 by Benoît Rouits
 * Published under the terms of the GNU General Public License (GPL).
 */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>
#include <glib.h>
#include <gtk/gtk.h>
#include "na.h"

GtkStatusIcon *global_tray_icon = NULL;
GList* global_script_list = NULL;
gchar global_tooltip_buffer [BUFSIZ];

void tray_icon_on_click(void)
{
        na_rescan(global_tray_icon, global_script_list);
}

void tray_icon_on_menu(void)
{
        na_popup_config(global_tray_icon, global_script_list);
}

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

void usage(char* prog, int exitcode) {
	fprintf(stderr,"Usage: %s [-r REAP_FREQ] (defaults to %d seconds)\n",
			basename(prog), NA_FALLBACK_REAP_FREQ);
	exit(exitcode);
}

int main(int argc, char **argv)
{
	GError* err = NULL;
	gchar* script_path = NULL;
	gint reap = NA_FALLBACK_REAP_FREQ;

	script_path = g_build_path ("/", g_get_home_dir(), ".nall", NULL);
	global_script_list = na_register_scripts(script_path, NULL);

	if (!global_script_list) {
		g_message("no script to run: aborting");
		exit(EXIT_SUCCESS);
	}
	g_free(script_path);

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

	na_init_reaper(reap, NULL);
	global_tray_icon = tray_icon_new();
        gtk_main();
        exit(EXIT_SUCCESS);
}

