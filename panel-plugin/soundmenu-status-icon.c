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

#include "libmpris2client.h"
#include "mpris2-metadata.h"

static GtkWidget     *icon_popup_menu   = NULL;
static GtkWidget     *mpris2_popup_menu = NULL;
static GtkStatusIcon *status_icon       = NULL;

/*
 * Callbacks
 */

static void
mpris2_status_icon_open_files_response (GtkDialog    *dialog,
                                        gint          response,
                                        Mpris2Client *mpris2)
{
	GSList *files;
	guint i, len;
	gchar *file;

	files = gtk_file_chooser_get_filenames (GTK_FILE_CHOOSER (dialog));

	gtk_widget_destroy (GTK_WIDGET(dialog));

	len = g_slist_length (files);
	for (i = 0; i < len; i++) {
		file = g_slist_nth_data (files, i);
		g_print ("Open file %s\n", file);
	}
	g_slist_foreach (files, (GFunc) g_free, NULL);
	g_slist_free (files);
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
	gtk_status_icon_set_tooltip (icon,  mpris2_metadata_get_url(metadata));
}

static void
mpris2_status_icon_playback_status (Mpris2Client *mpris2, PlaybackStatus playback_status, GtkStatusIcon *icon)
{
	switch (playback_status) {
		case PLAYING:
			gtk_status_icon_set_from_stock (icon, GTK_STOCK_MEDIA_PAUSE);
			break;
		case PAUSED:
		case STOPPED:
		default:
			gtk_status_icon_set_from_stock (icon, GTK_STOCK_MEDIA_PLAY);
			break;
	}
}

static void
mpris2_status_icon_coneccion (Mpris2Client *mpris2, gboolean connected, GtkStatusIcon *icon)
{
	if (connected) {
		gtk_status_icon_set_tooltip (status_icon,
		                             mpris2_client_get_player_identity(mpris2));
	}
	else {
		gtk_status_icon_set_tooltip (status_icon, _("Soundmenu"));
		gtk_status_icon_set_from_icon_name (icon, "xfce4-soundmenu-plugin");
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
mpris2_status_icon_show_mpris2_popup (GtkStatusIcon *icon,
                                      GdkEventButton *event,
                                      Mpris2Client *mpris2)
{
	GtkWidget *item;

	if (!mpris2_client_is_connected(mpris2))
		return;

	if (!mpris2_popup_menu) {
		mpris2_popup_menu = gtk_menu_new();

		item = gtk_menu_item_new_with_label ("Open files");
		gtk_menu_append (mpris2_popup_menu, item);
		g_signal_connect (G_OBJECT(item), "activate",
		                  G_CALLBACK(mpris2_status_icon_open_files), mpris2);

		item = gtk_menu_item_new_with_label ("Prev");
		gtk_menu_append (mpris2_popup_menu, item);
		g_signal_connect (G_OBJECT(item), "activate",
		                  G_CALLBACK(mpris2_status_icon_prev), mpris2);

		item = gtk_menu_item_new_with_label ("Play / Pause");
		gtk_menu_append (mpris2_popup_menu, item);
		g_signal_connect (G_OBJECT(item), "activate",
		                  G_CALLBACK(mpris2_status_icon_play_pause), mpris2);

		item = gtk_menu_item_new_with_label ("Stop");
		gtk_menu_append (mpris2_popup_menu, item);
		g_signal_connect (G_OBJECT(item), "activate",
		                  G_CALLBACK(mpris2_status_icon_stop), mpris2);

		item = gtk_menu_item_new_with_label ("Next");
		gtk_menu_append (mpris2_popup_menu, item);
		g_signal_connect (G_OBJECT(item), "activate",
		                  G_CALLBACK(mpris2_status_icon_next), mpris2);

		item = gtk_menu_item_new_with_label ("Close");
		gtk_menu_append (mpris2_popup_menu, item);
		g_signal_connect (G_OBJECT(item), "activate",
		                 G_CALLBACK(mpris2_status_icon_quit_player), mpris2);
	}
	gtk_widget_show_all (mpris2_popup_menu);

	gtk_menu_popup (GTK_MENU(mpris2_popup_menu), NULL, NULL, NULL, NULL,
	                event->button, gtk_get_current_event_time ());
}

static void
mpris2_status_icon_show_icon_popup (GtkStatusIcon *icon,
                                    GdkEventButton *event,
                                    Mpris2Client *mpris2)
{
	GtkWidget *item;

	if (!mpris2_client_is_connected(mpris2))
		return;

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
			mpris2_status_icon_show_mpris2_popup (icon, event, mpris2);
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

gint
main (gint argc,
      gchar *argv[])
{
	Mpris2Client *mpris2 = NULL;

	mpris2 = mpris2_client_new ();

	gtk_init (&argc, &argv);
	g_set_application_name (_("Soundmenu"));

	status_icon = gtk_status_icon_new_from_icon_name ("xfce4-soundmenu-plugin");

	gtk_status_icon_set_visible (status_icon, TRUE); 
	gtk_status_icon_set_tooltip (status_icon, _("Soundmenu"));

	/* Connect signals */
	g_signal_connect (G_OBJECT (status_icon), "button-press-event",
	                  G_CALLBACK (mpris2_status_icon_activate), mpris2);
	g_signal_connect (G_OBJECT (status_icon), "scroll_event",
	                  G_CALLBACK (mpris2_status_icon_scroll), mpris2);

	g_signal_connect (G_OBJECT (mpris2), "connection",
	                  G_CALLBACK(mpris2_status_icon_coneccion), status_icon);
	g_signal_connect (G_OBJECT (mpris2), "playback-status",
	                  G_CALLBACK(mpris2_status_icon_playback_status), status_icon);
	g_signal_connect (G_OBJECT (mpris2), "metadata",
	                  G_CALLBACK(mpris2_status_icon_metadada), status_icon);

	gtk_main ();

	g_object_unref (mpris2);

	return 0;
}
