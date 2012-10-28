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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef __SOUNDMENU_H__
#define __SOUNDMENU_H__

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <gtk/gtk.h>
#include <glib.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <libxfce4ui/libxfce4ui.h>
#include <libxfce4panel/xfce-panel-plugin.h>
#include <libxfce4util/libxfce4util.h>
#include <libxfce4panel/xfce-hvbox.h>

#ifdef HAVE_LIBCLASTFM
#include <clastfm.h>
#endif

#ifdef HAVE_LIBKEYBINDER
#include <keybinder.h>
#endif

#ifdef HAVE_LIBGLYR
#include <glyr/glyr.h>
#endif

#ifdef HAVE_LIBNOTIFY
#include <libnotify/notify.h>
#endif

#ifndef NOTIFY_CHECK_VERSION
#define NOTIFY_CHECK_VERSION(x,y,z) 0
#endif

#define LASTFM_API_KEY             "70c479ab2632e597fd9215cf35963c1b"
#define LASTFM_SECRET              "4cb5255d955edc8f651de339fd2f335b"

G_BEGIN_DECLS

typedef struct {
	char *trackid;
	char *url;
	char *title;
	char *artist;
	char *album;
	unsigned int length;
	unsigned int trackNumber;
	char *arturl;
} Metadata;


#ifdef HAVE_LIBCLASTFM
struct lastfm_pref {
	GtkWidget *lastfm_w;
	GtkWidget *lastfm_uname_w;
	GtkWidget *lastfm_pass_w;
};

struct con_lastfm {
	gboolean lastfm_support;
	gchar *lastfm_user;
	gchar *lastfm_pass;
	LASTFM_SESSION *session_id;
	enum LASTFM_STATUS_CODES status;
	gint lastfm_handler_id;
	time_t playback_started;
	GtkWidget *lastfm_menu;
};
#endif

enum player_state {
	ST_PLAYING = 1,
	ST_STOPPED,
	ST_PAUSED
};

/* plugin structure */
typedef struct
{
	XfcePanelPlugin	*plugin;

	/* panel widgets */
	GtkWidget		*hvbox;
	GtkWidget		*hvbox_buttons;
	GtkWidget		*album_art;
	GtkWidget		*ev_album_art;
	GtkWidget		*prev_button;
	GtkWidget		*play_button;
	GtkWidget		*stop_button;
	GtkWidget		*next_button;
	GtkWidget		*image_pause;
	GtkWidget		*image_play;

	/* Helper to obtain player name of preferences */
	GtkWidget	*w_player;
	#ifdef HAVE_LIBCLASTFM
	struct lastfm_pref lw;
	struct con_lastfm *clastfm;
	#endif

	/* Player states */
	enum player_state	state;
	Metadata		*metadata;
	gdouble			volume;

	/* Dbus conecction */
	DBusConnection 	*connection;
	gchar			*dbus_name;

	/* soundmenu settings */
	gchar			*player;
	gboolean		show_album_art;
	gboolean		show_tiny_album_art;
	gboolean		show_stop;
	gboolean		use_global_keys;
	gint 			size_request;

}
SoundmenuPlugin;

void
soundmenu_update_layout_changes (SoundmenuPlugin    *soundmenu);

void
soundmenu_update_state(gchar *state, SoundmenuPlugin *soundmenu);

void
soundmenu_save (XfcePanelPlugin *plugin,
             SoundmenuPlugin    *soundmenu);

void keybinder_bind_keys(SoundmenuPlugin *soundmenu);
void keybinder_unbind_keys(SoundmenuPlugin *soundmenu);

G_END_DECLS

#endif /* !__SOUNDMENU_H__ */
