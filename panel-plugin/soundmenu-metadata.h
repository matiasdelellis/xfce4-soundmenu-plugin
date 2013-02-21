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

#include <glib.h>

typedef struct {
	gchar *trackid;
	gchar *url;
	gchar *title;
	gchar *artist;
	gchar *album;
	guint length;
	guint trackNumber;
	gchar *arturl;
} SoundmenuMetadata;

SoundmenuMetadata *soundmenu_metadata_new(void);
void soundmenu_metadata_free(SoundmenuMetadata *metadata);
