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

#include "soundmenu-plugin.h"
#include "soundmenu-dialogs.h"

/* default settings */
#define DEFAULT_PLAYER "pragha"
#define DEFAULT_SETTING2 1
#define DEFAULT_SHOW_STOP TRUE

/* prototypes */

static void
soundmenu_construct (XfcePanelPlugin *plugin);

/* register the plugin */

XFCE_PANEL_PLUGIN_REGISTER_EXTERNAL (soundmenu_construct);

/* Dbus helpers to connect with Soundmenu */

void play_button_toggle_state (SoundmenuPlugin *soundmenu)
{
	if ((soundmenu->state == ST_PAUSED) || (soundmenu->state == ST_STOPPED))
		gtk_button_set_image(GTK_BUTTON(soundmenu->play_button), soundmenu->image_play);
	else
		gtk_button_set_image(GTK_BUTTON(soundmenu->play_button), soundmenu->image_pause);
}

static void update_state(gchar *state, SoundmenuPlugin *soundmenu)
{
	if (0 == g_ascii_strcasecmp(state, "Playing"))
		soundmenu->state = ST_PLAYING;
	else if (0 == g_ascii_strcasecmp(state, "Paused"))
		soundmenu->state = ST_PAUSED;
	else
		soundmenu->state = ST_STOPPED;

	play_button_toggle_state(soundmenu);
}

static void get_meta_item_array(DBusMessageIter *dict_entry, char **item)
{
	DBusMessageIter variant, array;
	char *str_buf;

	dbus_message_iter_next(dict_entry);
	dbus_message_iter_recurse(dict_entry, &array);

	dbus_message_iter_recurse(&array, &variant);
	dbus_message_iter_get_basic(&variant, (void*) &str_buf);

	*item = malloc(strlen(str_buf) + 1);
	strcpy(*item, str_buf);
}

static void get_meta_item_str(DBusMessageIter *dict_entry, char **item)
{
	DBusMessageIter variant;
	char *str_buf;

	dbus_message_iter_next(dict_entry);
	dbus_message_iter_recurse(dict_entry, &variant);
	dbus_message_iter_get_basic(&variant, (void*) &str_buf);

	*item = malloc(strlen(str_buf) + 1);
	strcpy(*item, str_buf);
}

static void get_meta_item_gint(DBusMessageIter *dict_entry, void *item)
{
	DBusMessageIter variant;

	dbus_message_iter_next(dict_entry);
	dbus_message_iter_recurse(dict_entry, &variant);
	dbus_message_iter_get_basic(&variant, (void*) item);
}

demarshal_metadata (DBusMessageIter *args, SoundmenuPlugin *soundmenu)	// arg inited on Metadata string
{
	DBG ("Demarshal_metadata");

	DBusMessageIter dict, dict_entry, variant;
	gchar *str_buf = NULL, *string = NULL;
	gchar *trackid = NULL, *url = NULL, *title = NULL, *artist = NULL, *album = NULL, *arturl = NULL;
	gchar *tooltip = NULL;

	gint64 length = 0;
	gint32 trackNumber = 0;

	dbus_message_iter_next(args);				// Next => args on "variant array []"
	dbus_message_iter_recurse(args, &dict);		// Recurse => dict on fist "dict entry()"
	
	dbus_message_iter_recurse(&dict, &dict_entry);	// Recurse => dict_entry on "string "mpris:trackid""
	do
	{
		dbus_message_iter_recurse(&dict_entry, &variant);
		dbus_message_iter_get_basic(&variant, (void*) &str_buf);

		if (0 == g_ascii_strcasecmp (str_buf, "mpris:trackid"))
			get_meta_item_str(&variant, &trackid);
		else if (0 == g_ascii_strcasecmp (str_buf, "xesam:url"))
			get_meta_item_str(&variant, &url);
		else if (0 == g_ascii_strcasecmp (str_buf, "xesam:title"))
			get_meta_item_str(&variant, &title);
		else if (0 == g_ascii_strcasecmp (str_buf, "xesam:artist"))
			get_meta_item_array(&variant, &artist);
		else if (0 == g_ascii_strcasecmp (str_buf, "xesam:album"))
			get_meta_item_str(&variant, &album);
		else if (0 == g_ascii_strcasecmp (str_buf, "xesam:genre"));
			/* (List of Strings.) Not use genre */
		else if (0 == g_ascii_strcasecmp (str_buf, "xesam:albumArtist"));
			// List of Strings.
		else if (0 == g_ascii_strcasecmp (str_buf, "xesam:comment"));
			/* (List of Strings) Not use comment */
		else if (0 == g_ascii_strcasecmp (str_buf, "xesam:audioBitrate"));
			/* (uint32) Not use audioBitrate */
		else if (0 == g_ascii_strcasecmp (str_buf, "mpris:length"))
			get_meta_item_gint(&variant, &length);
		else if (0 == g_ascii_strcasecmp (str_buf, "xesam:trackNumber"))
			get_meta_item_gint(&variant, &trackNumber);
		else if (0 == g_ascii_strcasecmp (str_buf, "xesam:useCount"));
			/* (Integer) Not use useCount */
		else if (0 == g_ascii_strcasecmp (str_buf, "xesam:userRating"));
			/* (Float) Not use userRating */
		else if (0 == g_ascii_strcasecmp (str_buf, "mpris:arturl"))
			get_meta_item_str(&variant, &arturl);
		else
			DBG ("New metadata message: %s. (Investigate)\n", str_buf);
	
	} while (dbus_message_iter_next(&dict_entry));

	tooltip = g_markup_printf_escaped(_("%s\nby %s in %s"),
			(title && strlen(title)) ? title : url,
			(artist && strlen(artist)) ? artist : _("Unknown Artist"),
			(album && strlen(album)) ? album : _("Unknown Album"));
	
	gtk_widget_set_tooltip_text(GTK_WIDGET(soundmenu->prev_button), tooltip);
	gtk_widget_set_tooltip_text(GTK_WIDGET(soundmenu->play_button), tooltip);
	gtk_widget_set_tooltip_text(GTK_WIDGET(soundmenu->stop_button), tooltip);
	gtk_widget_set_tooltip_text(GTK_WIDGET(soundmenu->next_button), tooltip);

	g_free(tooltip);
	g_free(trackid);
	g_free(url);
	g_free(title);
	g_free(artist);
	g_free(album);
	g_free(arturl);
}

static DBusHandlerResult
dbus_filter (DBusConnection *connection, DBusMessage *message, void *user_data)
{
	DBusMessageIter args, dict, dict_entry;
	gchar *str_buf = NULL, *state = NULL;

	SoundmenuPlugin *soundmenu = user_data;

	if ( dbus_message_is_signal (message, "org.freedesktop.DBus.Properties", "PropertiesChanged" ) )
	{
		dbus_message_iter_init(message, &args);

		/* Ignore the interface_name*/
		dbus_message_iter_next(&args);

		dbus_message_iter_recurse(&args, &dict);
		do
		{
			dbus_message_iter_recurse(&dict, &dict_entry);
			dbus_message_iter_get_basic(&dict_entry, (void*) &str_buf);

			if (0 == g_ascii_strcasecmp (str_buf, "PlaybackStatus"))
			{
				get_meta_item_str (&dict_entry, &state);
				update_state (state, soundmenu);
			}
			else if (0 == g_ascii_strcasecmp (str_buf, "Metadata"))
			{
				demarshal_metadata (&dict_entry, soundmenu);
			}
		} while (dbus_message_iter_next(&dict));

		return DBUS_HANDLER_RESULT_HANDLED;
	}
	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

void
send_message (SoundmenuPlugin *soundmenu, const char *msg)
{
	DBusMessage *message;
	gchar *destination = NULL;

	destination = g_strdup_printf ("org.mpris.MediaPlayer2.%s", soundmenu->player);
	message = dbus_message_new_method_call (destination, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player",  msg);
	g_free(destination);

	/* Send the message */
	dbus_connection_send (soundmenu->connection, message, NULL);
	dbus_message_unref (message);
}

void
prev_button_handler(GtkButton *button, SoundmenuPlugin *soundmenu)
{
	send_message (soundmenu, "Previous");
}

void
play_button_handler(GtkButton *button, SoundmenuPlugin *soundmenu)
{
	send_message (soundmenu, "PlayPause");
}

void
stop_button_handler(GtkButton *button, SoundmenuPlugin    *soundmenu)
{
	send_message (soundmenu, "Stop");
}

void
next_button_handler(GtkButton *button, SoundmenuPlugin    *soundmenu)
{
	send_message (soundmenu, "Next");
}

void
soundmenu_save (XfcePanelPlugin *plugin,
             SoundmenuPlugin    *soundmenu)
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
      if (soundmenu->player)
        xfce_rc_write_entry    (rc, "player", soundmenu->player);

      xfce_rc_write_int_entry  (rc, "setting2", soundmenu->setting2);
      xfce_rc_write_bool_entry (rc, "show_stop", soundmenu->show_stop);

      /* close the rc file */
      xfce_rc_close (rc);
    }
}

static void
soundmenu_read (SoundmenuPlugin *soundmenu)
{
  XfceRc      *rc;
  gchar       *file;
  const gchar *value;

  /* get the plugin config file location */
  file = xfce_panel_plugin_save_location (soundmenu->plugin, TRUE);

  if (G_LIKELY (file != NULL))
    {
      /* open the config file, readonly */
      rc = xfce_rc_simple_open (file, TRUE);

      /* cleanup */
      g_free (file);

      if (G_LIKELY (rc != NULL))
        {
          /* read the settings */
          value = xfce_rc_read_entry (rc, "player", DEFAULT_PLAYER);
          soundmenu->player = g_strdup (value);

          soundmenu->setting2 = xfce_rc_read_int_entry (rc, "setting2", DEFAULT_SETTING2);
          soundmenu->show_stop = xfce_rc_read_bool_entry (rc, "show_stop", DEFAULT_SHOW_STOP);
          
          soundmenu->state = ST_STOPPED;

          /* cleanup */
          xfce_rc_close (rc);

          /* leave the function, everything went well */
          return;
        }
    }

  /* something went wrong, apply default values */
  DBG ("Applying default settings");

  soundmenu->player = g_strdup (DEFAULT_PLAYER);
  soundmenu->setting2 = DEFAULT_SETTING2;
  soundmenu->show_stop = DEFAULT_SHOW_STOP;
  soundmenu->state = ST_STOPPED;
}



static SoundmenuPlugin *
soundmenu_new (XfcePanelPlugin *plugin)
{
  SoundmenuPlugin   *soundmenu;
  GtkOrientation  orientation;
	GtkWidget *play_button, *stop_button, *prev_button, *next_button;
  DBusConnection *connection;
  gchar *rule = NULL;

  /* allocate memory for the plugin structure */
  soundmenu = panel_slice_new0 (SoundmenuPlugin);

  /* pointer to plugin */
  soundmenu->plugin = plugin;

  /* read the user settings */
  soundmenu_read (soundmenu);

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

	soundmenu->prev_button = prev_button;
	soundmenu->play_button = play_button;
	soundmenu->stop_button = stop_button;
	soundmenu->next_button = next_button;

	/* Soundmenu dbus helpers */

	connection = dbus_bus_get (DBUS_BUS_SESSION, NULL);

	soundmenu->dbus_name = g_strdup_printf("org.mpris.MediaPlayer2.%s", soundmenu->player);

	rule = g_strdup_printf ("type='signal', sender='%s'", soundmenu->dbus_name);
	dbus_bus_add_match (connection, rule, NULL);
	g_free(rule);
  
	dbus_connection_add_filter (connection, dbus_filter, soundmenu, NULL);
	dbus_connection_setup_with_g_main (connection, NULL);

	soundmenu->connection = connection;

	return soundmenu;
}

static void
soundmenu_free (XfcePanelPlugin *plugin,
             SoundmenuPlugin    *soundmenu)
{
  GtkWidget *dialog;

  /* check if the dialog is still open. if so, destroy it */
  dialog = g_object_get_data (G_OBJECT (plugin), "dialog");
  if (G_UNLIKELY (dialog != NULL))
    gtk_widget_destroy (dialog);

  /* destroy the panel widgets */
  gtk_widget_destroy (soundmenu->hvbox);

  /* cleanup the settings */
  if (G_LIKELY (soundmenu->player != NULL))
    g_free (soundmenu->player);
  if (G_LIKELY (soundmenu->dbus_name != NULL))
    g_free (soundmenu->dbus_name);

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
