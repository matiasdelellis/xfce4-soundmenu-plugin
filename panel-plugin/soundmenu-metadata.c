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

#include "soundmenu-metadata.h"

/* Basic metadata object */

SoundmenuMetadata *
soundmenu_metadata_new (void)
{
	SoundmenuMetadata *metadata;
	metadata = g_slice_new0(SoundmenuMetadata);

	metadata->trackid = NULL;
	metadata->url = NULL;
	metadata->title = NULL;
	metadata->artist = NULL;
	metadata->album = NULL;
	metadata->length = 0;
	metadata->trackNumber = 0;
	metadata->arturl = NULL;

	return metadata;
}

void
soundmenu_metadata_free(SoundmenuMetadata *metadata)
{
	if(metadata == NULL)
		return;

	if(metadata->trackid)
		g_free(metadata->trackid);
	if(metadata->url)
		g_free(metadata->url);
	if(metadata->title)
		g_free(metadata->title);
	if(metadata->artist)
		g_free(metadata->artist);
	if(metadata->album) 
		g_free(metadata->album);
	if(metadata->arturl)
		g_free(metadata->arturl);

	g_slice_free(SoundmenuMetadata, metadata);
}
