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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "soundmenu-plugin.h"
#include "soundmenu-dialogs.h"
#include "soundmenu-lastfm.h"
#include "soundmenu-mpris2.h"
#include "soundmenu-utils.h"
#include "soundmenu-related.h"

#define WAIT_UPDATE 5

Metadata *malloc_metadata()
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

/* Dbus helpers to parse Metadata info, etc.. */

static void
get_meta_item_array(DBusMessageIter *dict_entry, char **item)
{
	DBusMessageIter variant, array;
	char *str_buf;

	dbus_message_iter_next(dict_entry);
	dbus_message_iter_recurse(dict_entry, &array);

	dbus_message_iter_recurse(&array, &variant);
	dbus_message_iter_get_basic(&variant, (void*) &str_buf);

	*item = malloc(strlen(str_buf) + 1);
	strcpy(*item, str_buf);
}

static void
get_meta_item_str(DBusMessageIter *dict_entry, char **item)
{
	DBusMessageIter variant;
	char *str_buf;

	dbus_message_iter_next(dict_entry);
	dbus_message_iter_recurse(dict_entry, &variant);
	dbus_message_iter_get_basic(&variant, (void*) &str_buf);

	*item = malloc(strlen(str_buf) + 1);
	strcpy(*item, str_buf);
}

static void
get_meta_item_gint(DBusMessageIter *dict_entry, void *item)
{
	DBusMessageIter variant;

	dbus_message_iter_next(dict_entry);
	dbus_message_iter_recurse(dict_entry, &variant);
	dbus_message_iter_get_basic(&variant, (void*) item);
}

void
mpris2_demarshal_metadata (DBusMessageIter *args, SoundmenuPlugin *soundmenu)	// arg inited on Metadata string
{
	DBG ("Demarshal_metadata");

	DBusMessageIter dict, dict_entry, variant;
	Metadata *metadata;
	gchar *str_buf = NULL, *string = NULL;

	gint64 length = 0;
	gint32 trackNumber = 0;
	
	metadata = malloc_metadata();

	dbus_message_iter_recurse(args, &dict);		// Recurse => dict on fist "dict entry()"
	
	dbus_message_iter_recurse(&dict, &dict_entry);	// Recurse => dict_entry on "string "mpris:trackid""
	do
	{
		dbus_message_iter_recurse(&dict_entry, &variant);
		dbus_message_iter_get_basic(&variant, (void*) &str_buf);

		if (0 == g_ascii_strcasecmp (str_buf, "mpris:trackid"))
			get_meta_item_str(&variant, &metadata->trackid);
		else if (0 == g_ascii_strcasecmp (str_buf, "xesam:url"))
			get_meta_item_str(&variant, &metadata->url);
		else if (0 == g_ascii_strcasecmp (str_buf, "xesam:title"))
			get_meta_item_str(&variant, &metadata->title);
		else if (0 == g_ascii_strcasecmp (str_buf, "xesam:artist"))
			get_meta_item_array(&variant, &metadata->artist);
		else if (0 == g_ascii_strcasecmp (str_buf, "xesam:album"))
			get_meta_item_str(&variant, &metadata->album);
		else if (0 == g_ascii_strcasecmp (str_buf, "xesam:genre"));
			/* (List of Strings.) Not use genre */
		else if (0 == g_ascii_strcasecmp (str_buf, "xesam:albumArtist"));
			// List of Strings.
		else if (0 == g_ascii_strcasecmp (str_buf, "xesam:comment"));
			/* (List of Strings) Not use comment */
		else if (0 == g_ascii_strcasecmp (str_buf, "xesam:audioBitrate"));
			/* (uint32) Not use audioBitrate */
		else if (0 == g_ascii_strcasecmp (str_buf, "mpris:length"))
			get_meta_item_gint(&variant, &length);
		else if (0 == g_ascii_strcasecmp (str_buf, "xesam:trackNumber"))
			get_meta_item_gint(&variant, &trackNumber);
		else if (0 == g_ascii_strcasecmp (str_buf, "xesam:useCount"));
			/* (Integer) Not use useCount */
		else if (0 == g_ascii_strcasecmp (str_buf, "xesam:userRating"));
			/* (Float) Not use userRating */
		else if (0 == g_ascii_strcasecmp (str_buf, "mpris:arturl"))
			get_meta_item_str(&variant, &metadata->arturl);
		else
			DBG ("New metadata message: %s. (Investigate)\n", str_buf);
	
	} while (dbus_message_iter_next(&dict_entry));

	metadata->length = length / 1000000l;
	metadata->trackNumber = trackNumber;

	free_metadata(soundmenu->metadata);
	soundmenu->metadata = metadata;
}

/* Basic dbus functions for interacting with MPRIS2*/

DBusHandlerResult
mpris2_dbus_filter (DBusConnection *connection, DBusMessage *message, void *user_data)
{
	DBusMessageIter args, dict, dict_entry;
	gchar *str_buf = NULL, *state = NULL;
	gdouble volume = 0;

	SoundmenuPlugin *soundmenu = user_data;

	if ( dbus_message_is_signal (message, "org.freedesktop.DBus.Properties", "PropertiesChanged" ) )
	{
		dbus_message_iter_init(message, &args);

		/* Ignore the interface_name*/
		dbus_message_iter_next(&args);

		dbus_message_iter_recurse(&args, &dict);
		do
		{
			dbus_message_iter_recurse(&dict, &dict_entry);
			dbus_message_iter_get_basic(&dict_entry, (void*) &str_buf);

			if (0 == g_ascii_strcasecmp (str_buf, "PlaybackStatus"))
			{
				get_meta_item_str (&dict_entry, &state);
				soundmenu_update_state (state, soundmenu);
			}
			else if (0 == g_ascii_strcasecmp (str_buf, "Volume"))
			{
				get_meta_item_gint(&dict_entry, &volume);
				soundmenu->volume = volume;
			}
			else if (0 == g_ascii_strcasecmp (str_buf, "Metadata"))
			{
				/* Ignore inferface string and send the pointer to metadata. */
				dbus_message_iter_next(&dict_entry);
				mpris2_demarshal_metadata (&dict_entry, soundmenu);
				#ifdef HAVE_LIBCLASTFM
				if (soundmenu->clastfm->lastfm_support)
					update_lastfm(soundmenu);
				#endif
			}
		} while (dbus_message_iter_next(&dict));

		return DBUS_HANDLER_RESULT_HANDLED;
	}
	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

void
mpris2_send_message (SoundmenuPlugin *soundmenu, const char *msg)
{
	DBusMessage *message;
	gchar *destination = NULL;

	destination = g_strdup_printf ("org.mpris.MediaPlayer2.%s", soundmenu->player);
	message = dbus_message_new_method_call (destination, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player",  msg);
	g_free(destination);

	/* Send the message */
	dbus_connection_send (soundmenu->connection, message, NULL);
	dbus_message_unref (message);
}

/*dbus-send --session  --print-reply --reply-timeout=2000
 * --type=method_call --dest=org.freedesktop.DBus /org/freedesktop/DBus org.freedesktop.DBus.ListNames
 */
gchar *
mpris2_get_player (SoundmenuPlugin *soundmenu)
{
	DBusError d_error;
	DBusMessageIter dict, list;
	DBusMessage *reply_msg = NULL, *message = NULL;
	char *str_buf = NULL, *player = NULL;

	dbus_error_init(&d_error);

	message = dbus_message_new_method_call ("org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus",  "ListNames");

	reply_msg = dbus_connection_send_with_reply_and_block(soundmenu->connection, message, -1, &d_error);

	dbus_message_iter_init(reply_msg, &list);
	dbus_message_iter_recurse(&list, &dict);

	do {
		dbus_message_iter_get_basic(&dict, (void*) &str_buf);
		if (g_str_has_prefix(str_buf, "org.mpris.MediaPlayer2.")) {
			player = g_strdup(str_buf + 23);
			g_print(player);
			//g_free(str_buf);
			break;
		}
		//g_free(str_buf);
	} while (dbus_message_iter_next(&dict));

	return player;
}

void
mpris2_get_playbackstatus (SoundmenuPlugin *soundmenu)
{
	DBusMessage *message = NULL, *reply_message = NULL;
	DBusMessageIter dict_entry, variant;
	gchar *destination = NULL, *state= NULL;

	const char * const interface_name = "org.mpris.MediaPlayer2.Player";
	const char * const query = "PlaybackStatus";

	destination = g_strdup_printf ("org.mpris.MediaPlayer2.%s", soundmenu->player);

	message = dbus_message_new_method_call (destination, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "Get");
	dbus_message_append_args(message,
					DBUS_TYPE_STRING, &interface_name,
					DBUS_TYPE_STRING, &query,
					DBUS_TYPE_INVALID);

	if(reply_message = dbus_connection_send_with_reply_and_block (soundmenu->connection, message, -1, NULL)) {
		dbus_message_iter_init(reply_message, &dict_entry);
		dbus_message_iter_recurse(&dict_entry, &variant);

		dbus_message_iter_get_basic(&variant, (void*) &state);
		soundmenu_update_state (state, soundmenu);
	}

	dbus_message_unref (message);
	g_free(destination);
}

void
mpris2_get_metadata (SoundmenuPlugin *soundmenu)
{
	DBusMessage *message = NULL, *reply_message = NULL;
	DBusMessageIter args;
	gchar *destination = NULL;

	const char * const interface_name = "org.mpris.MediaPlayer2.Player";
	const char * const query = "Metadata";

	destination = g_strdup_printf ("org.mpris.MediaPlayer2.%s", soundmenu->player);

	message = dbus_message_new_method_call (destination, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "Get");
	dbus_message_append_args(message,
					DBUS_TYPE_STRING, &interface_name,
					DBUS_TYPE_STRING, &query,
					DBUS_TYPE_INVALID);

	if(reply_message = dbus_connection_send_with_reply_and_block (soundmenu->connection, message, -1, NULL)) {
		dbus_message_iter_init(reply_message, &args);
		mpris2_demarshal_metadata (&args, soundmenu);
	}

	dbus_message_unref (message);
	g_free(destination);
}

void
mpris2_get_volume (SoundmenuPlugin *soundmenu)
{
	DBusMessage *message = NULL, *reply_message = NULL;
	DBusMessageIter dict_entry, variant;
	gchar *destination = NULL;
	gdouble volume = 0;

	const char * const interface_name = "org.mpris.MediaPlayer2.Player";
	const char * const query = "Volume";

	destination = g_strdup_printf ("org.mpris.MediaPlayer2.%s", soundmenu->player);

	message = dbus_message_new_method_call (destination, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "Get");
	dbus_message_append_args(message,
					DBUS_TYPE_STRING, &interface_name,
					DBUS_TYPE_STRING, &query,
					DBUS_TYPE_INVALID);

	if(reply_message = dbus_connection_send_with_reply_and_block (soundmenu->connection, message, -1, NULL)) {
		dbus_message_iter_init(reply_message, &dict_entry);
		dbus_message_iter_recurse(&dict_entry, &variant);
		dbus_message_iter_get_basic(&variant, &volume);
	}
	soundmenu->volume = volume;

	dbus_message_unref (message);
	g_free(destination);
}

void mpris2_get_player_status (SoundmenuPlugin *soundmenu)
{
	mpris2_get_playbackstatus (soundmenu);
	mpris2_get_metadata (soundmenu);
	mpris2_get_volume (soundmenu);
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