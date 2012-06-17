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
#include "soundmenu-dbus.h"
#include "soundmenu-dialogs.h"
#include "soundmenu-lastfm.h"
#include "soundmenu-mpris2.h"
#include "soundmenu-utils.h"
#include "soundmenu-related.h"

#define WAIT_UPDATE 5

#ifdef HAVE_LIBCLASTFM
gboolean do_lastfm_love (gpointer data)
{
	gint rv;

	SoundmenuPlugin *soundmenu = data;

	rv = LASTFM_track_love (soundmenu->clastfm->session_id,
		soundmenu->metadata->title,
		soundmenu->metadata->artist);

	if (rv != 0) {
		g_critical("Love song on Last.fm failed");
	}

	return FALSE;
}

void lastfm_track_love_action (GtkWidget *widget, SoundmenuPlugin *soundmenu)
{
	if(soundmenu->state == ST_STOPPED)
		return;

	if (soundmenu->clastfm->session_id == NULL) {
		g_critical("No connection Last.fm has been established.");
		return;
	}

	gdk_threads_add_idle(do_lastfm_love, soundmenu);
}

gboolean do_lastfm_unlove (gpointer data)
{
	gint rv;

	SoundmenuPlugin *soundmenu = data;

	rv = LASTFM_track_love (soundmenu->clastfm->session_id,
		soundmenu->metadata->title,
		soundmenu->metadata->artist);

	if (rv != 0) {
		g_critical("Unlove song on Last.fm failed");
	}

	return FALSE;
}

void lastfm_track_unlove_action (GtkWidget *widget, SoundmenuPlugin *soundmenu)
{
	if(soundmenu->state == ST_STOPPED)
		return;

	if (soundmenu->clastfm->session_id == NULL) {
		g_critical("No connection Last.fm has been established.");
		return;
	}

	gdk_threads_add_idle(do_lastfm_unlove, soundmenu);
}

gboolean lastfm_scrob_handler(gpointer data)
{
	gint rv;

	SoundmenuPlugin *soundmenu = data;

	if(soundmenu->state == ST_STOPPED)
		return FALSE;

	if (soundmenu->clastfm->session_id == NULL) {
		g_critical("No connection Last.fm has been established.");
		return FALSE;
	}

	rv = LASTFM_track_scrobble (soundmenu->clastfm->session_id,
		soundmenu->metadata->title,
		soundmenu->metadata->album,
		soundmenu->metadata->artist,
		soundmenu->clastfm->playback_started,
		soundmenu->metadata->length,
		soundmenu->metadata->trackNumber,
		0, NULL);

	if (rv != 0)
		g_critical("Last.fm submission failed");

	return FALSE;
}

gboolean do_lastfm_now_playing (gpointer data)
{
	gint rv;

	SoundmenuPlugin *soundmenu = data;

	rv = LASTFM_track_update_now_playing (soundmenu->clastfm->session_id,
		soundmenu->metadata->title,
		soundmenu->metadata->album,
		soundmenu->metadata->artist,
		soundmenu->metadata->length,
		soundmenu->metadata->trackNumber,
		0, NULL);

	if (rv != 0) {
		g_critical("Update current song on Last.fm failed");
	}
	return FALSE;
}

gboolean lastfm_now_playing_handler (gpointer data)
{
	int length = 0;

	SoundmenuPlugin *soundmenu = data;

	if(soundmenu->state == ST_STOPPED)
		return FALSE;

	if (soundmenu->clastfm->session_id == NULL) {
		g_critical("No connection Last.fm has been established.");
		return FALSE;
	}

	if ((soundmenu->metadata->artist == NULL) || (soundmenu->metadata->title == NULL) ||
	    (strlen(soundmenu->metadata->artist) == 0) || (strlen(soundmenu->metadata->title) == 0))
		return FALSE;

	/* Firt update now playing on lastfm */
	gdk_threads_add_idle(do_lastfm_now_playing, soundmenu);

	/* Kick the lastfm scrobbler on
	 * Note: Only scrob if tracks is more than 30s.
	 * and scrob when track is at 50% or 4mins, whichever comes
	 * first */
	if(soundmenu->metadata->length < 30) {
		if(soundmenu->metadata->length == 0)
			g_critical("The player no emit the length of track");
		return FALSE;
	}
	if((soundmenu->metadata->length / 2) > (240 - WAIT_UPDATE)) {
		length = 240 - WAIT_UPDATE;
	}
	else {
		length = (soundmenu->metadata->length / 2) - WAIT_UPDATE;
	}

	soundmenu->clastfm->lastfm_handler_id = gdk_threads_add_timeout_seconds_full(
			G_PRIORITY_DEFAULT_IDLE, length,
			lastfm_scrob_handler, soundmenu, NULL);

	return FALSE;
}

void update_lastfm (SoundmenuPlugin *soundmenu)
{
	if(soundmenu->clastfm->lastfm_handler_id)
		g_source_remove(soundmenu->clastfm->lastfm_handler_id);

	if(soundmenu->state != ST_PLAYING)
		return;

	time(&soundmenu->clastfm->playback_started);

	soundmenu->clastfm->lastfm_handler_id = gdk_threads_add_timeout_seconds_full(
			G_PRIORITY_DEFAULT_IDLE, WAIT_UPDATE,
			lastfm_now_playing_handler, soundmenu, NULL);
}

/* When just run soundmenu init lastfm with a timeuout of 30 sec. */

gboolean do_init_lastfm_idle (gpointer data)
{
	gint rv;

	struct con_lastfm *clastfm = data;

	clastfm->session_id = LASTFM_init(LASTFM_API_KEY, LASTFM_SECRET);

	if (clastfm->session_id != NULL) {
		if((strlen(clastfm->lastfm_user) != 0) &&
		   (strlen(clastfm->lastfm_pass) != 0)) {
			rv = LASTFM_login (clastfm->session_id,
					   clastfm->lastfm_user,
					   clastfm->lastfm_pass);

			if(rv != LASTFM_STATUS_OK) {
				LASTFM_dinit(clastfm->session_id);
				clastfm->session_id = NULL;

				g_critical("Unable to login on Lastfm");
			}
		}
	}
	else {
		g_critical("Failure to init libclastfm");
	}

	return FALSE;
}

/* When just init the soundmenu plugin init lastfm with a timeuout of 30 sec. */

gint init_lastfm_idle_timeout(SoundmenuPlugin *soundmenu)
{
	if (soundmenu->clastfm->lastfm_support)
		gdk_threads_add_timeout_seconds_full(
					G_PRIORITY_DEFAULT_IDLE, 30,
					do_init_lastfm_idle, soundmenu->clastfm, NULL);

	return 0;
}

/* Init lastfm with a simple thread when change preferences or init plugin with internet online. */

gint just_init_lastfm (SoundmenuPlugin *soundmenu)
{
	if (soundmenu->clastfm->lastfm_support)
		gdk_threads_add_idle (do_init_lastfm_idle, soundmenu->clastfm);

	return 0;
}

#endif
