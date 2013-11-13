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

#include "libmpris2control.h"

static GtkWidget     *popup_menu = NULL;
static GtkStatusIcon *status_icon = NULL;

/*
 * Callbacks
 */

static void
mpris2_status_icon_prev (GtkStatusIcon *widget,
                        Mpris2Control *mpris2)
{
	mpris2_control_prev (mpris2);
}

static void
mpris2_status_icon_play_pause (GtkStatusIcon *widget,
                               Mpris2Control *mpris2)
{
	mpris2_control_play_pause (mpris2);
}

static void
mpris2_status_icon_stop (GtkStatusIcon *widget,
                         Mpris2Control *mpris2)
{
	mpris2_control_stop (mpris2);
}

static void
mpris2_status_icon_next (GtkStatusIcon *widget,
                         Mpris2Control *mpris2)
{
	mpris2_control_next (mpris2);
}

/*
 * Signals.
 */

static void
mpris2_status_icon_coneccion (Mpris2Control *mpris2, GtkStatusIcon *icon)
{
	if (mpris2_control_is_connected(mpris2)) {
		gtk_status_icon_set_tooltip (status_icon,
			                         mpris2_control_get_player_identity(mpris2));
	}
	else {
		gtk_status_icon_set_tooltip (status_icon, _("Soundmenu"));
	}
}

static void
mpris2_status_icon_activate (GtkStatusIcon *icon,
                             Mpris2Control *mpris2)
{
	if (!mpris2_control_is_connected(mpris2))
		mpris2_control_auto_set_player (mpris2);
	else
		mpris2_control_raise_player (mpris2);
}

static void
mpris2_status_icon_show_popup (GtkStatusIcon *icon,
                               guint          button,
                               guint          activate_time,
                               Mpris2Control *mpris2)
{
	GtkWidget *item;

	if (!mpris2_control_is_connected(mpris2))
		return;

	if (!popup_menu) {
		popup_menu = gtk_menu_new();

		item = gtk_menu_item_new_with_label ("Prev");
		gtk_menu_append (popup_menu, item);
		g_signal_connect (G_OBJECT(item), "activate",
		                  G_CALLBACK(mpris2_status_icon_prev), mpris2);

		item = gtk_menu_item_new_with_label ("Play / Pause");
		gtk_menu_append (popup_menu, item);
		g_signal_connect (G_OBJECT(item), "activate",
		                  G_CALLBACK(mpris2_status_icon_play_pause), mpris2);

		item = gtk_menu_item_new_with_label ("Stop");
		gtk_menu_append (popup_menu, item);
		g_signal_connect (G_OBJECT(item), "activate",
		                  G_CALLBACK(mpris2_status_icon_stop), mpris2);

		item = gtk_menu_item_new_with_label ("Next");
		gtk_menu_append (popup_menu, item);
		g_signal_connect (G_OBJECT(item), "activate",
		                  G_CALLBACK(mpris2_status_icon_next), mpris2);

		item = gtk_menu_item_new_with_label ("Quit");
		gtk_menu_append (popup_menu, item);
		g_signal_connect (G_OBJECT(item), "activate",
		                 G_CALLBACK(gtk_main_quit), NULL);
	}
	gtk_widget_show_all (popup_menu);

	gtk_menu_popup (GTK_MENU(popup_menu),
	                NULL,
	                NULL,
	                gtk_status_icon_position_menu,
	                status_icon,
	                button,
	                activate_time);
}

gint
main (gint argc,
      gchar *argv[])
{
	Mpris2Control *mpris2 = NULL;

	mpris2 = mpris2_control_new ();

	gtk_init (&argc, &argv);
	g_set_application_name (_("Soundmenu"));

	status_icon = gtk_status_icon_new_from_icon_name ("xfce4-soundmenu-plugin");

	gtk_status_icon_set_visible (status_icon, TRUE); 
	gtk_status_icon_set_tooltip (status_icon, _("Soundmenu"));

	/* Connect signals */
	g_signal_connect (G_OBJECT (status_icon), "popup-menu",
	                  G_CALLBACK (mpris2_status_icon_show_popup), mpris2);

	g_signal_connect (G_OBJECT (status_icon), "activate",
	                  G_CALLBACK (mpris2_status_icon_activate), mpris2);

	g_signal_connect (G_OBJECT (mpris2), "connection",
	                  G_CALLBACK(mpris2_status_icon_coneccion), status_icon);

	gtk_main ();

	g_object_unref (mpris2);

	return 0;
}
