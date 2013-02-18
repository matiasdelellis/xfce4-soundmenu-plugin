/*************************************************************************/
/* Copyright (C) 2012-2013 matias <mati86dl@gmail.com>			 */
/* 									 */
/* This program is free software: you can redistribute it and/or modify	 */
/* it under the terms of the GNU General Public License as published by	 */
/* the Free Software Foundation, either version 3 of the License, or	 */
/* (at your option) any later version.					 */
/* 									 */
/* This program is distributed in the hope that it will be useful,	 */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of	 */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	 */
/* GNU General Public License for more details.				 */
/* 									 */
/* You should have received a copy of the GNU General Public License	 */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */
/*************************************************************************/

#ifndef SOUNDMENU_ALBUM_ART_H
#define SOUNDMENU_ALBUM_ART_H

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define SOUNDMENU_TYPE_ALBUM_ART (soundmenu_album_art_get_type())
#define SOUNDMENU_ALBUM_ART(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SOUNDMENU_TYPE_ALBUM_ART, SoundmenuAlbumArt))
#define SOUNDMENU_ALBUM_ART_CONST(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SOUNDMENU_TYPE_ALBUM_ART, SoundmenuAlbumArt const))
#define SOUNDMENU_ALBUM_ART_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), SOUNDMENU_TYPE_ALBUM_ART, SoundmenuAlbumArtClass))
#define SOUNDMENU_IS_ALBUM_ART(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SOUNDMENU_TYPE_ALBUM_ART))
#define SOUNDMENU_IS_ALBUM_ART_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SOUNDMENU_TYPE_ALBUM_ART))
#define SOUNDMENU_ALBUM_ART_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), SOUNDMENU_TYPE_ALBUM_ART, SoundmenuAlbumArtClass))


typedef struct _SoundmenuAlbumArt SoundmenuAlbumArt;
typedef struct _SoundmenuAlbumArtClass SoundmenuAlbumArtClass;
typedef struct _SoundmenuAlbumArtPrivate SoundmenuAlbumArtPrivate;

struct _SoundmenuAlbumArt
{
   GtkImage parent;

   /*< private >*/
   SoundmenuAlbumArtPrivate *priv;
};

struct _SoundmenuAlbumArtClass
{
   GtkImageClass parent_class;
};

SoundmenuAlbumArt *soundmenu_album_art_new (void);
GType soundmenu_album_art_get_type (void) G_GNUC_CONST;
const gchar *soundmenu_album_art_get_path (SoundmenuAlbumArt *albumart);
void soundmenu_album_art_set_path (SoundmenuAlbumArt *albumart,
                                   const char *path);
guint
soundmenu_album_art_get_size (SoundmenuAlbumArt *albumart);
void
soundmenu_album_art_set_size (SoundmenuAlbumArt *albumart,
                              guint size);
void
soundmenu_album_art_set_pixbuf (SoundmenuAlbumArt *albumart,
                                GdkPixbuf *pixbuf);
GdkPixbuf *
soundmenu_album_art_get_pixbuf (SoundmenuAlbumArt *albumart);

G_END_DECLS

#endif /* SOUNDMENU_ALBUM_ART_H */
