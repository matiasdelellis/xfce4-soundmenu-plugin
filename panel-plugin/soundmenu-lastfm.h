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
 *  Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
 */


/* Lastfm Helper */

void lastfm_track_love_action(GtkWidget *widget, SoundmenuPlugin    *soundmenu);
void lastfm_track_unlove_action (GtkWidget *widget, SoundmenuPlugin    *soundmenu);

gboolean lastfm_love_handler (SoundmenuPlugin  *soundmenu);
gboolean lastfm_scrob_handler (SoundmenuPlugin  *soundmenu);
gboolean lastfm_now_playing_handler (gpointer data);

void update_lastfm (SoundmenuPlugin    *soundmenu);

gint init_lastfm_idle_timeout(SoundmenuPlugin *soundmenu);
gint just_init_lastfm (SoundmenuPlugin *soundmenu);