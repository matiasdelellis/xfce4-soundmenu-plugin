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

#include "soundmenu-album-art.h"

G_DEFINE_TYPE(SoundmenuAlbumArt, soundmenu_album_art, GTK_TYPE_IMAGE)

struct _SoundmenuAlbumArtPrivate
{
   gchar *path;
   guint size;
};

enum
{
   PROP_0,
   PROP_PATH,
   PROP_SIZE,
   LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

SoundmenuAlbumArt *
soundmenu_album_art_new (void)
{
   return g_object_new(SOUNDMENU_TYPE_ALBUM_ART, NULL);
}

/**
 * soundmenu_album_art_update_image:
 *
 */

static void
soundmenu_album_art_update_image (SoundmenuAlbumArt *albumart)
{
   SoundmenuAlbumArtPrivate *priv;
   GdkPixbuf *pixbuf, *album_art, *frame;
   GError *error = NULL;

   g_return_if_fail(SOUNDMENU_IS_ALBUM_ART(albumart));

   priv = albumart->priv;

   frame = gdk_pixbuf_new_from_file (BASEICONDIR"/128x128/apps/xfce4-soundmenu-plugin.png", &error);

   if(priv->path != NULL) {
      album_art = gdk_pixbuf_new_from_file_at_scale(priv->path,
                                                    112,
                                                    112,
                                                    FALSE,
                                                    &error);
      if (album_art) {
         gdk_pixbuf_copy_area(album_art, 0 ,0 ,112 ,112, frame, 12, 8);
         g_object_unref(G_OBJECT(album_art));
      }
      else {
         g_critical("Unable to open image file: %s\n", priv->path);
         g_error_free(error);
      }
   }

   pixbuf = gdk_pixbuf_scale_simple (frame,
                                     priv->size,
                                     priv->size,
                                     GDK_INTERP_BILINEAR);

   soundmenu_album_art_set_pixbuf(albumart, pixbuf);

   g_object_unref(G_OBJECT(pixbuf));
   g_object_unref(G_OBJECT(frame));
}

/**
 * album_art_get_path:
 *
 */
const gchar *
soundmenu_album_art_get_path (SoundmenuAlbumArt *albumart)
{
   g_return_val_if_fail(SOUNDMENU_IS_ALBUM_ART(albumart), NULL);
   return albumart->priv->path;
}

/**
 * album_art_set_path:
 *
 */
void
soundmenu_album_art_set_path (SoundmenuAlbumArt *albumart,
                              const gchar *path)
{
   SoundmenuAlbumArtPrivate *priv;

   g_return_if_fail(SOUNDMENU_IS_ALBUM_ART(albumart));

   priv = albumart->priv;

   g_free(priv->path);
   if (path)
      priv->path = g_filename_from_uri(path, NULL, NULL);
   else
      priv->path = NULL;

   soundmenu_album_art_update_image(albumart);

   g_object_notify_by_pspec(G_OBJECT(albumart), gParamSpecs[PROP_PATH]);
}

/**
 * album_art_get_size:
 *
 */
guint
soundmenu_album_art_get_size (SoundmenuAlbumArt *albumart)
{
   g_return_val_if_fail(SOUNDMENU_IS_ALBUM_ART(albumart), 0);
   return albumart->priv->size;
}

/**
 * album_art_set_size:
 *
 */
void
soundmenu_album_art_set_size (SoundmenuAlbumArt *albumart,
                              guint size)
{
   SoundmenuAlbumArtPrivate *priv;

   g_return_if_fail(SOUNDMENU_IS_ALBUM_ART(albumart));

   priv = albumart->priv;

   priv->size = size;

   soundmenu_album_art_update_image(albumart);

   g_object_notify_by_pspec(G_OBJECT(albumart), gParamSpecs[PROP_SIZE]);
}

/**
 * album_art_set_pixbuf:
 *
 */
void
soundmenu_album_art_set_pixbuf (SoundmenuAlbumArt *albumart, GdkPixbuf *pixbuf)
{
   g_return_if_fail(SOUNDMENU_IS_ALBUM_ART(albumart));

   gtk_image_clear(GTK_IMAGE(albumart));
   gtk_image_set_from_pixbuf(GTK_IMAGE(albumart), pixbuf);
}

/**
 * album_art_get_pixbuf:
 *
 */
GdkPixbuf *
soundmenu_album_art_get_pixbuf (SoundmenuAlbumArt *albumart)
{
   GdkPixbuf *pixbuf = NULL;

   g_return_val_if_fail(SOUNDMENU_IS_ALBUM_ART(albumart), NULL);

   if(gtk_image_get_storage_type(GTK_IMAGE(albumart)) == GTK_IMAGE_PIXBUF)
      pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(albumart));

   return pixbuf;
}

static void
soundmenu_album_art_finalize (GObject *object)
{
   SoundmenuAlbumArtPrivate *priv;

   priv = SOUNDMENU_ALBUM_ART(object)->priv;

   g_free(priv->path);

   G_OBJECT_CLASS(soundmenu_album_art_parent_class)->finalize(object);
}

static void
soundmenu_album_art_get_property (GObject *object,
                                  guint prop_id,
                                  GValue *value,
                                  GParamSpec *pspec)
{
   SoundmenuAlbumArt *albumart = SOUNDMENU_ALBUM_ART(object);

   switch (prop_id) {
   case PROP_PATH:
      g_value_set_string(value, soundmenu_album_art_get_path(albumart));
      break;
   case PROP_SIZE:
      g_value_set_uint (value, soundmenu_album_art_get_size(albumart));
      break;
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
soundmenu_album_art_set_property (GObject *object,
                                  guint prop_id,
                                  const GValue *value,
                                  GParamSpec *pspec)
{
   SoundmenuAlbumArt *albumart = SOUNDMENU_ALBUM_ART(object);

   switch (prop_id) {
   case PROP_PATH:
      soundmenu_album_art_set_path(albumart, g_value_get_string(value));
      break;
   case PROP_SIZE:
      soundmenu_album_art_set_size(albumart, g_value_get_uint(value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
soundmenu_album_art_class_init (SoundmenuAlbumArtClass *klass)
{
   GObjectClass *object_class;

   object_class = G_OBJECT_CLASS(klass);
   object_class->finalize = soundmenu_album_art_finalize;
   object_class->get_property = soundmenu_album_art_get_property;
   object_class->set_property = soundmenu_album_art_set_property;
   g_type_class_add_private(object_class, sizeof(SoundmenuAlbumArtPrivate));

   /**
    * SoundmenuAlbumArt:path:
    *
    */
   gParamSpecs[PROP_PATH] =
      g_param_spec_string("path",
                          "Path",
                          "The album art path",
                          BASEICONDIR"/128x128/apps/xfce4-soundmenu-plugin.png",
                          G_PARAM_READWRITE |
                          G_PARAM_CONSTRUCT |
                          G_PARAM_STATIC_STRINGS);

   /**
    * SoundmenuAlbumArt:size:
    *
    */
   gParamSpecs[PROP_SIZE] =
      g_param_spec_uint("size",
                        "Size",
                        "The album art size",
                        36, 128,
                        48,
                        G_PARAM_READWRITE |
                        G_PARAM_CONSTRUCT |
                        G_PARAM_STATIC_STRINGS);

   g_object_class_install_properties(object_class, LAST_PROP, gParamSpecs);
}

static void
soundmenu_album_art_init (SoundmenuAlbumArt *albumart)
{
   albumart->priv = G_TYPE_INSTANCE_GET_PRIVATE(albumart,
                                               SOUNDMENU_TYPE_ALBUM_ART,
                                               SoundmenuAlbumArtPrivate);
}
