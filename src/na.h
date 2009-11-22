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

#define NA_FALLBACK_SCRIPT_FREQ 10
#define NA_BLINK_DURATION 3

typedef struct app_data app_data_t;

/* Management data used when running a script */
typedef struct {
	app_data_t* app_data;	/* Application global data */
	gchar* cmd;		/* full program path */
	gchar* name;		/* program name */
	gint freq;		/* program frequency (s) */
	gint in;		/* program stdin */
	gint out;		/* program stdout */
	gint err;		/* program stderr */
	gchar buf[BUFSIZ];	/* program relevant out */
	gint status;		/* program exit code */
	GPid pid;		/* program Glib pid */
	guint tag;		/* program Glib source id */
	gboolean firstrun;	/* true till the first update */
	gboolean running;	/* program currently running */
} run_data_t;

struct app_data {
	GtkStatusIcon* icon;
	GtkMenu* menu;
	GtkListStore* script_list;
	gchar tooltip_buffer[BUFSIZ];
	gchar* script_path;
	guint stop_blink_tag;
};

void na_schedule_script(GtkTreeModel* tree, GtkTreeIter* iter, int when, app_data_t* app_data);

void na_schedule_all(app_data_t* app_data);

void na_cancel_script(GtkTreeModel* tree, GtkTreeIter* iter);

void na_cancel_all(app_data_t* app_data);

void na_quit (app_data_t* app_data);

#endif /* _NA_H */
