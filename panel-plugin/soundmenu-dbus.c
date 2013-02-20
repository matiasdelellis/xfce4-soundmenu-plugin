/*
 *  Copyright (c) 2012 matias <mati86dl@gmail.com>
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

DBusHandlerResult
soundmenu_dbus_connection_filter (DBusConnection *connection, DBusMessage *message, void *user_data)
{
	SoundmenuPlugin *soundmenu = user_data;

	if(dbus_message_is_signal(message, "org.freedesktop.DBus.Properties", "PropertiesChanged"))
	{
		mpris2_dbus_filter (message, soundmenu);
		return DBUS_HANDLER_RESULT_HANDLED;
	}

	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

/*
 *Useful to debug..
 static void
print_variant(GVariant *value)
{
	gchar *s_variant;
	s_variant = g_variant_print (value, TRUE);

    g_print ("Variant '%s' has type '%s'\n", s_variant,
	           g_variant_get_type_string (value));

	g_free(s_variant);
}*/

static gchar *
g_avariant_dup_string(GVariant * variant)
{
	const char **strv = NULL;
	strv = g_variant_get_strv (variant, NULL);

	return g_strdup (strv[0]);
}

static Metadata *
soundmenu_mpris2_get_metadata (GVariant *dictionary)
{
	GVariantIter iter;
	GVariant *value;
	gchar *key;
	gsize *size;

	gint64 length = 0;
	gint32 trackNumber = 0;

	Metadata *metadata;

	metadata = malloc_metadata();

	g_variant_iter_init (&iter, dictionary);
	while (g_variant_iter_loop (&iter, "{sv}", &key, &value)) {
		if (0 == g_ascii_strcasecmp (key, "mpris:trackid"))
			metadata->trackid = g_variant_dup_string(value, NULL);
		else if (0 == g_ascii_strcasecmp (key, "xesam:url"))
			metadata->url= g_variant_dup_string(value, size);
		else if (0 == g_ascii_strcasecmp (key, "xesam:title"))
			metadata->url= g_variant_dup_string(value, size);
		else if (0 == g_ascii_strcasecmp (key, "xesam:artist"))
			metadata->artist = g_avariant_dup_string(value);
		else if (0 == g_ascii_strcasecmp (key, "xesam:album"))
			metadata->album = g_variant_dup_string(value, size);
		else if (0 == g_ascii_strcasecmp (key, "xesam:genre"));
			/* (List of Strings.) Not use genre */
		else if (0 == g_ascii_strcasecmp (key, "xesam:albumArtist"));
			// List of Strings.
		else if (0 == g_ascii_strcasecmp (key, "xesam:comment"));
			/* (List of Strings) Not use comment */
		else if (0 == g_ascii_strcasecmp (key, "xesam:audioBitrate"));
			/* (uint32) Not use audioBitrate */
		else if (0 == g_ascii_strcasecmp (key, "mpris:length"))
			length = g_variant_get_int64 (value);
		else if (0 == g_ascii_strcasecmp (key, "xesam:trackNumber"))
			trackNumber = g_variant_get_int32 (value);
		else if (0 == g_ascii_strcasecmp (key, "xesam:useCount"));
			/* (Integer) Not use useCount */
		else if (0 == g_ascii_strcasecmp (key, "xesam:userRating"));
			/* (Float) Not use userRating */
		else if (0 == g_ascii_strcasecmp (key, "mpris:artUrl"))
			metadata->url= g_variant_dup_string(value, size);
		else
			g_print ("Variant '%s' has type '%s'\n", key,
				     g_variant_get_type_string (value));
	}

	metadata->length = length / 1000000l;
	metadata->trackNumber = trackNumber;

	return metadata;
}

static void
soundmenu_mpris2_on_dbus_signal (GDBusProxy *proxy,
                                 gchar      *sender_name,
                                 gchar      *signal_name,
                                 GVariant   *parameters,
                                 gpointer    user_data)
{
	GVariantIter iter;
	GVariant *value;
	GVariant *child;
	const gchar *key;
	Metadata *metadata;

	SoundmenuPlugin *soundmenu = user_data;

	g_variant_iter_init (&iter, parameters);

	child = g_variant_iter_next_value (&iter); /* Interface name. */

	child = g_variant_iter_next_value (&iter); /* Property name. */
	g_variant_iter_init (&iter, child);
	while (g_variant_iter_loop (&iter, "{sv}", &key, &value)) {
		if (0 == g_ascii_strcasecmp (key, "PlaybackStatus"))
		{
			g_print ("Variant '%s' has type '%s'\n", key,
				     g_variant_get_type_string (value));
		}
		else if (0 == g_ascii_strcasecmp (key, "Volume"))
		{
			g_print ("Variant '%s' has type '%s'\n", key,
				     g_variant_get_type_string (value));
		}
		else if (0 == g_ascii_strcasecmp (key, "Metadata"))
		{
			metadata = soundmenu_mpris2_get_metadata (value);
			free_metadata(soundmenu->metadata);
			soundmenu->metadata = metadata;

			soundmenu_album_art_set_path(soundmenu->album_art, soundmenu->metadata->arturl);

			#ifdef HAVE_LIBCLASTFM
			if (soundmenu->clastfm->lastfm_support)
				update_lastfm(soundmenu);
			#endif
		}
	}
}

static void
soundmenu_mpris2_dbus_conected(GDBusConnection *connection,
                               const gchar *name,
                               const gchar *name_owner,
                               gpointer user_data)
{
	SoundmenuPlugin *soundmenu = user_data;

	gtk_widget_set_sensitive(GTK_WIDGET(soundmenu->plugin), TRUE);
}

static void
soundmenu_mpris2_dbus_losed(GDBusConnection *connection,
                            const gchar *name,
                            gpointer user_data)
{
	SoundmenuPlugin *soundmenu = user_data;

	gtk_widget_set_sensitive(GTK_WIDGET(soundmenu->plugin), FALSE);
}

void
init_dbus_session (SoundmenuPlugin *soundmenu)
{
	GDBusConnection *gconnection;
	GError          *gerror = NULL;
	GDBusProxy      *proxy;
	DBusConnection *connection;
	DBusError error;
	gchar *rule = NULL;

	/* Init gdbus connection. */

	gconnection = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, &gerror);
	if (gconnection == NULL) {
		g_message ("Failed to get session bus: %s", gerror->message);
		g_error_free (gerror);
		gerror = NULL;
	}

	soundmenu->dbus_name = g_strdup_printf("org.mpris.MediaPlayer2.%s", soundmenu->player);

	g_bus_watch_name_on_connection(gconnection,
	                               soundmenu->dbus_name,
	                               G_BUS_NAME_OWNER_FLAGS_REPLACE,
	                               soundmenu_mpris2_dbus_conected,
	                               soundmenu_mpris2_dbus_losed,
	                               soundmenu,
	                               NULL);

	proxy = g_dbus_proxy_new_sync (gconnection,
	                               G_DBUS_PROXY_FLAGS_NONE,
	                               NULL,
	                               soundmenu->dbus_name,
	                               "/org/mpris/MediaPlayer2",
	                               "org.freedesktop.DBus.Properties",
	                               NULL, /* GCancellable */
	                               &gerror);
	if (proxy == NULL) {
		g_printerr ("Error creating proxy: %s\n", gerror->message);
		g_error_free (gerror);
		gerror = NULL;
    }
    else {
		g_signal_connect (proxy,
			              "g-signal",
			              G_CALLBACK (soundmenu_mpris2_on_dbus_signal),
			              soundmenu);
	}

	soundmenu->gconnection = gconnection;

	/* Init dbus connection. */

	dbus_error_init (&error);

	connection = dbus_bus_get (DBUS_BUS_SESSION, &error);
	if (connection == NULL) {
		g_critical("Error connecting to DBUS_BUS_SESSION: %s", error.message);
		dbus_error_free (&error);
	}

	/* Configure rule according to player selected. */

	rule = g_strdup_printf ("type='signal', sender='%s'", soundmenu->dbus_name);
	dbus_bus_add_match (connection, rule, NULL);
	g_free(rule);

	dbus_connection_add_filter (connection, soundmenu_dbus_connection_filter, soundmenu, NULL);
	dbus_connection_setup_with_g_main (connection, NULL);

	soundmenu->connection = connection;
}