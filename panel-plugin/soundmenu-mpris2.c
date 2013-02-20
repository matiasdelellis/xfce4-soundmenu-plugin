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
