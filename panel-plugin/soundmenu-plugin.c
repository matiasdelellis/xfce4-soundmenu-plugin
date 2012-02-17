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
#include "soundmenu-dialogs.h"
#include "soundmenu-lastfm.h"
#include "soundmenu-mpris2.h"
#include "soundmenu-utils.h"
#include "soundmenu-related.h"

/* default settings */
#define DEFAULT_PLAYER "pragha"
#define DEFAULT_SHOW_STOP TRUE
#define DEFAULT_GLOBAL_KEYS TRUE
#define DEFAULT_LASTFM FALSE

/* prototypes */

static void
soundmenu_construct (XfcePanelPlugin *plugin);

/* register the plugin */

XFCE_PANEL_PLUGIN_REGISTER_EXTERNAL (soundmenu_construct);

/* Function to update the soundmenu state */

void
play_button_toggle_state (SoundmenuPlugin *soundmenu)
{
	if ((soundmenu->state == ST_PAUSED) || (soundmenu->state == ST_STOPPED))
		gtk_button_set_image(GTK_BUTTON(soundmenu->play_button), soundmenu->image_play);
	else
		gtk_button_set_image(GTK_BUTTON(soundmenu->play_button), soundmenu->image_pause);
}

gboolean status_get_tooltip_cb (GtkWidget        *widget,
					gint              x,
					gint              y,
					gboolean          keyboard_mode,
					GtkTooltip       *tooltip,
					SoundmenuPlugin *soundmenu)
{
	gchar *markup_text = NULL, *length = NULL;

	length = convert_length_str(soundmenu->metadata->length);

	if (soundmenu->state == ST_STOPPED)
		markup_text = g_strdup_printf("%s", _("Stopped"));
	else {
		markup_text = g_markup_printf_escaped(_("<b>%s</b> (%s)\nby %s in %s"),
						(soundmenu->metadata->title && strlen(soundmenu->metadata->title)) ?
						soundmenu->metadata->title : soundmenu->metadata->url,
						length,
						(soundmenu->metadata->artist && strlen(soundmenu->metadata->artist)) ?
						soundmenu->metadata->artist : _("Unknown Artist"),
						(soundmenu->metadata->album && strlen(soundmenu->metadata->album)) ?
						soundmenu->metadata->album : _("Unknown Album"));
	}

	gtk_tooltip_set_markup (tooltip, markup_text);

	g_free(markup_text);
	g_free(length);

	return TRUE;
}

void
soundmenu_update_state(gchar *state, SoundmenuPlugin *soundmenu)
{
	if (0 == g_ascii_strcasecmp(state, "Playing"))
		soundmenu->state = ST_PLAYING;
	else if (0 == g_ascii_strcasecmp(state, "Paused"))
		soundmenu->state = ST_PAUSED;
	else {
		soundmenu->state = ST_STOPPED;
	}

	play_button_toggle_state(soundmenu);
	#ifdef HAVE_LIBCLASTFM
	if (soundmenu->clastfm->lastfm_support)
		update_lastfm(soundmenu);
	#endif
}

/* Callbacks of button controls */

void
prev_button_handler(GtkButton *button, SoundmenuPlugin *soundmenu)
{
	mpris2_send_message (soundmenu, "Previous");
}

void
play_button_handler(GtkButton *button, SoundmenuPlugin *soundmenu)
{
	mpris2_send_message (soundmenu, "PlayPause");
}

void
stop_button_handler(GtkButton *button, SoundmenuPlugin    *soundmenu)
{
	mpris2_send_message (soundmenu, "Stop");
}

void
next_button_handler(GtkButton *button, SoundmenuPlugin    *soundmenu)
{
	mpris2_send_message (soundmenu, "Next");
}

#ifdef HAVE_LIBKEYBINDER
void keybind_play_handler (const char *keystring, SoundmenuPlugin *soundmenu)
{
	mpris2_send_message (soundmenu, "PlayPause");
}
void keybind_stop_handler (const char *keystring, SoundmenuPlugin *soundmenu)
{
	mpris2_send_message (soundmenu, "Stop");
}
void keybind_prev_handler (const char *keystring, SoundmenuPlugin *soundmenu)
{
	mpris2_send_message (soundmenu, "Previous");
}
void keybind_next_handler (const char *keystring, SoundmenuPlugin *soundmenu)
{
	mpris2_send_message (soundmenu, "Next");
}

void keybinder_bind_keys(SoundmenuPlugin *soundmenu)
{
	keybinder_bind("XF86AudioPlay", (KeybinderHandler) keybind_play_handler, soundmenu);
	keybinder_bind("XF86AudioStop", (KeybinderHandler) keybind_stop_handler, soundmenu);
	keybinder_bind("XF86AudioPrev", (KeybinderHandler) keybind_prev_handler, soundmenu);
	keybinder_bind("XF86AudioNext", (KeybinderHandler) keybind_next_handler, soundmenu);
}

void keybinder_unbind_keys(SoundmenuPlugin *soundmenu)
{
	keybinder_unbind("XF86AudioPlay", (KeybinderHandler) keybind_play_handler);
	keybinder_unbind("XF86AudioStop", (KeybinderHandler) keybind_stop_handler);
	keybinder_unbind("XF86AudioPrev", (KeybinderHandler) keybind_prev_handler);
	keybinder_unbind("XF86AudioNext", (KeybinderHandler) keybind_next_handler);
}
#endif

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
			if (soundmenu->player == NULL)
				soundmenu->player = mpris2_get_player(soundmenu);
			if (soundmenu->player == NULL)
				soundmenu->player = g_strdup (DEFAULT_PLAYER);
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

	soundmenu->player = mpris2_get_player(soundmenu);
	if (soundmenu->player == NULL)
		soundmenu->player = g_strdup (DEFAULT_PLAYER);
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
void
soundmenu_add_lastfm_menu_item (SoundmenuPlugin *soundmenu)
{
	GtkWidget *submenu, *item;

	item = gtk_menu_item_new_with_mnemonic (_("Last.fm"));
	xfce_panel_plugin_menu_insert_item (soundmenu->plugin, GTK_MENU_ITEM(item));
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
void
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
	GtkOrientation orientation;
	GtkWidget *play_button, *stop_button, *prev_button, *next_button;
	DBusConnection *connection;
	Metadata *metadata;
	gchar *rule = NULL;

	/* allocate memory for the plugin structure */
	soundmenu = panel_slice_new0 (SoundmenuPlugin);
	soundmenu->plugin = plugin;

	#ifdef HAVE_LIBCLASTFM
	soundmenu->clastfm = g_slice_new0(struct con_lastfm);
	#endif

	metadata = malloc_metadata();
	soundmenu->metadata = metadata;

	/* Init dbus connection */
	connection = dbus_bus_get (DBUS_BUS_SESSION, NULL);
	soundmenu->connection = connection;

	/* read the user settings */
	soundmenu_read (soundmenu);

	/* Init the services */
	#ifdef HAVE_LIBKEYBINDER
	keybinder_init ();
	if (soundmenu->use_global_keys)
		keybinder_bind_keys(soundmenu);
	#endif
	#ifdef HAVE_LIBCLASTFM
	init_lastfm_idle_timeout(soundmenu);
	#endif
	#ifdef HAVE_LIBGLYR
	init_glyr_related(soundmenu);
	#endif

	/* get the current orientation */
	orientation = xfce_panel_plugin_get_orientation (plugin);

	/* create some panel widgets */

	soundmenu->hvbox = xfce_hvbox_new (orientation, FALSE, 2);
	gtk_widget_show (soundmenu->hvbox);

	/* some soundmenu widgets */

	prev_button = gtk_button_new();
	play_button = gtk_button_new();
	stop_button = gtk_button_new();
	next_button = gtk_button_new();

	gtk_button_set_relief(GTK_BUTTON(prev_button), GTK_RELIEF_NONE);
	gtk_button_set_relief(GTK_BUTTON(stop_button), GTK_RELIEF_NONE);
	gtk_button_set_relief(GTK_BUTTON(next_button), GTK_RELIEF_NONE);
	gtk_button_set_relief(GTK_BUTTON(play_button), GTK_RELIEF_NONE);

	gtk_button_set_image(GTK_BUTTON(prev_button),
			     gtk_image_new_from_stock(GTK_STOCK_MEDIA_PREVIOUS,
						      GTK_ICON_SIZE_LARGE_TOOLBAR));
	gtk_button_set_image(GTK_BUTTON(stop_button),
			     gtk_image_new_from_stock(GTK_STOCK_MEDIA_STOP,
						      GTK_ICON_SIZE_LARGE_TOOLBAR));
	gtk_button_set_image(GTK_BUTTON(next_button),
			     gtk_image_new_from_stock(GTK_STOCK_MEDIA_NEXT,
						      GTK_ICON_SIZE_LARGE_TOOLBAR));

	soundmenu->image_pause =
		gtk_image_new_from_stock(GTK_STOCK_MEDIA_PAUSE,
					 GTK_ICON_SIZE_LARGE_TOOLBAR);
	soundmenu->image_play =
		gtk_image_new_from_stock(GTK_STOCK_MEDIA_PLAY,
					 GTK_ICON_SIZE_LARGE_TOOLBAR);

	g_object_ref(soundmenu->image_play);
	g_object_ref(soundmenu->image_pause);

	gtk_button_set_image(GTK_BUTTON(play_button),
			     soundmenu->image_play);

	gtk_box_pack_start(GTK_BOX(soundmenu->hvbox),
			   GTK_WIDGET(prev_button),
			   FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(soundmenu->hvbox),
			   GTK_WIDGET(play_button),
			   FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(soundmenu->hvbox),
			   GTK_WIDGET(stop_button),
			   FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(soundmenu->hvbox),
			   GTK_WIDGET(next_button),
			   FALSE, FALSE, 0);

	gtk_widget_show(prev_button);
	gtk_widget_show(play_button);
	if(soundmenu->show_stop)
		gtk_widget_show(stop_button);
	gtk_widget_show(next_button);

	/* Signal handlers */

	g_signal_connect(G_OBJECT(prev_button), "clicked",
			 G_CALLBACK(prev_button_handler), soundmenu);
	g_signal_connect(G_OBJECT(play_button), "clicked",
			 G_CALLBACK(play_button_handler), soundmenu);
	g_signal_connect(G_OBJECT(stop_button), "clicked",
			 G_CALLBACK(stop_button_handler), soundmenu);
	g_signal_connect(G_OBJECT(next_button), "clicked",
			 G_CALLBACK(next_button_handler), soundmenu);

	xfce_panel_plugin_add_action_widget (plugin, prev_button);
	xfce_panel_plugin_add_action_widget (plugin, play_button);
	xfce_panel_plugin_add_action_widget (plugin, stop_button);
	xfce_panel_plugin_add_action_widget (plugin, next_button);

	g_object_set (G_OBJECT(prev_button), "has-tooltip", TRUE, NULL);
	g_object_set (G_OBJECT(play_button), "has-tooltip", TRUE, NULL);
	g_object_set (G_OBJECT(stop_button), "has-tooltip", TRUE, NULL);
	g_object_set (G_OBJECT(next_button), "has-tooltip", TRUE, NULL);

	g_signal_connect(G_OBJECT(prev_button), "query-tooltip",
			G_CALLBACK(status_get_tooltip_cb), soundmenu);
	g_signal_connect(G_OBJECT(play_button), "query-tooltip",
			G_CALLBACK(status_get_tooltip_cb), soundmenu);
	g_signal_connect(G_OBJECT(stop_button), "query-tooltip",
			G_CALLBACK(status_get_tooltip_cb), soundmenu);
	g_signal_connect(G_OBJECT(next_button), "query-tooltip",
			G_CALLBACK(status_get_tooltip_cb), soundmenu);

	/* FIXME:
	 * See comments in the function panel_button_scrolled.
	g_signal_connect (G_OBJECT (play_button), "scroll-event",
			G_CALLBACK (panel_button_scrolled), soundmenu);*/

	soundmenu->prev_button = prev_button;
	soundmenu->play_button = play_button;
	soundmenu->stop_button = stop_button;
	soundmenu->next_button = next_button;

	/* Add lastfm menu  and search lyrics in panel plugin */
	#ifdef HAVE_LIBCLASTFM
	soundmenu_add_lastfm_menu_item(soundmenu);
	#endif
	#ifdef HAVE_LIBGLYR
	soundmenu_add_lyrics_menu_item (soundmenu);
	#endif

	/* Soundmenu dbus helpers */

	soundmenu->dbus_name = g_strdup_printf("org.mpris.MediaPlayer2.%s", soundmenu->player);

	rule = g_strdup_printf ("type='signal', sender='%s'", soundmenu->dbus_name);
	dbus_bus_add_match (connection, rule, NULL);
	g_free(rule);
  
	dbus_connection_add_filter (connection, mpris2_dbus_filter, soundmenu, NULL);
	dbus_connection_setup_with_g_main (connection, NULL);



	mpris2_get_player_status (soundmenu);

	return soundmenu;
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



static void
soundmenu_orientation_changed (XfcePanelPlugin *plugin,
                            GtkOrientation   orientation,
                            SoundmenuPlugin    *soundmenu)
{
	/* change the orienation of the box */
	xfce_hvbox_set_orientation (XFCE_HVBOX (soundmenu->hvbox), orientation);
}



static gboolean
soundmenu_size_changed (XfcePanelPlugin *plugin,
                     gint             size,
                     SoundmenuPlugin    *soundmenu)
{
	GtkOrientation orientation;

	/* get the orientation of the plugin */
	orientation = xfce_panel_plugin_get_orientation (plugin);

	/* set the widget size */
	if (orientation == GTK_ORIENTATION_HORIZONTAL)
		gtk_widget_set_size_request (GTK_WIDGET (plugin), -1, size);
	else
		gtk_widget_set_size_request (GTK_WIDGET (plugin), size, -1);

	/* we handled the orientation */
	return TRUE;
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

	g_signal_connect (G_OBJECT (plugin), "orientation-changed",
				G_CALLBACK (soundmenu_orientation_changed), soundmenu);

	/* show the configure menu item and connect signal */
	xfce_panel_plugin_menu_show_configure (plugin);

	g_signal_connect (G_OBJECT (plugin), "configure-plugin",
				G_CALLBACK (soundmenu_configure), soundmenu);

	/* show the about menu item and connect signal */
	xfce_panel_plugin_menu_show_about (plugin);

	g_signal_connect (G_OBJECT (plugin), "about",
				G_CALLBACK (soundmenu_about), NULL);
}
