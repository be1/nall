#ifndef NALL_NOTIFY_H
#define NALL_NOTIFY_H

#include <glib.h>

void nall_notify_init(void);
void nall_notify(const gchar *program, const gchar *output, int status);

#endif
