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

#ifndef MPRIS2_METADATA_H
#define MPRIS2_METADATA_H

#include <glib.h>

G_BEGIN_DECLS

typedef struct _Mpris2Metadata Mpris2Metadata;

void
mpris2_metadata_set_trackid(Mpris2Metadata *metadata, const gchar *trackid);
const gchar *
mpris2_metadata_get_trackid(Mpris2Metadata *metadata);

void
mpris2_metadata_set_url(Mpris2Metadata *metadata, const gchar *url);
const gchar *
mpris2_metadata_get_url(Mpris2Metadata *metadata);

void
mpris2_metadata_set_title(Mpris2Metadata *metadata, const gchar *title);
const gchar *
mpris2_metadata_get_title(Mpris2Metadata *metadata);

void
mpris2_metadata_set_artist(Mpris2Metadata *metadata, const gchar *artist);
const gchar *
mpris2_metadata_get_artist(Mpris2Metadata *metadata);

void
mpris2_metadata_set_album(Mpris2Metadata *metadata, const gchar *album);
const gchar *
mpris2_metadata_get_album(Mpris2Metadata *metadata);

void
mpris2_metadata_set_length(Mpris2Metadata *metadata, guint length);
guint
mpris2_metadata_get_length(Mpris2Metadata *metadata);

void
mpris2_metadata_set_track_no(Mpris2Metadata *metadata, guint track_no);
guint
mpris2_metadata_get_track_no(Mpris2Metadata *metadata);

void
mpris2_metadata_set_arturl(Mpris2Metadata *metadata, const gchar *arturl);
const gchar *
mpris2_metadata_get_arturl(Mpris2Metadata *metadata);


Mpris2Metadata *mpris2_metadata_new(void);
void mpris2_metadata_free(Mpris2Metadata *metadata);

G_END_DECLS

#endif
