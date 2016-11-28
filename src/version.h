#ifndef VERSION_H
#define VERSION_H

#include <libintl.h>
#define _(string) gettext(string)

#define NALL_AUTHOR0 "Benoît Rouits"
#define NALL_AUTHOR1 "Ingo van Lil"
#define NALL_COPYRIGHT "Copyright © 2008-2010 Benoît Rouits <brouits@free.fr>"
#define NALL_WEBSITE "http://brouits.free.fr/nall"
#define NALL_ICON GTK_STOCK_INFO
#define NALL_COMMENT _("a simple, non-intrusive, everything notifier in the system tray")

#define NALL_LICENSE "This program is free software; you can redistribute it and/or modify\nit under the terms of the GNU General Public License as published by\nthe Free Software Foundation; either version 2 of the License, or\n(at your option) any later version.\n\nThis program is distributed in the hope that it will be useful,\nbut WITHOUT ANY WARRANTY; without even the implied warranty of\nMERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\nGNU General Public License for more details.\n\nYou should have received a copy of the GNU General Public License along\nwith this program; if not, write to the Free Software Foundation, Inc.,\n51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA."

#endif /* VERSION_H */
