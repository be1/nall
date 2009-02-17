/* 
 * menu.c Copyright © 2008-2009 by Benoît Rouits <brouits@free.fr>
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

#include <glib.h>
#include <gtk/gtk.h>

GtkMenu* menu_new(void)
{
	GtkWidget* menu;

	menu = gtk_menu_new ();

	return GTK_MENU(menu);
}

/* append an item to the menu, and connect its callback on "activate" event */
void menu_append_item(GtkMenu* menu, gchar* label, GCallback callback, gpointer cb_data)
{
	GtkWidget* item;

	item = gtk_menu_item_new_with_label (label);
	gtk_menu_shell_append ((GtkMenuShell*) (menu), item);
	if (callback)
		g_signal_connect (G_OBJECT(item), "activate", G_CALLBACK(callback), cb_data);
	gtk_widget_show (item);

	return;
}

/* show the menu */
void menu_show(GtkMenu* menu, guint button, guint activate_time)
{
	gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL, button, activate_time);
	return;
}

/* hide the menu */
void menu_hide(GtkMenu* menu)
{
	gtk_menu_popdown(GTK_MENU(menu));
	return;
}
