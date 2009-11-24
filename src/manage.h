#ifndef NALL_MANAGE_H
#define NALL_MANAGE_H

#include <glib-object.h>
#include <gtk/gtk.h>

#include "na.h"

typedef struct _NallEditDialog NallEditDialog;

struct _NallEditDialog {
	GObject parent;

	GtkTreeView* tree_view;
	gboolean add_mode;

	GtkEntry* entry_name;
	GtkEntry* entry_description;
	GtkEntry* entry_command;
	GtkSpinButton* spin_interval;
	GtkComboBox* combo_unit;
	GtkComboBox* combo_blink_on;
	GtkComboBox* combo_notify_on;
};

void manage_dialog_present(void);

#endif
