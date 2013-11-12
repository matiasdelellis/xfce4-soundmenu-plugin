/*
 *  Copyright (c) 2011-2013 matias <mati86dl@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libnotify/notify.h>
#include <glib/gi18n.h>

#include "soundmenu-notify.h"

#ifndef NOTIFY_CHECK_VERSION
#define NOTIFY_CHECK_VERSION(x,y,z) 0
#endif

void
soundmenu_notify_message (const gchar *message)
{
	NotifyNotification *notify = NULL;
	#if NOTIFY_CHECK_VERSION (0, 7, 0)
	notify = notify_notification_new(_("Sound menu Plugin"), message, "xfce4-soundmenu-plugin");
	#else
	notify = notify_notification_new(_("Sound menu Plugin"), message, "xfce4-soundmenu-plugin", NULL);
	#endif
	if (!notify_notification_show (notify, NULL))
		g_warning("Failed to send notification: %s", message);
}

void
soundmenu_notify_uninit (void)
{
	notify_uninit ();
}

void
soundmenu_notify_init (void)
{
	notify_init ("xfce4-soundmenu-plugin");
}
