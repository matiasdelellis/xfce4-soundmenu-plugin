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

#ifndef SOUNDMENU_LASTFM_H
#define SOUNDMENU_LASTFM_H

#include <gtk/gtk.h>
typedef struct _SoundmenuLastfm SoundmenuLastfm;

#include "soundmenu-plugin.h"

G_BEGIN_DECLS

void lastfm_track_love_action   (GtkWidget *widget, SoundmenuPlugin *soundmenu);
void lastfm_track_unlove_action (GtkWidget *widget, SoundmenuPlugin *soundmenu);

void update_lastfm (SoundmenuPlugin    *soundmenu);
void soundmenu_update_lastfm_menu (SoundmenuLastfm *clastfm);

void             soundmenu_lastfm_uninit        (SoundmenuLastfm *lastfm);
gint             soundmenu_lastfm_init          (SoundmenuLastfm *lastfm);
gboolean         soundmenu_lastfm_is_initiated  (SoundmenuLastfm *lastfm);

gboolean         soundmenu_lastfm_is_supported  (SoundmenuLastfm *lastfm);
void             soundmenu_lastfm_set_supported (SoundmenuLastfm *lastfm, gboolean support);

const gchar     *soundmenu_lastfm_get_user      (SoundmenuLastfm *lastfm);
void             soundmenu_lastfm_set_user      (SoundmenuLastfm *lastfm, const gchar *user);

const gchar     *soundmenu_lastfm_get_password  (SoundmenuLastfm *lastfm);
void             soundmenu_lastfm_set_password  (SoundmenuLastfm *lastfm, const gchar *password);

void             soundmenu_lastfm_free          (SoundmenuLastfm *lastfm);
SoundmenuLastfm *soundmenu_lastfm_new           (void);

G_END_DECLS

#endif /* SOUNDMENU_LASTFM_H */
