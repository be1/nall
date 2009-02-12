/* 
 * na.h Copyright © 2008 by Benoît Rouits <brouits@free.fr>
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

#ifndef _NA_H
#define _NA_H
#include <gtk/gtk.h>

#define NA_FALLBACK_REAP_FREQ 3
#define NA_FALLBACK_SCRIPT_FREQ 5

struct _script {
	gchar* cmd;
	gchar* name;
	gint freq;
	gint in;
	gint out;
	gint err;
	GPid pid;
	gchar buf[BUFSIZ];
	gchar old[BUFSIZ];
	GError* error;
};

typedef struct _script Script;

enum {ICON, MENU, LIST, TIP, PATH};

GList* na_register_scripts (gchar* path);

void na_unregister_scripts (GList* script_list);

gboolean na_spawn_script(gpointer script);

void na_init_reaper(gint reap_freq, void** app_data);

gboolean na_reap(gpointer app_data);

void na_quit(gpointer app_data);
#endif /* _NA_H */
