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

/* Basic metadata object */

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

Metadata *
soundmenu_mpris2_get_metadata (GVariant *dictionary)
{
	GVariantIter iter;
	GVariant *value;
	gchar *key;

	gint64 length = 0;
	gint32 trackNumber = 0;

	Metadata *metadata;

	metadata = malloc_metadata();

	g_variant_iter_init (&iter, dictionary);
	while (g_variant_iter_loop (&iter, "{sv}", &key, &value)) {
		if (0 == g_ascii_strcasecmp (key, "mpris:trackid"))
			metadata->trackid = g_variant_dup_string(value, NULL);
		else if (0 == g_ascii_strcasecmp (key, "xesam:url"))
			metadata->url= g_variant_dup_string(value, NULL);
		else if (0 == g_ascii_strcasecmp (key, "xesam:title"))
			metadata->url= g_variant_dup_string(value, NULL);
		else if (0 == g_ascii_strcasecmp (key, "xesam:artist"))
			metadata->artist = g_avariant_dup_string(value);
		else if (0 == g_ascii_strcasecmp (key, "xesam:album"))
			metadata->album = g_variant_dup_string(value, NULL);
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
			metadata->arturl= g_variant_dup_string(value, NULL);
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

	metadata->length = length / 1000000l;
	metadata->trackNumber = trackNumber;

	return metadata;
}

void
soundmenu_mpris2_parse_properties(SoundmenuPlugin *soundmenu, GVariant *properties)
{
	GVariantIter iter;
	GVariant *value;
	const gchar *key;
	gchar *state = NULL;
	gdouble volume = 0;
	Metadata *metadata;

	g_variant_iter_init (&iter, properties);
	while (g_variant_iter_loop (&iter, "{sv}", &key, &value)) {
		if (0 == g_ascii_strcasecmp (key, "PlaybackStatus"))
		{
			state = g_variant_dup_string(value, NULL);
		}
		else if (0 == g_ascii_strcasecmp (key, "Volume"))
		{
			volume = g_variant_get_double(value);
			soundmenu->volume = volume;
		}
		else if (0 == g_ascii_strcasecmp (key, "Metadata"))
		{
			metadata = soundmenu_mpris2_get_metadata (value);
			soundmenu_album_art_set_path(soundmenu->album_art, metadata->arturl);

			free_metadata(soundmenu->metadata);
			soundmenu->metadata = metadata;

			#ifdef HAVE_LIBCLASTFM
			if (soundmenu->clastfm->lastfm_support)
				update_lastfm(soundmenu);
			#endif
		}
	}
	if (state != NULL)
		soundmenu_update_state (state, soundmenu);
}

void
soundmenu_mpris2_forse_update(SoundmenuPlugin *soundmenu)
{
	GVariant *result = NULL;

	if(!soundmenu->connected)
		return;

	result = soundmenu_mpris2_properties_get_all(soundmenu);
	soundmenu_mpris2_parse_properties(soundmenu, result);
}

/*
 *  Callbacks of button controls
 */

void
prev_button_handler(GtkButton *button, SoundmenuPlugin *soundmenu)
{
	soundmenu_mpris2_send_player_message (soundmenu, "Previous");
}

void
play_button_handler(GtkButton *button, SoundmenuPlugin *soundmenu)
{
	soundmenu_mpris2_send_player_message (soundmenu, "PlayPause");
}

void
stop_button_handler(GtkButton *button, SoundmenuPlugin    *soundmenu)
{
	soundmenu_mpris2_send_player_message (soundmenu, "Stop");
}

void
next_button_handler(GtkButton *button, SoundmenuPlugin    *soundmenu)
{
	soundmenu_mpris2_send_player_message (soundmenu, "Next");
}

gboolean
soundmenu_panel_button_scrolled (GtkWidget        *widget,
                                 GdkEventScroll   *event,
                                 SoundmenuPlugin *soundmenu)
{
	switch (event->direction) {
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

	soundmenu_mpris2_properties_set_volume(soundmenu, soundmenu->volume);

	return FALSE;
}
