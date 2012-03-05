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

#include "soundmenu-plugin.h"
#include "soundmenu-dialogs.h"

#define PLUGIN_WEBSITE "https://github.com/matiasdelellis/xfce4-soundmenu-plugin/"

static void
soundmenu_configure_response (GtkWidget    *dialog,
                           gint          response,
                           SoundmenuPlugin *soundmenu)
{
	gboolean result;
	gchar *rule = NULL, *player = NULL;

	if (response == GTK_RESPONSE_HELP)
	{
		/* show help */
		result = g_spawn_command_line_async ("exo-open --launch WebBrowser " PLUGIN_WEBSITE, NULL);

		if (G_UNLIKELY (result == FALSE))
			g_warning ("Unable to open the following url: %s", PLUGIN_WEBSITE);
	}
	else
	{
		player = g_strdup(gtk_entry_get_text(GTK_ENTRY(soundmenu->w_player)));
		if(G_LIKELY (player != NULL))
		{
			if (G_LIKELY (soundmenu->player != NULL))
				g_free (soundmenu->player);
			soundmenu->player = player;

			rule = g_strdup_printf ("type='signal', sender='%s'", soundmenu->dbus_name);
			dbus_bus_remove_match (soundmenu->connection, rule, NULL);
			g_free(rule);

			if (G_LIKELY (soundmenu->dbus_name != NULL))
				g_free(soundmenu->dbus_name);
			soundmenu->dbus_name = g_strdup_printf("org.mpris.MediaPlayer2.%s", soundmenu->player);

			rule = g_strdup_printf ("type='signal', sender='%s'", soundmenu->dbus_name);
			dbus_bus_add_match (soundmenu->connection, rule, NULL);
			g_free(rule);
		}
		#ifdef HAVE_LIBCLASTFM
		soundmenu->clastfm->lastfm_support = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(soundmenu->lw.lastfm_w));

		if (G_LIKELY (soundmenu->clastfm->lastfm_user != NULL))
			g_free (soundmenu->clastfm->lastfm_user);
		soundmenu->clastfm->lastfm_user = g_strdup(gtk_entry_get_text(GTK_ENTRY(soundmenu->lw.lastfm_uname_w)));

		if (G_LIKELY (soundmenu->clastfm->lastfm_pass != NULL))
			g_free (soundmenu->clastfm->lastfm_pass);
		soundmenu->clastfm->lastfm_pass = g_strdup(gtk_entry_get_text(GTK_ENTRY(soundmenu->lw.lastfm_pass_w)));
		#endif

		/* remove the dialog data from the plugin */
		g_object_set_data (G_OBJECT (soundmenu->plugin), "dialog", NULL);

		/* unlock the panel menu */
		xfce_panel_plugin_unblock_menu (soundmenu->plugin);

		/* save the plugin */
		soundmenu_save (soundmenu->plugin, soundmenu);

		/* destroy the properties dialog */
		gtk_widget_destroy (dialog);
	}
	mpris2_get_player_status (soundmenu);
	#ifdef HAVE_LIBCLASTFM
	if (soundmenu->clastfm->session_id == NULL)
		just_init_lastfm(soundmenu);
	#endif
}

static void
refresh_player (GtkEntry        *player_entry,
		gint             position,
		GdkEventButton  *event,
		SoundmenuPlugin *soundmenu)
{
	gchar *player = NULL;

        if (position == GTK_ENTRY_ICON_SECONDARY) {
		player = mpris2_get_player(soundmenu);
        	gtk_entry_set_text(GTK_ENTRY(soundmenu->w_player), player);
        }
}

static void
toggle_show_album_art(GtkToggleButton *button, SoundmenuPlugin    *soundmenu)
{
	soundmenu->show_album_art = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));

	if(soundmenu->show_album_art)
		gtk_widget_show(soundmenu->ev_album_art);
	else
		gtk_widget_hide(soundmenu->ev_album_art);
}

static void
toggle_show_stop(GtkToggleButton *button, SoundmenuPlugin    *soundmenu)
{
	soundmenu->show_stop = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));

	if(soundmenu->show_stop)
		gtk_widget_show(soundmenu->stop_button);
	else
		gtk_widget_hide(soundmenu->stop_button);
}

#ifdef HAVE_LIBKEYBINDER
static void
toggle_use_global_keys_check(GtkToggleButton *button, SoundmenuPlugin    *soundmenu)
{
	soundmenu->use_global_keys = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));
	if (soundmenu->use_global_keys)
		keybinder_bind_keys(soundmenu);
	else
		keybinder_unbind_keys(soundmenu);
}
#endif

#ifdef HAVE_LIBCLASTFM
static void
toggle_lastfm(GtkToggleButton *button, SoundmenuPlugin    *soundmenu)
{
	gboolean is_active;

	is_active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
						 soundmenu->lw.lastfm_w));

	gtk_widget_set_sensitive(soundmenu->lw.lastfm_uname_w, is_active);
	gtk_widget_set_sensitive(soundmenu->lw.lastfm_pass_w, is_active);

	if(!is_active && soundmenu->clastfm->session_id) {
		LASTFM_dinit(soundmenu->clastfm->session_id);
		soundmenu->clastfm->session_id = NULL;
	}
}
#endif

void
soundmenu_configure (XfcePanelPlugin *plugin,
                  SoundmenuPlugin    *soundmenu)
{
	GtkWidget *dialog;
	GtkWidget *pref_table, *player_label, *player_entry, *show_album_art_check, *show_stop_check;

	#ifdef HAVE_LIBKEYBINDER
	GtkWidget *use_global_keys_check;
	#endif
	#ifdef HAVE_LIBCLASTFM
	GtkWidget *support_lastfm, *lastfm_label_user, *lastfm_entry_user, *lastfm_label_pass, *lastfm_entry_pass;
	#endif

	/* block the plugin menu */
	xfce_panel_plugin_block_menu (plugin);

	/* create the dialog */
	dialog = xfce_titled_dialog_new_with_buttons (_("Sound menu Plugin"),
							GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (plugin))),
							GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR,
							GTK_STOCK_HELP, GTK_RESPONSE_HELP,
							GTK_STOCK_CLOSE, GTK_RESPONSE_OK,
							NULL);
	/* center dialog on the screen */
	gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);

	/* set dialog icon */
	gtk_window_set_icon_name (GTK_WINDOW (dialog), "xfce4-settings");

	pref_table = gtk_table_new(7, 2, FALSE);
 	gtk_table_set_col_spacings(GTK_TABLE(pref_table), 5);
 	gtk_table_set_row_spacings(GTK_TABLE(pref_table), 2);

	player_label = gtk_label_new(_("Player"));
	gtk_misc_set_alignment(GTK_MISC (player_label), 0, 0);

	player_entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(player_entry), soundmenu->player);
        gtk_entry_set_icon_from_stock (GTK_ENTRY(player_entry), GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_REFRESH);
        g_signal_connect (G_OBJECT(player_entry), "icon-press",
			  G_CALLBACK (refresh_player), soundmenu);
	soundmenu->w_player = player_entry;

	show_album_art_check = gtk_check_button_new_with_label(_("Show the cover art on the panel"));
	if(soundmenu->show_album_art)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(show_album_art_check), TRUE);
	g_signal_connect (G_OBJECT(show_album_art_check), "toggled",
				G_CALLBACK(toggle_show_album_art), soundmenu);

	show_stop_check = gtk_check_button_new_with_label(_("Show stop button"));
	if(soundmenu->show_stop)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(show_stop_check), TRUE);
	g_signal_connect (G_OBJECT(show_stop_check), "toggled",
				G_CALLBACK(toggle_show_stop), soundmenu);

	#ifdef HAVE_LIBKEYBINDER
	use_global_keys_check = gtk_check_button_new_with_label(_("Use multimedia keys"));
	if(soundmenu->use_global_keys)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(use_global_keys_check), TRUE);
	g_signal_connect (G_OBJECT(use_global_keys_check), "toggled",
				G_CALLBACK(toggle_use_global_keys_check), soundmenu);
	#endif

	#ifdef HAVE_LIBCLASTFM
	support_lastfm = gtk_check_button_new_with_label(_("Scrobble on Last.fm"));
	g_signal_connect (G_OBJECT(support_lastfm), "toggled",
				G_CALLBACK(toggle_lastfm), soundmenu);
	soundmenu->lw.lastfm_w = support_lastfm;

	lastfm_label_user = gtk_label_new(_("Last.fm user"));
	gtk_misc_set_alignment(GTK_MISC (lastfm_label_user), 0, 0);

	lastfm_entry_user = gtk_entry_new();
	if (G_LIKELY (soundmenu->clastfm->lastfm_user != NULL))
		gtk_entry_set_text(GTK_ENTRY(lastfm_entry_user), soundmenu->clastfm->lastfm_user);
	soundmenu->lw.lastfm_uname_w = lastfm_entry_user;

	lastfm_label_pass = gtk_label_new(_("Last.fm password"));
	gtk_misc_set_alignment(GTK_MISC (lastfm_label_pass), 0, 0);

	lastfm_entry_pass = gtk_entry_new();
	if (G_LIKELY (soundmenu->clastfm->lastfm_pass != NULL))
		gtk_entry_set_text(GTK_ENTRY(lastfm_entry_pass), soundmenu->clastfm->lastfm_pass);
	gtk_entry_set_visibility(GTK_ENTRY(lastfm_entry_pass), FALSE);
	gtk_entry_set_invisible_char(GTK_ENTRY(lastfm_entry_pass), '*');
	soundmenu->lw.lastfm_pass_w = lastfm_entry_pass;
	#endif

	gtk_table_attach(GTK_TABLE (pref_table), player_label,
			0, 1, 0, 1,
			GTK_FILL, GTK_SHRINK,
			0, 0);
	gtk_table_attach(GTK_TABLE (pref_table), player_entry,
			1, 2, 0, 1,
			GTK_FILL|GTK_EXPAND, GTK_SHRINK,
			0, 0);

	gtk_table_attach(GTK_TABLE (pref_table), show_album_art_check,
			0, 2, 1, 2,
			GTK_FILL, GTK_SHRINK,
			0, 0);

	gtk_table_attach(GTK_TABLE (pref_table), show_stop_check,
			0, 2, 2, 3,
			GTK_FILL, GTK_SHRINK,
			0, 0);

	#ifdef HAVE_LIBKEYBINDER
	gtk_table_attach(GTK_TABLE (pref_table), use_global_keys_check,
			0, 2, 3, 4,
			GTK_FILL, GTK_SHRINK,
			0, 0);
	#endif

	#ifdef HAVE_LIBCLASTFM
	gtk_table_attach(GTK_TABLE (pref_table), support_lastfm,
			0, 2, 4, 5,
			GTK_FILL, GTK_SHRINK,
			0, 0);

	gtk_table_attach(GTK_TABLE (pref_table), lastfm_label_user,
			0, 1, 5, 6,
			GTK_FILL, GTK_SHRINK,
			0, 0);
	gtk_table_attach(GTK_TABLE (pref_table), lastfm_entry_user,
			1, 2, 5, 6,
			GTK_FILL|GTK_EXPAND, GTK_SHRINK,
			0, 0);

	gtk_table_attach(GTK_TABLE (pref_table), lastfm_label_pass,
			0, 1, 7, 8,
			GTK_FILL, GTK_SHRINK,
			0, 0);
	gtk_table_attach(GTK_TABLE (pref_table), lastfm_entry_pass,
			1, 2, 7, 8,
			GTK_FILL|GTK_EXPAND, GTK_SHRINK,
			0, 0);
	#endif

	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), pref_table, TRUE, TRUE, 6);

	#ifdef HAVE_LIBCLASTFM
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(support_lastfm), soundmenu->clastfm->lastfm_support);
	#endif

	/* link the dialog to the plugin, so we can destroy it when the plugin
	* is closed, but the dialog is still open */
	g_object_set_data (G_OBJECT (plugin), "dialog", dialog);

	/* connect the reponse signal to the dialog */
	g_signal_connect (G_OBJECT (dialog), "response",
				G_CALLBACK(soundmenu_configure_response), soundmenu);

	/* show the entire dialog */
	gtk_widget_show_all(dialog);
}

void
soundmenu_about (XfcePanelPlugin *plugin)
{
	gboolean result;

	result = g_spawn_command_line_async ("exo-open --launch WebBrowser " PLUGIN_WEBSITE, NULL);

	if (G_UNLIKELY (result == FALSE))
		g_warning ("Unable to open the following url: %s", PLUGIN_WEBSITE);
}
