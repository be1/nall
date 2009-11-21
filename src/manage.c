#include <gtk/gtk.h>

#include "manage.h"

enum {
	RESPONSE_OK = 1,
	RESPONSE_CANCEL = 2,
};

enum {
	COLUMN_ENABLED,
	COLUMN_DESCRIPTION,
	COLUMN_INTERVAL,
};

static void on_response(GtkWidget *dialog, gint response, gpointer data)
{
	if (response == RESPONSE_OK) {
		printf("OK\n");
	} else {
		printf("Cancel or Close\n");
	}
	gtk_widget_destroy(dialog);
}

static GtkBuilder* builder_new(void)
{
	GtkBuilder* b = gtk_builder_new();

	/* developers shortcut: use local ui definition XML */
	if (gtk_builder_add_from_file(b, UI_FILENAME, NULL))
		return b;

	GError* error = NULL;
	if (gtk_builder_add_from_file(b, NALL_DATADIR "/" UI_FILENAME, &error))
		return b;

	g_critical("Could not open UI file: %s\n", error->message);
	g_error_free(error);
	g_object_unref(b);
	g_assert_not_reached();
	return NULL;
}

void manage_dialog_present(app_data_t* app_data)
{
	GtkBuilder* builder = builder_new();
	GtkDialog* dialog = GTK_DIALOG(gtk_builder_get_object(builder, "nall_manage_dialog"));
	GtkListStore* store = GTK_LIST_STORE(gtk_builder_get_object(builder, "list_store_scripts"));
	gtk_builder_connect_signals(builder, app_data);
	g_object_unref(builder);

	GtkTreeIter it;
	GList* script_list = app_data->script_list;

	while (script_list) {
		Script* s = script_list->data;
		gtk_list_store_append(store, &it);
		gtk_list_store_set(store, &it,
			COLUMN_ENABLED, TRUE,
			COLUMN_DESCRIPTION, s->name,
			COLUMN_INTERVAL, s->freq,
			-1);
		script_list = script_list->next;
	}

	g_signal_connect(dialog, "response", G_CALLBACK(on_response), app_data);
	gtk_window_present(GTK_WINDOW(dialog));
}

void manage_dialog_on_button_add(GtkButton *button, gpointer data)
{
	(void)button;
	(void)data;
	printf("Add\n");
}

void manage_dialog_on_button_edit(GtkButton *button, gpointer data)
{
	(void)button;
	(void)data;
	printf("Edit\n");
}

void manage_dialog_on_button_delete(GtkButton *button, gpointer data)
{
	(void)button;
	(void)data;
	printf("Delete\n");
}
