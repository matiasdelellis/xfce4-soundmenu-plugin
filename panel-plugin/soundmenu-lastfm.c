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

#include "soundmenu-plugin.h"
#include "soundmenu-dbus.h"
#include "soundmenu-dialogs.h"
#include "soundmenu-lastfm.h"
#include "soundmenu-mpris2.h"
#include "soundmenu-utils.h"
#include "soundmenu-related.h"

#define WAIT_UPDATE 5

#ifdef HAVE_LIBCLASTFM
static gpointer
do_lastfm_love (gpointer data)
{
	gint rv;

	SoundmenuPlugin *soundmenu = data;

	rv = LASTFM_track_love (soundmenu->clastfm->session_id,
	                        (gchar *)soundmenu_metatada_get_title(soundmenu->metadata),
	                        (gchar *)soundmenu_metatada_get_artist(soundmenu->metadata));

	if (rv != 0) {
		g_critical("Love song on Last.fm failed");
	}
	return NULL;
}

void lastfm_track_love_action (GtkWidget *widget, SoundmenuPlugin *soundmenu)
{
	if(soundmenu->state == ST_STOPPED)
		return;

	if (soundmenu->clastfm->session_id == NULL) {
		g_critical("No connection Last.fm has been established.");
		return;
	}

	if (g_str_empty0(soundmenu_metatada_get_artist(soundmenu->metadata)) ||
	    g_str_empty0(soundmenu_metatada_get_title(soundmenu->metadata)))
		return;

	#if GLIB_CHECK_VERSION(2,31,0)
	g_thread_new("Unlove", do_lastfm_love, soundmenu);
	#else
	g_thread_create(do_lastfm_love, soundmenu, FALSE, NULL);
	#endif
}

static gpointer
do_lastfm_unlove (gpointer data)
{
	gint rv;

	SoundmenuPlugin *soundmenu = data;

	rv = LASTFM_track_love (soundmenu->clastfm->session_id,
							soundmenu_metatada_get_title(soundmenu->metadata),
							soundmenu_metatada_get_artist(soundmenu->metadata));

	if (rv != 0) {
		g_critical("Unlove song on Last.fm failed");
	}

	return NULL;
}

void lastfm_track_unlove_action (GtkWidget *widget, SoundmenuPlugin *soundmenu)
{
	if(soundmenu->state == ST_STOPPED)
		return;

	if (soundmenu->clastfm->session_id == NULL) {
		g_critical("No connection Last.fm has been established.");
		return;
	}
	if (g_str_empty0(soundmenu_metatada_get_artist(soundmenu->metadata)) ||
	    g_str_empty0(soundmenu_metatada_get_title(soundmenu->metadata)))
		return;

    #if GLIB_CHECK_VERSION(2,31,0)
    g_thread_new("Unlove", do_lastfm_unlove, soundmenu);
    #else
    g_thread_create(do_lastfm_unlove, soundmenu, FALSE, NULL);
    #endif
}

static gpointer
do_lastfm_scrob (gpointer data)
{
    gint rv;
    SoundmenuPlugin *soundmenu = data;

    rv = LASTFM_track_scrobble(soundmenu->clastfm->session_id,
                               (gchar *)soundmenu_metatada_get_title(soundmenu->metadata),
                               soundmenu_metatada_get_album(soundmenu->metadata) ?
                               (gchar *)soundmenu_metatada_get_album(soundmenu->metadata) : "",
                               (gchar *)soundmenu_metatada_get_artist(soundmenu->metadata),
                               soundmenu->clastfm->playback_started,
                               soundmenu_metatada_get_length(soundmenu->metadata),
                               soundmenu_metatada_get_track_no(soundmenu->metadata),
                               0, NULL);

    if (rv != 0)
        g_critical("Last.fm submission failed");

    return NULL;
}

gboolean lastfm_scrob_handler(gpointer data)
{
	SoundmenuPlugin *soundmenu = data;

	if(soundmenu->state == ST_STOPPED)
		return FALSE;

	if (soundmenu->clastfm->status != LASTFM_STATUS_OK) {
		g_critical("No connection Last.fm has been established.");
		return FALSE;
	}

	#if GLIB_CHECK_VERSION(2,31,0)
	g_thread_new("Scroble", do_lastfm_scrob, soundmenu);
	#else
	g_thread_create(do_lastfm_scrob, soundmenu, FALSE, NULL);
	#endif

	return FALSE;
}

static gpointer
do_lastfm_now_playing (gpointer data)
{
	gint rv;

	SoundmenuPlugin *soundmenu = data;

	rv = LASTFM_track_update_now_playing (soundmenu->clastfm->session_id,
	                                      (gchar *)soundmenu_metatada_get_title(soundmenu->metadata),
	                                      soundmenu_metatada_get_album(soundmenu->metadata) ?
	                                      (gchar *)soundmenu_metatada_get_album(soundmenu->metadata) : "",
	                                      (gchar *)soundmenu_metatada_get_artist(soundmenu->metadata),
	                                      soundmenu_metatada_get_length(soundmenu->metadata),
	                                      soundmenu_metatada_get_track_no(soundmenu->metadata),
	                                      0, NULL);

	if (rv != 0) {
		g_critical("Update current song on Last.fm failed");
	}

	return NULL;
}

gboolean lastfm_now_playing_handler (gpointer data)
{
	gint length, time = 0;

	SoundmenuPlugin *soundmenu = data;

	if(soundmenu->state == ST_STOPPED)
		return FALSE;

	if (soundmenu->clastfm->session_id == NULL) {
		g_critical("No connection Last.fm has been established.");
		return FALSE;
	}

	if (g_str_empty0(soundmenu_metatada_get_artist(soundmenu->metadata)) ||
	    g_str_empty0(soundmenu_metatada_get_album(soundmenu->metadata)))
		return FALSE;

	/* Firt update now playing on lastfm */
	#if GLIB_CHECK_VERSION(2,31,0)
	g_thread_new("Lfm Now playing", do_lastfm_now_playing, soundmenu);
	#else
	g_thread_create(do_lastfm_now_playing, soundmenu, FALSE, NULL);
	#endif

	/* Kick the lastfm scrobbler on
	 * Note: Only scrob if tracks is more than 30s.
	 * and scrob when track is at 50% or 4mins, whichever comes
	 * first */
	length = soundmenu_metatada_get_length(soundmenu->metadata);
	if(length < 30) {
		if(length == 0)
			g_critical("The player no emit the length of track");
		return FALSE;
	}
	if((length / 2) > (240 - WAIT_UPDATE)) {
		time = 240 - WAIT_UPDATE;
	}
	else {
		time = (length / 2) - WAIT_UPDATE;
	}

    soundmenu->clastfm->lastfm_handler_id =
        g_timeout_add_seconds_full(G_PRIORITY_DEFAULT_IDLE,
                                   time,
                                   lastfm_scrob_handler,
                                   soundmenu,
                                   NULL);
	return FALSE;
}

void update_lastfm (SoundmenuPlugin *soundmenu)
{
    if(soundmenu->clastfm->lastfm_handler_id)
        g_source_remove(soundmenu->clastfm->lastfm_handler_id);

    if(soundmenu->state != ST_PLAYING)
        return;

    time(&soundmenu->clastfm->playback_started);

    soundmenu->clastfm->lastfm_handler_id =
	    g_timeout_add_seconds_full(G_PRIORITY_DEFAULT_IDLE,
	                               WAIT_UPDATE,
	                               lastfm_now_playing_handler,
	                               soundmenu,
	                               NULL);
}

void soundmenu_update_lastfm_menu (struct con_lastfm *clastfm)
{
	gtk_widget_set_sensitive(clastfm->lastfm_menu, (clastfm->status == LASTFM_STATUS_OK));
}

gboolean
do_soundmenu_init_lastfm(gpointer data)
{
    struct con_lastfm *clastfm = data;

    clastfm->session_id = LASTFM_init(LASTFM_API_KEY, LASTFM_SECRET);

    if (clastfm->session_id != NULL) {
		if((strlen(clastfm->lastfm_user) != 0) &&
		   (strlen(clastfm->lastfm_pass) != 0)) {
            clastfm->status = LASTFM_login (clastfm->session_id,
                                            clastfm->lastfm_user,
                                            clastfm->lastfm_pass);

			if(clastfm->status != LASTFM_STATUS_OK) {
                LASTFM_dinit(clastfm->session_id);
                clastfm->session_id = NULL;
            }
        }
    }

	soundmenu_update_lastfm_menu(clastfm);

    return FALSE;
}

gint
soundmenu_init_lastfm(SoundmenuPlugin *soundmenu)
{
    /* Test internet and launch threads.*/
#if GLIB_CHECK_VERSION(2,32,0)
    if (g_network_monitor_get_network_available (g_network_monitor_get_default ()))
#else
    if(nm_is_online () == TRUE)
#endif
        g_idle_add(do_soundmenu_init_lastfm,
                   soundmenu->clastfm);
    else
        g_timeout_add_seconds_full(G_PRIORITY_DEFAULT_IDLE,
                                   30,
                                   do_soundmenu_init_lastfm,
                                   soundmenu->clastfm,
                                   NULL);

    return 0;
}
#endif
