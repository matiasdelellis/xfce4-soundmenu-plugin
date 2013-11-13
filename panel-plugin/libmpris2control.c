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

#include "libmpris2control.h"

struct _Mpris2Control
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
};

enum
{
	CONNECTION,
	PLAYBACK_STATUS,
	LAST_SIGNAL
};
static int signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (Mpris2Control, mpris2_control, G_TYPE_OBJECT)

/*
 * Prototypes
 */
static void   mpris2_control_call_method  (Mpris2Control *mpris2, const char *method);
static gchar *mpris2_control_get_player   (Mpris2Control *mpris2);
static void   mpris2_control_connect_dbus (Mpris2Control *mpris2);

GVariant     *mpris2_control_get_player_properties (Mpris2Control *mpris2, const gchar *prop);

/*
 *  Basic control of gdbus functions
 */

void
mpris2_control_play_pause (Mpris2Control *mpris2)
{
	if (!mpris2->connected)
		return;

	mpris2_control_call_method (mpris2, "PlayPause");
}

void
mpris2_control_stop (Mpris2Control *mpris2)
{
	if (!mpris2->connected)
		return;

	mpris2_control_call_method (mpris2, "Stop");
}

void
mpris2_control_prev (Mpris2Control *mpris2)
{
	if (!mpris2->connected)
		return;

	mpris2_control_call_method (mpris2, "Previous");
}

void
mpris2_control_next (Mpris2Control *mpris2)
{
	if (!mpris2->connected)
		return;

	mpris2_control_call_method (mpris2, "Next");
}

void
mpris2_control_quit_player (Mpris2Control *mpris2)
{
	if (!mpris2->can_quit)
		return;

	mpris2_control_call_method (mpris2, "Quit");
}

void
mpris2_control_raise_player (Mpris2Control *mpris2)
{
	if (!mpris2->can_raise)
		return;

	mpris2_control_call_method (mpris2, "Raise");
}

/*
 * Interface MediaPlayer2.Player properties.
 */

PlaybackStatus
mpris2_control_get_playback_status (Mpris2Control *mpris2)
{
	return mpris2->playback_status;
}

/*
 * Interface MediaPlayer2 Properties.
 */

gboolean
mpris2_control_can_quit (Mpris2Control *mpris2)
{
	return mpris2->can_quit;
}

gboolean
mpris2_control_can_raise (Mpris2Control *mpris2)
{
	return mpris2->can_raise;
}

gboolean
mpris2_control_has_tracklist_support (Mpris2Control *mpris2)
{
	return mpris2->has_tracklist;
}

const gchar *
mpris2_control_get_player_identity (Mpris2Control *mpris2)
{
	return mpris2->identity;
}

gchar **
mpris2_control_get_supported_uri_schemes (Mpris2Control *mpris2)
{
	return mpris2->supported_uri_schemes;
}

gchar **
mpris2_control_get_supported_mime_types (Mpris2Control *mpris2)
{
	return mpris2->supported_mime_types;
}

void
mpris2_control_set_player (Mpris2Control *mpris2, const gchar *player)
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
	mpris2_control_connect_dbus (mpris2);
}

gchar *
mpris2_control_auto_set_player (Mpris2Control *mpris2)
{
	gchar *player = mpris2_control_get_player (mpris2);

	mpris2_control_set_player (mpris2, player);

	return player;
}

gboolean
mpris2_control_is_connected (Mpris2Control *mpris2)
{
	return mpris2->connected;
}

/*
 * SoundmenuDbus.
 */

/* Send mesages to use methods of org.mpris.MediaPlayer2.Player interfase. */

static void
mpris2_control_call_method (Mpris2Control *mpris2, const char *method)
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

/* Returns the first player name that compliant to mpris2 on dbus.  */

static gchar *
mpris2_control_get_player (Mpris2Control *mpris2)
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

GVariant *
mpris2_control_get_player_properties (Mpris2Control *mpris2, const gchar *prop)
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

/* These function intercepts the messages from the player. */

static void
mpris2_control_parse_properties (Mpris2Control *mpris2, GVariant *properties)
{
	GVariantIter iter;
	GVariant *value;
	const gchar *key;
	const gchar *playback_status = NULL;

	g_variant_iter_init (&iter, properties);

	while (g_variant_iter_loop (&iter, "{sv}", &key, &value)) {
		if (0 == g_ascii_strcasecmp (key, "PlaybackStatus")) {
			playback_status = g_variant_get_string(value, NULL);
		}
	}

	if (playback_status != NULL) {
		if (0 == g_ascii_strcasecmp(playback_status, "Playing")) {
			mpris2->playback_status = PLAYING;
		}
		else if (0 == g_ascii_strcasecmp(playback_status, "Paused")) {
			mpris2->playback_status = PAUSED;
		}
		else {
			mpris2->playback_status = STOPPED;
		}
		g_signal_emit (mpris2, signals[PLAYBACK_STATUS], 0);
	}
}

static void
mpris2_control_on_dbus_signal (GDBusProxy *proxy,
                               gchar      *sender_name,
                               gchar      *signal_name,
                               GVariant   *parameters,
                               gpointer    user_data)
{
	GVariantIter iter;
	GVariant *child;

	Mpris2Control *mpris2 = user_data;

	g_variant_iter_init (&iter, parameters);

	child = g_variant_iter_next_value (&iter); /* Interface name. */
	g_variant_unref (child);

	child = g_variant_iter_next_value (&iter); /* Property name. */
	mpris2_control_parse_properties (mpris2, child);
	g_variant_unref (child);
}

/* Functions that detect when the player is connected to mpris2 */

static void
mpris2_control_connected_dbus (GDBusConnection *connection,
                               const gchar *name,
                               const gchar *name_owner,
                               gpointer user_data)
{
	GVariant *vprop;

	Mpris2Control *mpris2 = user_data;

	mpris2->connected = TRUE;

	/* Get Properties..*/
	vprop = mpris2_control_get_player_properties (mpris2, "Identity");
	mpris2->identity = g_variant_dup_string (vprop, NULL);
	g_variant_unref(vprop);

	vprop = mpris2_control_get_player_properties (mpris2, "CanRaise");
	mpris2->can_raise = g_variant_get_boolean (vprop);
	g_variant_unref(vprop);

	vprop = mpris2_control_get_player_properties (mpris2, "CanQuit");
	mpris2->can_raise = g_variant_get_boolean (vprop);
	g_variant_unref(vprop);

	vprop = mpris2_control_get_player_properties (mpris2, "HasTrackList");
	mpris2->has_tracklist = g_variant_get_boolean (vprop);
	g_variant_unref(vprop);

	vprop = mpris2_control_get_player_properties (mpris2, "SupportedUriSchemes");
	mpris2->supported_uri_schemes = g_variant_dup_strv (vprop, NULL);
	g_variant_unref(vprop);

	vprop = mpris2_control_get_player_properties (mpris2, "SupportedMimeTypes");
	mpris2->supported_mime_types = g_variant_dup_strv (vprop, NULL);
	g_variant_unref(vprop);

	g_signal_emit (mpris2, signals[CONNECTION], 0);
}

static void
mpris2_control_lose_dbus (GDBusConnection *connection,
                          const gchar *name,
                          gpointer user_data)
{
	Mpris2Control *mpris2 = user_data;

	mpris2->connected = FALSE;
	if (mpris2->identity) {
		g_free (mpris2->identity);
		mpris2->identity = NULL;
	}
	mpris2->playback_status = STOPPED;

	g_signal_emit (mpris2, signals[CONNECTION], 0);
}

static void
mpris2_control_connect_dbus (Mpris2Control *mpris2)
{
	GDBusProxy *proxy;
	GError     *gerror = NULL;
	guint       watch_id;

	g_free(mpris2->dbus_name);
	mpris2->dbus_name = g_strdup_printf("org.mpris.MediaPlayer2.%s", mpris2->player);

	watch_id = g_bus_watch_name_on_connection(mpris2->gconnection,
	                                          mpris2->dbus_name,
	                                          G_BUS_NAME_OWNER_FLAGS_REPLACE,
	                                          mpris2_control_connected_dbus,
	                                          mpris2_control_lose_dbus,
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
			              G_CALLBACK (mpris2_control_on_dbus_signal), mpris2);
		mpris2->proxy = proxy;
	}

	mpris2->watch_id = watch_id;
}

static void
mpris2_control_finalize (GObject *object)
{
	Mpris2Control *mpris2 = MPRIS2_CONTROL (object);

	g_free (mpris2->player);
	g_free (mpris2->dbus_name);

	(*G_OBJECT_CLASS (mpris2_control_parent_class)->finalize) (object);
}


static void
mpris2_control_class_init (Mpris2ControlClass *klass)
{
	GObjectClass  *gobject_class;

	gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->finalize = mpris2_control_finalize;

	/*
	 * Signals:
	 */
	signals[CONNECTION] = g_signal_new ("connection",
	                                    G_TYPE_FROM_CLASS (gobject_class),
	                                    G_SIGNAL_RUN_LAST,
	                                    G_STRUCT_OFFSET (Mpris2ControlClass, connection),
	                                    NULL, NULL,
	                                    g_cclosure_marshal_VOID__VOID,
	                                    G_TYPE_NONE, 0);

	signals[PLAYBACK_STATUS] = g_signal_new ("playback-status",
	                                    G_TYPE_FROM_CLASS (gobject_class),
	                                    G_SIGNAL_RUN_LAST,
	                                    G_STRUCT_OFFSET (Mpris2ControlClass, playback_status),
	                                    NULL, NULL,
	                                    g_cclosure_marshal_VOID__VOID,
	                                    G_TYPE_NONE, 0);
}

static void
mpris2_control_init (Mpris2Control *mpris2)
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

	mpris2_control_connect_dbus (mpris2);
}

Mpris2Control *
mpris2_control_new (void)
{
	return g_object_new(MPRIS2_TYPE_CONTROL, NULL);
}
