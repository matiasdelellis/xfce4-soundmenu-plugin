/*
 *  Copyright (c) 2014 matias <mati86dl@gmail.com>
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

#include <math.h>
#include <gtk/gtk.h>
#include <libxfce4panel/libxfce4panel.h>
#include "soundmenu-pulseaudio.h"

static const
gchar *icon_names[] = {
	"audio-volume-muted",
	"audio-volume-low",
	"audio-volume-medium",
	"audio-volume-high",
	NULL
};

#define MIXER_NAME "Xfce Volume Control"

struct _PulseaudioButton {
	GtkButton       __parent;
	GvcMixerControl  *mixer;

	GtkWidget       **images;
};

struct _PulseaudioButtonClass {
	GtkToggleButtonClass __parent;
};

G_DEFINE_TYPE (PulseaudioButton, pulseaudio_button, GTK_TYPE_BUTTON)

static void
pulseaudio_button_update_mixer_volume (PulseaudioButton *button)
{
	GvcMixerStream *stream;
	gdouble vol_norm;
	pa_volume_t vol;
	gdouble db;
	gchar *tooltip = NULL;
	gint n;

	stream = gvc_mixer_control_get_default_sink (button->mixer);
	vol_norm = gvc_mixer_control_get_vol_max_norm (button->mixer);
	vol = gvc_mixer_stream_get_volume(stream);

	/* Same maths as computed by volume.js in gnome-shell */
	n = floor(3*vol/vol_norm) + 1;

	/* Remove prev icon */
	gtk_container_remove (GTK_CONTAINER(button),
	                      gtk_bin_get_child(GTK_BIN(button)));

	if (gvc_mixer_stream_get_is_muted(stream) || vol <= 0 || n < 0) {
		gtk_container_add (GTK_CONTAINER(button), button->images[0]);
	}
	else {
		if (n > 3)
			n = 3;
		gtk_container_add (GTK_CONTAINER(button), button->images[n]);
	}

	/* Now update the tooltip with dB level */
	if (gvc_mixer_stream_get_can_decibel(stream)) {
		db = gvc_mixer_stream_get_decibel(stream);
		tooltip = g_strdup_printf("%f dB", db);
		gtk_widget_set_tooltip_text(GTK_WIDGET(button), tooltip);
		g_free(tooltip);
	}
}

static void
gvm_mixer_volume_cb (GvcMixerStream *stream, gulong vol, gpointer userdata)
{
	pulseaudio_button_update_mixer_volume (userdata);
}

static void
gvm_mixer_muted_cb (GvcMixerStream *stream, gboolean mute, gpointer userdata)
{
	pulseaudio_button_update_mixer_volume (userdata);
}

static void
gvm_mixer_state_changed (GvcMixerControl *mix, guint status, gpointer userdata)
{
	GvcMixerStream *stream;

	/* First time we connect, update the volume */
	if (status == GVC_STATE_READY) {
		stream = gvc_mixer_control_get_default_sink(mix);

		g_signal_connect (stream, "notify::volume",
		                  G_CALLBACK(gvm_mixer_volume_cb), userdata);
		g_signal_connect (stream, "notify::is-muted",
		                  G_CALLBACK(gvm_mixer_muted_cb), userdata);

		pulseaudio_button_update_mixer_volume (userdata);
	}
}

static gboolean
pulseaudio_button_button_press (GtkWidget      *widget,
                                GdkEventButton *event)
{
	GvcMixerStream *stream;
	gboolean muted;

	PulseaudioButton *button = PULSEAUDIO_BUTTON (widget);

	if(event->button == 2) {
		stream = gvc_mixer_control_get_default_sink (button->mixer);
		muted = gvc_mixer_stream_get_is_muted (stream);
		gvc_mixer_stream_change_is_muted (stream, !muted);
		return TRUE;
	}

	return FALSE;
}

static gboolean
pulseaudio_button_scroll_event (GtkWidget *widget, GdkEventScroll *event)
{
	GvcMixerStream *stream;
	gdouble vol_norm, step_norm, volume = 0.0;

	PulseaudioButton *button = PULSEAUDIO_BUTTON (widget);

	vol_norm = gvc_mixer_control_get_vol_max_norm  (button->mixer);
	stream = gvc_mixer_control_get_default_sink (button->mixer);
	volume = (gdouble) gvc_mixer_stream_get_volume (stream);

	step_norm = vol_norm * 0.02;

	switch (event->direction) {
		case GDK_SCROLL_UP:
		case GDK_SCROLL_RIGHT:
			volume += step_norm;
			if (volume > vol_norm)
				volume = vol_norm;
			break;
		case GDK_SCROLL_DOWN:
		case GDK_SCROLL_LEFT:
			volume -= step_norm;
			if (volume < 0.0)
				volume = 0.0;
			break;
	}

	if (gvc_mixer_stream_set_volume (stream, (pa_volume_t) round(volume)) != FALSE)
		gvc_mixer_stream_push_volume (stream);

	return TRUE;
}

static void
pulseaudio_button_finalize (GObject *object)
{
	guint i = 0;

	PulseaudioButton *button = PULSEAUDIO_BUTTON (object);

	for (i = 0; i < G_N_ELEMENTS (icon_names)-1; ++i)
		g_object_unref (G_OBJECT (button->images[i]));
	g_free (button->images);

	if (button->mixer) {
		gvc_mixer_control_close (button->mixer);
		g_object_unref (button->mixer);
		button->mixer = NULL;
	}

	(*G_OBJECT_CLASS (pulseaudio_button_parent_class)->finalize) (object);
}

static void
pulseaudio_button_class_init (PulseaudioButtonClass *klass)
{
	GObjectClass *gobject_class;
	GtkWidgetClass *gtkwidget_class;

	gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->finalize = pulseaudio_button_finalize;

	gtkwidget_class = GTK_WIDGET_CLASS (klass);
	gtkwidget_class->button_press_event = pulseaudio_button_button_press;
	gtkwidget_class->scroll_event = pulseaudio_button_scroll_event;
}

static void
pulseaudio_button_init (PulseaudioButton *button)
{
	guint i = 0;

	button->mixer = gvc_mixer_control_new (MIXER_NAME);
	gvc_mixer_control_open (button->mixer);

	button->images = g_new0 (GtkWidget*, G_N_ELEMENTS (icon_names)-1);
	for (i = 0; i < G_N_ELEMENTS (icon_names)-1; ++i) {
		button->images[i] = xfce_panel_image_new_from_source(icon_names[i]);
		gtk_widget_show(button->images[i]);
		g_object_ref(button->images[i]);
	}
	gtk_container_add (GTK_CONTAINER (button), button->images[0]);

	g_signal_connect (button->mixer, "state-changed",
	                  G_CALLBACK(gvm_mixer_state_changed), button);

	gtk_widget_set_can_default (GTK_WIDGET (button), FALSE);
	gtk_widget_set_can_focus (GTK_WIDGET (button), FALSE);
	gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
	gtk_button_set_focus_on_click (GTK_BUTTON (button), FALSE);
	gtk_widget_set_name (GTK_WIDGET(button), "xfce-panel-button");

	gtk_widget_show (GTK_WIDGET(button));
}

PulseaudioButton *
pulseaudio_button_new (void)
{
	PulseaudioButton *button;
	button = g_object_new (TYPE_PULSEAUDIO_BUTTON, NULL);
	return button;
}
