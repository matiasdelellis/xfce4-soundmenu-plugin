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
/* Handler for 'Artist info' action in the Tools menu */
/*void *do_lastfm_get_artist_info (gpointer data)
{
	GtkWidget *dialog;
	GtkTextBuffer *buffer;
	GtkTextIter iter;
	GtkTextTag *btag;
	gchar *value = NULL, *playcount = NULL, *wiki = NULL;
	gchar *summary_helper = NULL, *summary = NULL;
	GtkWidget *view, *frame, *scrolled;
	gint i, result;

	SoundmenuPlugin *soundmenu = data;

	LASTFM_ARTIST_INFO *artist = NULL;

	gdk_threads_enter ();

	gdk_threads_leave ();

	artist = LASTFM_artist_get_info (soundmenu->clastfm->session_id, soundmenu->metadata->artist, NULL);

	gdk_threads_enter ();

	if(!artist) {
		//set_status_message(_("Artist information not found on Last.fm."), soundmenu);
		gdk_threads_leave ();
		return NULL;
	}

	view = gtk_text_view_new ();
	gtk_text_view_set_editable (GTK_TEXT_VIEW (view), FALSE);
	gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (view), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW (view), GTK_WRAP_WORD);

	frame = gtk_frame_new (NULL);
	scrolled = gtk_scrolled_window_new (NULL, NULL);

	gtk_container_add (GTK_CONTAINER (scrolled), view);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
					GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	gtk_container_set_border_width (GTK_CONTAINER (frame), 8);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
	gtk_container_add (GTK_CONTAINER (frame), scrolled);

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
	gtk_text_buffer_get_iter_at_offset (GTK_TEXT_BUFFER (buffer), &iter, 0);

	btag = gtk_text_buffer_create_tag (buffer, NULL, "weight", PANGO_WEIGHT_BOLD, NULL);

	playcount = g_strdup_printf("%d", artist->playcount);
	gtk_text_buffer_insert_with_tags(GTK_TEXT_BUFFER(buffer), &iter, _("Playcount:"), -1, btag, NULL);
	value = g_strdup_printf (" %s\n\n", playcount);
	gtk_text_buffer_insert(GTK_TEXT_BUFFER(buffer), &iter, value, -1);
	g_free (playcount);
	g_free (value);

	gtk_text_buffer_insert_with_tags(GTK_TEXT_BUFFER(buffer), &iter, _("Summary:"), -1, btag, NULL);

	if(artist->summary && strncmp (artist->summary, "<![CDATA[", 9) == 0) {
		summary_helper = artist->summary + 9;
		summary = g_strndup (summary_helper, strlen (summary_helper) - 3);
	}
	else {
		summary = g_strdup(artist->summary);
	}

	value = g_strdup_printf (" %s\n\n", summary);
	gtk_text_buffer_insert(GTK_TEXT_BUFFER(buffer), &iter, value, -1);
	g_free (summary);
	g_free (value);

	if(artist->similar != NULL) {
		gtk_text_buffer_insert_with_tags(GTK_TEXT_BUFFER(buffer), &iter, _("Similar artists:"), -1, btag, NULL);

		for(i=0; artist->similar[i]; i++){
			value = g_strdup_printf ("\n\t%i: %s", i, artist->similar[i]);
			gtk_text_buffer_insert(GTK_TEXT_BUFFER(buffer), &iter, value, -1);
			g_free (value);
		}
	}

	dialog = xfce_titled_dialog_new_with_buttons (_("Lastfm artist info"),
							GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (soundmenu->plugin))),
							GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR,
							GTK_STOCK_CLOSE, GTK_RESPONSE_OK,
							NULL);

	gtk_dialog_add_button(GTK_DIALOG(dialog), _("View more..."), GTK_RESPONSE_HELP);

	gtk_window_set_default_size(GTK_WINDOW (dialog), 450, 350);

	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), frame, TRUE, TRUE, 0);

	gtk_widget_show_all(dialog);

	result = gtk_dialog_run(GTK_DIALOG(dialog));
	switch (result) {
		case GTK_RESPONSE_HELP:
			wiki = g_strdup_printf("http://www.lastfm.es/music/%s/+wiki", artist->name);
			//open_url (soundmenu, wiki);
			g_free (wiki);
			break;
		case GTK_RESPONSE_OK:
			break;
		default:
			break;
	}

	gtk_widget_destroy(dialog);
	gdk_threads_leave ();

	LASTFM_free_artist_info(artist);

	return NULL;
}

void lastfm_artist_info_action (GtkAction *action, SoundmenuPlugin *soundmenu)
{
	pthread_t tid;

	if(soundmenu->state == ST_STOPPED)
		return;

	if (soundmenu->clastfm->session_id == NULL) {
		//set_status_message(_("No connection Last.fm has been established."), soundmenu);
		return;
	}
	pthread_create(&tid, NULL, do_lastfm_get_artist_info, soundmenu);
}*/

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

void lastfm_track_love_action (GtkAction *action, SoundmenuPlugin *soundmenu)
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

void lastfm_track_unlove_action (GtkAction *action, SoundmenuPlugin *soundmenu)
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

	if ((NULL == soundmenu->metadata->artist) || (NULL == soundmenu->metadata->title))
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
		length = soundmenu->metadata->length / 2;
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

void *do_init_lastfm (gpointer data)
{
	gint rv;

	SoundmenuPlugin *soundmenu = data;

	soundmenu->clastfm->session_id = LASTFM_init(LASTFM_API_KEY, LASTFM_SECRET);

	rv = LASTFM_login (soundmenu->clastfm->session_id, soundmenu->clastfm->lastfm_user, soundmenu->clastfm->lastfm_pass);

	if(rv != LASTFM_STATUS_OK) {
		LASTFM_dinit(soundmenu->clastfm->session_id);
		soundmenu->clastfm->session_id = NULL;

		g_critical("Unable to login on Lastfm");
	}

	return NULL;
}

gint init_lastfm (SoundmenuPlugin *soundmenu)
{
	pthread_t tid;
	if (soundmenu->clastfm->lastfm_support) {
		pthread_create (&tid, NULL, do_init_lastfm, soundmenu);
	}
	return 0;
}
#endif
