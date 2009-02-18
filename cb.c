/* 
 * cb.c Copyright © 2008-2009 by Benoît Rouits <brouits@free.fr>
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

#include <gtk/gtk.h>
#include "na.h"
#include "menu.h"
#include "about.h"

/* handler for left-button click */
void tray_icon_on_click(GtkStatusIcon* instance, gpointer app_data)
{
	gtk_status_icon_set_blinking(instance, FALSE);
	if (app_data)
		na_reap(app_data);
}

/* handler for right-button click */
void tray_icon_on_menu(GtkStatusIcon* instance, guint button, guint activate_time, gpointer app_data)
{
	gpointer* d = (gpointer*) app_data;

	gtk_status_icon_set_blinking(instance, FALSE);
	if (app_data)
		menu_show(GTK_MENU(d[MENU]), button, activate_time);
}

/* handler for the "Quit" menu item */
void menu_item_on_quit(GtkMenuItem* instance, gpointer app_data)
{
	na_quit(app_data);
}

/* handler for the "Rescan" menu item */
void menu_item_on_rescan(GtkMenuItem* instance, gpointer app_data)
{
	gpointer* d = (gpointer*) app_data;
	gchar* script_path = (gchar*) d[PATH];
	GList* script_list = (GList*) d[LIST];
	
	na_unregister_scripts(script_list);
	script_list = na_register_scripts(script_path);
	d[LIST] = (gpointer) script_list;
}

/* handler for the "About" menu item */
void menu_item_on_about(GtkMenuItem* instance, gpointer app_data)
{
	GtkAboutDialog* about;
	const gchar* authors [] = {
		"Benoît Rouits",
		NULL
	};

	about = about_create ("Nall!",
	"0.1",
	"Copyright © 2008-2009 Benoît Rouits <brouits@free.fr>",
	"a simple, non-intrusive, everything notifier in the system tray",
	"This program is free software; you can redistribute it and/or modify\n"	"it under the terms of the GNU General Public License as published by\n"
	"the Free Software Foundation; either version 2 of the License, or\n"
	"(at your option) any later version.\n\n"
	"This program is distributed in the hope that it will be useful,\n"
	"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
	"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
	"GNU General Public License for more details.\n\n"
	"You should have received a copy of the GNU General Public License along\n"
	"with this program; if not, write to the Free Software Foundation, Inc.,\n"
	"51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.",
	"http://brouits.free.fr/nall/", authors, GTK_STOCK_INFO);

	about_show(about);
	return;
}

/* handler for the "Schedule" menu item (mutex) */
void menu_item_on_schedule(GtkMenuItem* instance, gpointer app_data)
{
	int i = 0;
	gpointer* d = (gpointer*) app_data;
	GList* script_list = (GList*) d[LIST];
	static GStaticMutex schedule_mutex = G_STATIC_MUTEX_INIT;

	/* how to shade the menu item ? */
	g_static_mutex_lock (&schedule_mutex);
	while (script_list) {
		g_timeout_add_seconds
			(i, na_schedule_script_once, script_list->data);
		/* na_schedule_script_once must always return FALSE */
		++i;
		script_list = script_list->next;
	}
	g_static_mutex_unlock (&schedule_mutex);
}

