/*  $Id$
 *
 *  Copyright (c) 2011 John Doo <john@foo.org>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "soundmenu-plugin.h"

#define WAIT_UPDATE 5

#ifdef HAVE_LIBCLASTFM
void *do_lastfm_love (gpointer data)
{
	gint rv;

	SoundmenuPlugin *soundmenu = data;

	rv = LASTFM_track_love (soundmenu->clastfm->session_id,
		soundmenu->metadata->title,
		soundmenu->metadata->artist);

	if (rv != 0) {
		g_critical("Love song on Last.fm failed");
	}

	return NULL;
}

void lastfm_track_love_action (GtkWidget *widget, SoundmenuPlugin *soundmenu)
{
	pthread_t tid;

	if(soundmenu->state == ST_STOPPED)
		return;

	if (soundmenu->clastfm->session_id == NULL) {
		g_critical("No connection Last.fm has been established.");
		return;
	}

	pthread_create(&tid, NULL, do_lastfm_love, soundmenu);
}

void *do_lastfm_unlove (gpointer data)
{
	gint rv;

	SoundmenuPlugin *soundmenu = data;

	rv = LASTFM_track_love (soundmenu->clastfm->session_id,
		soundmenu->metadata->title,
		soundmenu->metadata->artist);

	if (rv != 0) {
		g_critical("Unlove song on Last.fm failed");
	}

	return NULL;
}

void lastfm_track_unlove_action (GtkWidget *widget, SoundmenuPlugin *soundmenu)
{
	pthread_t tid;

	if(soundmenu->state == ST_STOPPED)
		return;

	if (soundmenu->clastfm->session_id == NULL) {
		g_critical("No connection Last.fm has been established.");
		return;
	}

	pthread_create(&tid, NULL, do_lastfm_unlove, soundmenu);
}

void *do_lastfm_scrob (gpointer data)
{
	gint rv;

	SoundmenuPlugin *soundmenu = data;

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

	return NULL;
}

gboolean lastfm_scrob_handler(gpointer data)
{
	pthread_t tid;

	SoundmenuPlugin *soundmenu = data;

	if(soundmenu->state == ST_STOPPED)
		return FALSE;

	if (soundmenu->clastfm->session_id == NULL) {
		g_critical("No connection Last.fm has been established.");
		return FALSE;
	}

	pthread_create(&tid, NULL, do_lastfm_scrob, soundmenu);

	return FALSE;
}

void *do_lastfm_now_playing (gpointer data)
{
	gint rv;

	SoundmenuPlugin *soundmenu = data;

	LASTFM_ALBUM_INFO *album = NULL;

	rv = LASTFM_track_update_now_playing (soundmenu->clastfm->session_id,
		soundmenu->metadata->title,
		soundmenu->metadata->album,
		soundmenu->metadata->artist,
		soundmenu->metadata->length,
		soundmenu->metadata->trackNumber,
		0);

	if (rv != 0) {
		g_critical("Update current song on Last.fm failed");
	}
	return NULL;
}

gboolean lastfm_now_playing_handler (gpointer data)
{
	pthread_t tid;
	int length = 0;

	SoundmenuPlugin *soundmenu = data;

	if(soundmenu->state == ST_STOPPED)
		return FALSE;

	if (soundmenu->clastfm->session_id == NULL) {
		g_critical("No connection Last.fm has been established.");
		return FALSE;
	}

	if ((strlen(soundmenu->metadata->artist) == 0) ||
	    (strlen(soundmenu->metadata->title) == 0))
		return FALSE;

	/* Firt update now playing on lastfm */
	pthread_create(&tid, NULL, do_lastfm_now_playing, soundmenu);

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

void do_init_lastfm (SoundmenuPlugin *soundmenu)
{
	gint rv;

	soundmenu->clastfm->session_id = LASTFM_init(LASTFM_API_KEY, LASTFM_SECRET);

	if (soundmenu->clastfm->session_id != NULL) {
		if((strlen(soundmenu->clastfm->lastfm_user) != 0) &&
		   (strlen(soundmenu->clastfm->lastfm_pass) != 0)) {
			rv = LASTFM_login (soundmenu->clastfm->session_id,
					   soundmenu->clastfm->lastfm_user,
					   soundmenu->clastfm->lastfm_pass);

			if(rv != LASTFM_STATUS_OK) {
				LASTFM_dinit(soundmenu->clastfm->session_id);
				soundmenu->clastfm->session_id = NULL;

				g_critical("Unable to login on Lastfm");
			}
		}
	}
	else {
		g_critical("Failure to init libclastfm");
	}
}

/* When just init the soundmenu plugin init lastfm with a timeuout of 30 sec. */

gboolean do_init_lastfm_idle_timeout (gpointer data)
{
	SoundmenuPlugin *soundmenu = data;

	do_init_lastfm(soundmenu);

	return FALSE;
}

gint init_lastfm_idle_timeout(SoundmenuPlugin *soundmenu)
{
	if (soundmenu->clastfm->lastfm_support)
		gdk_threads_add_timeout_seconds_full(
					G_PRIORITY_DEFAULT_IDLE, 30,
					do_init_lastfm_idle_timeout, soundmenu, NULL);

	return 0;
}

/* Init lastfm with a simple thread when change preferences */

void *do_init_lastfm_idle (gpointer data)
{
	SoundmenuPlugin *soundmenu = data;

	do_init_lastfm(soundmenu);

	return NULL;
}

gint just_init_lastfm (SoundmenuPlugin *soundmenu)
{
	pthread_t tid;

	if (soundmenu->clastfm->lastfm_support)
		pthread_create (&tid, NULL, do_init_lastfm_idle, soundmenu);

	return 0;
}

#endif
