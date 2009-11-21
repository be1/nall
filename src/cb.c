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
#include <config.h>
#include "na.h"
#include "manage.h"
#include "menu.h"
#include "about.h"
#include "cfgfile.h"
#include "version.h"

/* handler for left-button click */
void tray_icon_on_click(GtkStatusIcon* instance, gpointer app_data)
{
	gtk_status_icon_set_blinking(instance, FALSE);
}

/* handler for right-button click */
void tray_icon_on_menu(GtkStatusIcon* instance, guint button, guint activate_time, app_data_t* app_data)
{
	gtk_status_icon_set_blinking(instance, FALSE);
	if (app_data)
		menu_show(app_data->menu, button, activate_time);
}

/* handler for the "Quit" menu item */
void menu_item_on_quit(GtkMenuItem* instance, app_data_t* app_data)
{
	na_quit(app_data);
	instance = NULL; /* useless but does not warn at compile time */
}

/* handler for the "Reload Config" menu item */
void menu_item_on_reload(GtkMenuItem* instance, app_data_t* app_data)
{
	na_unregister_scripts(app_data->script_list);
	app_data->script_list = nall_read_cfg(app_data);
	if (app_data->script_list)
		na_schedule_all(app_data);
}

/* handler for the "Manage Scripts" menu item */
void menu_item_on_manage(GtkMenuItem* instance, gpointer app_data)
{
	(void)instance;
	manage_dialog_present(app_data);
}

/* handler for the "About" menu item (see version.h) */
void menu_item_on_about(GtkMenuItem* instance, gpointer unused)
{
	GtkAboutDialog* about;
	const gchar* authors [] = {
		NALL_AUTHOR0,
		NALL_AUTHOR1,
		NULL
	};

	about = about_create (PACKAGE_NAME, PACKAGE_VERSION, NALL_COPYRIGHT,
				NALL_COMMENT, NALL_LICENSE, NALL_WEBSITE,
				authors, GTK_STOCK_INFO);
	about_show(about);
	unused = NULL; /* avoid compiler warnings */
	instance = NULL; /* _ */
	return;
}

/* handler for the "Schedule" menu item */
void menu_item_on_schedule(GtkMenuItem* instance, app_data_t* app_data)
{
	na_schedule_all(app_data);
}

