/*
 *  Copyright (c) 2013 matias <mati86dl@gmail.com>
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

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>
#include <gio-unix-2.0/gio/gdesktopappinfo.h>

#include <libmpris2client/libmpris2client.h>
#include <libmpris2client/mpris2-metadata.h>

#include "soundmenu-album-art.h"

static GtkWidget     *icon_popup_menu   = NULL;
static GtkWidget     *mpris2_popup_menu = NULL;

static GtkStatusIcon *status_icon       = NULL;
static GIcon         *player_icon       = NULL;
static GEmblem       *playing_emblem    = NULL;
static GEmblem       *paused_emblem     = NULL;
static GEmblem       *stopped_emblem    = NULL;

GtkWidget *popup_dialog, *vbox;
GtkWidget *track_box, *track_label;
GtkWidget *time_box, *time_label, *progress_bar, *length_label;
GtkWidget *buttons_box, *player_button, *button;
SoundmenuAlbumArt *album_art;

/*
 * Some private
 */

#define g_str_empty0(s) (!(s) || !(s)[0])
#define g_str_nempty0(s) ((s) && (s)[0])

static gchar *
get_string_from_time (gint time)
{
	static gchar *str, tmp[24];
	gint days = 0, hours = 0, minutes = 0, seconds = 0;

	str = g_new0(char, 128);
	memset(tmp, '\0', 24);

	if (time > 86400) {
		days = time/86400;
		time = time%86400;
		g_sprintf(tmp, "%d %s, ", days, (days>1)?_("days"):_("day"));
		g_strlcat(str, tmp, 24);
	}

	if (time > 3600) {
		hours = time/3600;
		time = time%3600;
		memset(tmp, '\0', 24);
		g_sprintf(tmp, "%d:", hours);
		g_strlcat(str, tmp, 24);
	}

	if (time > 60) {
		minutes = time/60;
		time = time%60;
		memset(tmp, '\0', 24);
		g_sprintf(tmp, "%02d:", minutes);
		g_strlcat(str, tmp, 24);
	}
	else
		g_strlcat(str, "00:", 4);

	seconds = time;
	memset(tmp, '\0', 24);
	g_sprintf(tmp, "%02d", seconds);
	g_strlcat(str, tmp, 24);

	return str;
}

/*
 * Callbacks
 */

static void
mpris2_status_icon_open_files_response (GtkDialog    *dialog,
                                        gint          response,
                                        Mpris2Client *mpris2)
{
	GSList *uris;
	guint i, len;
	gchar *uri;

	uris = gtk_file_chooser_get_uris (GTK_FILE_CHOOSER (dialog));

	gtk_widget_destroy (GTK_WIDGET(dialog));

	len = g_slist_length (uris);
	for (i = 0; i < len; i++) {
		uri = g_slist_nth_data (uris, i);
		mpris2_client_open_uri (mpris2, uri);
	}
	g_slist_foreach (uris, (GFunc) g_free, NULL);
	g_slist_free (uris);
}

static void
mpris2_status_icon_open_files (GtkStatusIcon *widget,
                               Mpris2Client *mpris2)
{
	GtkWidget *dialog;
	GtkFileFilter *filter;
	gchar **mime_types = NULL;
	guint i = 0;

	dialog = gtk_file_chooser_dialog_new ("Open File",
	                                      NULL,
	                                      GTK_FILE_CHOOSER_ACTION_OPEN,
	                                      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
	                                      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
	                                      NULL);

	gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER(dialog), TRUE);

	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("Supported files"));

	mime_types = mpris2_client_get_supported_mime_types(mpris2);
	for (i = 0; i < g_strv_length(mime_types); i++)
		gtk_file_filter_add_mime_type (GTK_FILE_FILTER (filter), mime_types[i]);
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(dialog), filter);

	g_signal_connect (G_OBJECT(dialog), "response",
	                  G_CALLBACK(mpris2_status_icon_open_files_response), mpris2);

	gtk_widget_show_all (dialog);
}

static void
mpris2_status_icon_prev (GtkStatusIcon *widget,
                         Mpris2Client *mpris2)
{
	mpris2_client_prev (mpris2);
}

static void
mpris2_status_icon_play_pause (GtkStatusIcon *widget,
                               Mpris2Client *mpris2)
{
	mpris2_client_play_pause (mpris2);
}

static void
mpris2_status_icon_stop (GtkStatusIcon *widget,
                         Mpris2Client *mpris2)
{
	mpris2_client_stop (mpris2);
}

static void
mpris2_status_icon_next (GtkStatusIcon *widget,
                         Mpris2Client *mpris2)
{
	mpris2_client_next (mpris2);
}

static void
mpris2_status_icon_quit_player (GtkStatusIcon *widget,
                                Mpris2Client *mpris2)
{
	mpris2_client_quit_player (mpris2);
}

/*
 * Signals.
 */

static void
mpris2_status_icon_metadada (Mpris2Client *mpris2, Mpris2Metadata *metadata, GtkStatusIcon *icon)
{
	const gchar *title = NULL, *artist = NULL, *album = NULL, *url = NULL;
	gchar *markup_text = NULL, *s_length = NULL, *filename = NULL, *name = NULL;
	gint length = 0;
	GError *error = NULL;

	title = mpris2_metadata_get_title (metadata);
	artist = mpris2_metadata_get_artist (metadata);
	album = mpris2_metadata_get_album (metadata);
	url = mpris2_metadata_get_url (metadata);
	length  = mpris2_metadata_get_length (metadata);

	s_length = get_string_from_time (length);

	if (g_str_empty0(url))
	    return;

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

	markup_text = g_strdup_printf (_("%s\nby %s\nin %s"),
	                               name,
	                               g_str_nempty0(artist) ? artist : _("Unknown Artist"),
	                               g_str_nempty0(album)  ? album  : _("Unknown Album"));

	gtk_status_icon_set_tooltip (icon, markup_text);
	gtk_label_set_text (GTK_LABEL(track_label), markup_text);
	gtk_label_set_text (GTK_LABEL(length_label), s_length);

	soundmenu_album_art_set_path (album_art,
		mpris2_metadata_get_arturl(metadata));

	g_free(filename);
	g_free(name);
	g_free(s_length);
}

static void
mpris2_status_icon_playback_status (Mpris2Client *mpris2, PlaybackStatus playback_status, GtkStatusIcon *icon)
{
	g_emblemed_icon_clear_emblems (G_EMBLEMED_ICON(player_icon));

	switch (playback_status) {
		case PLAYING:
			g_emblemed_icon_add_emblem (G_EMBLEMED_ICON(player_icon), playing_emblem);
			break;
		case PAUSED:
			g_emblemed_icon_add_emblem (G_EMBLEMED_ICON(player_icon), paused_emblem);
			break;
		case STOPPED:
		default:
			g_emblemed_icon_add_emblem (G_EMBLEMED_ICON(player_icon), stopped_emblem);
			gtk_status_icon_set_tooltip (status_icon, _("Soundmenu"));

			gtk_label_set_text (GTK_LABEL(track_label), _("Soundmenu"));

			gtk_label_set_text (GTK_LABEL(time_label), "00:00");
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), 0.0);
			gtk_label_set_text (GTK_LABEL(length_label), "--:--");
			break;
	}

	gtk_status_icon_set_from_gicon (status_icon, player_icon);
}

static void
mpris2_status_icon_playback_tick (Mpris2Client *mpris2, gint position, GtkStatusIcon *icon)
{
	Mpris2Metadata *metadata = NULL;
	gdouble fraction = 0.0;
	gint length = 0;
	gchar *s_time = NULL;

	s_time = get_string_from_time (position/1000000);
	gtk_label_set_text (GTK_LABEL(time_label), s_time);

	metadata = mpris2_client_get_metadata (mpris2);

	length = mpris2_metadata_get_length (metadata);
	if (length) {
		fraction = (gdouble) position/1000000/(gdouble)length;
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), fraction);
	}


	g_free(s_time);
}

static void
mpris2_status_icon_connecion (Mpris2Client *mpris2, gboolean connected, GtkStatusIcon *icon)
{
	GDesktopAppInfo *player_info = NULL;
	const gchar *desktop_entry   = NULL;
	const gchar *player_identity = NULL;
	gchar *desktop_id            = NULL;
	GIcon *gicon                 = NULL;

	if (connected) {
		desktop_entry = mpris2_client_get_player_desktop_entry (mpris2);
		player_identity = mpris2_client_get_player_identity (mpris2);

		desktop_id = g_strdup_printf("%s.desktop", desktop_entry);

		player_info = g_desktop_app_info_new (desktop_id);

		gicon = g_app_info_get_icon (G_APP_INFO(player_info));
		player_icon = g_emblemed_icon_new (gicon, NULL);

		gtk_status_icon_set_from_gicon (status_icon, player_icon);
		gtk_status_icon_set_tooltip (status_icon, player_identity);

		gtk_button_set_label (GTK_BUTTON(player_button), player_identity);

		g_object_unref (player_info);
		g_free (desktop_id);
	}
	else {
		gtk_status_icon_set_tooltip (status_icon, _("Soundmenu"));
		gtk_status_icon_set_from_icon_name (icon, "xfce4-soundmenu-plugin");

		gtk_label_set_text (GTK_LABEL(track_label), _("Soundmenu"));

		gtk_label_set_text (GTK_LABEL(time_label), "00:00");
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), 0.0);
		gtk_label_set_text (GTK_LABEL(length_label), "--:--");
	}
}

static void
mpris2_status_icon_scroll (GtkStatusIcon *icon,
                           GdkEventScroll *event,
                           Mpris2Client *mpris2)
{
	gdouble volume = 0.0;

	if (event->type != GDK_SCROLL)
		return;

	if(!mpris2_client_is_connected(mpris2))
		return;

	volume = mpris2_client_get_volume (mpris2);

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
	mpris2_client_set_volume (mpris2, volume);
}

static void
mpris2_status_icon_show_mpris2_popup (GtkWidget    *widget,
                                      GdkEvent     *event,
                                      Mpris2Client *mpris2)
{
	GtkWidget *item;

	if (!mpris2_client_is_connected(mpris2))
		return;

	if (!mpris2_popup_menu) {
		mpris2_popup_menu = gtk_menu_new();
		gtk_menu_shell_set_take_focus (GTK_MENU_SHELL(mpris2_popup_menu), FALSE);

		item = gtk_menu_item_new_with_label ("Open files");
		gtk_menu_append (mpris2_popup_menu, item);
		g_signal_connect (G_OBJECT(item), "activate",
		                  G_CALLBACK(mpris2_status_icon_open_files), mpris2);

		item = gtk_menu_item_new_with_label ("Close");
		gtk_menu_append (mpris2_popup_menu, item);
		g_signal_connect (G_OBJECT(item), "activate",
		                 G_CALLBACK(mpris2_status_icon_quit_player), mpris2);

		gtk_widget_show_all (mpris2_popup_menu);
	}

	gtk_menu_popup (GTK_MENU(mpris2_popup_menu), NULL, NULL, NULL, NULL,
	                1, gtk_get_current_event_time ());
}

static void
mpris2_status_icon_show_icon_popup (GtkStatusIcon *icon,
                                    GdkEventButton *event,
                                    Mpris2Client *mpris2)
{
	GtkWidget *item;

	if (!icon_popup_menu) {
		icon_popup_menu = gtk_menu_new();

		item = gtk_menu_item_new_with_label ("Quit");
		gtk_menu_append (icon_popup_menu, item);
		g_signal_connect (G_OBJECT(item), "activate",
		                 G_CALLBACK(gtk_main_quit), NULL);
	}
	gtk_widget_show_all (icon_popup_menu);

	gtk_menu_popup (GTK_MENU(icon_popup_menu), NULL, NULL, NULL, NULL,
	                event->button, gtk_get_current_event_time ());
}

static gboolean
mpris2_status_icon_activate (GtkStatusIcon *icon,
                             GdkEventButton *event,
                             Mpris2Client *mpris2)
{
	switch (event->button)
	{
		case 1:
			if (!mpris2_client_is_connected(mpris2))
				mpris2_client_auto_set_player (mpris2);
			else
				gtk_widget_show (popup_dialog);
			break;
		case 2:
			mpris2_client_play_pause (mpris2);
			break;
		case 3:
			mpris2_status_icon_show_icon_popup (icon, event, mpris2);
		default:
			break;
	}
	return TRUE;
}

/**/

static void
progress_bar_event_seek (GtkWidget *widget,
                         GdkEventButton *event,
                         Mpris2Client *mpris2)
{
	Mpris2Metadata *metadata = NULL;
	GtkAllocation allocation;
	gdouble fraction = 0.0;

	if (event->button != 1)
		return;

	gtk_widget_get_allocation(widget, &allocation);
	fraction = (gdouble) event->x / allocation.width;

	metadata = mpris2_client_get_metadata (mpris2);
	mpris2_client_set_position (mpris2,
	                            mpris2_metadata_get_trackid(metadata),
	                            fraction*1000000*mpris2_metadata_get_length(metadata));
}

static gboolean
mpris_control_widgets_unfocus_popup (GtkWidget *widget, GdkEventFocus * event, gpointer user_data)
{
	gtk_widget_hide (popup_dialog);

	return FALSE;
}

static void
mpris_control_widgets_popup (Mpris2Client *mpris2)
{
	popup_dialog = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	gtk_window_activate_focus(GTK_WINDOW(popup_dialog));
	gtk_window_set_decorated (GTK_WINDOW(popup_dialog), FALSE);
	gtk_window_set_position (GTK_WINDOW(popup_dialog), GTK_WIN_POS_MOUSE);
	gtk_window_set_keep_above (GTK_WINDOW(popup_dialog), TRUE);
	g_signal_connect (G_OBJECT(popup_dialog), "focus-out-event",
	                  G_CALLBACK(mpris_control_widgets_unfocus_popup), NULL);

	vbox = gtk_vbox_new(FALSE, 0);

	/* Pack track info */

	player_button = gtk_button_new ();
	gtk_button_set_relief(GTK_BUTTON(player_button), GTK_RELIEF_NONE);

	album_art = soundmenu_album_art_new ();
	soundmenu_album_art_set_size(album_art, 200);

	track_label = gtk_label_new (_("Stopped"));
	gtk_label_set_ellipsize (GTK_LABEL(track_label), PANGO_ELLIPSIZE_END);
	gtk_misc_set_alignment(GTK_MISC(track_label), 0.5, 0.5);

	/* Pack timer box */
	time_box = gtk_hbox_new(FALSE, 2);

	time_label = gtk_label_new ("00:00");
	progress_bar = gtk_progress_bar_new();
	gtk_widget_set_size_request(GTK_WIDGET(progress_bar), -1, 12);
	gtk_widget_set_events(progress_bar, GDK_BUTTON_PRESS_MASK);
	g_signal_connect (G_OBJECT(progress_bar), "button-press-event",
	                  G_CALLBACK(progress_bar_event_seek), mpris2);
	length_label = gtk_label_new ("--:--");

	gtk_box_pack_start (GTK_BOX(time_box), GTK_WIDGET(time_label),
	                    FALSE, FALSE, 2);
	gtk_box_pack_start (GTK_BOX(time_box), GTK_WIDGET(progress_bar),
	                    TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX(time_box), GTK_WIDGET(length_label),
	                    FALSE, FALSE, 2);

	/* Pack buttons box */
	buttons_box = gtk_hbox_new(TRUE, 2);

	button = GTK_WIDGET(gtk_tool_button_new_from_stock (GTK_STOCK_MEDIA_PREVIOUS));
	g_signal_connect (G_OBJECT(button), "clicked",
	                  G_CALLBACK(mpris2_status_icon_prev), mpris2);
	gtk_box_pack_start (GTK_BOX(buttons_box), GTK_WIDGET(button),
	                    FALSE, FALSE, 0);

	button = GTK_WIDGET(gtk_tool_button_new_from_stock (GTK_STOCK_MEDIA_PLAY));
	g_signal_connect (G_OBJECT(button), "clicked",
	                  G_CALLBACK(mpris2_status_icon_play_pause), mpris2);
	gtk_box_pack_start (GTK_BOX(buttons_box), GTK_WIDGET(button),
	                    FALSE, FALSE, 0);

	button = GTK_WIDGET(gtk_tool_button_new_from_stock (GTK_STOCK_MEDIA_STOP));
	g_signal_connect (G_OBJECT(button), "clicked",
	                  G_CALLBACK(mpris2_status_icon_stop), mpris2);
	gtk_box_pack_start (GTK_BOX(buttons_box), GTK_WIDGET(button),
	                    FALSE, FALSE, 0);

	button = GTK_WIDGET(gtk_tool_button_new_from_stock (GTK_STOCK_MEDIA_NEXT));
	g_signal_connect (G_OBJECT(button), "clicked",
	                  G_CALLBACK(mpris2_status_icon_next), mpris2);
	gtk_box_pack_start (GTK_BOX(buttons_box), GTK_WIDGET(button),
	                    FALSE, FALSE, 0);

	button = GTK_WIDGET(gtk_tool_button_new_from_stock (GTK_STOCK_PROPERTIES));
	g_signal_connect (G_OBJECT(button), "clicked",
	                  G_CALLBACK(mpris2_status_icon_show_mpris2_popup), mpris2);
	gtk_box_pack_start (GTK_BOX(buttons_box), GTK_WIDGET(button),
	                    FALSE, FALSE, 0);

	/* Pack everting */
	gtk_box_pack_start (GTK_BOX(vbox), GTK_WIDGET(player_button),
	                    FALSE, FALSE, 2);
	gtk_box_pack_start (GTK_BOX(vbox), GTK_WIDGET(album_art),
	                    FALSE, FALSE, 2);
	gtk_box_pack_start (GTK_BOX(vbox), GTK_WIDGET(track_label),
	                    TRUE, TRUE, 2);
	gtk_box_pack_start (GTK_BOX(vbox), GTK_WIDGET(time_box),
	                    FALSE, FALSE, 2);
	gtk_box_pack_start (GTK_BOX(vbox), GTK_WIDGET(buttons_box),
	                    TRUE, TRUE, 2);

	gtk_widget_show_all (GTK_WIDGET(vbox));

	gtk_container_add (GTK_CONTAINER(popup_dialog), vbox);
}

static GtkStatusIcon *
mpris2_status_icon_new (void)
{
	GtkStatusIcon *statusicon = NULL;
	GIcon *gicon = NULL;

	statusicon = gtk_status_icon_new ();

	gicon = g_themed_icon_new ("xfce4-soundmenu-plugin");
	gtk_status_icon_set_from_gicon (statusicon, gicon);

	gicon = g_themed_icon_new ("media-playback-start");
	playing_emblem = g_emblem_new (gicon);

	gicon = g_themed_icon_new ("media-playback-pause");
	paused_emblem = g_emblem_new (gicon);

	gicon = g_themed_icon_new ("media-playback-stop");
	stopped_emblem = g_emblem_new (gicon);

	return statusicon;
}

gint
main (gint argc,
      gchar *argv[])
{
	Mpris2Client *mpris2 = NULL;

	mpris2 = mpris2_client_new ();

	gtk_init (&argc, &argv);
	g_set_application_name (_("Soundmenu"));

	status_icon = mpris2_status_icon_new ();

	gtk_status_icon_set_visible (status_icon, TRUE);
	gtk_status_icon_set_tooltip (status_icon, _("Soundmenu"));

	/* Connect signals */
	g_signal_connect (G_OBJECT (status_icon), "button-press-event",
	                  G_CALLBACK (mpris2_status_icon_activate), mpris2);
	g_signal_connect (G_OBJECT (status_icon), "scroll_event",
	                  G_CALLBACK (mpris2_status_icon_scroll), mpris2);

	g_signal_connect (G_OBJECT (mpris2), "connection",
	                  G_CALLBACK(mpris2_status_icon_connecion), status_icon);
	g_signal_connect (G_OBJECT (mpris2), "playback-status",
	                  G_CALLBACK(mpris2_status_icon_playback_status), status_icon);
	g_signal_connect (G_OBJECT (mpris2), "playback-tick",
	                  G_CALLBACK(mpris2_status_icon_playback_tick), status_icon);
	g_signal_connect (G_OBJECT (mpris2), "metadata",
	                  G_CALLBACK(mpris2_status_icon_metadada), status_icon);

	mpris_control_widgets_popup(mpris2);

	gtk_main ();

	g_object_unref (mpris2);

	return 0;
}
