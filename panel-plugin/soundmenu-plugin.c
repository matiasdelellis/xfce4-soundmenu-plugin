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

#ifdef HAVE_LIBKEYBINDER
#include "soundmenu-keybinder.h"
#endif

/* default settings */

#define DEFAULT_PLAYER "pragha"
#define DEFAULT_SHOW_STOP TRUE
#define DEFAULT_GLOBAL_KEYS TRUE
#define DEFAULT_LASTFM FALSE

/* prototypes */

static void
soundmenu_construct (XfcePanelPlugin *plugin);

/* register the plugin */

XFCE_PANEL_PLUGIN_REGISTER (soundmenu_construct);

static gboolean
soundmenu_set_query_tooltip_cb (GtkWidget       *widget,
                                gint             x,
                                gint             y,
                                gboolean         keyboard_mode,
                                GtkTooltip      *tooltip,
                                SoundmenuPlugin *soundmenu)
{
	gchar *markup_text = NULL, *length = NULL;

	length = convert_length_str(soundmenu->metadata->length);
	if(soundmenu->connected) {
		if (soundmenu->state == ST_STOPPED)
			markup_text = g_strdup_printf("%s", _("Stopped"));
		else {
			markup_text = g_markup_printf_escaped(_("<b>%s</b> (%s)\nby %s in %s"),
				                                  g_str_nempty0(soundmenu->metadata->title) ? soundmenu->metadata->title :soundmenu->metadata->url,
				                                  length,
				                                  g_str_nempty0(soundmenu->metadata->artist) ? soundmenu->metadata->artist : _("Unknown Artist"),
				                                  g_str_nempty0(soundmenu->metadata->album) ? soundmenu->metadata->album : _("Unknown Album"));
		}
	}
	else
		markup_text = g_strdup_printf("%s", _("Double-click to launch the music player"));

	gtk_tooltip_set_markup (tooltip, markup_text);

	gtk_tooltip_set_icon (tooltip,
		soundmenu_album_art_get_pixbuf(soundmenu->album_art));

	g_free(markup_text);
	g_free(length);

	return TRUE;
}

static void
soundmenu_toggle_play_button_state (SoundmenuPlugin *soundmenu)
{
	gtk_container_remove(GTK_CONTAINER(soundmenu->play_button),
                       gtk_bin_get_child(GTK_BIN(soundmenu->play_button)));
	if ((soundmenu->state == ST_PAUSED) || (soundmenu->state == ST_STOPPED))
		gtk_container_add(GTK_CONTAINER(soundmenu->play_button), soundmenu->image_play);
	else
		gtk_container_add(GTK_CONTAINER(soundmenu->play_button), soundmenu->image_pause);
	gtk_widget_show_all(soundmenu->play_button);
}

void
soundmenu_update_state(gchar *state, SoundmenuPlugin *soundmenu)
{
	if (0 == g_ascii_strcasecmp(state, "Playing")) {
		soundmenu_album_art_set_path(soundmenu->album_art, soundmenu->metadata->arturl);
		soundmenu->state = ST_PLAYING;
	}
	else if (0 == g_ascii_strcasecmp(state, "Paused"))
		soundmenu->state = ST_PAUSED;
	else {
		soundmenu->state = ST_STOPPED;
		soundmenu_album_art_set_path(soundmenu->album_art, NULL);
	}
	soundmenu_toggle_play_button_state(soundmenu);
	#ifdef HAVE_LIBCLASTFM
	if (soundmenu->clastfm->lastfm_support)
		update_lastfm(soundmenu);
	#endif
}

/* Sound menu plugin construct */

void
soundmenu_save (XfcePanelPlugin *plugin,
             SoundmenuPlugin    *soundmenu)
{
	XfceRc *rc;
	gchar  *file;

	/* get the config file location */
	file = xfce_panel_plugin_save_location (plugin, TRUE);

	if (G_UNLIKELY (file == NULL)) {
		DBG ("Failed to open config file");
		return;
	}

	/* open the config file, read/write */
	rc = xfce_rc_simple_open (file, FALSE);
	g_free (file);

	if (G_LIKELY (rc != NULL)) {
		/* save the settings */
		DBG(".");
		if (soundmenu->player)
			xfce_rc_write_entry    (rc, "player", soundmenu->player);

		xfce_rc_write_bool_entry (rc, "show_album_art", soundmenu->show_album_art);
		xfce_rc_write_bool_entry (rc, "huge_on_deskbar_mode", soundmenu->huge_on_deskbar_mode);
		xfce_rc_write_bool_entry (rc, "show_stop", soundmenu->show_stop);
		#ifdef HAVE_LIBKEYBINDER
		xfce_rc_write_bool_entry (rc, "use_global_keys", soundmenu->use_global_keys);
		#endif
		#ifdef HAVE_LIBCLASTFM
		xfce_rc_write_bool_entry (rc, "use_lastfm", soundmenu->clastfm->lastfm_support);
		if(soundmenu->clastfm->lastfm_support) {
			if (soundmenu->clastfm->lastfm_user)
				xfce_rc_write_entry(rc, "lastfm_user", soundmenu->clastfm->lastfm_user);
			if (soundmenu->clastfm->lastfm_pass)
				xfce_rc_write_entry(rc, "lastfm_pass", soundmenu->clastfm->lastfm_pass);
		}
		else {
			xfce_rc_delete_entry(rc, "lastfm_user", TRUE);
			xfce_rc_delete_entry(rc, "lastfm_pass", TRUE);
		}
		#endif

		/* close the rc file */
		xfce_rc_close (rc);
	}
}

static void
soundmenu_read (SoundmenuPlugin *soundmenu)
{
	XfceRc      *rc;
	gchar       *file;

	/* get the plugin config file location */
	file = xfce_panel_plugin_save_location (soundmenu->plugin, TRUE);

	if (G_LIKELY (file != NULL)) {
		/* open the config file, readonly */
		rc = xfce_rc_simple_open (file, TRUE);

		/* cleanup */
		g_free (file);

		if (G_LIKELY (rc != NULL)) {
			/* read the settings */
			soundmenu->player = g_strdup (xfce_rc_read_entry (rc, "player", NULL));
			soundmenu->show_album_art = xfce_rc_read_bool_entry (rc, "show_album_art", FALSE);
			soundmenu->huge_on_deskbar_mode = xfce_rc_read_bool_entry (rc, "huge_on_deskbar_mode", FALSE);
			soundmenu->show_stop = xfce_rc_read_bool_entry (rc, "show_stop", FALSE);
			#ifdef HAVE_LIBKEYBINDER
			soundmenu->use_global_keys = xfce_rc_read_bool_entry (rc, "use_global_keys", DEFAULT_GLOBAL_KEYS);
			#endif
			#ifdef HAVE_LIBCLASTFM
			soundmenu->clastfm->lastfm_support = xfce_rc_read_bool_entry (rc, "use_lastfm", DEFAULT_LASTFM);
			soundmenu->clastfm->lastfm_user = g_strdup(xfce_rc_read_entry (rc, "lastfm_user", NULL));
			soundmenu->clastfm->lastfm_pass = g_strdup(xfce_rc_read_entry (rc, "lastfm_pass", NULL));
			/* Also init session id */
			soundmenu->clastfm->session_id = NULL;
			#endif
			soundmenu->state = ST_STOPPED;

			/* cleanup */
			xfce_rc_close (rc);

			/* leave the function, everything went well */
			return;
		}
	}

	/* something went wrong, apply default values */
	DBG ("Applying default settings");

	soundmenu->player = NULL;
	soundmenu->show_album_art = FALSE;
	soundmenu->huge_on_deskbar_mode = FALSE;
	soundmenu->show_stop = FALSE;
	#ifdef HAVE_LIBKEYBINDER
	soundmenu->use_global_keys = DEFAULT_GLOBAL_KEYS;
	#endif
	#ifdef HAVE_LIBCLASTFM
	/* Read lastfm support and init session id */
	soundmenu->clastfm->lastfm_support = DEFAULT_LASTFM;
	soundmenu->clastfm->lastfm_user = NULL;
	soundmenu->clastfm->lastfm_pass = NULL;
	soundmenu->clastfm->session_id = NULL;
	#endif
	soundmenu->state = ST_STOPPED;
}

#ifdef HAVE_LIBCLASTFM
static void
soundmenu_add_lastfm_menu_item (SoundmenuPlugin *soundmenu)
{
	GtkWidget *submenu, *item;

	item = gtk_menu_item_new_with_mnemonic (_("Last.fm"));
	xfce_panel_plugin_menu_insert_item (soundmenu->plugin, GTK_MENU_ITEM(item));

	soundmenu->clastfm->lastfm_menu = item;
	gtk_widget_set_sensitive(item, FALSE);
	gtk_widget_show (item);

	submenu = gtk_menu_new ();
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), submenu);

	item = gtk_menu_item_new_with_label (_("Love"));
	g_signal_connect (G_OBJECT (item), "activate", G_CALLBACK (lastfm_track_love_action), soundmenu);
	gtk_menu_append (GTK_MENU(submenu), item);
	item = gtk_menu_item_new_with_label (_("Unlove"));
	g_signal_connect (G_OBJECT (item), "activate", G_CALLBACK (lastfm_track_unlove_action), soundmenu);
	gtk_menu_append (GTK_MENU(submenu), item);

	gtk_widget_show_all (submenu);
}
#endif

#ifdef HAVE_LIBGLYR
static void
soundmenu_add_lyrics_menu_item (SoundmenuPlugin *soundmenu)
{
	GtkWidget *item;

	item = gtk_menu_item_new_with_mnemonic (_("Search lyrics"));
	g_signal_connect (G_OBJECT (item), "activate", G_CALLBACK (soundmenu_search_lyric_dialog), soundmenu);
	gtk_widget_show (item);
	xfce_panel_plugin_menu_insert_item (soundmenu->plugin, GTK_MENU_ITEM(item));

	item = gtk_menu_item_new_with_mnemonic (_("Search artist info"));
	g_signal_connect (G_OBJECT (item), "activate", G_CALLBACK (soundmenu_search_artistinfo_dialog), soundmenu);
	gtk_widget_show (item);
	xfce_panel_plugin_menu_insert_item (soundmenu->plugin, GTK_MENU_ITEM(item));
}
#endif

static SoundmenuPlugin *
soundmenu_new (XfcePanelPlugin *plugin)
{
	SoundmenuPlugin   *soundmenu;
	GtkOrientation panel_orientation, orientation;
	GtkWidget *ev_album_art, *play_button, *stop_button, *prev_button, *next_button;
	SoundmenuAlbumArt *album_art;
	Metadata *metadata;

	/* allocate memory for the plugin structure */
	soundmenu = panel_slice_new0 (SoundmenuPlugin);
	soundmenu->plugin = plugin;

	#ifdef HAVE_LIBCLASTFM
	soundmenu->clastfm = g_slice_new0(struct con_lastfm);
	#endif

	metadata = malloc_metadata();
	soundmenu->metadata = metadata;

	/* read the user settings */
	soundmenu_read (soundmenu);

	/* get the current orientation */
#if LIBXFCE4PANEL_CHECK_VERSION (4,9,0)
	orientation =
		(xfce_panel_plugin_get_mode (plugin) == XFCE_PANEL_PLUGIN_MODE_VERTICAL) ?
		GTK_ORIENTATION_VERTICAL : GTK_ORIENTATION_HORIZONTAL;
#else
	orientation = xfce_panel_plugin_get_orientation (plugin);
#endif
	panel_orientation = xfce_panel_plugin_get_orientation (plugin);

	/* create some panel widgets */

	soundmenu->hvbox = xfce_hvbox_new (panel_orientation, FALSE, 2);
	gtk_widget_show (soundmenu->hvbox);

	soundmenu->hvbox_buttons = xfce_hvbox_new (orientation, FALSE, 0);
	gtk_widget_show (soundmenu->hvbox_buttons);

	/* some soundmenu widgets */

	ev_album_art = gtk_event_box_new ();
	gtk_event_box_set_visible_window(GTK_EVENT_BOX(ev_album_art), FALSE);
	gtk_event_box_set_above_child(GTK_EVENT_BOX(ev_album_art), TRUE);

	album_art = soundmenu_album_art_new ();
	gtk_container_add (GTK_CONTAINER (ev_album_art), GTK_WIDGET(album_art));

	prev_button = xfce_panel_create_button();
	play_button = xfce_panel_create_button();
	stop_button = xfce_panel_create_button();
	next_button = xfce_panel_create_button();

	gtk_container_add(GTK_CONTAINER(prev_button),
		xfce_panel_image_new_from_source("media-skip-backward"));
	gtk_container_add(GTK_CONTAINER(stop_button),
		xfce_panel_image_new_from_source("media-playback-stop"));
	gtk_container_add(GTK_CONTAINER(next_button),
		xfce_panel_image_new_from_source("media-skip-forward"));

	soundmenu->image_pause =
		xfce_panel_image_new_from_source("media-playback-pause");
	soundmenu->image_play =
		xfce_panel_image_new_from_source("media-playback-start");

	g_object_ref(soundmenu->image_play);
	g_object_ref(soundmenu->image_pause);

	gtk_container_add(GTK_CONTAINER(play_button),
		soundmenu->image_play);

	gtk_box_pack_start(GTK_BOX(soundmenu->hvbox),
			   GTK_WIDGET(ev_album_art),
			   FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(soundmenu->hvbox),
			   GTK_WIDGET(soundmenu->hvbox_buttons),
			   FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(soundmenu->hvbox_buttons),
			   GTK_WIDGET(prev_button),
			   TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(soundmenu->hvbox_buttons),
			   GTK_WIDGET(play_button),
			   TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(soundmenu->hvbox_buttons),
			   GTK_WIDGET(stop_button),
			   TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(soundmenu->hvbox_buttons),
			   GTK_WIDGET(next_button),
			   TRUE, TRUE, 0);

	gtk_widget_show(GTK_WIDGET(album_art));
	if(soundmenu->show_album_art)
		gtk_widget_show(ev_album_art);
	gtk_widget_show_all(prev_button);
	gtk_widget_show_all(play_button);
	if(soundmenu->show_stop)
		gtk_widget_show_all(stop_button);
	gtk_widget_show_all(next_button);

	/* Signal handlers */

	g_signal_connect(G_OBJECT (ev_album_art), "button_press_event",
	                 G_CALLBACK (soundmenu_album_art_frame_press_callback), soundmenu);
	g_signal_connect(G_OBJECT (ev_album_art), "scroll-event",
	                  G_CALLBACK (soundmenu_panel_button_scrolled), soundmenu);
	g_signal_connect(G_OBJECT(prev_button), "clicked",
	                 G_CALLBACK(prev_button_handler), soundmenu);
	g_signal_connect(G_OBJECT(play_button), "clicked",
	                 G_CALLBACK(play_button_handler), soundmenu);
	g_signal_connect(G_OBJECT(stop_button), "clicked",
	                 G_CALLBACK(stop_button_handler), soundmenu);
	g_signal_connect(G_OBJECT(next_button), "clicked",
	                 G_CALLBACK(next_button_handler), soundmenu);

	xfce_panel_plugin_add_action_widget (plugin, GTK_WIDGET(album_art));
	xfce_panel_plugin_add_action_widget (plugin, GTK_WIDGET(ev_album_art));
	xfce_panel_plugin_add_action_widget (plugin, prev_button);
	xfce_panel_plugin_add_action_widget (plugin, play_button);
	xfce_panel_plugin_add_action_widget (plugin, stop_button);
	xfce_panel_plugin_add_action_widget (plugin, next_button);

	g_object_set (G_OBJECT(album_art), "has-tooltip", TRUE, NULL);
	g_object_set (G_OBJECT(prev_button), "has-tooltip", TRUE, NULL);
	g_object_set (G_OBJECT(play_button), "has-tooltip", TRUE, NULL);
	g_object_set (G_OBJECT(stop_button), "has-tooltip", TRUE, NULL);
	g_object_set (G_OBJECT(next_button), "has-tooltip", TRUE, NULL);

	g_signal_connect(G_OBJECT(album_art), "query-tooltip",
			G_CALLBACK(soundmenu_set_query_tooltip_cb), soundmenu);
	g_signal_connect(G_OBJECT(prev_button), "query-tooltip",
			G_CALLBACK(soundmenu_set_query_tooltip_cb), soundmenu);
	g_signal_connect(G_OBJECT(play_button), "query-tooltip",
			G_CALLBACK(soundmenu_set_query_tooltip_cb), soundmenu);
	g_signal_connect(G_OBJECT(stop_button), "query-tooltip",
			G_CALLBACK(soundmenu_set_query_tooltip_cb), soundmenu);
	g_signal_connect(G_OBJECT(next_button), "query-tooltip",
			G_CALLBACK(soundmenu_set_query_tooltip_cb), soundmenu);

	soundmenu->album_art = album_art;
	soundmenu->ev_album_art = ev_album_art;
	soundmenu->prev_button = prev_button;
	soundmenu->play_button = play_button;
	soundmenu->stop_button = stop_button;
	soundmenu->next_button = next_button;

	return soundmenu;
}

static void init_soundmenu_plugin(SoundmenuPlugin *soundmenu)
{
	/* Init dbus and configure filters. */

	init_dbus_session (soundmenu);

	/* If no has a player selected, search it with dbus. */

	if (soundmenu->player == NULL)
		soundmenu->player = soundmenu_get_mpris2_player_running(soundmenu);
	if (soundmenu->player == NULL)
		soundmenu->player = g_strdup (DEFAULT_PLAYER);

	/* Init the goodies services .*/

	#ifdef HAVE_LIBKEYBINDER
	soundmenu_init_keybinder();
	if (soundmenu->use_global_keys)
		keybinder_bind_keys(soundmenu);
	#endif
	#ifdef HAVE_LIBCLASTFM
	if(soundmenu->clastfm->lastfm_support)
        soundmenu_init_lastfm(soundmenu);
	#endif
	#ifdef HAVE_LIBGLYR
	init_glyr_related(soundmenu);
	#endif
	#ifdef HAVE_LIBNOTIFY
	notify_init ("xfce4-soundmenu-plugin");
	#endif

	/* Add lastfm and glyr options in panel plugin. */

	#ifdef HAVE_LIBCLASTFM
	soundmenu_add_lastfm_menu_item(soundmenu);
	#endif
	#ifdef HAVE_LIBGLYR
	soundmenu_add_lyrics_menu_item (soundmenu);
	#endif
}

static void
soundmenu_free (XfcePanelPlugin *plugin,
             SoundmenuPlugin    *soundmenu)
{
	GtkWidget *dialog;

	#ifdef HAVE_LIBKEYBINDER
	keybinder_unbind_keys(soundmenu);
	#endif

	#ifdef HAVE_LIBCLASTFM
	if (soundmenu->clastfm->session_id)
		LASTFM_dinit(soundmenu->clastfm->session_id);
	g_slice_free(struct con_lastfm, soundmenu->clastfm);
	if (G_LIKELY (soundmenu->clastfm->lastfm_user != NULL))
		g_free (soundmenu->clastfm->lastfm_user);
	if (G_LIKELY (soundmenu->clastfm->lastfm_pass != NULL))
		g_free (soundmenu->clastfm->lastfm_pass);
	#endif
	#ifdef HAVE_LIBGLYR
	uninit_glyr_related(soundmenu);
	#endif
	#ifdef HAVE_LIBNOTIFY
	notify_uninit();
	#endif

	/* check if the dialog is still open. if so, destroy it */
	dialog = g_object_get_data (G_OBJECT (plugin), "dialog");
	if (G_UNLIKELY (dialog != NULL))
	gtk_widget_destroy (dialog);

	/* destroy the panel widgets */
	gtk_widget_destroy (soundmenu->hvbox);

	/* cleanup the metadata and settings */
	if (G_LIKELY (soundmenu->player != NULL))
		g_free (soundmenu->player);
	if (G_LIKELY (soundmenu->dbus_name != NULL))
		g_free (soundmenu->dbus_name);
	if (G_LIKELY (soundmenu->metadata != NULL))
		free_metadata(soundmenu->metadata);

	/* free the plugin structure */
	panel_slice_free (SoundmenuPlugin, soundmenu);
}



static gboolean
soundmenu_size_changed (XfcePanelPlugin *plugin,
                        gint             panel_size,
                        SoundmenuPlugin *soundmenu)
{
	GtkOrientation panel_orientation;
	gint           size, album_size, rows = 3;

	/* get the orientation of the plugin */
	panel_orientation = xfce_panel_plugin_get_orientation (plugin);

	/* set the widget size */
	if (panel_orientation == GTK_ORIENTATION_HORIZONTAL)
		gtk_widget_set_size_request (GTK_WIDGET (plugin), -1, panel_size);
	else
		gtk_widget_set_size_request (GTK_WIDGET (plugin), panel_size, -1);

	size = album_size = panel_size;

#if LIBXFCE4PANEL_CHECK_VERSION (4,9,0)
	if (xfce_panel_plugin_get_mode (plugin) == XFCE_PANEL_PLUGIN_MODE_DESKBAR)
	{
		if (soundmenu->show_stop)
			rows++;
		if (soundmenu->show_album_art &&
		    !soundmenu->huge_on_deskbar_mode)
			rows++;

		size = panel_size / rows;

		if (soundmenu->show_album_art &&
		    soundmenu->huge_on_deskbar_mode)
			album_size = panel_size * 0.80;
		else
			album_size = size;
	}
#endif

	gtk_widget_set_size_request (GTK_WIDGET (soundmenu->next_button), size, size);
	gtk_widget_set_size_request (GTK_WIDGET (soundmenu->prev_button), size, size);
	gtk_widget_set_size_request (GTK_WIDGET (soundmenu->stop_button), size, size);
	gtk_widget_set_size_request (GTK_WIDGET (soundmenu->play_button), size, size);

	soundmenu_album_art_set_size(soundmenu->album_art, album_size);

	/* we handled the orientation */
	return TRUE;
}

#if LIBXFCE4PANEL_CHECK_VERSION (4,9,0)
static void
soundmenu_mode_changed (XfcePanelPlugin *plugin,
                            XfcePanelPluginMode   mode,
                            SoundmenuPlugin    *soundmenu)
{
	GtkOrientation panel_orientation, orientation;

	orientation = (mode == XFCE_PANEL_PLUGIN_MODE_VERTICAL) ?
		GTK_ORIENTATION_VERTICAL : GTK_ORIENTATION_HORIZONTAL;
	panel_orientation = xfce_panel_plugin_get_orientation (plugin);

	if (mode == XFCE_PANEL_PLUGIN_MODE_DESKBAR)
	{
		if (soundmenu->huge_on_deskbar_mode)
			xfce_hvbox_set_orientation (XFCE_HVBOX (soundmenu->hvbox), GTK_ORIENTATION_VERTICAL);
		else
			xfce_hvbox_set_orientation (XFCE_HVBOX (soundmenu->hvbox), GTK_ORIENTATION_HORIZONTAL);

		xfce_hvbox_set_orientation (XFCE_HVBOX (soundmenu->hvbox_buttons), GTK_ORIENTATION_HORIZONTAL);
	}
	else
	{
		xfce_hvbox_set_orientation (XFCE_HVBOX (soundmenu->hvbox), panel_orientation);
		xfce_hvbox_set_orientation (XFCE_HVBOX (soundmenu->hvbox_buttons), orientation);
	}

	/* update size after orientation change */
	soundmenu_size_changed (plugin, xfce_panel_plugin_get_size (plugin), soundmenu);
}

#else
static void
soundmenu_orientation_changed (XfcePanelPlugin *plugin,
                            GtkOrientation   orientation,
                            SoundmenuPlugin    *soundmenu)
{
	/* change the orienation of the box */
	xfce_hvbox_set_orientation (XFCE_HVBOX (soundmenu->hvbox), orientation);
	xfce_hvbox_set_orientation (XFCE_HVBOX (soundmenu->hvbox_buttons), orientation);

	/* update size after orientation change */
	soundmenu_size_changed (plugin, xfce_panel_plugin_get_size (plugin), soundmenu);
}
#endif

void soundmenu_update_layout_changes (SoundmenuPlugin    *soundmenu)
{
	/* Update orientations and size. */

	soundmenu_mode_changed(soundmenu->plugin, xfce_panel_plugin_get_mode (soundmenu->plugin), soundmenu);
}

static void
soundmenu_construct (XfcePanelPlugin *plugin)
{
	SoundmenuPlugin *soundmenu;

	/* setup transation domain */
	xfce_textdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

	/* create the plugin */
	soundmenu = soundmenu_new (plugin);

	/* add the hvbox to the panel */
	gtk_container_add (GTK_CONTAINER (plugin), soundmenu->hvbox);

	/* connect plugin signals */
	g_signal_connect (G_OBJECT (plugin), "free-data",
				G_CALLBACK (soundmenu_free), soundmenu);

	g_signal_connect (G_OBJECT (plugin), "save",
				G_CALLBACK (soundmenu_save), soundmenu);

	g_signal_connect (G_OBJECT (plugin), "size-changed",
				G_CALLBACK (soundmenu_size_changed), soundmenu);

#if LIBXFCE4PANEL_CHECK_VERSION (4,9,0)
	g_signal_connect (G_OBJECT (plugin), "mode-changed",
				G_CALLBACK (soundmenu_mode_changed), soundmenu);
#else
	g_signal_connect (G_OBJECT (plugin), "orientation-changed",
				G_CALLBACK (soundmenu_orientation_changed), soundmenu);
#endif

	/* show the configure menu item and connect signal */
	xfce_panel_plugin_menu_show_configure (plugin);

	g_signal_connect (G_OBJECT (plugin), "configure-plugin",
				G_CALLBACK (soundmenu_configure), soundmenu);

	/* show the about menu item and connect signal */
	xfce_panel_plugin_menu_show_about (plugin);

	g_signal_connect (G_OBJECT (plugin), "about",
				G_CALLBACK (soundmenu_about), NULL);

	/* Init dbus, services, get player status, etc. */
	init_soundmenu_plugin(soundmenu);
}
