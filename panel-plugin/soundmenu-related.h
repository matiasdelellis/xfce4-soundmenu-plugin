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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef SOUNDMENU_RELATED_H
#define SOUNDMENU_RELATED_H

#include "soundmenu-plugin.h"

void soundmenu_search_lyric_dialog (GtkWidget *widget, SoundmenuPlugin *soundmenu);
void soundmenu_search_artistinfo_dialog (GtkWidget *widget, SoundmenuPlugin *soundmenu);

int uninit_glyr_related (SoundmenuPlugin *soundmenu);
int init_glyr_related (SoundmenuPlugin *soundmenu);

#endif
