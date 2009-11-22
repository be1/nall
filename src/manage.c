#include <gtk/gtk.h>

#include "manage.h"
#include "script_list.h"

enum {
	RESPONSE_OK = 1,
	RESPONSE_CANCEL = 2,
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

void manage_dialog_present(app_data_t* app_data)
{
	GtkBuilder* builder = nall_gtk_builder_new();
	GtkDialog* dialog = GTK_DIALOG(gtk_builder_get_object(builder, "nall_manage_dialog"));
	GtkTreeView* tree_view = GTK_TREE_VIEW(gtk_builder_get_object(builder, "manage_treeview"));
	gtk_builder_connect_signals(builder, app_data);
	g_object_unref(builder);

	gtk_tree_view_set_model(tree_view, GTK_TREE_MODEL(app_data->script_list));

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
