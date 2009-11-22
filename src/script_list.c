#include "script_list.h"

#define UI_FILENAME "nall-dialogs.ui"

GtkBuilder* nall_gtk_builder_new(void)
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

static void script_from_keyfile(GKeyFile* keyfile, const gchar* group, GtkListStore* store)
{
	GError *error = NULL;

	gboolean enabled = g_key_file_get_boolean(keyfile, group, "enabled", &error);
	if (error) {
		g_error_free(error);
		error = NULL;
		enabled = FALSE;
	}

	gchar* description = g_key_file_get_string(keyfile, group, "description", NULL);
	if (!description)
		description = g_strdup(group);

	gchar* command = g_key_file_get_string(keyfile, group, "command", NULL);
	if (!command) {
		command = g_strdup("");
		enabled = FALSE;
	}

	gint interval = g_key_file_get_integer(keyfile, group, "interval", &error);
	if (error) {
		g_warning("No interval defined for %s", group);
		g_error_free(error);
		error = NULL;
		enabled = FALSE;
		interval = 60;
	}

	GtkTreeIter it;
	gtk_list_store_append(store, &it);
	gtk_list_store_set(store, &it,
		COLUMN_NAME, group,
		COLUMN_DESCRIPTION, description,
		COLUMN_COMMAND, command,
		COLUMN_INTERVAL, interval,
		COLUMN_ENABLED, enabled,
		-1);

#ifdef EBUG
	printf("Script:\n");
	printf(" * Name: %s\n", group);
	printf(" * Description: %s\n", description);
	printf(" * Command: %s\n", command);
	printf(" * Interval: %d\n", interval);
	printf(" * Enabled: %s\n", enabled ? "yes" : "no");
#endif

	g_free(description);
	g_free(command);
}

GtkListStore* script_list_load(void)
{
	GtkBuilder* builder = nall_gtk_builder_new();
	GtkListStore* store = GTK_LIST_STORE(gtk_builder_get_object(builder, "list_store_scripts"));
	g_object_unref(builder);

	GKeyFile* keyfile = g_key_file_new();
	GError *error = NULL;

	gchar* cfgfile = g_build_path ("/", g_get_home_dir(), ".nall/nall.ini", NULL);
	if (!g_key_file_load_from_file(keyfile, cfgfile, 0, &error)) {
		g_warning("Failed to read %s: %s", cfgfile, error->message);
		g_free(cfgfile);
		return store;
	}
	g_free(cfgfile);

	gsize num;
	gchar** groups = g_key_file_get_groups(keyfile, &num);

	gsize i;
	for (i = 0; i < num; i++) {
		script_from_keyfile(keyfile, groups[i], store);
		g_free(groups[i]);
	}

	g_free(groups);
	g_key_file_free(keyfile);

	return store;
}
