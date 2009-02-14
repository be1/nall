/* 
 * na.c Copyright © 2008 by Benoît Rouits <brouits@free.fr>
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
 * see the COPYING file included in the csv2latex package or
 * http://www.gnu.org/licenses/gpl.txt for the full licence
 * 
 */

#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#ifdef EBUG
#include <time.h>
#endif
#ifdef TRASHCODE
#include <sys/stat.h>
#include <sys/types.h>
#endif
#include <string.h>
#include <glib.h>
#include <gtk/gtk.h>
#include "na.h"

#ifdef TRASHCODE
/* check if path is a file */
static int is_file (const char* path)
{
	struct stat st;

	if(stat(path, &st) == -1)
		return 0;
	return S_ISREG(st.st_mode);
}
#endif

/* check if path is an executable file */
static int is_exe_file (const char* path)
{
	return !access(path,F_OK|X_OK);
}

/* compare order of 2 scripts based on their freq (for sorting) */
gint script_freq_cmp (gconstpointer a, gconstpointer b)
{
	Script* s1, *s2;
	if ((!a) || (!b))
		return 0;
	s1 = (Script*)a;
	s2 = (Script*)b;
	return (s1->freq > s2->freq);
}

/* schedule a script given its frequency (script->freq) */
gboolean na_schedule_script (gpointer script)
{
	Script* s = (Script*)script;

	g_timeout_add_seconds (s->freq, na_spawn_script, s);
	return FALSE; /* FALSE is required here */
}

/* registering scripts from 'path' to launch into the main G loop */
GList* na_register_scripts (gchar* path)
{
	GDir* dir = NULL;
	const gchar* entry;
	char* script_path = NULL;
	int script_freq = 0;
	int i = 0;
	char buf [BUFSIZ];
	Script* script = NULL;
	GList* script_list = NULL;
	
	if (access(path, R_OK|X_OK))
	{
		g_warning("cannot acces to %s\n", path);
		return NULL;
	}

	dir = g_dir_open(path,0,NULL);
	while ((entry = g_dir_read_name(dir))) {
		/* store script path */
		script_path = g_build_path("/", path, entry, NULL);
		if (!is_exe_file(script_path)) {
			g_free(script_path);
			continue;
		}
		/* build script environment */
		script = malloc (sizeof(Script));
		script->cmd = script_path;
		script->in = 0;
		script->out = 0;
		script->err = 0;
		script->pid = 0;
		script->buf[0] = '\0';
		script->error = NULL;
#ifdef EBUG
		script->dbg = TRUE;
#else
		script->dbg = FALSE;
#endif
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
		/* postpone scheduling at i seconds */
		g_timeout_add_seconds (i, na_schedule_script, script);
		script_list = g_list_prepend(script_list, script);
		++i;

	}
	g_dir_close(dir);
	script_list = g_list_sort(script_list, script_freq_cmp);
	return script_list;
}

/* spawn a registered script */
gboolean na_spawn_script(gpointer script)
{
	gchar* argv [2] = { NULL, NULL };
	gboolean ret;
	ssize_t nread;

	Script* s = (Script*)script;

	if (!s) /* disable the schedule */
		return FALSE;

	if (s->cmd) {
		argv[0] = (gchar*) s->cmd;
		argv[1] = NULL;
	} else
		return FALSE; /* disable schedule */

	ret = g_spawn_async_with_pipes
		(NULL, argv, NULL, 0, NULL, NULL, 
		&s->pid, &s->in, &s->out, &s->err, &s->error);

	/* read() blocks until s->out gives EOF (child hanged up) */
	nread = read(s->out, s->buf, BUFSIZ);
	if (nread < BUFSIZ)
		s->buf[nread]='\0';
	if(ret == FALSE || s->error) {
		g_warning("na_spawn_script: %s\n", s->error->message);
		g_error_free(s->error);
		s->error=NULL;
	}
	close(s->in);
	close(s->out);
	close(s->err);
	g_spawn_close_pid(s->pid);

	/* could also output to dbus from here... */
#ifdef EBUG
	if(s->dbg) {
		time_t t;
		gchar* info;

		t = time(NULL);
		info = g_path_get_basename(s->cmd);
		g_message("%s\t%s [%d] %s", ctime(&t), info, s->pid, s->buf);
		g_free(info);
	}
#endif
	return TRUE; /* we want it re-scheduled */
}

/* purge a script object without to free it */
void na_script_purge(gpointer script, gpointer unused)
{
	Script* s = (Script*)script;

	g_free(s->cmd);
	s->cmd=NULL;
	g_free(s->name);
	s->name=NULL;
	close(s->in);
	close(s->out);
	close(s->err);
	g_spawn_close_pid(s->pid);
	if (s->error) {
		g_error_free(s->error);
		s->error=NULL;
	}
	return;
}

/* unregister scripts and free the script_list */
void na_unregister_scripts (GList* script_list)
{
	g_list_foreach(script_list, na_script_purge, NULL);
	g_list_free(script_list);
	return;
}

/* append a script output into the tooltip buffer */
void na_script_append_out(gpointer script, gpointer tooltip_buffer)
{
	Script* s = (Script*)script;
	gchar* p = tooltip_buffer;
	gchar* b = tooltip_buffer;
	guint rst;

	while(*p)
		p++;
	rst = b + BUFSIZ - p;
	if (strlen(s->name) < rst)
		p = g_stpcpy(p, s->name);
	if (strlen(s->name) + strlen(": ") + strlen(s->buf) >= rst){
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

/* reap each script output and refresh the tooltip buffer (mutex) */
gboolean na_reap(gpointer app_data)
{
	gchar temp_buffer[BUFSIZ]; /* WARN: same size as tooltip_buffer */
	gpointer* d = (gpointer*)app_data;
	gchar* tooltip_buffer = (gchar*)d[TIP];
	GList* script_list = (GList*)d[LIST];
	GtkStatusIcon* tray_icon = GTK_STATUS_ICON(d[ICON]);
	static GStaticMutex reap_mutex = G_STATIC_MUTEX_INIT;

	g_static_mutex_lock (&reap_mutex);

	strncpy(temp_buffer, tooltip_buffer, BUFSIZ);
	tooltip_buffer[0] = '\0';
	g_list_foreach(script_list, na_script_append_out, tooltip_buffer);
	tooltip_buffer[strlen(tooltip_buffer)-1]='\0';
        gtk_status_icon_set_tooltip(tray_icon, tooltip_buffer);

	/* blink on message changes */
	if (strncmp(tooltip_buffer, temp_buffer, BUFSIZ))
			gtk_status_icon_set_blinking (tray_icon, TRUE);
	else	/* remove blink on second pass (REAP_FREQ) */
			gtk_status_icon_set_blinking (tray_icon, FALSE);

	g_static_mutex_unlock (&reap_mutex);
	return TRUE;
}

/* add the na_reap into the main G loop */
void na_init_reaper (gint reap_freq, void** app_data)
{
	g_timeout_add_seconds(reap_freq, na_reap, (gpointer)app_data);
	
}

/* quit nall */
void na_quit(gpointer app_data)
{
	gpointer* d = (gpointer*) app_data;
	GList* script_list = (GList*)d[LIST];

	na_unregister_scripts(script_list);
	g_list_free(script_list);
	gtk_main_quit();
}

