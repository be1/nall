#include "config.h"
#include "notify.h"

#ifdef HAVE_LIBNOTIFY

#include <libnotify/notify.h>

void nall_notify_init(void)
{
	notify_init("nall");
}

void nall_notify(Script* s)
{
	NotifyNotification* notification = notify_notification_new(s->name, s->buf, NULL, NULL);
	if (s->status == 0)
		notify_notification_set_urgency(notification, NOTIFY_URGENCY_NORMAL);
	else
		notify_notification_set_urgency(notification, NOTIFY_URGENCY_CRITICAL);
	notify_notification_show(notification, NULL);
	g_object_unref(G_OBJECT(notification));
}

#else

void nall_notify_init(void) {}
void nall_notify(Script* s) {}

#endif
