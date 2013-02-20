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
#include "soundmenu-album-art.h"
#include "soundmenu-dbus.h"
#include "soundmenu-dialogs.h"
#include "soundmenu-lastfm.h"
#include "soundmenu-mpris2.h"
#include "soundmenu-utils.h"
#include "soundmenu-related.h"

Metadata *malloc_metadata (void)
{
	Metadata *m;
	m = malloc(sizeof(Metadata));

	m->trackid = NULL;
	m->url = NULL;
	m->title = NULL;
	m->artist = NULL;
	m->album = NULL;
	m->length = 0;
	m->trackNumber = 0;
	m->arturl = NULL;

	return m;
}

void free_metadata(Metadata *m)
{
	if(m == NULL)
		return;

	if(m->trackid)	free(m->trackid);
	if(m->url)		free(m->url);
	if(m->title)	free(m->title);
	if(m->artist)	free(m->artist);
	if(m->album) 	free(m->album);
	if(m->arturl)	free(m->arturl);

	free(m);
}

void
mpris2_send_message (SoundmenuPlugin *soundmenu, const char *msg)
{
	GDBusMessage *message;
	gchar        *destination;
	GError       *error = NULL;

	destination = g_strdup_printf ("org.mpris.MediaPlayer2.%s", soundmenu->player);
	message = g_dbus_message_new_method_call (destination,
	                                          "/org/mpris/MediaPlayer2",
	                                          "org.mpris.MediaPlayer2.Player",
	                                          msg);
	g_free(destination);

	g_dbus_connection_send_message (soundmenu->gconnection,
	                                message,
	                                G_DBUS_SEND_MESSAGE_FLAGS_NONE,
	                                NULL,
	                                &error);
	if (error != NULL) {
		g_warning ("unable to send message: %s", error->message);
		g_clear_error (&error);
		error = NULL;
	}

	g_dbus_connection_flush_sync (soundmenu->gconnection, NULL, &error);
	if (error != NULL) {
		g_warning ("unable to flush message queue: %s", error->message);
		g_clear_error (&error);
	}

	g_object_unref (message);
}

/*
 * First intent to set the volume.
 * DBUS_TYPE_VARIANT is not implemented on dbus_message_append_args.
 * I did not know implement a variant of double into a container.
 * Any Help?????

static gboolean
panel_button_scrolled (GtkWidget        *widget,
				GdkEventScroll   *event,
				SoundmenuPlugin *soundmenu)
{
	DBusMessage *message = NULL;
	DBusMessageIter value_iter, iter_dict_entry, variant;
	gchar *destination = NULL;

	const char * const interface_name = "org.mpris.MediaPlayer2.Player";
	const char * const query = "Volume";

	switch (event->direction)
	{
	case GDK_SCROLL_UP:
	case GDK_SCROLL_RIGHT:
		soundmenu->volume += 0.02;
		break;
	case GDK_SCROLL_DOWN:
	case GDK_SCROLL_LEFT:
		soundmenu->volume -= 0.02;
		break;
	}

	soundmenu->volume = CLAMP (soundmenu->volume, 0.0, 1.0);

	destination = g_strdup_printf ("org.mpris.MediaPlayer2.%s", soundmenu->player);
	message = dbus_message_new_method_call (destination, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "Set");
	dbus_message_append_args(message,
				DBUS_TYPE_STRING, &interface_name,
				DBUS_TYPE_STRING, &query,
				DBUS_TYPE_VARIANT, &iter_dict_entry,
				DBUS_TYPE_INVALID);

	// FIXME: DBUS_TYPE_VARIANT not implemented, therefore you have to do in a container.
	if(dbus_message_iter_open_container(&iter_dict_entry, DBUS_TYPE_VARIANT, DBUS_TYPE_DOUBLE_AS_STRING, &variant)) {
		dbus_message_iter_append_basic(&variant, DBUS_TYPE_DOUBLE, &(soundmenu->volume));
		dbus_message_iter_close_container(&iter_dict_entry, &variant);
	}

	dbus_connection_send (soundmenu->connection, message, NULL);
		
	dbus_message_unref (message);
	g_free(destination);

	return TRUE;
} */