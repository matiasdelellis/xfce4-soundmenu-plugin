/*
 *  Copyright (c) 2011-2013 matias <mati86dl@gmail.com>
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

/* Lastfm Helper */

#ifndef SOUNDMENU_LASTFM_H
#define SOUNDMENU_LASTFM_H

#include "soundmenu-plugin.h"

void lastfm_track_love_action(GtkWidget *widget, SoundmenuPlugin    *soundmenu);
void lastfm_track_unlove_action (GtkWidget *widget, SoundmenuPlugin    *soundmenu);

gboolean lastfm_scrob_handler(gpointer data);
gboolean lastfm_now_playing_handler (gpointer data);

void update_lastfm (SoundmenuPlugin    *soundmenu);
void soundmenu_update_lastfm_menu (struct con_lastfm *clastfm);

gboolean do_soundmenu_init_lastfm(gpointer data);
gint init_lastfm_idle_timeout(SoundmenuPlugin *soundmenu);
gint just_init_lastfm (SoundmenuPlugin *soundmenu);
gint soundmenu_init_lastfm(SoundmenuPlugin *soundmenu);

#endif
