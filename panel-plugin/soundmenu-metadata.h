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

#ifndef SOUNDMENU_METADATA_H
#define SOUNDMENU_METADATA_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>

G_BEGIN_DECLS

typedef struct _SoundmenuMetadata SoundmenuMetadata;

void
soundmenu_metatada_set_trackid(SoundmenuMetadata *metadata, const gchar *trackid);
const gchar *
soundmenu_metatada_get_trackid(SoundmenuMetadata *metadata);

void
soundmenu_metatada_set_url(SoundmenuMetadata *metadata, const gchar *url);
const gchar *
soundmenu_metatada_get_url(SoundmenuMetadata *metadata);

void
soundmenu_metatada_set_title(SoundmenuMetadata *metadata, const gchar *title);
const gchar *
soundmenu_metatada_get_title(SoundmenuMetadata *metadata);

void
soundmenu_metatada_set_artist(SoundmenuMetadata *metadata, const gchar *artist);
const gchar *
soundmenu_metatada_get_artist(SoundmenuMetadata *metadata);

void
soundmenu_metatada_set_album(SoundmenuMetadata *metadata, const gchar *album);
const gchar *
soundmenu_metatada_get_album(SoundmenuMetadata *metadata);

void
soundmenu_metatada_set_length(SoundmenuMetadata *metadata, guint length);
guint
soundmenu_metatada_get_length(SoundmenuMetadata *metadata);

void
soundmenu_metatada_set_track_no(SoundmenuMetadata *metadata, guint track_no);
guint
soundmenu_metatada_get_track_no(SoundmenuMetadata *metadata);

void
soundmenu_metatada_set_arturl(SoundmenuMetadata *metadata, const gchar *arturl);
const gchar *
soundmenu_metatada_get_arturl(SoundmenuMetadata *metadata);


SoundmenuMetadata *soundmenu_metadata_new(void);
void soundmenu_metadata_free(SoundmenuMetadata *metadata);

G_END_DECLS

#endif
