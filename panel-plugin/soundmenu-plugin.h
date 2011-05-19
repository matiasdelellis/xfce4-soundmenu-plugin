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

Metadata *malloc_metadata();
void free_metadata(Metadata *m);

void
soundmenu_save (XfcePanelPlugin *plugin,
             SoundmenuPlugin    *soundmenu);

G_END_DECLS

#endif /* !__SOUNDMENU_H__ */
