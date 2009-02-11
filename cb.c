/* 
 * cb.c Copyright © 2008 by Benoît Rouits <brouits@free.fr>
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

#include <gtk/gtk.h>
#include "na.h"

/* handler for the "Quit" menu item */
void menu_item_on_quit(GtkMenuItem* instance, gpointer app_data)
{
	na_quit(app_data);
}

/* handler for left-button click */
void tray_icon_on_click(GtkStatusIcon* instance, gpointer app_data)
{
	if (app_data)
		na_reap(app_data);
}

/* handler for right-button click */
void tray_icon_on_menu(GtkStatusIcon* instance, guint button, guint activate_time, gpointer app_data)
{
	gpointer* d = (gpointer*) app_data;
	if (app_data)
		menu_show(GTK_MENU(d[MENU]), button, activate_time);
}

