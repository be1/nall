#ifndef NALL_SCRIPT_LIST_H
#define NALL_SCRIPT_LIST_H

#include <gtk/gtk.h>

enum script_column {
	COLUMN_NAME,
	COLUMN_DESCRIPTION,
	COLUMN_COMMAND,
	COLUMN_INTERVAL,
	COLUMN_ENABLED,
	COLUMN_BLINK_ON,
	COLUMN_NOTIFY_ON,
	COLUMN_RUN_DATA,
};

enum script_event {
	EVENT_ON_UPDATE,
	EVENT_ON_ERROR,
	EVENT_NEVER,
	EVENT_COUNT,
};

GtkBuilder* nall_gtk_builder_new(void);

GtkListStore* script_list_load(void);

gboolean script_list_save(GtkListStore* script_list, GError** error);

#endif
