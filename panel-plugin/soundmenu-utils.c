/*  $Id$
 *
 *  Copyright (c) 2011 John Doo <john@foo.org>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "soundmenu-plugin.h"
#include "soundmenu-dialogs.h"
#include "soundmenu-lastfm.h"
#include "soundmenu-mpris2.h"
#include "soundmenu-utils.h"
#include "soundmenu-related.h"

/* Functions to check the network manager status. */

static NMState
dbus_check_nm_status (DBusConnection *connection)
{
	DBusMessage *message, *reply;
	DBusError error;
	dbus_uint32_t state;
	
	message = dbus_message_new_method_call (NM_DBUS_SERVICE, NM_DBUS_PATH,
						NM_DBUS_INTERFACE, "state");
	if (!message)
		return NM_STATE_UNKNOWN;

	dbus_error_init (&error);
	reply = dbus_connection_send_with_reply_and_block (connection, message,
							   -1, &error);
	dbus_message_unref (message);
	if (!reply)
		return NM_STATE_UNKNOWN;

	if (!dbus_message_get_args (reply, NULL, DBUS_TYPE_UINT32, &state,
				    DBUS_TYPE_INVALID))
		return NM_STATE_UNKNOWN;

	return state;
}

gboolean
nm_is_online ()
{
	DBusConnection *connection;
	DBusError error;
	NMState state;

	dbus_error_init (&error);
	connection = dbus_bus_get (DBUS_BUS_SYSTEM, &error);
	if (connection == NULL) {
		g_critical("Error connecting to DBUS_BUS_SYSTEM to get nm status: %s", error.message);
		dbus_error_free (&error);
		return FALSE;
	}

	state = dbus_check_nm_status (connection);

	dbus_connection_unref(connection);

	if (state == NM_STATE_CONNECTED_LOCAL ||
	    state == NM_STATE_CONNECTED_SITE ||
	    state == NM_STATE_CONNECTED_GLOBAL)
		return TRUE;

	return FALSE;
}

/* Set and remove the watch cursor to suggest background work.*/

void
set_watch_cursor_on_thread(SoundmenuPlugin *soundmenu)
{
	GdkCursor *cursor;

	gdk_threads_enter ();
	cursor = gdk_cursor_new(GDK_WATCH);
	gdk_window_set_cursor(GDK_WINDOW(gtk_widget_get_toplevel (GTK_WIDGET (soundmenu->plugin))->window), cursor);
	gdk_cursor_unref(cursor);
	gdk_threads_leave ();
}

void
remove_watch_cursor_on_thread(gchar *message, SoundmenuPlugin *soundmenu)
{
	gdk_threads_enter ();
	gdk_window_set_cursor(GDK_WINDOW(gtk_widget_get_toplevel (GTK_WIDGET (soundmenu->plugin))->window), NULL);
	#ifdef HAVE_LIBNOTIFY
	NotifyNotification *notify = NULL;
	if(message != NULL) {
		notify = notify_notification_new(_("Sound menu Plugin"), message, "xfce4-soundmenu-plugin");
		if (!notify_notification_show (notify, NULL))
			g_warning("Failed to send notification: %s", message);
	}
	#endif
	gdk_threads_leave ();
}

gchar* convert_length_str(gint length)
{
	static gchar *str, tmp[24];
	gint days = 0, hours = 0, minutes = 0, seconds = 0;

	str = g_new0(char, 128);
	memset(tmp, '\0', 24);

	if (length > 86400) {
		days = length/86400;
		length = length%86400;
		g_sprintf(tmp, "%d %s, ", days, (days>1)?_("days"):_("day"));
		g_strlcat(str, tmp, 24);
	}

	if (length > 3600) {
		hours = length/3600;
		length = length%3600;
		memset(tmp, '\0', 24);
		g_sprintf(tmp, "%d:", hours);
		g_strlcat(str, tmp, 24);
	}

	if (length > 60) {
		minutes = length/60;
		length = length%60;
		memset(tmp, '\0', 24);
		g_sprintf(tmp, "%02d:", minutes);
		g_strlcat(str, tmp, 24);
	}
	else
		g_strlcat(str, "00:", 4);

	seconds = length;
	memset(tmp, '\0', 24);
	g_sprintf(tmp, "%02d", seconds);
	g_strlcat(str, tmp, 24);

	return str;
}