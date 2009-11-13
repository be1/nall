/* 
 * na.h Copyright © 2008-2009 by Benoît Rouits <brouits@free.fr>
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

#ifndef _NA_H
#define _NA_H
#include <gtk/gtk.h>

#define NA_FALLBACK_REFRESH_FREQ 1
#define NA_FALLBACK_SCRIPT_FREQ 10

struct _script {
	gchar* cmd;		/* full program path */
	gchar* name;		/* program name */
	gint freq;		/* program frequency (s) */
	gint in;		/* program stdin */
	gint out;		/* program stdout */
	gint err;		/* program stderr */
	gchar buf[BUFSIZ];	/* program relevant out */
	gint status;		/* program exit code */
	gboolean dbg;		/* debug (verbose) mode */
	GPid pid;		/* program Glib pid */
	guint tag;		/* program Glib source id */
	GError* error;		/* program start error */
};

typedef struct _script Script;

typedef struct {
    GtkStatusIcon *icon;
    GtkMenu *menu;
    GList *script_list;
    gchar tooltip_buffer[BUFSIZ];
    gchar *script_path;
} app_data_t;

GList* na_register_scripts (gchar* path);

void na_unregister_scripts (GList* script_list);

gboolean na_spawn_script(gpointer script); /* return TRUE if script exist */

gboolean na_schedule_script_once(gpointer script); /* always return FALSE */

gboolean na_schedule_script_freq(gpointer script); /* always return FALSE */

guint na_init_reaper(gint reap_freq, app_data_t* app_data);

gboolean na_reap(gpointer arg);

void na_quit(app_data_t *app_data);
#endif /* _NA_H */
