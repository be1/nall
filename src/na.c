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
#include "notify.h"
#include "script_list.h"

static gboolean na_spawn_script(gpointer script);

/* append a script output into the tooltip buffer */
static void na_script_append_out(run_data_t* s, gchar* tooltip_buffer)
{
	gchar* p = tooltip_buffer;
	guint rst;
	char name[32];

	while(*p)
		p++;
	rst = tooltip_buffer + BUFSIZ - p;

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

static void na_script_collect_status(run_data_t* s, gint* status_ptr)
{
	gint* status = status_ptr;
	if (*status == 0 && s->status != 0)
		*status = s->status;
}

static gboolean na_stop_blinking(gpointer data)
{
	nall_globals.stop_blink_tag = 0;
	gtk_status_icon_set_blinking(nall_globals.icon, FALSE);
	return FALSE;
}

static void na_start_blinking(void)
{
	gtk_status_icon_set_blinking(nall_globals.icon, TRUE);
	if (nall_globals.stop_blink_tag)
		g_source_remove(nall_globals.stop_blink_tag);
	nall_globals.stop_blink_tag = g_timeout_add_seconds(NA_BLINK_DURATION, na_stop_blinking, NULL);
}

/* reap each script output and refresh the tooltip buffer */
static void na_update_tooltip(void)
{
	gchar* tooltip_buffer = nall_globals.tooltip_buffer;
	gint status;

	status = 0;
	tooltip_buffer[0] = '\0';

	GtkTreeModel* tree = GTK_TREE_MODEL(nall_globals.script_list);
	GtkTreeIter iter;
	gboolean valid = gtk_tree_model_get_iter_first(tree, &iter);
	while (valid) {
		gboolean enabled;
		run_data_t* s;
		gtk_tree_model_get(tree, &iter,
			COLUMN_ENABLED, &enabled,
			COLUMN_RUN_DATA, &s,
			-1);
		if (s && enabled) {
			na_script_append_out(s, tooltip_buffer);
			na_script_collect_status(s, &status);
		}
		valid = gtk_tree_model_iter_next(tree, &iter);
	}

	int len = strlen(tooltip_buffer);
	if (len > 0 && tooltip_buffer[len - 1] == '\n')
		tooltip_buffer[len - 1]='\0';
	gtk_status_icon_set_tooltip(nall_globals.icon, tooltip_buffer);

	const gchar* icon = (status == 0) ? GTK_STOCK_INFO : GTK_STOCK_DIALOG_WARNING;
	gtk_status_icon_set_from_icon_name(nall_globals.icon, icon);
}

/* read child output on child termination event */
static void na_reap_child (GPid pid, gint status, gpointer script)
{
	run_data_t* s = script;
	gchar buf[sizeof(s->buf)];
	ssize_t nread;

	nread = read(s->out, buf, sizeof(s->buf)-1);
	buf[nread]='\0';

	close(s->in);
	close(s->out);
	close(s->err);
	g_spawn_close_pid(pid); /* or s->pid */

	s->running = FALSE;

	if (strcmp(buf, s->buf) || status != s->status) {
		/* program output has changed */
		strncpy(s->buf, buf, sizeof(buf));
		s->status = status;
		na_update_tooltip();

		if (s->blink_on == EVENT_ON_UPDATE ||
		   (s->blink_on == EVENT_ON_ERROR && s->status != 0))
			na_start_blinking();
		if (s->notify_on == EVENT_ON_UPDATE ||
		   (s->notify_on == EVENT_ON_ERROR && s->status != 0))
			if (!s->firstrun)
				nall_notify(s);
	}
	s->firstrun = FALSE;

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
	gboolean ret; /* spawn success */
	GError* error = NULL;

	run_data_t* s = script;
	s->tag = 0;

	if (!s || !s->cmd)
		return FALSE;

	gchar* argv[] = { "sh", "-c", (gchar*) s->cmd, NULL };

	/* FIXME: set any working directory ? ($HOME or /tmp) */
	ret = g_spawn_async_with_pipes
		(NULL, argv, NULL, G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD, NULL, NULL,
		&s->pid, &s->in, &s->out, &s->err, &error);
	if(ret == FALSE || error) {
		g_warning("na_spawn_script: %s\n", error->message);
		g_error_free(error);
	} else {
		/* set the child watcher */
		g_child_watch_add (s->pid, na_reap_child, s);
		s->running = TRUE;
	}

	return FALSE;
}

static run_data_t* create_run_data(GtkTreeModel* tree, GtkTreeIter* iter)
{
	run_data_t* s = g_malloc(sizeof(*s));
	memset(s, 0, sizeof(*s));
	gtk_tree_model_get(tree, iter,
		COLUMN_NAME, &s->name,
		COLUMN_COMMAND, &s->cmd,
		COLUMN_INTERVAL, &s->freq,
		COLUMN_BLINK_ON, &s->blink_on,
		COLUMN_NOTIFY_ON, &s->notify_on,
		-1);
	return s;
}

void na_schedule_script(GtkTreeModel* tree, GtkTreeIter* iter, int when)
{
	gboolean enabled;
	run_data_t* s;
	gtk_tree_model_get(tree, iter,
		COLUMN_ENABLED, &enabled,
		COLUMN_RUN_DATA, &s,
		-1);
	if (enabled) {
		if (s == NULL) {
			s = create_run_data(tree, iter);
			gtk_list_store_set(GTK_LIST_STORE(tree), iter, COLUMN_RUN_DATA, s, -1);
		}
		if (!s->running) {
			s->firstrun = TRUE;
			if (s->tag)
				g_source_remove(s->tag);
			s->tag = g_timeout_add_seconds(when, na_spawn_script, s);
		}
	}
}

void na_schedule_all(void)
{
	int i = 0;
	GtkTreeModel* tree = GTK_TREE_MODEL(nall_globals.script_list);
	GtkTreeIter iter;
	gboolean valid = gtk_tree_model_get_iter_first(tree, &iter);
	while (valid) {
		na_schedule_script(tree, &iter, i++);
		valid = gtk_tree_model_iter_next(tree, &iter);
	}
}

void na_cancel_script(GtkTreeModel* tree, GtkTreeIter* iter)
{
	run_data_t* s;
	gtk_tree_model_get(tree, iter, COLUMN_RUN_DATA, &s, -1);
	gtk_list_store_set(GTK_LIST_STORE(tree), iter, COLUMN_RUN_DATA, NULL, -1);

	if (s) {
		if (s->running) {
			close(s->in);
			close(s->out);
			close(s->err);
		}
		if (s->tag)
			g_source_remove(s->tag);
		g_free(s->name);
		g_free(s->cmd);
		g_free(s);
	}
}

void na_cancel_all(void)
{
	GtkTreeModel* tree = GTK_TREE_MODEL(nall_globals.script_list);
	GtkTreeIter iter;
	gboolean valid = gtk_tree_model_get_iter_first(tree, &iter);
	while (valid) {
		na_cancel_script(tree, &iter);
		valid = gtk_tree_model_iter_next(tree, &iter);
	}
}

/* quit nall */
void na_quit(void)
{
	na_cancel_all();
	gtk_main_quit();
}

