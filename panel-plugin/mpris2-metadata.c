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

#include "mpris2-metadata.h"

struct _Mpris2Metadata {
	gchar *trackid;
	gchar *url;
	gchar *title;
	gchar *artist;
	gchar *album;
	guint length;
	guint track_no;
	gchar *arturl;
};

/*
 * Set and get metadata.
 */
void
mpris2_metadata_set_trackid(Mpris2Metadata *metadata, const gchar *trackid)
{
	if(!metadata)
		return;

	if(metadata->trackid)
		g_free(metadata->trackid);

	metadata->trackid = g_strdup(trackid);
}

const gchar *
mpris2_metadata_get_trackid(Mpris2Metadata *metadata)
{
	if(!metadata)
		return NULL;

	return metadata->trackid;
}

void
mpris2_metadata_set_url(Mpris2Metadata *metadata, const gchar *url)
{
	if(!metadata)
		return;

	if(metadata->url)
		g_free(metadata->url);

	metadata->url = g_strdup(url);
}

const gchar *
mpris2_metadata_get_url(Mpris2Metadata *metadata)
{
	if(!metadata)
		return NULL;

	return metadata->url;
}

void
mpris2_metadata_set_title(Mpris2Metadata *metadata, const gchar *title)
{
	if(!metadata)
		return;

	if(metadata->title)
		g_free(metadata->title);

	metadata->title = g_strdup(title);
}

const gchar *
mpris2_metadata_get_title(Mpris2Metadata *metadata)
{
	if(!metadata)
		return NULL;

	return metadata->title;
}

void
mpris2_metadata_set_artist(Mpris2Metadata *metadata, const gchar *artist)
{
	if(!metadata)
		return;

	if(metadata->artist)
		g_free(metadata->artist);

	metadata->artist = g_strdup(artist);
}

const gchar *
mpris2_metadata_get_artist(Mpris2Metadata *metadata)
{
	if(!metadata)
		return NULL;

	return metadata->artist;
}

void
mpris2_metadata_set_album(Mpris2Metadata *metadata, const gchar *album)
{
	if(!metadata)
		return;

	if(metadata->album)
		g_free(metadata->album);

	metadata->album = g_strdup(album);
}

const gchar *
mpris2_metadata_get_album(Mpris2Metadata *metadata)
{
	if(!metadata)
		return NULL;

	return metadata->album;
}

void
mpris2_metadata_set_length(Mpris2Metadata *metadata, guint length)
{
	if(!metadata)
		return;

	metadata->length = length;
}

guint
mpris2_metadata_get_length(Mpris2Metadata *metadata)
{
	if(!metadata)
		return 0;

	return metadata->length;
}

void
mpris2_metadata_set_track_no(Mpris2Metadata *metadata, guint track_no)
{
	if(!metadata)
		return;

	metadata->track_no = track_no;
}

guint
mpris2_metadata_get_track_no(Mpris2Metadata *metadata)
{
	if(!metadata)
		return 0;

	return metadata->track_no;
}

void
mpris2_metadata_set_arturl(Mpris2Metadata *metadata, const gchar *arturl)
{
	if(!metadata)
		return;

	if(metadata->arturl)
		g_free(metadata->arturl);

	metadata->arturl = g_strdup(arturl);
}

const gchar *
mpris2_metadata_get_arturl(Mpris2Metadata *metadata)
{
	if(!metadata)
		return NULL;

	return metadata->arturl;
}

/*
 * Construction and destruction of metadata.
 */
Mpris2Metadata *
mpris2_metadata_new (void)
{
	Mpris2Metadata *metadata;
	metadata = g_slice_new0(Mpris2Metadata);

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
mpris2_metadata_free(Mpris2Metadata *metadata)
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

	g_slice_free(Mpris2Metadata, metadata);
}
