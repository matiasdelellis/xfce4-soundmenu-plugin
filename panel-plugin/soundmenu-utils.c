/*
 *  Copyright (c) 2011-2012 matias <mati86dl@gmail.com>
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

#include "soundmenu-plugin.h"
#include "soundmenu-dbus.h"
#include "soundmenu-dialogs.h"
#include "soundmenu-lastfm.h"
#include "soundmenu-mpris2.h"
#include "soundmenu-utils.h"
#include "soundmenu-related.h"

#if !GLIB_CHECK_VERSION(2,32,0)
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
#endif

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
	#ifdef HAVE_LIBNOTIFY
	NotifyNotification *notify = NULL;
	#endif

	gdk_threads_enter ();
	gdk_window_set_cursor(GDK_WINDOW(gtk_widget_get_toplevel (GTK_WIDGET (soundmenu->plugin))->window), NULL);
	#ifdef HAVE_LIBNOTIFY
	if(message != NULL) {
		#if NOTIFY_CHECK_VERSION (0, 7, 0)
		notify = notify_notification_new(_("Sound menu Plugin"), message, "xfce4-soundmenu-plugin");
		#else
		notify = notify_notification_new(_("Sound menu Plugin"), message, "xfce4-soundmenu-plugin", NULL);
		#endif
		if (!notify_notification_show (notify, NULL))
			g_warning("Failed to send notification: %s", message);
	}
	#endif
	gdk_threads_leave ();
}

/* Open the image when double click.. */

gboolean
soundmenu_album_art_frame_press_callback (GtkWidget         *event_box,
                                          GdkEventButton    *event,
                                          SoundmenuAlbumArt *albumart)
{
	gchar *command = NULL;
	const gchar *art_url;
	gboolean result;

	art_url = soundmenu_album_art_get_path(albumart);

	if ((art_url != NULL) &&
	   (event->type==GDK_2BUTTON_PRESS ||
	    event->type==GDK_3BUTTON_PRESS)) {

	   	command = g_strdup_printf("exo-open \"%s\"", art_url);
		result = g_spawn_command_line_async (command, NULL);

		if (G_UNLIKELY (result == FALSE))
			g_warning ("Unable to show the current album art: %s", art_url);

		g_free(command);

		return TRUE;
	}
	else
		return FALSE;
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