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

Metadata *malloc_metadata();
void free_metadata(Metadata *m);

Metadata *
soundmenu_mpris2_get_metadata (GVariant *dictionary);
void
soundmenu_mpris2_parse_properties(SoundmenuPlugin *soundmenu, GVariant *properties);
void
soundmenu_mpris2_forse_update(SoundmenuPlugin *soundmenu);

void
prev_button_handler(GtkButton *button, SoundmenuPlugin *soundmenu);
void
play_button_handler(GtkButton *button, SoundmenuPlugin *soundmenu);
void
stop_button_handler(GtkButton *button, SoundmenuPlugin *soundmenu);
void
next_button_handler(GtkButton *button, SoundmenuPlugin *soundmenu);

gboolean
soundmenu_panel_button_scrolled (GtkWidget        *widget,
                                 GdkEventScroll   *event,
                                 SoundmenuPlugin *soundmenu);