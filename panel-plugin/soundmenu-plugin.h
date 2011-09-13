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

#ifndef __SOUNDMENU_H__
#define __SOUNDMENU_H__

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

#ifdef HAVE_LIBCLASTFM
#include <clastfm.h>
#include <pthread.h>
#endif

#ifdef HAVE_LIBKEYBINDER
#include <keybinder.h>
#endif

#define LASTFM_API_KEY             "ecdc2d21dbfe1139b1f0da35daca9309"
#define LASTFM_SECRET              "f3498ce387f30eeae8ea1b1023afb32b"

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
	gint lastfm_handler_id;
	time_t playback_started;
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
	enum player_state state;
	Metadata		*metadata;
	gdouble		volume;

	/* Dbus conecction */
	DBusConnection 	*connection;
	gchar			*dbus_name;

	/* soundmenu settings */
	gchar			*player;
	gint			setting2;
	gboolean		show_stop;
}
SoundmenuPlugin;

void
suondmenu_update_state(gchar *state, SoundmenuPlugin *soundmenu);

void
soundmenu_save (XfcePanelPlugin *plugin,
             SoundmenuPlugin    *soundmenu);

G_END_DECLS

#endif /* !__SOUNDMENU_H__ */
