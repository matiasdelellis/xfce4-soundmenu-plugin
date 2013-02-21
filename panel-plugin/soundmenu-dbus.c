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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "soundmenu-plugin.h"
#include "soundmenu-dbus.h"
#include "soundmenu-mpris2.h"
#include "soundmenu-utils.h"

/*
 * Send mesages to use methods of org.mpris.MediaPlayer2.Player interfase.
 */
void
soundmenu_mpris2_send_player_message (SoundmenuPlugin *soundmenu, const char *msg)
{
	GDBusMessage *message;
	GError       *error = NULL;

	message = g_dbus_message_new_method_call (soundmenu->dbus_name,
	                                          "/org/mpris/MediaPlayer2",
	                                          "org.mpris.MediaPlayer2.Player",
	                                          msg);

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
 * Change the player volume using org.freedesktop.DBus.Properties interfase.
 */
void
soundmenu_mpris2_properties_set_volume(SoundmenuPlugin *soundmenu, gdouble volume)
{
	GVariant *reply;
	GError   *error = NULL;

	reply = g_dbus_connection_call_sync (soundmenu->gconnection,
	                                     soundmenu->dbus_name,
	                                     "/org/mpris/MediaPlayer2",
	                                     "org.freedesktop.DBus.Properties",
	                                     "Set",
	                                     g_variant_new ("(ssv)",
	                                                    "org.mpris.MediaPlayer2.Player",
	                                                    "Volume",
	                                                     g_variant_new_double(volume)),
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

/*
 * Returns the first player name that compliant to mpris2 on dbus.
 */
gchar *
soundmenu_get_mpris2_player_running(SoundmenuPlugin *soundmenu)
{
	GError *error = NULL;
	GVariant *v;
	GVariantIter *iter;
	const gchar *str = NULL;
	gchar *player = NULL;

	v = g_dbus_connection_call_sync (soundmenu->gconnection,
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

/*
 * Get all properties using org.freedesktop.DBus.Properties interface.
 */
GVariant *
soundmenu_mpris2_properties_get_all(SoundmenuPlugin *soundmenu)
{
	GVariantIter iter;
	GVariant *result, *child = NULL;

	result = g_dbus_connection_call_sync (soundmenu->gconnection,
	                                      soundmenu->dbus_name,
	                                      "/org/mpris/MediaPlayer2",
	                                      "org.freedesktop.DBus.Properties",
	                                      "GetAll",
	                                      g_variant_new ("(s)", "org.mpris.MediaPlayer2.Player"),
	                                      G_VARIANT_TYPE ("(a{sv})"),
	                                      G_DBUS_CALL_FLAGS_NONE,
	                                      -1,
	                                      NULL,
	                                      NULL);

	if(result) {
		g_variant_iter_init (&iter, result);
		child = g_variant_iter_next_value (&iter);
	}

	return child;
}

/*
 * This function intercepts the messages from the player.
 */
static void
soundmenu_mpris2_on_dbus_signal (GDBusProxy *proxy,
                                 gchar      *sender_name,
                                 gchar      *signal_name,
                                 GVariant   *parameters,
                                 gpointer    user_data)
{
	GVariantIter iter;
	GVariant *child;

	SoundmenuPlugin *soundmenu = user_data;

	g_variant_iter_init (&iter, parameters);

	child = g_variant_iter_next_value (&iter); /* Interface name. */
	g_variant_unref (child);

	child = g_variant_iter_next_value (&iter); /* Property name. */
	soundmenu_mpris2_parse_properties(soundmenu, child);
	g_variant_unref (child);
}

/*
 * Functions that detect when the player is connected to mpris2
 */
static void
soundmenu_mpris2_dbus_conected(GDBusConnection *connection,
                               const gchar *name,
                               const gchar *name_owner,
                               gpointer user_data)
{
	SoundmenuPlugin *soundmenu = user_data;

	gtk_widget_show(GTK_WIDGET(soundmenu->hvbox_buttons));
	gtk_widget_set_sensitive(GTK_WIDGET(soundmenu->hvbox_buttons), TRUE);

	if(!soundmenu->show_album_art)
		gtk_widget_hide(GTK_WIDGET(soundmenu->ev_album_art));

	soundmenu->connected = TRUE;

	soundmenu_mpris2_forse_update(soundmenu);
}

static void
soundmenu_mpris2_dbus_losed(GDBusConnection *connection,
                            const gchar *name,
                            gpointer user_data)
{
	SoundmenuPlugin *soundmenu = user_data;

	if(soundmenu->hide_controls_if_loose)
		gtk_widget_hide(GTK_WIDGET(soundmenu->hvbox_buttons));
	else
		gtk_widget_show(GTK_WIDGET(soundmenu->hvbox_buttons));
	gtk_widget_set_sensitive(GTK_WIDGET(soundmenu->hvbox_buttons), FALSE);

	gtk_widget_show(GTK_WIDGET(soundmenu->ev_album_art));
	soundmenu_album_art_set_path(soundmenu->album_art, NULL);

	soundmenu->connected = FALSE;
}

/*
 *  Basic control of gdbus functions
 */
static void
soundmenu_mpris2_disconnect_dbus(SoundmenuPlugin *soundmenu)
{
	g_bus_unwatch_name(soundmenu->watch_id);
	g_object_unref(soundmenu->proxy);
}

static void
soundmenu_mpris2_connect_dbus(SoundmenuPlugin *soundmenu)
{
	GDBusProxy *proxy;
	guint       watch_id;
	GError     *gerror = NULL;

	g_free(soundmenu->dbus_name);
	soundmenu->dbus_name = g_strdup_printf("org.mpris.MediaPlayer2.%s", soundmenu->player);

	watch_id = g_bus_watch_name_on_connection(soundmenu->gconnection,
	                                          soundmenu->dbus_name,
	                                          G_BUS_NAME_OWNER_FLAGS_REPLACE,
	                                          soundmenu_mpris2_dbus_conected,
	                                          soundmenu_mpris2_dbus_losed,
	                                          soundmenu,
	                                          NULL);
	soundmenu->watch_id = watch_id;

	proxy = g_dbus_proxy_new_sync (soundmenu->gconnection,
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
		soundmenu->proxy = proxy;
	}
}

void
soundmenu_mpris2_reinit_dbus(SoundmenuPlugin *soundmenu)
{
	soundmenu_mpris2_disconnect_dbus(soundmenu);
	soundmenu_mpris2_connect_dbus(soundmenu);
}

void
init_dbus_session (SoundmenuPlugin *soundmenu)
{
	GDBusConnection *gconnection;
	GError          *gerror = NULL;

	/* Init gdbus connection. */

	gconnection = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, &gerror);
	if (gconnection == NULL) {
		g_message ("Failed to get session bus: %s", gerror->message);
		g_error_free (gerror);
		gerror = NULL;
	}
	soundmenu->gconnection = gconnection;

	soundmenu_mpris2_connect_dbus(soundmenu);
}