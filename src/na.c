/* 
 * na.c Copyright © 2008-2009 by Benoît Rouits <brouits@free.fr>
 * Published under the terms of the GNU General Public License v2 (GPLv2).
 * 
 ***************************************************** 
 * nall: a non-intrusive desktop all-purpose notifer *
 *****************************************************
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, 
 * Boston, MA  02110-1301, USA.
 * 
 * see the COPYING file included in the nall package or
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt for the full licence
 * 
 */

#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#ifdef EBUG
#include <time.h>
#endif
#include <string.h>
#include <sys/wait.h>
#include <glib.h>
#include <gtk/gtk.h>
#include "na.h"

static gboolean na_spawn_script(gpointer script);

/* check if path is an executable file */
static int is_exe_file (const char* path)
{
	return !access(path,F_OK|X_OK);
}

/* compare order of 2 scripts based on their freq (for sorting) */
static gint script_freq_cmp (gconstpointer a, gconstpointer b)
{
	Script* s1, *s2;
	if ((!a) || (!b))
		return 0;
	s1 = (Script*)a;
	s2 = (Script*)b;
	return (s1->freq > s2->freq);
}

/* registering scripts from 'path' to launch into the main G loop */
GList* na_register_scripts (app_data_t* app_data)
{
	const gchar* path = app_data->script_path;
	GDir* dir = NULL;
	const gchar* entry;
	char* script_path = NULL;
	int script_freq = 0;
	char buf [BUFSIZ];
	Script* script = NULL;
	GList* script_list = NULL;
	
	if (access(path, R_OK|X_OK))
	{
		g_warning("cannot acces to %s\n", path);
		return NULL;
	}

	dir = g_dir_open(path, 0, NULL);
	while ((entry = g_dir_read_name(dir))) {
		/* store script path */
		script_path = g_build_path("/", path, entry, NULL);
		if (!is_exe_file(script_path)) {
			g_free(script_path);
			continue;
		}
		/* build script environment */
		script = calloc(1, sizeof(Script));
		script->app_data = app_data;
		script->cmd = script_path;
		/* store script freq (in seconds) */
		if (!isdigit(entry[0])) {
			script_freq = NA_FALLBACK_SCRIPT_FREQ;
			script->name = g_strdup(entry);
		} else { /* scan the frequency */
			unsigned int i;
			for (i=0; i<strlen(entry); i++) {
				if(!isdigit(entry[i])) {
					buf[i]='\0';
					break;
				} else {
					buf[i] = entry[i];
				}
			}
			script_freq = atoi(buf);
			script->name = g_strdup(entry+i);
		}
		/* register script */
		script->freq = script_freq;
		script_list = g_list_prepend(script_list, script);
	}
	g_dir_close(dir);
	script_list = g_list_sort(script_list, script_freq_cmp);
	return script_list;
}

/*  remove script periodic spawn, and free data members */
static void na_script_purge(gpointer script, gpointer unused)
{
	Script* s = (Script*)script;

	unused=NULL;
	if (s->running) {
		close(s->in);
		close(s->out);
		close(s->err);
	}
	if (s->tag)
		g_source_remove(s->tag);
	g_free(s->cmd);
	g_free(s->name);
	g_spawn_close_pid(s->pid);
	if (s->error)
		g_error_free(s->error);
	free(s);
}

/* unregister scripts and free the script_list */
void na_unregister_scripts (GList* script_list)
{
	g_list_foreach(script_list, na_script_purge, NULL);
	g_list_free(script_list);
}

/* append a script output into the tooltip buffer */
static void na_script_append_out(gpointer script, gpointer tooltip_buffer)
{
	Script* s = (Script*)script;
	gchar* p = tooltip_buffer;
	gchar* b = tooltip_buffer;
	guint rst;
	char name[32];

	while(*p)
		p++;
	rst = b + BUFSIZ - p;

	if (WIFSIGNALED(s->status))
		snprintf(name, sizeof(name), "%s(sig%d)", s->name, WTERMSIG(s->status));
	else if (WIFEXITED(s->status) && WEXITSTATUS(s->status) != 0)
		snprintf(name, sizeof(name), "%s(%d)", s->name, WEXITSTATUS(s->status));
	else
		snprintf(name, sizeof(name), "%s", s->name);

	if (strlen(name) < rst)
		p = g_stpcpy(p, name);
	if (strlen(name) + strlen(": ") + strlen(s->buf) >= rst){
		p = g_stpcpy(p, "too long!");
	} else {
		p = g_stpcpy(p, ": ");
		p = g_stpcpy(p, s->buf);
		if(p[-1] == '\n') {
			p[-1] = '\0';
			p--;
		}
	}
	p = g_stpcpy(p, "\n");
	return;
}

static void na_script_collect_status(gpointer script, gpointer status_ptr)
{
	Script* s = (Script*)script;
	gint* status = status_ptr;
	if (*status == 0 && s->status != 0)
		*status = s->status;
}

static gboolean na_stop_blinking(gpointer _app_data)
{
	app_data_t* app_data = _app_data;
	app_data->stop_blink_tag = 0;
	gtk_status_icon_set_blinking(app_data->icon, FALSE);
	return FALSE;
}

/* reap each script output and refresh the tooltip buffer */
static void na_update_tooltip(app_data_t* app_data)
{
	gchar temp_buffer[sizeof(app_data->tooltip_buffer)];
	gchar* tooltip_buffer = app_data->tooltip_buffer;
	gint status;

	strncpy(temp_buffer, tooltip_buffer, sizeof(temp_buffer));
	tooltip_buffer[0] = '\0';
	g_list_foreach(app_data->script_list, na_script_append_out, tooltip_buffer);
	tooltip_buffer[strlen(tooltip_buffer)-1]='\0';
	gtk_status_icon_set_tooltip(app_data->icon, tooltip_buffer);

	status = 0;
	g_list_foreach(app_data->script_list, na_script_collect_status, &status);
	const gchar* icon = (status == 0) ? GTK_STOCK_INFO : GTK_STOCK_DIALOG_WARNING;
	gtk_status_icon_set_from_icon_name(app_data->icon, icon);

	/* blink on message changes */
	if (strncmp(tooltip_buffer, temp_buffer, sizeof(temp_buffer))) {
		gtk_status_icon_set_blinking (app_data->icon, TRUE);
		if (app_data->stop_blink_tag)
			g_source_remove(app_data->stop_blink_tag);
		app_data->stop_blink_tag = g_timeout_add_seconds(NA_BLINK_DURATION, na_stop_blinking, app_data);
	}
}

/* read child output on child termination event */
static void na_reap_child (GPid pid, gint status, gpointer script)
{
	Script* s = (Script*)script;
	ssize_t nread;

	nread = read(s->out, s->buf, BUFSIZ);
	if (nread < BUFSIZ)
		s->buf[nread]='\0';
	else s->buf[BUFSIZ-1]='\0';

	close(s->in);
	close(s->out);
	close(s->err);
	g_spawn_close_pid(pid); /* or s->pid */

	s->running = FALSE;
	s->status = status;

	na_update_tooltip(s->app_data);

	/* re-schedule */
	s->tag = g_timeout_add_seconds(s->freq, na_spawn_script, s);

	/* could also output to dbus from here... */

#ifdef EBUG
	time_t t;
	gchar* name;

	t = time(NULL);
	name = g_path_get_basename(s->cmd);
	g_message("%s[%d]\t%s (%d) %s", ctime(&t), status, name, pid, s->buf);
	g_free(name);
#endif
}

/* spawn a registered script */
static gboolean na_spawn_script(gpointer script)
{
	gchar* argv[2];
	gboolean ret; /* spawn success */

	Script* s = (Script*)script;
	s->tag = 0;

	if (!s || !s->cmd)
		return FALSE;

	argv[0] = (gchar*) s->cmd;
	argv[1] = NULL;

	/* FIXME: set any working directory ? ($HOME or /tmp) */
	ret = g_spawn_async_with_pipes
		(NULL, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD, NULL, NULL, 
		&s->pid, &s->in, &s->out, &s->err, &s->error);
	if(ret == FALSE || s->error) {
		g_warning("na_spawn_script: %s\n", s->error->message);
		g_error_free(s->error);
		s->error=NULL;
	} else {
		/* set the child watcher */
		g_child_watch_add (s->pid, na_reap_child, s);
		s->running = TRUE;
	}

	return FALSE;
}

void na_schedule_all(app_data_t* app_data)
{
	int i = 0;
	GList* script_list = app_data->script_list;

	while (script_list) {
		Script* s = script_list->data;
		if (!s->running) {
			if (s->tag)
				g_source_remove(s->tag);
			s->tag = g_timeout_add_seconds(i++, na_spawn_script, s);
			script_list = script_list->next;
		}
	}
}

/* quit nall */
void na_quit(app_data_t* app_data)
{
	na_unregister_scripts(app_data->script_list);
	g_list_free(app_data->script_list);
	gtk_main_quit();
}

