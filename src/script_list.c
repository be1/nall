#include "script_list.h"

#define UI_FILENAME "nall-dialogs.ui"

static const char* event_str[] = {
	[EVENT_ON_UPDATE] = "update",
	[EVENT_ON_ERROR] = "error",
	[EVENT_NEVER] = "never",
};

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

	enum script_event blink_on;
	gchar* blink_on_str = g_key_file_get_string(keyfile, group, "blink_on", NULL);
	if (blink_on_str) {
		for (blink_on = EVENT_ON_UPDATE; blink_on < EVENT_COUNT; blink_on++) {
			if (!g_ascii_strcasecmp(blink_on_str, event_str[blink_on]))
				break;
		}
		if (blink_on == EVENT_COUNT) {
			g_warning("Invalid blink_on parameter ('%s') for %s\n", blink_on_str, group);
			blink_on = EVENT_ON_UPDATE;
		}
		g_free(blink_on_str);
	}

	enum script_event notify_on;
	gchar* notify_on_str = g_key_file_get_string(keyfile, group, "notify_on", NULL);
	if (notify_on_str) {
		for (notify_on = EVENT_ON_UPDATE; notify_on < EVENT_COUNT; notify_on++) {
			if (!g_ascii_strcasecmp(notify_on_str, event_str[notify_on]))
				break;
		}
		if (notify_on == EVENT_COUNT) {
			g_warning("Invalid notify_on parameter ('%s') for %s\n", notify_on_str, group);
			notify_on = EVENT_ON_ERROR;
		}
		g_free(notify_on_str);
	}

	GtkTreeIter it;
	gtk_list_store_append(store, &it);
	gtk_list_store_set(store, &it,
		COLUMN_NAME, group,
		COLUMN_DESCRIPTION, description,
		COLUMN_COMMAND, command,
		COLUMN_INTERVAL, interval,
		COLUMN_ENABLED, enabled,
		COLUMN_BLINK_ON, blink_on,
		COLUMN_NOTIFY_ON, notify_on,
		-1);

#ifdef EBUG
	printf("Script:\n");
	printf(" * Name: %s\n", group);
	printf(" * Description: %s\n", description);
	printf(" * Command: %s\n", command);
	printf(" * Interval: %d\n", interval);
	printf(" * Enabled: %s\n", enabled ? "yes" : "no");
	printf(" * Blink On: %s\n", event_str[blink_on]);
	printf(" * Notify On: %s\n", event_str[notify_on]);
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

gboolean script_list_save(GtkListStore* script_list, GError** error)
{
	gboolean ret = FALSE;
	GtkTreeModel* model = GTK_TREE_MODEL(script_list);
	GKeyFile* keyfile = g_key_file_new();

	GtkTreeIter iter;
	gboolean valid = gtk_tree_model_get_iter_first(model, &iter);
	while (valid) {
		gchar* name;
		gchar* description;
		gchar* command;
		gint interval;
		gboolean enabled;
		enum script_event blink_on;
		enum script_event notify_on;

		gtk_tree_model_get(model, &iter,
			COLUMN_NAME, &name,
			COLUMN_DESCRIPTION, &description,
			COLUMN_COMMAND, &command,
			COLUMN_INTERVAL, &interval,
			COLUMN_ENABLED, &enabled,
			COLUMN_BLINK_ON, &blink_on,
			COLUMN_NOTIFY_ON, &notify_on,
			-1);

		g_key_file_set_string(keyfile, name, "description", description);
		g_key_file_set_string(keyfile, name, "command", command);
		g_key_file_set_integer(keyfile, name, "interval", interval);
		g_key_file_set_boolean(keyfile, name, "enabled", enabled);
		g_assert(blink_on < EVENT_COUNT);
		g_key_file_set_string(keyfile, name, "blink_on", event_str[blink_on]);
		g_assert(notify_on < EVENT_COUNT);
		g_key_file_set_string(keyfile, name, "notify_on", event_str[notify_on]);

		valid = gtk_tree_model_iter_next(model, &iter);
	}

	gsize length;
	gchar* data = g_key_file_to_data(keyfile, &length, error);

	if (data) {
		gchar* nall_dir = g_build_path("/", g_get_home_dir(), ".nall", NULL);
		g_mkdir_with_parents(nall_dir, 0755);

		gchar* cfgfile = g_build_path("/", nall_dir, "nall.ini", NULL);
		ret = g_file_set_contents(cfgfile, data, length, error);

		g_free(cfgfile);
		g_free(nall_dir);
		g_free(data);
	}

	g_key_file_free(keyfile);
	return ret;
}
