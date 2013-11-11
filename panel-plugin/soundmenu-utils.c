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

#include "soundmenu-utils.h"
#include "soundmenu-dbus.h"
#include "soundmenu-dialogs.h"
#include "soundmenu-lastfm.h"
#include "soundmenu-mpris2.h"
#include "soundmenu-related.h"
#include "soundmenu-plugin.h"

const gchar *
g_avariant_get_string(GVariant * variant)
{
	const gchar **strv = NULL;
	const gchar *string = NULL;
	gsize len;

	strv = g_variant_get_strv (variant, &len);
	if (len > 0) {
		string = strv[0];
		g_free (strv);
	}
	else {
		string = "";
	}

	return string;
}

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
set_watch_cursor (GtkWidget *widget)
{
	GdkCursor *cursor;
	GtkWidget  *toplevel;

	toplevel = gtk_widget_get_toplevel(GTK_WIDGET(widget));
	if (G_LIKELY (toplevel != NULL)) {
		cursor = gdk_cursor_new (GDK_WATCH);

		gdk_window_set_cursor (gtk_widget_get_window (toplevel), cursor);
		gdk_cursor_unref (cursor);
	}
}

void
remove_watch_cursor (GtkWidget *widget)
{
	GtkWidget  *toplevel;

	toplevel = gtk_widget_get_toplevel(GTK_WIDGET(widget));
	if (G_LIKELY (toplevel != NULL))
		gdk_window_set_cursor (gtk_widget_get_window (toplevel), NULL);
}

/* Launch the player configured. */

void
soundmenu_launch_player(SoundmenuPlugin *soundmenu)
{
	gboolean result;

	if(g_str_nempty0(soundmenu->player)) {
		result = g_spawn_command_line_async (soundmenu->player, NULL);
		if (G_UNLIKELY (result == FALSE))
			soundmenu_configure(soundmenu->plugin, soundmenu);
	}
}

/* Open the image when double click.. */

gboolean
soundmenu_album_art_frame_press_callback (GtkWidget       *event_box,
                                          GdkEventButton  *event,
                                          SoundmenuPlugin *soundmenu)
{
	gchar *command = NULL;
	const gchar *url;
	gboolean result;

	if(event->button == 3) {
		g_signal_emit_by_name (G_OBJECT (soundmenu->play_button), "button-press-event", event, &result);
		return TRUE;
	}

	if(event->type != GDK_2BUTTON_PRESS &&
	   event->type != GDK_3BUTTON_PRESS)
		return TRUE;

	if(!soundmenu->connected) {
		soundmenu_launch_player(soundmenu);
		return TRUE;
	}

	url = soundmenu_album_art_get_path(soundmenu->album_art);
	if(url) {
		command = g_strdup_printf("exo-open \"%s\"", url);
		result = g_spawn_command_line_async (command, NULL);
		if (G_UNLIKELY (result == FALSE))
			g_warning ("Unable to launch command: %s", command);

		g_free(command);
	}

	return TRUE;
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