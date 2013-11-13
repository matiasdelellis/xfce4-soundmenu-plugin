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
	gchar			*dbus_name;
	guint            watch_id;

	/* Conf. */
	gchar           *player;

	/* Status */
	gboolean         connected;
};

enum
{
	CONNECTION,
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
mpris2_control_set_player (Mpris2Control *mpris2, const gchar *player)
{
	g_free(mpris2->player);
	if (player)
		mpris2->player = g_strdup(player);
	else
		mpris2->player = g_strdup("unknown");

	/* Disconnect dbus */
	g_bus_unwatch_name (mpris2->watch_id);

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

/* Functions that detect when the player is connected to mpris2 */

static void
mpris2_control_connected_dbus (GDBusConnection *connection,
                               const gchar *name,
                               const gchar *name_owner,
                               gpointer user_data)
{
	Mpris2Control *mpris2 = user_data;

	mpris2->connected = TRUE;
	g_signal_emit (mpris2, signals[CONNECTION], 0);
}

static void
mpris2_control_lose_dbus (GDBusConnection *connection,
                          const gchar *name,
                          gpointer user_data)
{
	Mpris2Control *mpris2 = user_data;

	mpris2->connected = FALSE;
	g_signal_emit (mpris2, signals[CONNECTION], 0);
}

static void
mpris2_control_connect_dbus (Mpris2Control *mpris2)
{
	guint watch_id;

	g_free(mpris2->dbus_name);
	mpris2->dbus_name = g_strdup_printf("org.mpris.MediaPlayer2.%s", mpris2->player);

	watch_id = g_bus_watch_name_on_connection(mpris2->gconnection,
	                                          mpris2->dbus_name,
	                                          G_BUS_NAME_OWNER_FLAGS_REPLACE,
	                                          mpris2_control_connected_dbus,
	                                          mpris2_control_lose_dbus,
	                                          mpris2,
	                                          NULL);
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
