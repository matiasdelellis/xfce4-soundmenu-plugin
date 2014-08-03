/*
 *  Copyright (c) 2011-2014 matias <mati86dl@gmail.com>
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

#include "soundmenu-controls.h"
#include "soundmenu-dialogs.h"
#include "soundmenu-utils.h"
#include "soundmenu-related.h"
#include "soundmenu-simple-async.h"
#include "soundmenu-panel-plugin.h"

#ifdef HAVE_LIBKEYBINDER
#include "soundmenu-keybinder.h"
#endif
#ifdef HAVE_LIBCLASTFM
#include "soundmenu-lastfm.h"
#endif
#ifdef HAVE_LIBNOTIFY
#include "soundmenu-notify.h"
#endif

/* default settings */

#define DEFAULT_SHOW_STOP TRUE
#define DEFAULT_GLOBAL_KEYS TRUE
#define DEFAULT_LASTFM FALSE

/* prototypes */

static void
soundmenu_construct (XfcePanelPlugin *plugin);

/* register the plugin */

XFCE_PANEL_PLUGIN_REGISTER (soundmenu_construct);

/*
 * Public Api.
 */

Mpris2Client *
soundmenu_get_mpris2_client (SoundmenuPlugin *soundmenu)
{
	return soundmenu->mpris2;
}

void
soundmenu_set_visible_stop_button (SoundmenuPlugin *soundmenu,
                                   gboolean         visible)
{
	soundmenu->show_stop = visible;

	soundmenu_update_layout_changes (soundmenu);
}

gboolean
soundmenu_get_visible_stop_button (SoundmenuPlugin *soundmenu)
{
	return soundmenu->show_stop;
}

void
soundmenu_set_visible_album_art (SoundmenuPlugin *soundmenu,
                                 gboolean         visible)
{
	soundmenu->show_album_art = visible;

	soundmenu_update_layout_changes (soundmenu);
}

gboolean
soundmenu_get_visible_album_art (SoundmenuPlugin *soundmenu)
{
	return soundmenu->show_album_art;
}

void
soundmenu_set_huge_album_art (SoundmenuPlugin *soundmenu,
                              gboolean         huge)
{
	soundmenu->huge_on_deskbar_mode = huge;

	soundmenu_update_layout_changes (soundmenu);
}

gboolean
soundmenu_get_huge_album_art (SoundmenuPlugin *soundmenu)
{
	return soundmenu->huge_on_deskbar_mode;
}

/*
 * Some Private api.
 */

static void
mpris2_panel_plugin_metadada (Mpris2Client *mpris2, Mpris2Metadata *metadata, SoundmenuPlugin *soundmenu)
{
	soundmenu_album_art_set_path (soundmenu->album_art,
		mpris2_metadata_get_arturl(metadata));
}

static void
mpris2_panel_plugin_playback_status (Mpris2Client *mpris2, PlaybackStatus playback_status, SoundmenuPlugin *soundmenu)
{
	switch (playback_status) {
		case PLAYING:
		case PAUSED:
			break;
		case STOPPED:
		default:
			soundmenu_album_art_set_path (soundmenu->album_art, NULL);
			break;
	}

	#ifdef HAVE_LIBCLASTFM
	if (soundmenu_lastfm_is_supported(soundmenu->clastfm))
		soundmenu_update_playback_lastfm(soundmenu);
	#endif
}

static void
mpris2_panel_plugin_coneccion (Mpris2Client *mpris2, gboolean connected, SoundmenuPlugin *soundmenu)
{
	if (connected) {
		/* Set visible acording player props */
		gtk_widget_set_visible (soundmenu->loop_menu_item,
			mpris2_client_player_has_loop_status (soundmenu->mpris2));
		gtk_widget_set_visible (soundmenu->shuffle_menu_item,
			mpris2_client_player_has_shuffle (soundmenu->mpris2));
	}
	else {
		/* Hide loop_status and shuffle options */
		gtk_widget_set_visible (soundmenu->loop_menu_item, FALSE);
		gtk_widget_set_visible (soundmenu->shuffle_menu_item, FALSE);

		/* Ensure remove old album art */
		soundmenu_album_art_set_path (soundmenu->album_art, NULL);
	}
	soundmenu_update_layout_changes (soundmenu);
}

static gboolean
soundmenu_set_query_tooltip_cb (GtkWidget       *widget,
                                gint             x,
                                gint             y,
                                gboolean         keyboard_mode,
                                GtkTooltip      *tooltip,
                                SoundmenuPlugin *soundmenu)
{
	const gchar *title, *artist, *album, *url;
	gchar *markup_text = NULL, *length = NULL, *filename = NULL,*name = NULL;
	GError *error = NULL;
	Mpris2Metadata *metadata = NULL;

	if (mpris2_client_is_connected(soundmenu->mpris2)) {
		if (mpris2_client_get_playback_status (soundmenu->mpris2) == STOPPED)
			markup_text = g_strdup_printf("%s", _("Stopped"));
		else {
			metadata = mpris2_client_get_metadata (soundmenu->mpris2);

			title = mpris2_metadata_get_title (metadata);
			artist = mpris2_metadata_get_artist (metadata);
			album = mpris2_metadata_get_album (metadata);
			url = mpris2_metadata_get_url (metadata);

			if (g_str_empty0(url))
			    return TRUE;

			if (g_str_nempty0(title)) {
				name = g_strdup(title);
			}
			else {
				filename = g_filename_from_uri (url, NULL, &error);
				if (filename) {
					name = g_filename_display_basename(filename);
				}
				else {
					name = g_strdup(url);
				}
			}
			length = convert_length_str(mpris2_metadata_get_length (metadata));

			markup_text = g_markup_printf_escaped(_("<b>%s</b> (%s)\nby %s in %s"),
				                                  name, length,
				                                  g_str_nempty0(artist) ? artist : _("Unknown Artist"),
				                                  g_str_nempty0(album) ? album : _("Unknown Album"));
			g_free(filename);
			g_free(name);
			g_free(length);
		}
	}
	else
		markup_text = g_strdup_printf("%s", _("Double-click to launch the music player"));

	gtk_tooltip_set_markup (tooltip, markup_text);

	gtk_tooltip_set_icon (tooltip,
		soundmenu_album_art_get_pixbuf(soundmenu->album_art));

	g_free(markup_text);

	return TRUE;
}

/* Open the image when double click.. */

static gboolean
soundmenu_album_art_frame_press_callback (GtkWidget       *event_box,
                                          GdkEventButton  *event,
                                          SoundmenuPlugin *soundmenu)
{
	gchar *command = NULL;
	const gchar *url;
	gboolean result = FALSE;
 
	if (event->button == 3) {
		//g_signal_emit_by_name (G_OBJECT (soundmenu->play_button), "button-press-event", event, &result);
		return TRUE;
	}

	if (event->type != GDK_2BUTTON_PRESS &&
	    event->type != GDK_3BUTTON_PRESS)
		return TRUE;

	if (mpris2_client_is_connected(soundmenu->mpris2) == FALSE) {
		if (g_str_nempty0(soundmenu->player)) {
			soundmenu_launch_player (soundmenu->player);
		}
		else {
			soundmenu->player = mpris2_client_auto_set_player(soundmenu->mpris2);
			if (g_str_empty0(soundmenu->player))
				soundmenu_configure(soundmenu->plugin, soundmenu);
		}
		return TRUE;
	}

	url = soundmenu_album_art_get_path(soundmenu->album_art);
	if (url) {
		command = g_strdup_printf("exo-open \"%s\"", url);
		result = g_spawn_command_line_async (command, NULL);
		if (G_UNLIKELY (result == FALSE))
			g_warning ("Unable to launch command: %s", command);

		g_free(command);
	}

	return TRUE;
}

/*
 *  Callbacks of button controls
 */

static gboolean
soundmenu_panel_button_scrolled (GtkWidget        *widget,
                                 GdkEventScroll   *event,
                                 SoundmenuPlugin *soundmenu)
{
	gdouble volume = 0.0;

	if(!mpris2_client_is_connected(soundmenu->mpris2))
		return FALSE;

	volume = mpris2_client_get_volume (soundmenu->mpris2);

	switch (event->direction) {
		case GDK_SCROLL_UP:
		case GDK_SCROLL_RIGHT:
			volume += 0.02;
			break;
		case GDK_SCROLL_DOWN:
		case GDK_SCROLL_LEFT:
			volume -= 0.02;
			break;
	}
	volume = CLAMP (volume, 0.0, 1.0);

	mpris2_client_set_volume (soundmenu->mpris2, volume);

	return FALSE;
}

/*
 *
 */

static void
mpris2_panel_plugin_loop_status (Mpris2Client *mpris2, LoopStatus loop_status, SoundmenuPlugin *soundmenu)
{
	if (loop_status == PLAYLIST)
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(soundmenu->loop_menu_item), TRUE);
	else
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(soundmenu->loop_menu_item), FALSE);

	gtk_widget_show (soundmenu->loop_menu_item);
}

static void
soundmenu_toggled_loop_action (GtkWidget *widget, SoundmenuPlugin *soundmenu)
{
	if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget)))
		mpris2_client_set_loop_status (soundmenu->mpris2, PLAYLIST);
	else
		mpris2_client_set_loop_status (soundmenu->mpris2, NONE);
}

static void
mpris2_panel_plugin_shuffle (Mpris2Client *mpris2, gboolean shuffle, SoundmenuPlugin *soundmenu)
{
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(soundmenu->shuffle_menu_item), shuffle);

	gtk_widget_show (soundmenu->shuffle_menu_item);
}

static void
soundmenu_toggled_shuffle_action (GtkWidget *widget, SoundmenuPlugin *soundmenu)
{
	mpris2_client_set_shuffle (soundmenu->mpris2,
		gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget)));
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

		if (g_str_nempty0(soundmenu->player))
			xfce_rc_write_entry    (rc, "player", soundmenu->player);

		xfce_rc_write_bool_entry (rc, "show_album_art", soundmenu->show_album_art);
		xfce_rc_write_bool_entry (rc, "huge_on_deskbar_mode", soundmenu->huge_on_deskbar_mode);
		xfce_rc_write_bool_entry (rc, "show_stop", soundmenu->show_stop);
		xfce_rc_write_bool_entry (rc, "hide_controls_if_loose", soundmenu->hide_controls_if_loose);
		#ifdef HAVE_LIBKEYBINDER
		xfce_rc_write_bool_entry (rc, "use_global_keys", soundmenu->use_global_keys);
		#endif
		#ifdef HAVE_LIBCLASTFM
		xfce_rc_write_bool_entry (rc, "use_lastfm", soundmenu_lastfm_is_supported (soundmenu->clastfm));
		if(soundmenu_lastfm_is_supported (soundmenu->clastfm)) {
			if (soundmenu_lastfm_get_user (soundmenu->clastfm))
				xfce_rc_write_entry(rc, "lastfm_user", soundmenu_lastfm_get_user (soundmenu->clastfm));
			if (soundmenu_lastfm_get_password (soundmenu->clastfm))
				xfce_rc_write_entry(rc, "lastfm_pass", soundmenu_lastfm_get_password (soundmenu->clastfm));
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
			soundmenu->hide_controls_if_loose = xfce_rc_read_bool_entry (rc, "hide_controls_if_loose", FALSE);
			#ifdef HAVE_LIBKEYBINDER
			soundmenu->use_global_keys = xfce_rc_read_bool_entry (rc, "use_global_keys", DEFAULT_GLOBAL_KEYS);
			#endif
			#ifdef HAVE_LIBCLASTFM
			soundmenu_lastfm_set_supported (soundmenu->clastfm, xfce_rc_read_bool_entry (rc, "use_lastfm", DEFAULT_LASTFM));
			soundmenu_lastfm_set_user (soundmenu->clastfm, xfce_rc_read_entry (rc, "lastfm_user", NULL));
			soundmenu_lastfm_set_password (soundmenu->clastfm, xfce_rc_read_entry (rc, "lastfm_pass", NULL));
			#endif

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
	soundmenu->hide_controls_if_loose = FALSE;
	#ifdef HAVE_LIBKEYBINDER
	soundmenu->use_global_keys = DEFAULT_GLOBAL_KEYS;
	#endif
}

#ifdef HAVE_LIBCLASTFM
static void
soundmenu_add_lastfm_menu_item (SoundmenuPlugin *soundmenu)
{
	GtkWidget *item;

	item = gtk_menu_item_new_with_label (_("Love"));
	g_signal_connect (G_OBJECT (item), "activate", G_CALLBACK (lastfm_track_love_action), soundmenu);
	gtk_menu_append (GTK_MENU(soundmenu->tools_submenu), item);

	item = gtk_menu_item_new_with_label (_("Unlove"));
	g_signal_connect (G_OBJECT (item), "activate", G_CALLBACK (lastfm_track_unlove_action), soundmenu);
	gtk_menu_append (GTK_MENU(soundmenu->tools_submenu), item);

	gtk_widget_show_all (soundmenu->tools_submenu);
}
#endif

#ifdef HAVE_LIBGLYR
static void
soundmenu_add_lyrics_menu_item (SoundmenuPlugin *soundmenu)
{
	GtkWidget *item;

	item = gtk_menu_item_new_with_mnemonic (_("Search lyrics"));
	g_signal_connect (G_OBJECT (item), "activate", G_CALLBACK (soundmenu_search_lyric_dialog), soundmenu);
	gtk_menu_append (GTK_MENU(soundmenu->tools_submenu), item);

	item = gtk_menu_item_new_with_mnemonic (_("Search artist info"));
	g_signal_connect (G_OBJECT (item), "activate", G_CALLBACK (soundmenu_search_artistinfo_dialog), soundmenu);
	gtk_menu_append (GTK_MENU(soundmenu->tools_submenu), item);

	gtk_widget_show_all (soundmenu->tools_submenu);
}
#endif

static SoundmenuPlugin *
soundmenu_new (XfcePanelPlugin *plugin)
{
	SoundmenuPlugin   *soundmenu;
	GtkOrientation panel_orientation, orientation;
	GtkWidget *ev_album_art;
	GtkWidget *separator, *loop_menu_item, *shuffle_menu_item, *tools_menu_item, *tools_submenu;
	PulseaudioButton *vol_button;
	SoundmenuAlbumArt *album_art;

	/* allocate memory for the plugin structure */
	soundmenu = panel_slice_new0 (SoundmenuPlugin);
	soundmenu->plugin = plugin;

	/**/
	soundmenu->mpris2 = mpris2_client_new ();

	#ifdef HAVE_LIBCLASTFM
	soundmenu->clastfm = soundmenu_lastfm_new ();
	#endif

	/* read the user settings */
	soundmenu_read (soundmenu);

	/* get the current orientation */

	orientation =
		(xfce_panel_plugin_get_mode (plugin) == XFCE_PANEL_PLUGIN_MODE_VERTICAL) ?
		GTK_ORIENTATION_VERTICAL : GTK_ORIENTATION_HORIZONTAL;

	panel_orientation = xfce_panel_plugin_get_orientation (plugin);

	/* create some panel widgets */

	soundmenu->layout_box = xfce_hvbox_new (panel_orientation, FALSE, 2);
	gtk_widget_show (soundmenu->layout_box);

	soundmenu->controls = mpris2_controls_new (soundmenu->mpris2);
	mpris2_controls_set_orientation (soundmenu->controls, orientation);
	gtk_widget_show (GTK_WIDGET(soundmenu->controls));

	/* some soundmenu widgets */

	ev_album_art = gtk_event_box_new ();
	gtk_event_box_set_visible_window(GTK_EVENT_BOX(ev_album_art), FALSE);
	gtk_event_box_set_above_child(GTK_EVENT_BOX(ev_album_art), TRUE);

	album_art = soundmenu_album_art_new ();
	gtk_container_add (GTK_CONTAINER (ev_album_art), GTK_WIDGET(album_art));

	vol_button = pulseaudio_button_new();

	g_object_ref(soundmenu->vol_button);

	/* Layout */

	gtk_box_pack_start (GTK_BOX(soundmenu->layout_box),
	                    GTK_WIDGET(vol_button),
	                    FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(soundmenu->layout_box),
	                    GTK_WIDGET(ev_album_art),
	                    FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX(soundmenu->layout_box),
	                    GTK_WIDGET(soundmenu->controls),
	                    FALSE, FALSE, 0);

	/* Show widgets */
	gtk_widget_show_all(GTK_WIDGET(vol_button));
	gtk_widget_show(GTK_WIDGET(album_art));
	if(soundmenu->show_album_art)
		gtk_widget_show(ev_album_art);

	if (soundmenu->show_stop)
		mpris2_controls_set_show_stop_button (soundmenu->controls, TRUE);

	/* Signal handlers */

	g_signal_connect(G_OBJECT (ev_album_art), "button_press_event",
	                 G_CALLBACK (soundmenu_album_art_frame_press_callback), soundmenu);
	g_signal_connect(G_OBJECT (ev_album_art), "scroll-event",
	                  G_CALLBACK (soundmenu_panel_button_scrolled), soundmenu);

	xfce_panel_plugin_add_action_widget (plugin, GTK_WIDGET(vol_button));
	xfce_panel_plugin_add_action_widget (plugin, GTK_WIDGET(album_art));
	xfce_panel_plugin_add_action_widget (plugin, GTK_WIDGET(ev_album_art));

	/*xfce_panel_plugin_add_action_widget (plugin, prev_button);
	xfce_panel_plugin_add_action_widget (plugin, play_button);
	xfce_panel_plugin_add_action_widget (plugin, stop_button);
	xfce_panel_plugin_add_action_widget (plugin, next_button);*/

	g_object_set (G_OBJECT(album_art), "has-tooltip", TRUE, NULL);

	g_signal_connect(G_OBJECT(album_art), "query-tooltip",
			G_CALLBACK(soundmenu_set_query_tooltip_cb), soundmenu);

	/* Attach menus actions */

	separator = gtk_separator_menu_item_new();
	xfce_panel_plugin_menu_insert_item (soundmenu->plugin, GTK_MENU_ITEM(separator));
	gtk_widget_show (separator);

	loop_menu_item = gtk_check_menu_item_new_with_mnemonic (_("Loop playlist"));
	xfce_panel_plugin_menu_insert_item (soundmenu->plugin, GTK_MENU_ITEM(loop_menu_item));
	g_signal_connect (G_OBJECT (loop_menu_item), "toggled",
	                  G_CALLBACK (soundmenu_toggled_loop_action), soundmenu);

	shuffle_menu_item = gtk_check_menu_item_new_with_mnemonic (_("Shuffle"));
	xfce_panel_plugin_menu_insert_item (soundmenu->plugin, GTK_MENU_ITEM(shuffle_menu_item));
	g_signal_connect (G_OBJECT (shuffle_menu_item), "toggled",
	                  G_CALLBACK (soundmenu_toggled_shuffle_action), soundmenu);

	separator = gtk_separator_menu_item_new();
	xfce_panel_plugin_menu_insert_item (soundmenu->plugin, GTK_MENU_ITEM(separator));
	gtk_widget_show (separator);

	tools_submenu = gtk_menu_new ();
	tools_menu_item = gtk_menu_item_new_with_mnemonic (_("Tools"));
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (tools_menu_item), tools_submenu);
	xfce_panel_plugin_menu_insert_item (soundmenu->plugin, GTK_MENU_ITEM(tools_menu_item));
	gtk_widget_show (tools_menu_item);

	soundmenu->vol_button = vol_button;
	soundmenu->album_art = album_art;
	soundmenu->ev_album_art = ev_album_art;
	soundmenu->loop_menu_item = loop_menu_item;
	soundmenu->shuffle_menu_item = shuffle_menu_item;
	soundmenu->tools_submenu = tools_submenu;

	/* Add lastfm and glyr options in panel plugin. */

	#ifdef HAVE_LIBCLASTFM
	soundmenu_add_lastfm_menu_item(soundmenu);
	#endif
	#ifdef HAVE_LIBGLYR
	soundmenu_add_lyrics_menu_item (soundmenu);
	#endif

	return soundmenu;
}

static void init_soundmenu_plugin(SoundmenuPlugin *soundmenu)
{
	g_signal_connect (G_OBJECT (soundmenu->mpris2), "connection",
	                  G_CALLBACK(mpris2_panel_plugin_coneccion), soundmenu);
	g_signal_connect (G_OBJECT (soundmenu->mpris2), "playback-status",
	                  G_CALLBACK(mpris2_panel_plugin_playback_status), soundmenu);
	g_signal_connect (G_OBJECT (soundmenu->mpris2), "metadata",
	                  G_CALLBACK(mpris2_panel_plugin_metadada), soundmenu);
	g_signal_connect (G_OBJECT (soundmenu->mpris2), "loop-status",
	                  G_CALLBACK(mpris2_panel_plugin_loop_status), soundmenu);
	g_signal_connect (G_OBJECT (soundmenu->mpris2), "shuffle",
	                  G_CALLBACK(mpris2_panel_plugin_shuffle), soundmenu);

	if (g_str_nempty0(soundmenu->player))
		mpris2_client_set_player (soundmenu->mpris2, soundmenu->player);

	/* Init the goodies services .*/

	#ifdef HAVE_LIBKEYBINDER
	soundmenu_init_keybinder();
	if (soundmenu->use_global_keys)
		keybinder_bind_keys(soundmenu);
	#endif
	#ifdef HAVE_LIBCLASTFM
	if (soundmenu_lastfm_is_supported (soundmenu->clastfm))
		soundmenu_lastfm_init (soundmenu->clastfm);
	#endif
	#ifdef HAVE_LIBGLYR
	init_glyr_related(soundmenu);
	#endif
	#ifdef HAVE_LIBNOTIFY
	soundmenu_notify_init();
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
	soundmenu_lastfm_free (soundmenu->clastfm);
	#endif
	#ifdef HAVE_LIBGLYR
	uninit_glyr_related(soundmenu);
	#endif
	#ifdef HAVE_LIBNOTIFY
	soundmenu_notify_uninit();
	#endif

	/* check if the dialog is still open. if so, destroy it */
	dialog = g_object_get_data (G_OBJECT (plugin), "dialog");
	if (G_UNLIKELY (dialog != NULL))
		gtk_widget_destroy (dialog);

	/* destroy the panel widgets */
	gtk_widget_destroy (soundmenu->layout_box);

	/* cleanup the metadata and settings */
	if (G_LIKELY (soundmenu->player != NULL))
		g_free (soundmenu->player);

	/* free the plugin structure */
	panel_slice_free (SoundmenuPlugin, soundmenu);
}

static gboolean
soundmenu_size_changed (XfcePanelPlugin *plugin,
                        gint             panel_size,
                        SoundmenuPlugin *soundmenu)
{
	GtkOrientation panel_orientation;
	gint           size, album_size, rows = 4;

	/* get the orientation of the plugin */
	panel_orientation = xfce_panel_plugin_get_orientation (plugin);

	/* set the widget size */
	if (panel_orientation == GTK_ORIENTATION_HORIZONTAL)
		gtk_widget_set_size_request (GTK_WIDGET (plugin), -1, panel_size);
	else
		gtk_widget_set_size_request (GTK_WIDGET (plugin), panel_size, -1);

	size = album_size = panel_size;

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

	gtk_widget_set_size_request (GTK_WIDGET (soundmenu->vol_button), size, size);
	mpris2_controls_set_size (soundmenu->controls, size);
	soundmenu_album_art_set_size (soundmenu->album_art, album_size);

	/* we handled the orientation */
	return TRUE;
}

static void
soundmenu_container_remove (GtkContainer *container, GtkWidget *widget)
{
	GList *list = NULL;
	gint index = 0;

	list = gtk_container_get_children (container);
	index = g_list_index (list, widget);
	if (index >= 0) {
		g_object_ref(widget);
		gtk_container_remove (container, widget);
	}
	 g_list_free (list);
}

static void
soundmenu_mode_changed (XfcePanelPlugin     *plugin,
                        XfcePanelPluginMode  mode,
                        SoundmenuPlugin     *soundmenu)
{
	GtkOrientation panel_orientation, orientation;

	orientation = (mode == XFCE_PANEL_PLUGIN_MODE_VERTICAL) ?
		GTK_ORIENTATION_VERTICAL : GTK_ORIENTATION_HORIZONTAL;
	panel_orientation = xfce_panel_plugin_get_orientation (plugin);

	soundmenu_container_remove (GTK_CONTAINER(soundmenu->controls),
	                            GTK_WIDGET(soundmenu->vol_button));
	soundmenu_container_remove (GTK_CONTAINER(soundmenu->layout_box),
	                            GTK_WIDGET(soundmenu->vol_button));

	if (mode == XFCE_PANEL_PLUGIN_MODE_DESKBAR)
	{
		if (soundmenu->huge_on_deskbar_mode) {
			mpris2_controls_set_orientation (soundmenu->controls, GTK_ORIENTATION_VERTICAL);
			gtk_box_pack_start (GTK_BOX(soundmenu->controls),
			                    GTK_WIDGET(soundmenu->vol_button),
			                    FALSE, FALSE, 0);
		}
		else {
			xfce_hvbox_set_orientation (XFCE_HVBOX (soundmenu->layout_box), GTK_ORIENTATION_HORIZONTAL);
			gtk_box_pack_start (GTK_BOX(soundmenu->layout_box),
			                    GTK_WIDGET(soundmenu->vol_button),
			                    FALSE, FALSE, 0);
		}
		mpris2_controls_set_orientation (soundmenu->controls, GTK_ORIENTATION_HORIZONTAL);
	}
	else
	{
		gtk_box_pack_start (GTK_BOX(soundmenu->layout_box),
		                    GTK_WIDGET(soundmenu->vol_button),
		                    FALSE, FALSE, 0);

		xfce_hvbox_set_orientation (XFCE_HVBOX (soundmenu->layout_box), panel_orientation);
		mpris2_controls_set_orientation (soundmenu->controls, orientation);
	}

	/* update size after orientation change */
	soundmenu_size_changed (plugin, xfce_panel_plugin_get_size (plugin), soundmenu);
}

void soundmenu_update_layout_changes (SoundmenuPlugin *soundmenu)
{
	gboolean connection = FALSE;
	connection = mpris2_client_is_connected (soundmenu->mpris2);

	/* Set visible widgets acording preferences */

	if (connection) {
		gtk_widget_show(GTK_WIDGET(soundmenu->controls));

		if(soundmenu->show_album_art)
			gtk_widget_show(soundmenu->ev_album_art);
		else
			gtk_widget_hide(soundmenu->ev_album_art);

		mpris2_controls_set_show_stop_button (soundmenu->controls, soundmenu->show_stop);
	}
	else {
		if(soundmenu->hide_controls_if_loose) {
			gtk_widget_hide(GTK_WIDGET(soundmenu->controls));
			gtk_widget_show(soundmenu->ev_album_art);
		}
		else {
			gtk_widget_show(GTK_WIDGET(soundmenu->controls));

			if(soundmenu->show_album_art)
				gtk_widget_show(soundmenu->ev_album_art);
			else
				gtk_widget_hide(soundmenu->ev_album_art);

			mpris2_controls_set_show_stop_button (soundmenu->controls, soundmenu->show_stop);
		}
	}

	/* Update orientations and size. */

	soundmenu_mode_changed (soundmenu->plugin,
	                        xfce_panel_plugin_get_mode (soundmenu->plugin),
	                        soundmenu);
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
	gtk_container_add (GTK_CONTAINER (plugin), soundmenu->layout_box);

	/* connect plugin signals */
	g_signal_connect (G_OBJECT (plugin), "free-data",
				G_CALLBACK (soundmenu_free), soundmenu);

	g_signal_connect (G_OBJECT (plugin), "save",
				G_CALLBACK (soundmenu_save), soundmenu);

	g_signal_connect (G_OBJECT (plugin), "size-changed",
				G_CALLBACK (soundmenu_size_changed), soundmenu);

	g_signal_connect (G_OBJECT (plugin), "mode-changed",
				G_CALLBACK (soundmenu_mode_changed), soundmenu);

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
