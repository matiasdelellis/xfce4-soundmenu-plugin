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

#ifndef SOUNDMENU_DBUS_H
#define SOUNDMENU_DBUS_H

#include "soundmenu-plugin.h"

void      soundmenu_mpris2_send_player_message    (SoundmenuPlugin *soundmenu, const char *msg);
void      soundmenu_mpris2_properties_set_by_name (SoundmenuPlugin *soundmenu, const gchar *name, const gchar *prop);
void      soundmenu_mpris2_properties_set_volume  (SoundmenuPlugin *soundmenu, gdouble volume);

gchar    *soundmenu_get_mpris2_player_running     (SoundmenuPlugin *soundmenu);

GVariant *soundmenu_mpris2_properties_get_all     (SoundmenuPlugin *soundmenu);

void      soundmenu_mpris2_reinit_dbus            (SoundmenuPlugin *soundmenu);
void      init_dbus_session                       (SoundmenuPlugin *soundmenu);

#endif