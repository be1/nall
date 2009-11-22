#ifndef NALL_SCRIPT_LIST_H
#define NALL_SCRIPT_LIST_H

#include <gtk/gtk.h>

#include "na.h"

enum script_column {
	COLUMN_NAME,
	COLUMN_DESCRIPTION,
	COLUMN_COMMAND,
	COLUMN_INTERVAL,
	COLUMN_ENABLED,
	COLUMN_RUN_DATA,
};

GtkBuilder* nall_gtk_builder_new(void);

GtkListStore* script_list_load(void);

#endif
