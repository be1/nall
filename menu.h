/* 
 * menu.h Copyright © 2008 by Benoît Rouits <brouits@free.fr>
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

#ifndef _MENU_H
#define _MENU_H
#include <glib.h>
#include <gtk/gtk.h>

/* create a new (empty) menu */
GtkMenu* menu_new(void);

/* append an item to the menu, and connect its callback on "activate" event */
void menu_append_item(GtkMenu* menu, gchar* label, GCallback callback, gpointer cb_data);

/* show the menu */
void menu_show(GtkMenu* menu, guint button, guint activate_time);

/* hide the menu */
void menu_hide(GtkMenu* menu);
#endif /* _MENU_H */
