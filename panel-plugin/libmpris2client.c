/*
 *  Copyright (c) 2013 matias <mati86dl@gmail.com>
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

#include <gio/gio.h>

#include "libmpris2client.h"
#include "mpris2-metadata.h"
#include "mpris2-utils.h"

struct _Mpris2Client
{
	GObject parent_instance;

	/* Priv */
	GDBusConnection *gconnection;
	GDBusProxy      *proxy;
	gchar			*dbus_name;
	guint            watch_id;

	/* Conf. */
	gchar           *player;

	/* Status */
	gboolean         connected;

	/* Interface MediaPlayer2 */
	gboolean         can_quit;
	gboolean         can_raise;
	gboolean         has_tracklist;
	gchar           *identity;
	gchar          **supported_uri_schemes;
	gchar          **supported_mime_types;

	/* Interface MediaPlayer2.Player */
	PlaybackStatus   playback_status;
	Mpris2Metadata  *metadata;
	gdouble          volume;
};

enum
{
	CONNECTION,
	PLAYBACK_STATUS,
	METADATA,
	VOLUME,
	LAST_SIGNAL
};
static int signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (Mpris2Client, mpris2_client, G_TYPE_OBJECT)

/*
 * Prototypes
 */
static void   mpris2_client_call_player_method  (Mpris2Client *mpris2, const char *method);
static void   mpris2_client_call_method         (Mpris2Client *mpris2, const char *method);

static gchar *mpris2_client_get_player   (Mpris2Client *mpris2);
static void   mpris2_client_connect_dbus (Mpris2Client *mpris2);

GVariant     *mpris2_client_get_player_properties (Mpris2Client *mpris2, const gchar *prop);
static void   mpris2_client_set_player_properties (Mpris2Client *mpris2, const gchar *prop, GVariant *vprop);

/*
 *  Basic control of gdbus functions
 */

void
mpris2_client_play_pause (Mpris2Client *mpris2)
{
	if (!mpris2->connected)
		return;

	mpris2_client_call_player_method (mpris2, "PlayPause");
}

void
mpris2_client_stop (Mpris2Client *mpris2)
{
	if (!mpris2->connected)
		return;

	mpris2_client_call_player_method (mpris2, "Stop");
}

void
mpris2_client_prev (Mpris2Client *mpris2)
{
	if (!mpris2->connected)
		return;

	mpris2_client_call_player_method (mpris2, "Previous");
}

void
mpris2_client_next (Mpris2Client *mpris2)
{
	if (!mpris2->connected)
		return;

	mpris2_client_call_player_method (mpris2, "Next");
}

void
mpris2_client_quit_player (Mpris2Client *mpris2)
{
	if (!mpris2->can_quit)
		return;

	mpris2_client_call_method (mpris2, "Quit");
}

void
mpris2_client_raise_player (Mpris2Client *mpris2)
{
	if (!mpris2->can_raise)
		return;

	mpris2_client_call_method (mpris2, "Raise");
}

/*
 * Interface MediaPlayer2.Player properties.
 */

PlaybackStatus
mpris2_client_get_playback_status (Mpris2Client *mpris2)
{
	return mpris2->playback_status;
}

Mpris2Metadata *
mpris2_client_get_metadata (Mpris2Client *mpris2)
{
	return mpris2->metadata;
}

gdouble
mpris2_client_get_volume (Mpris2Client *mpris2)
{
	return mpris2->volume;
}

void
mpris2_client_set_volume (Mpris2Client *mpris2, gdouble volume)
{
	mpris2_client_set_player_properties (mpris2, "Volume", g_variant_new_double(volume));
}

/*
 * Interface MediaPlayer2 Properties.
 */

gboolean
mpris2_client_can_quit (Mpris2Client *mpris2)
{
	return mpris2->can_quit;
}

gboolean
mpris2_client_can_raise (Mpris2Client *mpris2)
{
	return mpris2->can_raise;
}

gboolean
mpris2_client_has_tracklist_support (Mpris2Client *mpris2)
{
	return mpris2->has_tracklist;
}

const gchar *
mpris2_client_get_player_identity (Mpris2Client *mpris2)
{
	return mpris2->identity;
}

gchar **
mpris2_client_get_supported_uri_schemes (Mpris2Client *mpris2)
{
	return mpris2->supported_uri_schemes;
}

gchar **
mpris2_client_get_supported_mime_types (Mpris2Client *mpris2)
{
	return mpris2->supported_mime_types;
}

void
mpris2_client_set_player (Mpris2Client *mpris2, const gchar *player)
{
	g_free(mpris2->player);
	if (player)
		mpris2->player = g_strdup(player);
	else
		mpris2->player = g_strdup("unknown");

	/* Disconnect dbus */
	g_bus_unwatch_name (mpris2->watch_id);
	g_object_unref (mpris2->proxy);

	/* Connect again */
	mpris2_client_connect_dbus (mpris2);
}

gchar *
mpris2_client_auto_set_player (Mpris2Client *mpris2)
{
	gchar *player = mpris2_client_get_player (mpris2);

	mpris2_client_set_player (mpris2, player);

	return player;
}

gboolean
mpris2_client_is_connected (Mpris2Client *mpris2)
{
	return mpris2->connected;
}

/*
 * SoundmenuDbus.
 */

/* Send mesages to use methods of org.mpris.MediaPlayer2.Player interfase. */

static void
mpris2_client_call_player_method (Mpris2Client *mpris2, const char *method)
{
	GDBusMessage *message;
	GError       *error = NULL;

	message = g_dbus_message_new_method_call (mpris2->dbus_name,
	                                          "/org/mpris/MediaPlayer2",
	                                          "org.mpris.MediaPlayer2.Player",
	                                          method);

	g_dbus_connection_send_message (mpris2->gconnection,
	                                message,
	                                G_DBUS_SEND_MESSAGE_FLAGS_NONE,
	                                NULL,
	                                &error);
	if (error != NULL) {
		g_warning ("unable to send message: %s", error->message);
		g_clear_error (&error);
		error = NULL;
	}

	g_dbus_connection_flush_sync (mpris2->gconnection, NULL, &error);
	if (error != NULL) {
		g_warning ("unable to flush message queue: %s", error->message);
		g_clear_error (&error);
	}

	g_object_unref (message);
}

/* Send mesages to use methods of org.mpris.MediaPlayer2 interfase. */

static void
mpris2_client_call_method (Mpris2Client *mpris2, const char *method)
{
	GDBusMessage *message;
	GError       *error = NULL;

	message = g_dbus_message_new_method_call (mpris2->dbus_name,
	                                          "/org/mpris/MediaPlayer2",
	                                          "org.mpris.MediaPlayer2",
	                                          method);

	g_dbus_connection_send_message (mpris2->gconnection,
	                                message,
	                                G_DBUS_SEND_MESSAGE_FLAGS_NONE,
	                                NULL,
	                                &error);
	if (error != NULL) {
		g_warning ("unable to send message: %s", error->message);
		g_clear_error (&error);
		error = NULL;
	}

	g_dbus_connection_flush_sync (mpris2->gconnection, NULL, &error);
	if (error != NULL) {
		g_warning ("unable to flush message queue: %s", error->message);
		g_clear_error (&error);
	}

	g_object_unref (message);
}

/* Returns the first player name that compliant to mpris2 on dbus.  */

static gchar *
mpris2_client_get_player (Mpris2Client *mpris2)
{
	GError *error = NULL;
	GVariant *v;
	GVariantIter *iter;
	const gchar *str = NULL;
	gchar *player = NULL;

	v = g_dbus_connection_call_sync (mpris2->gconnection,
	                                 "org.freedesktop.DBus",
	                                 "/org/freedesktop/DBus",
	                                 "org.freedesktop.DBus",
	                                 "ListNames",
	                                 NULL,
	                                 G_VARIANT_TYPE ("(as)"),
	                                 G_DBUS_CALL_FLAGS_NONE,
	                                 -1,
	                                 NULL,
	                                 &error);
	if (error) {
		g_critical ("Could not get a list of names registered on the session bus, %s",
		            error ? error->message : "no error given");
		g_clear_error (&error);
		return NULL;
	}

	g_variant_get (v, "(as)", &iter);
	while (g_variant_iter_loop (iter, "&s", &str)) {
		if (g_str_has_prefix(str, "org.mpris.MediaPlayer2.")) {
			player = g_strdup(str + 23);
			break;
		}
	}
	g_variant_iter_free (iter);
	g_variant_unref (v);

	return player;
}

/* Get any player propertie using org.freedesktop.DBus.Properties interfase. */

GVariant *
mpris2_client_get_player_properties (Mpris2Client *mpris2, const gchar *prop)
{
	GVariant *v, *iter;
	GError *error = NULL;

	v = g_dbus_connection_call_sync (mpris2->gconnection,
	                                 mpris2->dbus_name,
	                                 "/org/mpris/MediaPlayer2",
	                                 "org.freedesktop.DBus.Properties",
	                                 "Get",
	                                  g_variant_new ("(ss)",
                                                     "org.mpris.MediaPlayer2",
                                                     prop),
	                                 G_VARIANT_TYPE ("(v)"),
	                                 G_DBUS_CALL_FLAGS_NONE,
	                                 -1,
	                                 NULL,
	                                 &error);
	if (error) {
		g_critical ("Could not get a list of names registered on the session bus, %s",
		            error ? error->message : "no error given");
		g_clear_error (&error);
		return NULL;
	}

	g_variant_get (v, "(v)", &iter);

	return iter;
}

/* Change any player propertie using org.freedesktop.DBus.Properties interfase. */

static void
mpris2_client_set_player_properties (Mpris2Client *mpris2, const gchar *prop, GVariant *vprop)
{
	GVariant *reply;
	GError   *error = NULL;

	reply = g_dbus_connection_call_sync (mpris2->gconnection,
	                                     mpris2->dbus_name,
	                                     "/org/mpris/MediaPlayer2",
	                                     "org.freedesktop.DBus.Properties",
	                                     "Set",
	                                     g_variant_new ("(ssv)",
	                                                    "org.mpris.MediaPlayer2.Player",
	                                                    prop,
	                                                    vprop),
	                                     NULL,
	                                     G_DBUS_CALL_FLAGS_NONE,
	                                     -1,
	                                     NULL,
	                                     NULL);
	if (reply == NULL) {
		g_warning ("Unable to set session: %s", error->message);
		g_error_free (error);
		return;
	}
	g_variant_unref(reply);
}

/* These function intercepts the messages from the player. */

static Mpris2Metadata *
mpris2_metadata_new_from_variant (GVariant *dictionary)
{
	GVariantIter iter;
	GVariant *value;
	gchar *key;

	gint64 length = 0;

	Mpris2Metadata *metadata;

	metadata = mpris2_metadata_new ();

	g_variant_iter_init (&iter, dictionary);
	while (g_variant_iter_loop (&iter, "{sv}", &key, &value)) {
		if (0 == g_ascii_strcasecmp (key, "mpris:trackid"))
			mpris2_metadata_set_trackid (metadata, g_variant_get_string(value, NULL));
		else if (0 == g_ascii_strcasecmp (key, "xesam:url"))
			mpris2_metadata_set_url (metadata, g_variant_get_string(value, NULL));
		else if (0 == g_ascii_strcasecmp (key, "xesam:title"))
			mpris2_metadata_set_title (metadata, g_variant_get_string(value, NULL));
		else if (0 == g_ascii_strcasecmp (key, "xesam:artist"))
			mpris2_metadata_set_artist(metadata, g_avariant_get_string(value));
		else if (0 == g_ascii_strcasecmp (key, "xesam:album"))
			mpris2_metadata_set_album (metadata, g_variant_get_string(value, NULL));
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
			mpris2_metadata_set_track_no(metadata, g_variant_get_int32 (value));
		else if (0 == g_ascii_strcasecmp (key, "xesam:useCount"));
			/* (Integer) Not use useCount */
		else if (0 == g_ascii_strcasecmp (key, "xesam:userRating"));
			/* (Float) Not use userRating */
		else if (0 == g_ascii_strcasecmp (key, "mpris:artUrl"))
			mpris2_metadata_set_arturl (metadata, g_variant_get_string(value, NULL));
		else if (0 == g_ascii_strcasecmp (key, "xesam:contentCreated"));
			/* has type 's' */
		else if (0 == g_ascii_strcasecmp (key, "audio-bitrate"));
			/* has type 'i' */
		else if (0 == g_ascii_strcasecmp (key, "audio-channels"));
			/* has type 'i' */
		else if (0 == g_ascii_strcasecmp (key, "audio-samplerate"));
			/* has type 'i' */
		else if (0 == g_ascii_strcasecmp (key, "xesam:contentCreated"));
			/* has type 's' */
		else if (0 == g_ascii_strcasecmp (key, "audio-bitrate"));
			/* has type 'i' */
		else if (0 == g_ascii_strcasecmp (key, "audio-channels"));
			/* has type 'i' */
		else if (0 == g_ascii_strcasecmp (key, "audio-samplerate"));
			/* has type 'i'*/
		else
			g_print ("Variant '%s' has type '%s'\n", key,
				     g_variant_get_type_string (value));
	}

	mpris2_metadata_set_length (metadata, length / 1000000l);

	return metadata;
}

static void
mpris2_client_parse_playback_status (Mpris2Client *mpris2, const gchar *playback_status)
{
	if (0 == g_ascii_strcasecmp(playback_status, "Playing")) {
		mpris2->playback_status = PLAYING;
	}
	else if (0 == g_ascii_strcasecmp(playback_status, "Paused")) {
		mpris2->playback_status = PAUSED;
	}
	else {
		mpris2->playback_status = STOPPED;
	}
}

static void
mpris2_client_parse_properties (Mpris2Client *mpris2, GVariant *properties)
{
	GVariantIter iter;
	GVariant *value;
	const gchar *key;
	const gchar *playback_status = NULL;
	Mpris2Metadata *metadata = NULL;
	gdouble volume = 0;

	g_variant_iter_init (&iter, properties);

	while (g_variant_iter_loop (&iter, "{sv}", &key, &value)) {
		if (0 == g_ascii_strcasecmp (key, "PlaybackStatus")) {
			playback_status = g_variant_get_string(value, NULL);
		}
		else if (0 == g_ascii_strcasecmp (key, "Metadata")) {
			metadata = mpris2_metadata_new_from_variant (value);
		}
		else if (0 == g_ascii_strcasecmp (key, "Volume")) {
			volume = g_variant_get_double(value);
		}
	}

	if (playback_status != NULL) {
		mpris2_client_parse_playback_status (mpris2, playback_status);
		g_signal_emit (mpris2, signals[PLAYBACK_STATUS], 0);
	}

	if (metadata != NULL) {
		mpris2_metadata_free (mpris2->metadata);
		mpris2->metadata = metadata;
		g_signal_emit (mpris2, signals[METADATA], 0, metadata);
	}
	if (volume != 0) {
		mpris2->volume = volume;
		g_signal_emit (mpris2, signals[VOLUME], 0);
	}
}

static void
mpris2_client_on_dbus_signal (GDBusProxy *proxy,
                              gchar      *sender_name,
                              gchar      *signal_name,
                              GVariant   *parameters,
                              gpointer    user_data)
{
	GVariantIter iter;
	GVariant *child;

	Mpris2Client *mpris2 = user_data;

	g_variant_iter_init (&iter, parameters);

	child = g_variant_iter_next_value (&iter); /* Interface name. */
	g_variant_unref (child);

	child = g_variant_iter_next_value (&iter); /* Property name. */
	mpris2_client_parse_properties (mpris2, child);
	g_variant_unref (child);
}

/* Functions that detect when the player is connected to mpris2 */

static void
mpris2_client_connected_dbus (GDBusConnection *connection,
                              const gchar *name,
                              const gchar *name_owner,
                              gpointer user_data)
{
	GVariant *vprop;

	Mpris2Client *mpris2 = user_data;

	mpris2->connected = TRUE;

	/* Get Properties..*/
	vprop = mpris2_client_get_player_properties (mpris2, "Identity");
	mpris2->identity = g_variant_dup_string (vprop, NULL);
	g_variant_unref(vprop);

	vprop = mpris2_client_get_player_properties (mpris2, "CanRaise");
	mpris2->can_raise = g_variant_get_boolean (vprop);
	g_variant_unref(vprop);

	vprop = mpris2_client_get_player_properties (mpris2, "CanQuit");
	mpris2->can_quit = g_variant_get_boolean (vprop);
	g_variant_unref(vprop);

	vprop = mpris2_client_get_player_properties (mpris2, "HasTrackList");
	mpris2->has_tracklist = g_variant_get_boolean (vprop);
	g_variant_unref(vprop);

	vprop = mpris2_client_get_player_properties (mpris2, "SupportedUriSchemes");
	mpris2->supported_uri_schemes = g_variant_dup_strv (vprop, NULL);
	g_variant_unref(vprop);

	vprop = mpris2_client_get_player_properties (mpris2, "SupportedMimeTypes");
	mpris2->supported_mime_types = g_variant_dup_strv (vprop, NULL);
	g_variant_unref(vprop);

	g_signal_emit (mpris2, signals[CONNECTION], 0);
}

static void
mpris2_client_lose_dbus (GDBusConnection *connection,
                         const gchar *name,
                         gpointer user_data)
{
	Mpris2Client *mpris2 = user_data;

	mpris2->connected = FALSE;
	if (mpris2->identity) {
		g_free (mpris2->identity);
		mpris2->identity = NULL;
	}
	mpris2->playback_status = STOPPED;

	g_signal_emit (mpris2, signals[CONNECTION], 0);
}

static void
mpris2_client_connect_dbus (Mpris2Client *mpris2)
{
	GDBusProxy *proxy;
	GError     *gerror = NULL;
	guint       watch_id;

	g_free(mpris2->dbus_name);
	mpris2->dbus_name = g_strdup_printf("org.mpris.MediaPlayer2.%s", mpris2->player);

	watch_id = g_bus_watch_name_on_connection(mpris2->gconnection,
	                                          mpris2->dbus_name,
	                                          G_BUS_NAME_OWNER_FLAGS_REPLACE,
	                                          mpris2_client_connected_dbus,
	                                          mpris2_client_lose_dbus,
	                                          mpris2,
	                                          NULL);

	proxy = g_dbus_proxy_new_sync (mpris2->gconnection,
	                               G_DBUS_PROXY_FLAGS_NONE,
	                               NULL,
	                               mpris2->dbus_name,
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
		g_signal_connect (proxy, "g-signal",
			              G_CALLBACK (mpris2_client_on_dbus_signal), mpris2);
		mpris2->proxy = proxy;
	}

	mpris2->watch_id = watch_id;
}

static void
mpris2_client_finalize (GObject *object)
{
	Mpris2Client *mpris2 = MPRIS2_CLIENT (object);

	g_free (mpris2->player);
	g_free (mpris2->dbus_name);

	(*G_OBJECT_CLASS (mpris2_client_parent_class)->finalize) (object);
}


static void
mpris2_client_class_init (Mpris2ClientClass *klass)
{
	GObjectClass  *gobject_class;

	gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->finalize = mpris2_client_finalize;

	/*
	 * Signals:
	 */
	signals[CONNECTION] = g_signal_new ("connection",
	                                    G_TYPE_FROM_CLASS (gobject_class),
	                                    G_SIGNAL_RUN_LAST,
	                                    G_STRUCT_OFFSET (Mpris2ClientClass, connection),
	                                    NULL, NULL,
	                                    g_cclosure_marshal_VOID__VOID,
	                                    G_TYPE_NONE, 0);

	signals[PLAYBACK_STATUS] = g_signal_new ("playback-status",
	                                         G_TYPE_FROM_CLASS (gobject_class),
	                                         G_SIGNAL_RUN_LAST,
	                                         G_STRUCT_OFFSET (Mpris2ClientClass, playback_status),
	                                         NULL, NULL,
	                                         g_cclosure_marshal_VOID__VOID,
	                                         G_TYPE_NONE, 0);

	signals[METADATA] = g_signal_new ("metadata",
	                                  G_TYPE_FROM_CLASS (gobject_class),
	                                  G_SIGNAL_RUN_LAST,
	                                  G_STRUCT_OFFSET (Mpris2ClientClass, metadata),
	                                  NULL, NULL,
	                                  g_cclosure_marshal_VOID__POINTER,
	                                  G_TYPE_NONE, 1, G_TYPE_POINTER);

	signals[VOLUME] = g_signal_new ("volume",
	                                G_TYPE_FROM_CLASS (gobject_class),
	                                G_SIGNAL_RUN_LAST,
	                                G_STRUCT_OFFSET (Mpris2ClientClass, volume),
	                                NULL, NULL,
	                                g_cclosure_marshal_VOID__VOID,
	                                G_TYPE_NONE, 0);
}

static void
mpris2_client_init (Mpris2Client *mpris2)
{
	GDBusConnection *gconnection;
	GError          *gerror = NULL;

	mpris2->player    = g_strdup("unknown");
	mpris2->connected = FALSE;

	gconnection = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, &gerror);
	if (gconnection == NULL) {
		g_message ("Failed to get session bus: %s", gerror->message);
		g_error_free (gerror);
		gerror = NULL;
	}
	mpris2->gconnection = gconnection;

	mpris2_client_connect_dbus (mpris2);
}

Mpris2Client *
mpris2_client_new (void)
{
	return g_object_new(MPRIS2_TYPE_CLIENT, NULL);
}
