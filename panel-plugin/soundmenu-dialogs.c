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

#include "soundmenu-dialogs.h"
#include "soundmenu-dialog-hig.h"
#include "soundmenu-dbus.h"
#include "mpris2-utils.h"
#include "libmpris2client.h"

#ifdef HAVE_LIBKEYBINDER
#include "soundmenu-keybinder.h"
#endif
#ifdef HAVE_LIBCLASTFM
#include "soundmenu-lastfm.h"
#endif

#include "soundmenu-panel-plugin.h"

#define PLUGIN_WEBSITE "https://github.com/matiasdelellis/xfce4-soundmenu-plugin/"

static void
soundmenu_configure_response (GtkWidget       *dialog,
                              gint             response,
                              SoundmenuPlugin *soundmenu)
{
	gboolean result;
	const gchar *player = NULL;

	if (response == GTK_RESPONSE_HELP) {
		result = g_spawn_command_line_async ("exo-open --launch WebBrowser " PLUGIN_WEBSITE, NULL);

		if (G_UNLIKELY (result == FALSE))
			g_warning ("Unable to open the following url: %s", PLUGIN_WEBSITE);
	}
	else {
		player = gtk_entry_get_text (GTK_ENTRY(soundmenu->w_player));
		if (g_str_nempty0 (player)) {
			if (g_str_nempty0(soundmenu->player))
				g_free (soundmenu->player);
			soundmenu->player = g_strdup(player);

			mpris2_client_set_player (soundmenu->mpris2, player);
			soundmenu_mpris2_reinit_dbus (soundmenu);
		}

		#ifdef HAVE_LIBCLASTFM
		soundmenu_lastfm_set_supported (soundmenu->clastfm,
		                                gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(soundmenu->lw.lastfm_w)));
		soundmenu_lastfm_set_user (soundmenu->clastfm,
		                           gtk_entry_get_text(GTK_ENTRY(soundmenu->lw.lastfm_uname_w)));
		soundmenu_lastfm_set_password (soundmenu->clastfm,
		                               gtk_entry_get_text(GTK_ENTRY(soundmenu->lw.lastfm_pass_w)));
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
	#ifdef HAVE_LIBCLASTFM
    if (soundmenu_lastfm_is_supported (soundmenu->clastfm))
		soundmenu_lastfm_init (soundmenu->clastfm);
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
		player = soundmenu_get_mpris2_player_running(soundmenu);
		gtk_entry_set_text(GTK_ENTRY(soundmenu->w_player), player);
	}
}

static void
toggle_show_album_art(GtkToggleButton *button,
                      SoundmenuPlugin *soundmenu)
{
	soundmenu_set_visible_album_art (soundmenu,
	                                 gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)));
}

#if LIBXFCE4PANEL_CHECK_VERSION (4,9,0)
static void
toggle_huge_on_deskbar_mode(GtkToggleButton *button,
                            SoundmenuPlugin *soundmenu)
{
	soundmenu_set_huge_album_art (soundmenu,
	                              gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)));
}
#endif

static void
toggle_show_stop(GtkToggleButton *button,
                 SoundmenuPlugin *soundmenu)
{
	soundmenu_set_visible_stop_button (soundmenu,
	                                   gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)));
}

static void
toggle_hide_controls_if_loose(GtkToggleButton *button,
                              SoundmenuPlugin *soundmenu)
{
	soundmenu->hide_controls_if_loose = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));

	soundmenu_update_layout_changes (soundmenu);
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
toggle_lastfm(GtkToggleButton *button,
              SoundmenuPlugin *soundmenu)
{
	gboolean is_active;

	is_active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
						 soundmenu->lw.lastfm_w));

	gtk_widget_set_sensitive(soundmenu->lw.lastfm_uname_w, is_active);
	gtk_widget_set_sensitive(soundmenu->lw.lastfm_pass_w, is_active);

	if(!is_active && soundmenu_lastfm_is_initiated (soundmenu->clastfm))
		soundmenu_lastfm_uninit (soundmenu->clastfm);
}
#endif

void
soundmenu_configure (XfcePanelPlugin *plugin,
                     SoundmenuPlugin *soundmenu)
{
	GtkWidget *dialog;
	GtkWidget *pref_table, *player_label, *player_entry, *show_album_art_check, *huge_on_deskbar_mode_check, *show_stop_check, *hide_controls_if_loose_check;
	guint row = 0;

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

	pref_table = gtk_table_new(8, 2, FALSE);
 	gtk_table_set_col_spacings(GTK_TABLE(pref_table), 5);
 	gtk_table_set_row_spacings(GTK_TABLE(pref_table), 2);

	player_label = gtk_label_new(_("Player"));
	gtk_misc_set_alignment(GTK_MISC (player_label), 0, 0);

	player_entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(player_entry), soundmenu->player);
        gtk_entry_set_icon_from_stock (GTK_ENTRY(player_entry), GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_REFRESH);
        g_signal_connect (G_OBJECT(player_entry), "icon-press",
			  G_CALLBACK (refresh_player), soundmenu);
	gtk_entry_set_activates_default (GTK_ENTRY(player_entry), TRUE);
	soundmenu->w_player = player_entry;

	show_album_art_check = gtk_check_button_new_with_label(_("Show the cover art on the panel"));
	if (soundmenu_get_visible_album_art(soundmenu))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(show_album_art_check), TRUE);
	g_signal_connect (G_OBJECT(show_album_art_check), "toggled",
	                  G_CALLBACK(toggle_show_album_art), soundmenu);

	#if LIBXFCE4PANEL_CHECK_VERSION (4,9,0)
	huge_on_deskbar_mode_check = gtk_check_button_new_with_label(_("Show huge cover art when deskbar panel mode"));
	if (soundmenu_get_huge_album_art(soundmenu))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(huge_on_deskbar_mode_check), TRUE);
	g_signal_connect (G_OBJECT(huge_on_deskbar_mode_check), "toggled",
	                  G_CALLBACK(toggle_huge_on_deskbar_mode), soundmenu);
	#endif

	show_stop_check = gtk_check_button_new_with_label(_("Show stop button"));
	if (soundmenu_get_visible_stop_button (soundmenu))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(show_stop_check), TRUE);
	g_signal_connect (G_OBJECT(show_stop_check), "toggled",
	                  G_CALLBACK(toggle_show_stop), soundmenu);

	hide_controls_if_loose_check = gtk_check_button_new_with_label(_("Hide the controls if the player is not present"));
	if(soundmenu->hide_controls_if_loose)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hide_controls_if_loose_check), TRUE);
	g_signal_connect (G_OBJECT(hide_controls_if_loose_check), "toggled",
				G_CALLBACK(toggle_hide_controls_if_loose), soundmenu);

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

	lastfm_label_user = gtk_label_new(_("User"));
	gtk_misc_set_alignment(GTK_MISC (lastfm_label_user), 0, 0);

	lastfm_entry_user = gtk_entry_new();
	gtk_entry_set_text (GTK_ENTRY(lastfm_entry_user), soundmenu_lastfm_get_user (soundmenu->clastfm));
	soundmenu->lw.lastfm_uname_w = lastfm_entry_user;

	lastfm_label_pass = gtk_label_new(_("Password"));
	gtk_misc_set_alignment(GTK_MISC (lastfm_label_pass), 0, 0);

	lastfm_entry_pass = gtk_entry_new();
	gtk_entry_set_text (GTK_ENTRY(lastfm_entry_pass), soundmenu_lastfm_get_password (soundmenu->clastfm));
	gtk_entry_set_visibility(GTK_ENTRY(lastfm_entry_pass), FALSE);
	gtk_entry_set_invisible_char(GTK_ENTRY(lastfm_entry_pass), '*');
	gtk_entry_set_activates_default (GTK_ENTRY(lastfm_entry_pass), TRUE);
	soundmenu->lw.lastfm_pass_w = lastfm_entry_pass;
	#endif

	pref_table = soundmenu_hig_workarea_table_new();

	soundmenu_hig_workarea_table_add_section_title(pref_table, &row, _("Player"));
	soundmenu_hig_workarea_table_add_row (pref_table, &row, player_label, player_entry);

	soundmenu_hig_workarea_table_add_section_title(pref_table, &row, _("Appearance"));
	soundmenu_hig_workarea_table_add_wide_control(pref_table, &row, show_album_art_check);
	#if LIBXFCE4PANEL_CHECK_VERSION (4,9,0)
	soundmenu_hig_workarea_table_add_wide_control(pref_table, &row, huge_on_deskbar_mode_check);
	#endif
	soundmenu_hig_workarea_table_add_wide_control(pref_table, &row, show_stop_check);
	soundmenu_hig_workarea_table_add_wide_control(pref_table, &row, hide_controls_if_loose_check);

	soundmenu_hig_workarea_table_add_section_title(pref_table, &row, _("Behavior"));
	#ifdef HAVE_LIBKEYBINDER
	soundmenu_hig_workarea_table_add_wide_control(pref_table, &row, use_global_keys_check);
	#endif
	#ifdef HAVE_LIBCLASTFM
	soundmenu_hig_workarea_table_add_section_title(pref_table, &row, _("Last.fm"));
	soundmenu_hig_workarea_table_add_wide_control(pref_table, &row, support_lastfm);
	soundmenu_hig_workarea_table_add_row (pref_table, &row, lastfm_label_user, lastfm_entry_user);
	soundmenu_hig_workarea_table_add_row (pref_table, &row, lastfm_label_pass, lastfm_entry_pass);
	#endif
	soundmenu_hig_workarea_table_finish(pref_table, &row);

	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), pref_table, TRUE, TRUE, 6);

	#ifdef HAVE_LIBCLASTFM
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(support_lastfm),
	                              soundmenu_lastfm_is_supported (soundmenu->clastfm));
	#endif

	/* link the dialog to the plugin, so we can destroy it when the plugin
	* is closed, but the dialog is still open */
	g_object_set_data (G_OBJECT (plugin), "dialog", dialog);

	/* connect the reponse signal to the dialog */
	g_signal_connect (G_OBJECT (dialog), "response",
				G_CALLBACK(soundmenu_configure_response), soundmenu);

	/* show the entire dialog */
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
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
