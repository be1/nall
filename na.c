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
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <glib.h>
#include <gtk/gtk.h>
#include "na.h"

/* global stuff (FIXME) */
extern GtkStatusIcon *global_tray_icon;
extern GList* global_script_list;
extern gchar global_tooltip_buffer[BUFSIZ];

/* check if path is a file */
static int is_file (const char* path)
{
	struct stat st;

	if(stat(path, &st) == -1)
		return 0;
	return S_ISREG(st.st_mode);
}

/* registering scripts from 'path' to launch into the main G loop */
GList* na_register_scripts (gchar* path, gpointer null)
{
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

	dir = g_dir_open(path,0,NULL);
	while (entry = g_dir_read_name(dir)) {
		/* FIXME: which order ? */
		/* store script path */
		script_path = g_build_path("/", path, entry, NULL);
		if (!is_file(script_path)) {
			/* FIXME: check if is_exec */
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
		/* store script freq (in seconds) */
		if (!isdigit(entry[0])) {
			script_freq = NA_FALLBACK_SCRIPT_FREQ;
			script->name = g_strdup(entry);
		} else {
			int i;
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
		g_timeout_add_seconds
			(script_freq, na_spawn_script, script);
		script_list = g_list_prepend(script_list, script);

	}
	g_dir_close(dir);
	return script_list;
}

/* spawn a registered script */
gboolean na_spawn_script(gpointer script)
{
	gchar* argv [2] = { NULL, NULL };
	gboolean ret;
	ssize_t nread;

	Script* s = (Script*)script;

	if (s->cmd) {
		argv[0] = (gchar*) s->cmd;
		argv[1] = NULL;
	}
	ret = g_spawn_async_with_pipes
		(NULL, argv, NULL, 0, NULL, NULL, 
		&s->pid, &s->in, &s->out, &s->err, &s->error);
	nread = read(s->out,s->buf,BUFSIZ);
	if (nread < BUFSIZ)
		s->buf[nread]='\0';
	if(ret == FALSE || s->error) {
		g_warning("na_spawn_script: %s\n", s->error->message);
		g_error_free(s->error);
	}
	close(s->in);
	close(s->out);
	close(s->err);
	g_spawn_close_pid(s->pid);
	return TRUE; /* we always want it re-scheduled */
}

/* purge a script object without to free it */
void na_script_purge(gpointer script, gpointer null)
{
	Script* s = (Script*)script;

	g_free(s->cmd);
	g_free(s->name);
	close(s->in);
	close(s->out);
	close(s->err);
	g_spawn_close_pid(s->pid);
	if(s->error)
		g_error_free(s->error);
	return;
}

/* unregister scripts and free the script_list */
void na_unregister_scripts (GList* script_list, gpointer null)
{
	g_list_foreach(script_list, na_script_purge, NULL);
	g_list_free(script_list);
	return;
}

/* append a script output into the tooltip buffer (uses a mutex) */
void na_script_append_out(gpointer script, gpointer null)
{
	Script* s = (Script*)script;
	gchar* p = global_tooltip_buffer;
	gint rst;
	static GStaticMutex tooltip_mutex = G_STATIC_MUTEX_INIT;

	g_static_mutex_lock (&tooltip_mutex);
	while(*p)
		p++;
	rst = global_tooltip_buffer + BUFSIZ - p;
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
	g_static_mutex_unlock (&tooltip_mutex);
	return;
}

/* reap each script output */
gboolean na_reap(gpointer script_list)
{
	GList* l = (GList*)script_list;
	global_tooltip_buffer[0] = '\0';
	g_list_foreach(l, na_script_append_out, NULL);
	global_tooltip_buffer[strlen(global_tooltip_buffer)-1]='\0';
        gtk_status_icon_set_tooltip(global_tray_icon, global_tooltip_buffer);
	return TRUE;
}

/* add the na_reap into the main G loop */
void na_init_reaper (gint reap_freq, gpointer null)
{
	g_timeout_add_seconds(reap_freq, na_reap, global_script_list);
	
}

/* refresh the tooltip buffer */
void na_refresh_tooltip(GtkStatusIcon* tray_icon, GList* script_list)
{
	na_reap(script_list);
}

/* popup the config menu (FIXME) */
void na_quit(GtkStatusIcon* tray_icon, GList* script_list)
{
	/* this does quit: */
	na_unregister_scripts(script_list, NULL);
	g_list_free(script_list);
	gtk_main_quit();
}

