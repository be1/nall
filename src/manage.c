#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include "manage.h"
#include "script_list.h"

enum {
	RESPONSE_OK = 1,
	RESPONSE_CANCEL = 2,
};

enum {
	UNIT_SECONDS,
	UNIT_MINUTES,
	UNIT_HOURS,
	UNIT_DAYS,
};

static const struct {
	int multiplier;
	const gchar* singular;
	const gchar* plural;
} unit_desc[] = {
	[UNIT_SECONDS] = {            1, N_("second"), N_("seconds") },
	[UNIT_MINUTES] = {           60, N_("minute"), N_("minutes") },
	[UNIT_HOURS]   = {      60 * 60, N_(  "hour"), N_(  "hours") },
	[UNIT_DAYS]    = { 24 * 60 * 60, N_(   "day"), N_(   "days") },
};

static GtkDialog* manage_dialog = NULL;

static NallEditDialog edit_dialog; // FIXME: Hack! Define real glib object!

static void edit_dialog_on_response(GtkWidget *dialog, gint response, gpointer data)
{
	if (response == RESPONSE_OK) {
		const gchar* name = gtk_entry_get_text(edit_dialog.entry_name);
		const gchar* description = gtk_entry_get_text(edit_dialog.entry_description);
		const gchar* command = gtk_entry_get_text(edit_dialog.entry_command);
		gint interval = gtk_spin_button_get_value_as_int(edit_dialog.spin_interval);
		gint unit = gtk_combo_box_get_active(edit_dialog.combo_unit);
		enum script_event blink_on = gtk_combo_box_get_active(edit_dialog.combo_blink_on);
		enum script_event notify_on = gtk_combo_box_get_active(edit_dialog.combo_notify_on);

#ifdef EBUG
		printf("New Properties:\n");
		printf(" * Name: %s\n", name);
		printf(" * Description: %s\n", description);
		printf(" * Command: %s\n", command);
		printf(" * Interval: %d %s\n", interval, interval == 1 ? unit_desc[unit].singular : unit_desc[unit].plural);
		printf(" * Blink On: %d\n", blink_on);
		printf(" * Notify On: %d\n", notify_on);
#endif

		GtkTreeModel* model = gtk_tree_view_get_model(edit_dialog.tree_view);
		GtkListStore* store = GTK_LIST_STORE(model);
		GtkTreeSelection* selection = gtk_tree_view_get_selection(edit_dialog.tree_view);
		GtkTreeIter iter;
		if (gtk_tree_selection_get_selected(selection, NULL, &iter)) {
			if (edit_dialog.add_mode)
				gtk_list_store_insert_after(store, &iter, &iter);
		} else {
			gtk_list_store_append(store, &iter);
		}

		if (edit_dialog.add_mode)
			gtk_list_store_set(store, &iter, COLUMN_ENABLED, TRUE, -1);

		gtk_list_store_set(store, &iter,
			COLUMN_NAME, name,
			COLUMN_DESCRIPTION, description,
			COLUMN_COMMAND, command,
			COLUMN_INTERVAL, interval * unit_desc[unit].multiplier,
			COLUMN_BLINK_ON, blink_on,
			COLUMN_NOTIFY_ON, notify_on,
			-1);

		na_cancel_script(model, &iter);
		na_schedule_script(model, &iter, 1);
	}
	gtk_widget_destroy(dialog);
}

static void edit_dialog_open(GtkTreeView* tree_view, gboolean add_mode)
{
	memset(&edit_dialog, 0, sizeof(edit_dialog));
	edit_dialog.tree_view = tree_view;
	edit_dialog.add_mode = add_mode;

	GtkBuilder* builder = nall_gtk_builder_new();
	GtkDialog* dialog = GTK_DIALOG(gtk_builder_get_object(builder, "nall_edit_dialog"));
	edit_dialog.entry_name = GTK_ENTRY(gtk_builder_get_object(builder, "edit_entry_name"));
	edit_dialog.entry_description = GTK_ENTRY(gtk_builder_get_object(builder, "edit_entry_description"));
	edit_dialog.entry_command = GTK_ENTRY(gtk_builder_get_object(builder, "edit_entry_command"));
	edit_dialog.spin_interval = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "edit_spin_interval"));
	edit_dialog.combo_unit = GTK_COMBO_BOX(gtk_builder_get_object(builder, "edit_combo_interval_unit"));
	edit_dialog.combo_blink_on = GTK_COMBO_BOX(gtk_builder_get_object(builder, "edit_combo_blink_on"));
	edit_dialog.combo_notify_on = GTK_COMBO_BOX(gtk_builder_get_object(builder, "edit_combo_notify_on"));
	gtk_builder_connect_signals(builder, NULL);
	g_object_unref(builder);

	if (add_mode) {
		gtk_window_set_title(GTK_WINDOW(dialog), "Add New Script");
		gtk_spin_button_set_value(edit_dialog.spin_interval, 30);
	} else {
		gtk_window_set_title(GTK_WINDOW(dialog), "Script Properties");

		gchar* name = NULL;
		gchar* description = NULL;
		gchar* command = NULL;
		gint interval;
		enum script_event blink_on;
		enum script_event notify_on;

		GtkTreeModel* model = gtk_tree_view_get_model(tree_view);
		GtkTreeSelection* selection = gtk_tree_view_get_selection(tree_view);
		GtkTreeIter iter;
		gtk_tree_selection_get_selected(selection, NULL, &iter);

		gtk_tree_model_get(model, &iter,
			COLUMN_NAME, &name,
			COLUMN_DESCRIPTION, &description,
			COLUMN_COMMAND, &command,
			COLUMN_INTERVAL, &interval,
			COLUMN_BLINK_ON, &blink_on,
			COLUMN_NOTIFY_ON, &notify_on,
			-1);

		gtk_entry_set_text(edit_dialog.entry_name, name);
		g_free(name);

		gtk_entry_set_text(edit_dialog.entry_description, description);
		g_free(description);

		gtk_entry_set_text(edit_dialog.entry_command, command);
		g_free(command);

		int unit = UNIT_DAYS;
		for (unit = UNIT_DAYS; unit >= UNIT_SECONDS; unit--) {
			int mul = unit_desc[unit].multiplier;
			if (interval % mul == 0) {
				gtk_spin_button_set_value(edit_dialog.spin_interval, interval / mul);
				gtk_combo_box_set_active(edit_dialog.combo_unit, unit);
				break;
			}
		}

		gtk_combo_box_set_active(edit_dialog.combo_blink_on, blink_on);
		gtk_combo_box_set_active(edit_dialog.combo_notify_on, notify_on);
	}

	g_signal_connect(dialog, "response", G_CALLBACK(edit_dialog_on_response), NULL);
	gtk_widget_show(GTK_WIDGET(dialog));
}

static void manage_dialog_on_button_add(GtkButton *button, gpointer tree_view_ptr)
{
	GtkTreeView* tree_view = GTK_TREE_VIEW(tree_view_ptr);
	edit_dialog_open(tree_view, TRUE);
}

static void manage_dialog_on_button_edit(GtkButton *button, gpointer tree_view_ptr)
{
	GtkTreeView* tree_view = GTK_TREE_VIEW(tree_view_ptr);
	edit_dialog_open(tree_view, FALSE);
}

static void manage_dialog_on_button_delete(GtkButton *button, gpointer tree_view_ptr)
{
	GtkTreeView* tree_view = GTK_TREE_VIEW(tree_view_ptr);
	GtkListStore* model = GTK_LIST_STORE(gtk_tree_view_get_model(tree_view));
	GtkTreeSelection* selection = gtk_tree_view_get_selection(tree_view);
	GtkTreeIter iter;
	if (gtk_tree_selection_get_selected(selection, NULL, &iter)) {
		na_cancel_script(GTK_TREE_MODEL(model), &iter);
		gtk_list_store_remove(model, &iter);
	}
}

static void manage_dialog_on_response(GtkWidget *dialog, gint response, gpointer data)
{
	gtk_widget_destroy(dialog);
	manage_dialog = NULL;
	// TODO: Error checking
	script_list_save(nall_globals.script_list, NULL);
}

static void manage_dialog_on_enabled_toggled(GtkCellRendererToggle* cell, gchar* path, gpointer data)
{
	GtkTreePath* tree_path = gtk_tree_path_new_from_string(path);
	GtkTreeView* tree_view = GTK_TREE_VIEW(data);
	GtkTreeModel* model = gtk_tree_view_get_model(tree_view);

	GtkTreeIter iter;
	gtk_tree_model_get_iter(model, &iter, tree_path);

	gboolean enabled;
	gtk_tree_model_get(model, &iter, COLUMN_ENABLED, &enabled, -1);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, COLUMN_ENABLED, !enabled, -1);
	if (enabled)
		na_cancel_script(model, &iter);
	else
		na_schedule_script(model, &iter, 1);

	gtk_tree_path_free(tree_path);
}

void manage_dialog_on_selection_changed(GtkTreeSelection* selection, gpointer editbutton)
{
	gboolean sensitive = (gtk_tree_selection_count_selected_rows(selection) > 0);
	gtk_widget_set_sensitive(GTK_WIDGET(editbutton), sensitive);
}

void manage_dialog_on_row_activated(GtkTreeView* tree_view, GtkTreePath* path, GtkTreeViewColumn* column, gpointer data)
{
	edit_dialog_open(tree_view, FALSE);
}

static void interval_cell_data_func(GtkTreeViewColumn* col, GtkCellRenderer* renderer, GtkTreeModel* model, GtkTreeIter* iter, gpointer data)
{
	gint interval;
	gtk_tree_model_get(model, iter, COLUMN_INTERVAL, &interval, -1);

	int unit = UNIT_DAYS;
	for (unit = UNIT_DAYS; unit >= UNIT_SECONDS; unit--) {
		int mul = unit_desc[unit].multiplier;
		if (interval % mul == 0) {
			gint val = interval / mul;
			const gchar* u = (val == 1) ? unit_desc[unit].singular : unit_desc[unit].plural;
			gchar buf[30];
			g_snprintf(buf, sizeof(buf), "%d %s", val, _(u));
			g_object_set(renderer, "text", buf, NULL);
			break;
		}
	}
}

static GtkDialog* manage_dialog_create(void)
{
	GtkBuilder* builder = nall_gtk_builder_new();
	GtkDialog* dialog = GTK_DIALOG(gtk_builder_get_object(builder, "nall_manage_dialog"));
	GtkTreeView* tree_view = GTK_TREE_VIEW(gtk_builder_get_object(builder, "manage_treeview"));
	GtkTreeViewColumn* column_interval = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "treeview_column_interval"));
	GtkCellRenderer* cell_enabled = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "cellrenderertoggle_enabled"));
	GtkCellRenderer* cell_interval = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "cellrenderertext_interval"));
	GtkButton* button_add = GTK_BUTTON(gtk_builder_get_object(builder, "manage_button_add"));
	GtkButton* button_edit = GTK_BUTTON(gtk_builder_get_object(builder, "manage_button_edit"));
	GtkButton* button_delete = GTK_BUTTON(gtk_builder_get_object(builder, "manage_button_delete"));
	gtk_builder_connect_signals(builder, NULL);
	g_object_unref(builder);

	g_signal_connect(cell_enabled, "toggled", G_CALLBACK(manage_dialog_on_enabled_toggled), tree_view);

	gtk_tree_view_set_model(tree_view, GTK_TREE_MODEL(nall_globals.script_list));
	gtk_tree_view_column_set_cell_data_func(column_interval, cell_interval, interval_cell_data_func, NULL, NULL);
	GtkTreeSelection* selection = gtk_tree_view_get_selection(tree_view);
	gboolean sensitive = (gtk_tree_selection_count_selected_rows(selection) > 0);
	gtk_widget_set_sensitive(GTK_WIDGET(button_edit), sensitive);
	g_signal_connect(selection, "changed", G_CALLBACK(manage_dialog_on_selection_changed), button_edit);

	g_signal_connect(button_add, "clicked", G_CALLBACK(manage_dialog_on_button_add), tree_view);
	g_signal_connect(button_edit, "clicked", G_CALLBACK(manage_dialog_on_button_edit), tree_view);
	g_signal_connect(button_delete, "clicked", G_CALLBACK(manage_dialog_on_button_delete), tree_view);

	g_signal_connect(dialog, "response", G_CALLBACK(manage_dialog_on_response), NULL);
	return dialog;
}

void manage_dialog_present(void)
{
	if (!manage_dialog)
		manage_dialog = manage_dialog_create();
	gtk_window_present(GTK_WINDOW(manage_dialog));
}
