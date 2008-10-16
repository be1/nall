/* Copyright © 2008 by Benoît Rouits
 * Published under the terms of the GNU General Public License (GPL).
 */
#ifndef _NA_H
#define _NA_H
#include <gtk/gtk.h>

#define NA_FALLBACK_REAP_FREQ 3
#define NA_FALLBACK_SCRIPT_FREQ 5

struct _script {
	gchar* cmd;
	gchar* name;
	gint in;
	gint out;
	gint err;
	GPid pid;
	gchar buf[BUFSIZ];
	gchar old[BUFSIZ];
	GError* error;
};

typedef struct _script Script;

GList* na_register_scripts (gchar* path, gpointer null);

void na_unregister_scripts (GList* script_list, gpointer null);

gboolean na_spawn_script(gpointer script);

void na_popup_info(GtkStatusIcon* tray_icon, GList* script_list);

void na_popup_config(GtkStatusIcon* tray_icon, GList* script_list);

void na_script_refresh_tooltip (gpointer script);
#endif
