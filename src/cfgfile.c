#include <string.h>

#include "cfgfile.h"

static Script* script_from_keyfile(GKeyFile* keyfile, const gchar* group)
{
	GError *error = NULL;
	Script script;
	memset(&script, 0, sizeof(script));

	script.freq = g_key_file_get_integer(keyfile, group, "interval", &error);
	if (error) {
		g_warning("No interval defined for %s", group);
		return NULL;
	}

	script.cmd = g_key_file_get_string(keyfile, group, "command", &error);
	if (error) {
		g_warning("No command defined for %s", group);
		return NULL;
	}

	Script* s = g_malloc(sizeof(script));
	memcpy(s, &script, sizeof(script));
	return s;
}

GList* nall_read_cfg(app_data_t* app_data)
{
	GList* script_list = NULL;
	GKeyFile* keyfile = g_key_file_new();
	GError *error = NULL;

	gchar* cfgfile = g_build_path ("/", g_get_home_dir(), ".nall/nall.ini", NULL);
	if (!g_key_file_load_from_file(keyfile, cfgfile, G_KEY_FILE_KEEP_COMMENTS, &error)) {
		g_warning("Failed to read %s: %s", cfgfile, error->message);
		g_free(cfgfile);
		return NULL;
	}
	g_free(cfgfile);

	gsize num;
	gchar** groups = g_key_file_get_groups(keyfile, &num);

	gsize i;
	for (i = 0; i < num; i++) {
		Script *s = script_from_keyfile(keyfile, groups[i]);
		if (s) {
			s->app_data = app_data;
			s->name = groups[i];
			script_list = g_list_append(script_list, s);
		} else {
			g_free(groups[i]);
		}
	}

	g_free(groups);
	g_key_file_free(keyfile);
	return script_list;
}
