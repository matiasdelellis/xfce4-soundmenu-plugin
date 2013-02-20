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
	DBusConnection *connection;
	DBusError error;
	gchar *rule = NULL;

	/* Init gdbus connection. */

	gconnection = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, &gerror);
	if (gconnection == NULL) {
		g_message ("Failed to get session bus: %s", gerror->message);
		g_error_free (gerror);
	}
	soundmenu->dbus_name = g_strdup_printf("org.mpris.MediaPlayer2.%s", soundmenu->player);

	g_bus_watch_name_on_connection(gconnection,
	                               soundmenu->dbus_name,
	                               G_BUS_NAME_OWNER_FLAGS_REPLACE,
	                               soundmenu_mpris2_dbus_conected,
	                               soundmenu_mpris2_dbus_losed,
	                               soundmenu,
	                               NULL);

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