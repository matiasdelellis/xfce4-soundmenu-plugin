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

/*
 * Set and get metadata.
 */
void
soundmenu_metatada_set_trackid(SoundmenuMetadata *metadata, const gchar *trackid)
{
	if(!metadata)
		return;

	if(metadata->trackid)
		g_free(metadata->trackid);

	metadata->trackid = g_strdup(trackid);
}

const gchar *
soundmenu_metatada_get_trackid(SoundmenuMetadata *metadata)
{
	if(!metadata)
		return NULL;

	return metadata->trackid;
}

void
soundmenu_metatada_set_url(SoundmenuMetadata *metadata, const gchar *url)
{
	if(!metadata)
		return;

	if(metadata->url)
		g_free(metadata->url);

	metadata->url = g_strdup(url);
}

const gchar *
soundmenu_metatada_get_url(SoundmenuMetadata *metadata)
{
	if(!metadata)
		return NULL;

	return metadata->url;
}

void
soundmenu_metatada_set_title(SoundmenuMetadata *metadata, const gchar *title)
{
	if(!metadata)
		return;

	if(metadata->title)
		g_free(metadata->title);

	metadata->title = g_strdup(title);
}

const gchar *
soundmenu_metatada_get_title(SoundmenuMetadata *metadata)
{
	if(!metadata)
		return NULL;

	return metadata->title;
}

void
soundmenu_metatada_set_artist(SoundmenuMetadata *metadata, const gchar *artist)
{
	if(!metadata)
		return;

	if(metadata->artist)
		g_free(metadata->artist);

	metadata->artist = g_strdup(artist);
}

const gchar *
soundmenu_metatada_get_artist(SoundmenuMetadata *metadata)
{
	if(!metadata)
		return NULL;

	return metadata->artist;
}

void
soundmenu_metatada_set_album(SoundmenuMetadata *metadata, const gchar *album)
{
	if(!metadata)
		return;

	if(metadata->album)
		g_free(metadata->album);

	metadata->album = g_strdup(album);
}

const gchar *
soundmenu_metatada_get_album(SoundmenuMetadata *metadata)
{
	if(!metadata)
		return NULL;

	return metadata->album;
}

void
soundmenu_metatada_set_length(SoundmenuMetadata *metadata, guint length)
{
	if(!metadata)
		return;

	metadata->length = length;
}

guint
soundmenu_metatada_get_length(SoundmenuMetadata *metadata)
{
	if(!metadata)
		return 0;

	return metadata->length;
}

void
soundmenu_metatada_set_track_no(SoundmenuMetadata *metadata, guint track_no)
{
	if(!metadata)
		return;

	metadata->track_no = track_no;
}

guint
soundmenu_metatada_get_track_no(SoundmenuMetadata *metadata)
{
	if(!metadata)
		return 0;

	return metadata->track_no;
}

void
soundmenu_metatada_set_arturl(SoundmenuMetadata *metadata, const gchar *arturl)
{
	if(!metadata)
		return;

	if(metadata->arturl)
		g_free(metadata->arturl);

	metadata->arturl = g_strdup(arturl);
}

const gchar *
soundmenu_metatada_get_arturl(SoundmenuMetadata *metadata)
{
	if(!metadata)
		return NULL;

	return metadata->arturl;
}

/*
 * Construction and destruction of metadata.
 */
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
	metadata->track_no = 0;
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
