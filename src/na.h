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

#include "script_list.h"

#define NA_FALLBACK_SCRIPT_FREQ 10
#define NA_BLINK_DURATION 3

/* Management data used when running a script */
typedef struct {
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
	enum script_event blink_on;	/* blink on which events? */
	enum script_event notify_on;	/* notify on which events? */
} run_data_t;

/* Global application data */
typedef struct {
	GtkStatusIcon* icon;
	GtkMenu* menu;
	GtkListStore* script_list;
	gchar tooltip_buffer[BUFSIZ];
	gchar* script_path;
	guint stop_blink_tag;
} nall_globals_t;

extern nall_globals_t nall_globals;

void na_update_tooltip(void);

void na_schedule_script(GtkTreeModel* tree, GtkTreeIter* iter, int when);

void na_schedule_all(void);

void na_cancel_script(GtkTreeModel* tree, GtkTreeIter* iter);

void na_cancel_all(void);

void na_quit(void);

#endif /* _NA_H */
