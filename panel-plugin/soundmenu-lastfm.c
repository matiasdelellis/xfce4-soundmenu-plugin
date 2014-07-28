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

#include <clastfm.h>
#include "soundmenu-lastfm.h"
#include "soundmenu-utils.h"
#include "soundmenu-simple-async.h"
#include "soundmenu-panel-plugin.h"

#define WAIT_UPDATE 5

#define LASTFM_API_KEY "70c479ab2632e597fd9215cf35963c1b"
#define LASTFM_SECRET  "4cb5255d955edc8f651de339fd2f335b"

struct _SoundmenuLastfm {
	SoundmenuPlugin *soundmenu;

	gboolean        lastfm_support;
	gchar          *lastfm_user;
	gchar          *lastfm_pass;

	LASTFM_SESSION *session_id;
	enum LASTFM_STATUS_CODES status;

	guint           lastfm_handler_id;
	time_t          playback_started;

	GtkWidget      *lastfm_love_item;
	GtkWidget      *lastfm_unlove_item;
};

struct _LastfmData {
	SoundmenuPlugin *soundmenu;

	gchar           *title;
	gchar           *artist;
	gchar           *album;
	gint             length;
	gint             track_no;
	time_t           started_t;
};
typedef struct _LastfmData LastfmData;

static void
soundmenu_lastfm_data_free (LastfmData *data)
{
	if (g_str_nempty0(data->title))
		g_free (data->title);
	if (g_str_nempty0(data->artist))
		g_free (data->artist);
	if (g_str_nempty0(data->album))
		g_free (data->album);

	g_slice_free(LastfmData, data);
}

static LastfmData *
soundmenu_lastfm_data_new (SoundmenuPlugin *soundmenu, Mpris2Metadata *metadata, time_t started_t)
{
	LastfmData *data = NULL;

	data = g_slice_new0(LastfmData);

	data->soundmenu = soundmenu;

	if (g_str_nempty0(mpris2_metadata_get_title(metadata)))
		data->title = g_strdup(mpris2_metadata_get_title(metadata));
	if (g_str_nempty0(mpris2_metadata_get_artist(metadata)))
		data->artist = g_strdup(mpris2_metadata_get_artist(metadata));
	if (g_str_nempty0(mpris2_metadata_get_album(metadata)))
		data->album = g_strdup(mpris2_metadata_get_album(metadata));

	data->length = mpris2_metadata_get_length(metadata);
	data->track_no = mpris2_metadata_get_track_no(metadata);

	data->started_t = started_t;

	return data;
}

static gpointer
do_lastfm_current_song_love (gpointer userdata)
{
	AsycMessageData *message_data = NULL;
	gint rv;

	LastfmData *data = userdata;
	SoundmenuPlugin *soundmenu = data->soundmenu;
	SoundmenuLastfm *lastfm = soundmenu->clastfm;

	rv = LASTFM_track_love (lastfm->session_id,
	                        data->title, data->artist);

	message_data = soundmenu_async_finished_message_new(soundmenu,
		(rv != 0) ? _("Love song on Last.fm failed.") : NULL);

	soundmenu_lastfm_data_free (data);

	return message_data;
}

void
lastfm_track_love_action (GtkWidget *widget, SoundmenuPlugin *soundmenu)
{
	Mpris2Metadata *metadata = NULL;
	SoundmenuLastfm *lastfm = NULL;
	LastfmData *data = NULL;

	if (mpris2_client_get_playback_status (soundmenu->mpris2) == STOPPED)
		return;

	lastfm = soundmenu->clastfm;

	if (lastfm->session_id == NULL) {
		g_critical("No connection Last.fm has been established.");
		return;
	}

	metadata = mpris2_client_get_metadata (soundmenu->mpris2);
	if (g_str_empty0(mpris2_metadata_get_artist(metadata)) ||
	    g_str_empty0(mpris2_metadata_get_title(metadata)))
		return;

	data = soundmenu_lastfm_data_new (soundmenu, metadata, 0);
	set_watch_cursor (GTK_WIDGET(soundmenu->plugin));
	soundmenu_async_launch(do_lastfm_current_song_love,
	                       soundmenu_async_set_idle_message,
	                       data);
}

static gpointer
do_lastfm_current_song_unlove (gpointer userdata)
{
	AsycMessageData *message_data = NULL;
	gint rv;

	LastfmData *data = userdata;
	SoundmenuPlugin *soundmenu = data->soundmenu;
	SoundmenuLastfm *lastfm = soundmenu->clastfm;

	rv = LASTFM_track_unlove (lastfm->session_id,
	                          data->title, data->artist);

	message_data = soundmenu_async_finished_message_new(soundmenu,
		(rv != 0) ? _("Unlove song on Last.fm failed.") : NULL);

	soundmenu_lastfm_data_free (data);

	return message_data;
}

void
lastfm_track_unlove_action (GtkWidget *widget, SoundmenuPlugin *soundmenu)
{
	Mpris2Metadata *metadata = NULL;
	SoundmenuLastfm *lastfm = NULL;
	LastfmData *data = NULL;

	if (mpris2_client_get_playback_status (soundmenu->mpris2) == STOPPED)
		return;

	lastfm = soundmenu->clastfm;

	if (lastfm->session_id == NULL) {
		g_critical("No connection Last.fm has been established.");
		return;
	}

	metadata = mpris2_client_get_metadata (soundmenu->mpris2);
	if (g_str_empty0(mpris2_metadata_get_artist(metadata)) ||
	    g_str_empty0(mpris2_metadata_get_title(metadata)))
		return;

	data = soundmenu_lastfm_data_new (soundmenu, metadata, 0);
	set_watch_cursor (GTK_WIDGET(soundmenu->plugin));
	soundmenu_async_launch(do_lastfm_current_song_unlove,
	                       soundmenu_async_set_idle_message,
	                       data);
}

static gpointer
do_lastfm_scrob (gpointer data)
{
    Mpris2Metadata *metadata = NULL;
	gchar *title, *artist, *album;
	gint length, track_no;
	time_t playback_started;
	gint rv;

    SoundmenuPlugin *soundmenu = data;
	SoundmenuLastfm *lastfm = soundmenu->clastfm;

	//soundmenu_mutex_lock(soundmenu->metadata_mtx);
	metadata = mpris2_client_get_metadata (soundmenu->mpris2);

	title = g_strdup(mpris2_metadata_get_title(metadata));
	artist = g_strdup(mpris2_metadata_get_artist(metadata));
	album = g_strdup(mpris2_metadata_get_album(metadata));
	playback_started = lastfm->playback_started;
	length = mpris2_metadata_get_length(metadata);
	track_no = mpris2_metadata_get_track_no(metadata);
	//soundmenu_mutex_unlock(soundmenu->metadata_mtx);

	rv = LASTFM_track_scrobble (lastfm->session_id,
	                            title,
	                            album ? album : "",
	                            artist,
	                            playback_started,
	                            length,
	                            track_no,
	                            0, NULL);

    if (rv != 0)
        g_critical("Last.fm submission failed");

	g_free(title);
	g_free(artist);
	g_free(album);

    return NULL;
}

static gboolean
lastfm_scrob_handler(gpointer data)
{
	SoundmenuLastfm *lastfm = NULL;
	SoundmenuPlugin *soundmenu = data;

	if (mpris2_client_get_playback_status (soundmenu->mpris2) == STOPPED)
		return FALSE;

	lastfm = soundmenu->clastfm;

	if (lastfm->status != LASTFM_STATUS_OK) {
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
	Mpris2Metadata *metadata = NULL;
	gchar *title, *artist, *album;
	gint length, track_no;
	gint rv;

	SoundmenuPlugin *soundmenu = data;
	SoundmenuLastfm *lastfm = soundmenu->clastfm;

	//soundmenu_mutex_lock(soundmenu->metadata_mtx);
	metadata = mpris2_client_get_metadata (soundmenu->mpris2);
	title = g_strdup(mpris2_metadata_get_title(metadata));
	artist = g_strdup(mpris2_metadata_get_artist(metadata));
	album = g_strdup(mpris2_metadata_get_album(metadata));
	length = mpris2_metadata_get_length(metadata);
	track_no = mpris2_metadata_get_track_no(metadata);
	//soundmenu_mutex_unlock(soundmenu->metadata_mtx);

	rv = LASTFM_track_update_now_playing (lastfm->session_id,
	                                      title,
	                                      album ? album : "",
	                                      artist,
	                                      length,
	                                      track_no,
	                                      0, NULL);

	if (rv != 0) {
		g_critical("Update current song on Last.fm failed");
	}

	g_free(title);
	g_free(artist);
	g_free(album);

	return NULL;
}

static gboolean
lastfm_now_playing_handler (gpointer data)
{
	Mpris2Metadata *metadata = NULL;
	gint length, time = 0;

	SoundmenuPlugin *soundmenu = data;
	SoundmenuLastfm *lastfm = soundmenu->clastfm;

	if (mpris2_client_get_playback_status (soundmenu->mpris2) == STOPPED)
		return FALSE;

	if (lastfm->session_id == NULL) {
		g_critical("No connection Last.fm has been established.");
		return FALSE;
	}

	metadata = mpris2_client_get_metadata (soundmenu->mpris2);
	if (g_str_empty0(mpris2_metadata_get_artist(metadata)) ||
	    g_str_empty0(mpris2_metadata_get_album(metadata)))
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
	length = mpris2_metadata_get_length(metadata);
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

    lastfm->lastfm_handler_id = g_timeout_add_seconds_full (G_PRIORITY_DEFAULT_IDLE,
                                                            time,
                                                            lastfm_scrob_handler,
                                                            soundmenu,
                                                            NULL);
	return FALSE;
}

void
soundmenu_update_playback_lastfm (SoundmenuPlugin *soundmenu)
{
	SoundmenuLastfm *lastfm = soundmenu->clastfm;

	if (lastfm->lastfm_handler_id)
		g_source_remove (lastfm->lastfm_handler_id);

	if (mpris2_client_get_playback_status (soundmenu->mpris2) != PLAYING)
		return;

	//soundmenu_mutex_lock(soundmenu->metadata_mtx);
	time (&lastfm->playback_started);
	//soundmenu_mutex_unlock(soundmenu->metadata_mtx);

	lastfm->lastfm_handler_id = g_timeout_add_seconds_full (G_PRIORITY_DEFAULT_IDLE,
	                                                        WAIT_UPDATE,
	                                                        lastfm_now_playing_handler,
	                                                        soundmenu,
	                                                        NULL);
}

void soundmenu_update_lastfm_menu (SoundmenuLastfm *clastfm)
{
	gtk_widget_set_sensitive(clastfm->lastfm_love_item, (clastfm->status == LASTFM_STATUS_OK));
	gtk_widget_set_sensitive(clastfm->lastfm_unlove_item, (clastfm->status == LASTFM_STATUS_OK));
}

static gboolean
do_soundmenu_init_lastfm(gpointer data)
{
	SoundmenuLastfm *clastfm = data;

	clastfm->session_id = LASTFM_init(LASTFM_API_KEY, LASTFM_SECRET);
	if (clastfm->session_id != NULL) {
		clastfm->status = LASTFM_login (clastfm->session_id,
		                                clastfm->lastfm_user,
		                                clastfm->lastfm_pass);

		if (clastfm->status != LASTFM_STATUS_OK) {
			LASTFM_dinit(clastfm->session_id);
			clastfm->session_id = NULL;
		}
	}
	soundmenu_update_lastfm_menu(clastfm);

	return FALSE;
}

void
soundmenu_lastfm_uninit (SoundmenuLastfm *lastfm)
{
	LASTFM_dinit (lastfm->session_id);
	lastfm->session_id = NULL;

	soundmenu_update_lastfm_menu (lastfm);
}

gint
soundmenu_lastfm_init (SoundmenuLastfm *lastfm)
{
	if (g_str_empty0(lastfm->lastfm_user) ||
	    g_str_empty0(lastfm->lastfm_pass))
		return FALSE;

    /* Test internet and launch threads.*/
#if GLIB_CHECK_VERSION(2,32,0)
    if (g_network_monitor_get_network_available (g_network_monitor_get_default ()))
#else
    if(nm_is_online () == TRUE)
#endif
        g_idle_add (do_soundmenu_init_lastfm, lastfm);
    else
        g_timeout_add_seconds_full(G_PRIORITY_DEFAULT_IDLE,
                                   30,
                                   do_soundmenu_init_lastfm,
                                   lastfm,
                                   NULL);

    return 0;
}

gboolean
soundmenu_lastfm_is_initiated (SoundmenuLastfm *lastfm)
{
	return (lastfm->session_id != NULL);
}

gboolean
soundmenu_lastfm_is_supported (SoundmenuLastfm *lastfm)
{
	return lastfm->lastfm_support;
}

void
soundmenu_lastfm_set_supported (SoundmenuLastfm *lastfm, gboolean support)
{
	lastfm->lastfm_support = support;
}

const gchar *
soundmenu_lastfm_get_user (SoundmenuLastfm *lastfm)
{
	return lastfm->lastfm_user;
}

void
soundmenu_lastfm_set_user (SoundmenuLastfm *lastfm, const gchar *user)
{
	if (g_str_nempty0(lastfm->lastfm_user))
		g_free (lastfm->lastfm_user);

	lastfm->lastfm_user = g_strdup(user);
}

const gchar *
soundmenu_lastfm_get_password (SoundmenuLastfm *lastfm)
{
	return lastfm->lastfm_pass;
}

void
soundmenu_lastfm_set_password (SoundmenuLastfm *lastfm, const gchar *password)
{
	if (g_str_nempty0(lastfm->lastfm_pass))
		g_free (lastfm->lastfm_pass);

	lastfm->lastfm_pass = g_strdup(password);
}

void
soundmenu_lastfm_free (SoundmenuLastfm *lastfm)
{
	if (lastfm->session_id)
		LASTFM_dinit (lastfm->session_id);

	if (g_str_nempty0(lastfm->lastfm_user))
		g_free (lastfm->lastfm_user);
	if (g_str_nempty0(lastfm->lastfm_pass))
		g_free (lastfm->lastfm_pass);

	g_slice_free(SoundmenuLastfm, lastfm);
}

SoundmenuLastfm *
soundmenu_lastfm_new (void)
{
	SoundmenuLastfm *lastfm = NULL;

	lastfm = g_slice_new0(SoundmenuLastfm);

	lastfm->lastfm_support = FALSE;
	lastfm->lastfm_user    = NULL;
	lastfm->lastfm_pass    = NULL;
	lastfm->session_id     = NULL;

	return lastfm;
}
