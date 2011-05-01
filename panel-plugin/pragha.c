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
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <gtk/gtk.h>
#include <glib.h>
#include <libxfce4util/libxfce4util.h>
#include <libxfce4panel/xfce-panel-plugin.h>
#include <libxfce4panel/xfce-hvbox.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>

#include "pragha.h"
#include "pragha-dialogs.h"

/* default settings */
#define DEFAULT_SETTING1 NULL
#define DEFAULT_SETTING2 1
#define DEFAULT_SETTING3 FALSE

/* prototypes */

static void
pragha_construct (XfcePanelPlugin *plugin);

/* register the plugin */

XFCE_PANEL_PLUGIN_REGISTER_EXTERNAL (pragha_construct);

/* Dbus helpers to connect with Pragha */

void play_button_toggle_state (PraghaPlugin *pragha)
{
	if ((pragha->state == ST_PAUSED) || (pragha->state == ST_STOPPED))
		gtk_button_set_image(GTK_BUTTON(pragha->play_button), pragha->image_play);
	else
		gtk_button_set_image(GTK_BUTTON(pragha->play_button), pragha->image_pause);
}

void cmd_current_state (PraghaPlugin *pragha)
{
	DBusMessage *msg = NULL;
	DBusMessage *reply_msg = NULL;
	DBusError d_error;
	const char *state;

	dbus_error_init(&d_error);

	msg = dbus_message_new_method_call("org.pragha.DBus",
					   "/org/pragha/DBus",
					   "org.pragha.DBus",
					   "curent_state");

	reply_msg = dbus_connection_send_with_reply_and_block(pragha->connection, msg,
							      -1, &d_error);

	dbus_message_get_args(reply_msg, &d_error,
				   DBUS_TYPE_STRING, &state,
				   DBUS_TYPE_INVALID);

	if (0 == g_ascii_strcasecmp(state, "Playing"))
		pragha->state = ST_PLAYING;
	else if (0 == g_ascii_strcasecmp(state, "Paused"))
		pragha->state = ST_PAUSED;
	else
		pragha->state = ST_STOPPED;

	play_button_toggle_state(pragha);

	dbus_message_unref(msg);
}

static DBusHandlerResult
dbus_filter (DBusConnection *connection, DBusMessage *message, void *user_data)
{
    if ( dbus_message_is_signal (message, "org.pragha.DBus", "update_state" ) )
    {
    		PraghaPlugin *pragha = user_data;
				cmd_current_state (pragha);
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

void
send_message (PraghaPlugin *pragha, const char *msg)
{
    DBusMessage *message;
 
    message = dbus_message_new_signal ("/org/pragha/DBus",
                                       "org.pragha.DBus",
                                       msg);
    /* Send the message */
    dbus_connection_send (pragha->connection, message, NULL);
    dbus_message_unref (message);
}

void
prev_button_handler(GtkButton *button, PraghaPlugin *pragha)
{
	send_message (pragha, "prev");
}

void
play_button_handler(GtkButton *button, PraghaPlugin    *pragha)
{
	/* Pragha play, pause and resume with pause action */
	send_message (pragha, "pause");
}

void
stop_button_handler(GtkButton *button, PraghaPlugin    *pragha)
{
	send_message (pragha, "stop");
}

void
next_button_handler(GtkButton *button, PraghaPlugin    *pragha)
{
	send_message (pragha, "next");
}

void
pragha_save (XfcePanelPlugin *plugin,
             PraghaPlugin    *pragha)
{
  XfceRc *rc;
  gchar  *file;

  /* get the config file location */
  file = xfce_panel_plugin_save_location (plugin, TRUE);

  if (G_UNLIKELY (file == NULL))
    {
       DBG ("Failed to open config file");
       return;
    }

  /* open the config file, read/write */
  rc = xfce_rc_simple_open (file, FALSE);
  g_free (file);

  if (G_LIKELY (rc != NULL))
    {
      /* save the settings */
      DBG(".");
      if (pragha->setting1)
        xfce_rc_write_entry    (rc, "setting1", pragha->setting1);

      xfce_rc_write_int_entry  (rc, "setting2", pragha->setting2);
      xfce_rc_write_bool_entry (rc, "setting3", pragha->setting3);

      /* close the rc file */
      xfce_rc_close (rc);
    }
}



static void
pragha_read (PraghaPlugin *pragha)
{
  XfceRc      *rc;
  gchar       *file;
  const gchar *value;

  /* get the plugin config file location */
  file = xfce_panel_plugin_save_location (pragha->plugin, TRUE);

  if (G_LIKELY (file != NULL))
    {
      /* open the config file, readonly */
      rc = xfce_rc_simple_open (file, TRUE);

      /* cleanup */
      g_free (file);

      if (G_LIKELY (rc != NULL))
        {
          /* read the settings */
          value = xfce_rc_read_entry (rc, "setting1", DEFAULT_SETTING1);
          pragha->setting1 = g_strdup (value);

          pragha->setting2 = xfce_rc_read_int_entry (rc, "setting2", DEFAULT_SETTING2);
          pragha->setting3 = xfce_rc_read_bool_entry (rc, "setting3", DEFAULT_SETTING3);
          
          pragha->state = ST_STOPPED;

          /* cleanup */
          xfce_rc_close (rc);

          /* leave the function, everything went well */
          return;
        }
    }

  /* something went wrong, apply default values */
  DBG ("Applying default settings");

  pragha->setting1 = g_strdup (DEFAULT_SETTING1);
  pragha->setting2 = DEFAULT_SETTING2;
  pragha->setting3 = DEFAULT_SETTING3;
  pragha->state = ST_STOPPED;
}



static PraghaPlugin *
pragha_new (XfcePanelPlugin *plugin)
{
  PraghaPlugin   *pragha;
  GtkOrientation  orientation;
	GtkWidget *play_button, *stop_button, *prev_button, *next_button;
  DBusConnection *connection;

  /* allocate memory for the plugin structure */
  pragha = panel_slice_new0 (PraghaPlugin);

  /* pointer to plugin */
  pragha->plugin = plugin;

  /* read the user settings */
  pragha_read (pragha);

  /* get the current orientation */
  orientation = xfce_panel_plugin_get_orientation (plugin);

  /* create some panel widgets */
  pragha->ebox = gtk_event_box_new ();
  gtk_widget_show (pragha->ebox);

  pragha->hvbox = xfce_hvbox_new (orientation, FALSE, 2);
  gtk_widget_show (pragha->hvbox);
  gtk_container_add (GTK_CONTAINER (pragha->ebox), pragha->hvbox);

  /* some pragha widgets */

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

	pragha->image_pause =
		gtk_image_new_from_stock(GTK_STOCK_MEDIA_PAUSE,
					 GTK_ICON_SIZE_LARGE_TOOLBAR);
	pragha->image_play =
		gtk_image_new_from_stock(GTK_STOCK_MEDIA_PLAY,
					 GTK_ICON_SIZE_LARGE_TOOLBAR);

	g_object_ref(pragha->image_play);
	g_object_ref(pragha->image_pause);

	gtk_button_set_image(GTK_BUTTON(play_button),
			     pragha->image_play);

	gtk_box_pack_start(GTK_BOX(pragha->hvbox),
			   GTK_WIDGET(prev_button),
			   FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(pragha->hvbox),
			   GTK_WIDGET(play_button),
			   FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(pragha->hvbox),
			   GTK_WIDGET(stop_button),
			   FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(pragha->hvbox),
			   GTK_WIDGET(next_button),
			   FALSE, FALSE, 0);

	gtk_widget_show(prev_button);
	gtk_widget_show(play_button);
	gtk_widget_show(stop_button);
	gtk_widget_show(next_button);

	/* Signal handlers */

	g_signal_connect(G_OBJECT(prev_button), "clicked",
			 G_CALLBACK(prev_button_handler), pragha);
	g_signal_connect(G_OBJECT(play_button), "clicked",
			 G_CALLBACK(play_button_handler), pragha);
	g_signal_connect(G_OBJECT(stop_button), "clicked",
			 G_CALLBACK(stop_button_handler), pragha);
	g_signal_connect(G_OBJECT(next_button), "clicked",
			 G_CALLBACK(next_button_handler), pragha);

  xfce_panel_plugin_add_action_widget (plugin, prev_button);
  xfce_panel_plugin_add_action_widget (plugin, play_button);
  xfce_panel_plugin_add_action_widget (plugin, stop_button);
  xfce_panel_plugin_add_action_widget (plugin, next_button);

	pragha->play_button = play_button;

	/* Pragha dbus helpers */

  connection = dbus_bus_get (DBUS_BUS_SESSION, NULL);
  
  dbus_bus_add_match (connection, "type=\'signal\',interface=\'org.pragha.DBus\'", NULL);
  dbus_connection_add_filter (connection, dbus_filter, pragha, NULL);
  dbus_connection_setup_with_g_main (connection, NULL);

  pragha->connection = connection;

  return pragha;
}

static void
pragha_free (XfcePanelPlugin *plugin,
             PraghaPlugin    *pragha)
{
  GtkWidget *dialog;

  /* check if the dialog is still open. if so, destroy it */
  dialog = g_object_get_data (G_OBJECT (plugin), "dialog");
  if (G_UNLIKELY (dialog != NULL))
    gtk_widget_destroy (dialog);

  /* destroy the panel widgets */
  gtk_widget_destroy (pragha->hvbox);

  /* cleanup the settings */
  if (G_LIKELY (pragha->setting1 != NULL))
    g_free (pragha->setting1);

  /* free the plugin structure */
  panel_slice_free (PraghaPlugin, pragha);
}



static void
pragha_orientation_changed (XfcePanelPlugin *plugin,
                            GtkOrientation   orientation,
                            PraghaPlugin    *pragha)
{
  /* change the orienation of the box */
  xfce_hvbox_set_orientation (XFCE_HVBOX (pragha->hvbox), orientation);
}



static gboolean
pragha_size_changed (XfcePanelPlugin *plugin,
                     gint             size,
                     PraghaPlugin    *pragha)
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
pragha_construct (XfcePanelPlugin *plugin)
{
  PraghaPlugin *pragha;

  /* setup transation domain */
  xfce_textdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

  /* create the plugin */
  pragha = pragha_new (plugin);

  /* add the ebox to the panel */
  gtk_container_add (GTK_CONTAINER (plugin), pragha->ebox);

  /* show the panel's right-click menu on this ebox */
  xfce_panel_plugin_add_action_widget (plugin, pragha->ebox);

  /* connect plugin signals */
  g_signal_connect (G_OBJECT (plugin), "free-data",
                    G_CALLBACK (pragha_free), pragha);

  g_signal_connect (G_OBJECT (plugin), "save",
                    G_CALLBACK (pragha_save), pragha);

  g_signal_connect (G_OBJECT (plugin), "size-changed",
                    G_CALLBACK (pragha_size_changed), pragha);

  g_signal_connect (G_OBJECT (plugin), "orientation-changed",
                    G_CALLBACK (pragha_orientation_changed), pragha);

  /* show the configure menu item and connect signal */
  xfce_panel_plugin_menu_show_configure (plugin);
  g_signal_connect (G_OBJECT (plugin), "configure-plugin",
                    G_CALLBACK (pragha_configure), pragha);

  /* show the about menu item and connect signal */
  xfce_panel_plugin_menu_show_about (plugin);
  g_signal_connect (G_OBJECT (plugin), "about",
                    G_CALLBACK (pragha_about), NULL);
}
