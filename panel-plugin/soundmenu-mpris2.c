/*
 *  Copyright (c) 2011-2013 matias <mati86dl@gmail.com>
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

#include "soundmenu-mpris2.h"
#include "soundmenu-album-art.h"
#include "soundmenu-dbus.h"
#include "soundmenu-lastfm.h"
#include "soundmenu-utils.h"

#ifdef HAVE_LIBCLASTFM
#include "soundmenu-lastfm.h"
#endif

#include "soundmenu-plugin.h"

SoundmenuMetadata *
soundmenu_mpris2_get_metadata (GVariant *dictionary)
{
	GVariantIter iter;
	GVariant *value;
	gchar *key;

	gint64 length = 0;

	SoundmenuMetadata *metadata;

	metadata = soundmenu_metadata_new();

	g_variant_iter_init (&iter, dictionary);
	while (g_variant_iter_loop (&iter, "{sv}", &key, &value)) {
		if (0 == g_ascii_strcasecmp (key, "mpris:trackid"))
			soundmenu_metatada_set_trackid(metadata, g_variant_get_string(value, NULL));
		else if (0 == g_ascii_strcasecmp (key, "xesam:url"))
			soundmenu_metatada_set_url(metadata, g_variant_get_string(value, NULL));
		else if (0 == g_ascii_strcasecmp (key, "xesam:title"))
			soundmenu_metatada_set_title(metadata, g_variant_get_string(value, NULL));
		else if (0 == g_ascii_strcasecmp (key, "xesam:artist"))
			soundmenu_metatada_set_artist(metadata, g_avariant_get_string(value));
		else if (0 == g_ascii_strcasecmp (key, "xesam:album"))
			soundmenu_metatada_set_album(metadata, g_variant_get_string(value, NULL));
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
			soundmenu_metatada_set_track_no(metadata, g_variant_get_int32 (value));
		else if (0 == g_ascii_strcasecmp (key, "xesam:useCount"));
			/* (Integer) Not use useCount */
		else if (0 == g_ascii_strcasecmp (key, "xesam:userRating"));
			/* (Float) Not use userRating */
		else if (0 == g_ascii_strcasecmp (key, "mpris:artUrl"))
			soundmenu_metatada_set_arturl(metadata, g_variant_get_string(value, NULL));
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

	soundmenu_metatada_set_length(metadata, length / 1000000l);

	return metadata;
}

void
soundmenu_mpris2_parse_properties(SoundmenuPlugin *soundmenu, GVariant *properties)
{
	GVariantIter iter;
	GVariant *value;
	const gchar *key;
	const gchar *state = NULL, *loop_status = NULL;
	gboolean shuffle;
	gdouble volume = 0;
	SoundmenuMetadata *metadata;

	shuffle = soundmenu->shuffle;

	g_variant_iter_init (&iter, properties);
	while (g_variant_iter_loop (&iter, "{sv}", &key, &value)) {
		if (0 == g_ascii_strcasecmp (key, "PlaybackStatus"))
		{
			state = g_variant_get_string(value, NULL);
		}
		else if (0 == g_ascii_strcasecmp (key, "LoopStatus"))
		{
			loop_status = g_variant_get_string(value, NULL);
		}
		else if (0 == g_ascii_strcasecmp (key, "Shuffle"))
		{
			shuffle = g_variant_get_boolean(value);
		}
		else if (0 == g_ascii_strcasecmp (key, "Volume"))
		{
			volume = g_variant_get_double(value);
			soundmenu->volume = volume;
		}
		else if (0 == g_ascii_strcasecmp (key, "Metadata"))
		{
			metadata = soundmenu_mpris2_get_metadata (value);

			soundmenu_mutex_lock(soundmenu->metadata_mtx);
			soundmenu_metadata_free(soundmenu->metadata);
			soundmenu->metadata = metadata;
			soundmenu_mutex_unlock(soundmenu->metadata_mtx);

			soundmenu_album_art_set_path(soundmenu->album_art,
				soundmenu_metatada_get_arturl(soundmenu->metadata));

			#ifdef HAVE_LIBCLASTFM
			if (soundmenu_lastfm_is_supported (soundmenu->clastfm))
				update_lastfm(soundmenu);
			#endif
		}
	}
	if (state != NULL)
		soundmenu_update_state (state, soundmenu);
	if (loop_status != NULL)
		soundmenu_update_loop_status (soundmenu, loop_status);
	if (shuffle != soundmenu->shuffle)
		soundmenu_update_shuffle (soundmenu, shuffle);
}

void
soundmenu_mpris2_forse_update(SoundmenuPlugin *soundmenu)
{
	GVariant *reply = NULL;

	if(!soundmenu->connected)
		return;

	reply = soundmenu_mpris2_properties_get_all(soundmenu);
	soundmenu_mpris2_parse_properties(soundmenu, reply);
	g_variant_unref (reply);
}

/*
 *  Callbacks of button controls
 */

void
prev_button_handler(GtkButton *button, SoundmenuPlugin *soundmenu)
{
	if(!soundmenu->connected)
		return;

	soundmenu_mpris2_send_player_message (soundmenu, "Previous");
}

void
play_button_handler(GtkButton *button, SoundmenuPlugin *soundmenu)
{
	if(!soundmenu->connected) {
		soundmenu_launch_player (soundmenu->player);
		return;
	}

	soundmenu_mpris2_send_player_message (soundmenu, "PlayPause");
}

void
stop_button_handler(GtkButton *button, SoundmenuPlugin    *soundmenu)
{
	if(!soundmenu->connected)
		return;

	soundmenu_mpris2_send_player_message (soundmenu, "Stop");
}

void
next_button_handler(GtkButton *button, SoundmenuPlugin    *soundmenu)
{
	if(!soundmenu->connected)
		return;

	soundmenu_mpris2_send_player_message (soundmenu, "Next");
}

gboolean
soundmenu_panel_button_scrolled (GtkWidget        *widget,
                                 GdkEventScroll   *event,
                                 SoundmenuPlugin *soundmenu)
{
	if(!soundmenu->connected)
		return FALSE;

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
