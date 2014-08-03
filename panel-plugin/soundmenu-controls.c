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
#include <libxfce4panel/libxfce4panel.h>

struct _Mpris2Controls
{
	GtkHBox          __parent__;

	Mpris2Client    *mpris2;

	GtkWidget		*prev_button;
	GtkWidget		*play_button;
	GtkWidget		*stop_button;
	GtkWidget		*next_button;

	GtkWidget		*image_pause;
	GtkWidget		*image_play;
};

struct _Mpris2ControlsClass {
	GtkHBoxClass __parent;
};

G_DEFINE_TYPE (Mpris2Controls, mpris2_controls, GTK_TYPE_HBOX)

/*
 * Public Api.
 */
void
mpris2_controls_set_show_stop_button (Mpris2Controls *controls, gboolean show)
{
	if (TRUE == show) {
		gtk_widget_show_all (controls->stop_button);
	}
	else {
		gtk_widget_hide (controls->stop_button);
	}
}

void
mpris2_controls_set_orientation (Mpris2Controls *controls, GtkOrientation orientation)
{
	gtk_orientable_set_orientation (GTK_ORIENTABLE(controls), orientation);
}

void
mpris2_controls_set_size (Mpris2Controls *controls, gint size)
{
	gtk_widget_set_size_request (GTK_WIDGET(controls->next_button), size, size);
	gtk_widget_set_size_request (GTK_WIDGET(controls->prev_button), size, size);
	gtk_widget_set_size_request (GTK_WIDGET(controls->stop_button), size, size);
	gtk_widget_set_size_request (GTK_WIDGET(controls->play_button), size, size);
}

/*
 * Private
 */

static void
soundmenu_prev_button_handler(GtkButton *button, Mpris2Controls *controls)
{
	if (mpris2_client_is_connected(controls->mpris2) == FALSE)
		return;
	mpris2_client_prev (controls->mpris2);
}

static void
soundmenu_play_button_handler(GtkButton *button, Mpris2Controls *controls)
{
	if (mpris2_client_is_connected(controls->mpris2)) {
		mpris2_client_play_pause (controls->mpris2);
	}
	else {
		/*if (g_str_nempty0(controls->player)) {
			soundmenu_launch_player (controls->player);
		}
		else {
			controls->player = mpris2_client_auto_set_player(controls->mpris2);
			if (g_str_empty0(controls->player))
				soundmenu_configure(controls->plugin, soundmenu);
		}*/
	}
}

static void
soundmenu_stop_button_handler (GtkButton *button, Mpris2Controls *controls)
{
	if (mpris2_client_is_connected(controls->mpris2) == FALSE)
		return;
	mpris2_client_stop (controls->mpris2);
}

static void
soundmenu_next_button_handler (GtkButton *button, Mpris2Controls *controls)
{
	if (mpris2_client_is_connected(controls->mpris2) == FALSE)
		return;
	mpris2_client_next (controls->mpris2);
}

static void
mpris2_controls_playback_status (Mpris2Client   *mpris2,
                                 PlaybackStatus  playback_status,
                                 Mpris2Controls *controls)
{
	gtk_container_remove (GTK_CONTAINER(controls->play_button),
	                      gtk_bin_get_child(GTK_BIN(controls->play_button)));

	switch (playback_status) {
		case PLAYING:
			gtk_container_add (GTK_CONTAINER(controls->play_button),
			                   controls->image_pause);
			break;
		case PAUSED:
			gtk_container_add (GTK_CONTAINER(controls->play_button),
			                   controls->image_play);
			break;
		case STOPPED:
		default:
			gtk_container_add (GTK_CONTAINER(controls->play_button),
			                   controls->image_play);
			break;
	}
	gtk_widget_show_all (controls->play_button);
}

static void
mpris2_controls_coneccion (Mpris2Client   *mpris2,
                           gboolean        connected,
                           Mpris2Controls *controls)
{
	if (connected) {
		/* Sensitive all controls */
		gtk_widget_set_sensitive (GTK_WIDGET(controls->prev_button), TRUE);
		gtk_widget_set_sensitive (GTK_WIDGET(controls->play_button), TRUE);
		gtk_widget_set_sensitive (GTK_WIDGET(controls->stop_button), TRUE);
		gtk_widget_set_sensitive (GTK_WIDGET(controls->next_button), TRUE);
	}
	else {
		/* Insensitive controls */
		gtk_widget_set_sensitive (GTK_WIDGET(controls->prev_button), FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET(controls->play_button), TRUE);
		gtk_widget_set_sensitive (GTK_WIDGET(controls->stop_button), FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET(controls->next_button), FALSE);
	}
}

static void
mpris2_controls_finalize (GObject *object)
{
	Mpris2Controls *controls = MPRIS2_CONTROLS(object);

	if (controls->mpris2) {
		g_object_unref(controls->mpris2);
		controls->mpris2 = NULL;
	}

	(*G_OBJECT_CLASS (mpris2_controls_parent_class)->finalize) (object);
}

static void
mpris2_controls_class_init (Mpris2ControlsClass *klass)
{
	GObjectClass *gobject_class;
	gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->finalize = mpris2_controls_finalize;
}

static void
mpris2_controls_init (Mpris2Controls *controls)
{
	controls->prev_button = xfce_panel_create_button ();
	controls->play_button = xfce_panel_create_button ();
	controls->stop_button = xfce_panel_create_button ();
	controls->next_button = xfce_panel_create_button ();

	gtk_container_add (GTK_CONTAINER(controls->prev_button),
		               xfce_panel_image_new_from_source("media-skip-backward"));
	gtk_container_add (GTK_CONTAINER(controls->stop_button),
	                   xfce_panel_image_new_from_source("media-playback-stop"));
	gtk_container_add (GTK_CONTAINER(controls->next_button),
	                   xfce_panel_image_new_from_source("media-skip-forward"));

	controls->image_pause =
		xfce_panel_image_new_from_source ("media-playback-pause");
	controls->image_play =
		xfce_panel_image_new_from_source ("media-playback-start");

	g_object_ref (controls->image_play);
	g_object_ref (controls->image_pause);

	gtk_container_add (GTK_CONTAINER(controls->play_button),
	                   controls->image_play);

	gtk_box_pack_start (GTK_BOX(controls),
	                    GTK_WIDGET(controls->prev_button),
	                    TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX(controls),
	                    GTK_WIDGET(controls->play_button),
	                    TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX(controls),
	                    GTK_WIDGET(controls->stop_button),
	                    TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX(controls),
	                    GTK_WIDGET(controls->next_button),
	                    TRUE, TRUE, 0);

	gtk_widget_show_all (controls->prev_button);
	/* Stop button depend on preferences */
	gtk_widget_show_all (controls->play_button);
	gtk_widget_show_all (controls->next_button);

	/* Tooltips */

	g_object_set (G_OBJECT(controls->prev_button), "has-tooltip", TRUE, NULL);
	g_object_set (G_OBJECT(controls->play_button), "has-tooltip", TRUE, NULL);
	g_object_set (G_OBJECT(controls->stop_button), "has-tooltip", TRUE, NULL);
	g_object_set (G_OBJECT(controls->next_button), "has-tooltip", TRUE, NULL);

	/*g_signal_connect (G_OBJECT(controls->prev_button), "query-tooltip",
	                  G_CALLBACK(soundmenu_set_query_tooltip_cb), controls);
	g_signal_connect (G_OBJECT(controls->play_button), "query-tooltip",
	                  G_CALLBACK(soundmenu_set_query_tooltip_cb), controls);
	g_signal_connect (G_OBJECT(controls->stop_button), "query-tooltip",
	                  G_CALLBACK(soundmenu_set_query_tooltip_cb), controls);
	g_signal_connect (G_OBJECT(controls->next_button), "query-tooltip",
	                  G_CALLBACK(soundmenu_set_query_tooltip_cb), controls);*/

	/* CLick signals */
	g_signal_connect (G_OBJECT(controls->prev_button), "clicked",
	                  G_CALLBACK(soundmenu_prev_button_handler), controls);
	g_signal_connect (G_OBJECT(controls->play_button), "clicked",
	                 G_CALLBACK(soundmenu_play_button_handler), controls);
	g_signal_connect (G_OBJECT(controls->stop_button), "clicked",
	                  G_CALLBACK(soundmenu_stop_button_handler), controls);
	g_signal_connect (G_OBJECT(controls->next_button), "clicked",
	                  G_CALLBACK(soundmenu_next_button_handler), controls);
}

Mpris2Controls *
mpris2_controls_new (Mpris2Client *mpris2)
{
	Mpris2Controls *controls;

	controls = g_object_new (TYPE_MPRIS2_CONTROLS, NULL);

	controls->mpris2 = g_object_ref(mpris2);
	g_signal_connect (G_OBJECT(controls->mpris2), "connection",
	                  G_CALLBACK(mpris2_controls_coneccion), controls);
	g_signal_connect (G_OBJECT(controls->mpris2), "playback-status",
	                  G_CALLBACK(mpris2_controls_playback_status), controls);

	gtk_widget_show (GTK_WIDGET(controls));

	return controls;
}
