/*
 *  Copyright (c) 2013 matias <mati86dl@gmail.com>
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

#ifndef LIB_MPRIS2_CLIENT_H
#define LIB_MPRIS2_CLIENT_H

#include <glib-object.h>
#include "mpris2-metadata.h"

typedef enum {
	PLAYING = 1,
	PAUSED,
	STOPPED
} PlaybackStatus;

typedef enum {
	NONE = 1,
	TRACK,
	PLAYLIST
} LoopStatus;

#define MPRIS2_TYPE_CLIENT              (mpris2_client_get_type ())
#define MPRIS2_CLIENT(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), MPRIS2_TYPE_CLIENT, Mpris2Client))
#define MPRIS2_IS_CLIENT(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MPRIS2_TYPE_CLIENT))
#define MPRIS2_CLIENT_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MPRIS2_TYPE_CLIENT, Mpris2ClientClass))
#define MPRIS2_IS_CLIENT_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MPRIS2_TYPE_CLIENT))
#define MPRIS2_CLIENT_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MPRIS2_TYPE_CLIENT, Mpris2ClientClass))

typedef struct _Mpris2Client Mpris2Client;
typedef struct _Mpris2ClientClass Mpris2ClientClass;

struct _Mpris2ClientClass {
	GObjectClass parent_class;
	void (*connection)      (Mpris2Client *mpris2, gboolean        connected);
	void (*playback_status) (Mpris2Client *mpris2, PlaybackStatus  playback_status);
	void (*metadata)        (Mpris2Client *mpris2, Mpris2Metadata *metadata);
	void (*volume)          (Mpris2Client *mpris2, gdouble         volume);
	void (*loop_status)     (Mpris2Client *mpris2, LoopStatus      loop_status);
	void (*shuffle)         (Mpris2Client *mpris2, gboolean        shuffle);
};

/*
 * Methods
 */

void            mpris2_client_open_uri                  (Mpris2Client *mpris2, const gchar *uri);
void            mpris2_client_play_pause                (Mpris2Client *mpris2);
void            mpris2_client_stop                      (Mpris2Client *mpris2);
void            mpris2_client_prev                      (Mpris2Client *mpris2);
void            mpris2_client_next                      (Mpris2Client *mpris2);

/*
 * Interface MediaPlayer2 Methods.
 */
void            mpris2_client_quit_player               (Mpris2Client *mpris2);
void            mpris2_client_set_fullscreen_player     (Mpris2Client *mpris2, gboolean fullscreen);
void            mpris2_client_raise_player              (Mpris2Client *mpris2);

/*
 * Interface MediaPlayer2 Properies.
 */
gboolean        mpris2_client_can_quit                  (Mpris2Client *mpris2);
gboolean        mpris2_client_can_set_fullscreen        (Mpris2Client *mpris2);
gboolean        mpris2_client_can_raise                 (Mpris2Client *mpris2);
gboolean        mpris2_client_has_tracklist_support     (Mpris2Client *mpris2);
const gchar    *mpris2_client_get_player_identity       (Mpris2Client *mpris2);
const gchar    *mpris2_client_get_player_desktop_entry  (Mpris2Client *mpris2);
gchar         **mpris2_client_get_supported_uri_schemes (Mpris2Client *mpris2);
gchar         **mpris2_client_get_supported_mime_types  (Mpris2Client *mpris2);

/*
 * Interface MediaPlayer2.Player properties.
 */
PlaybackStatus  mpris2_client_get_playback_status       (Mpris2Client *mpris2);

Mpris2Metadata *mpris2_client_get_metadata              (Mpris2Client *mpris2);

gdouble         mpris2_client_get_volume                (Mpris2Client *mpris2);
void            mpris2_client_set_volume                (Mpris2Client *mpris2, gdouble volume);

gboolean        mpris2_client_player_has_loop_status    (Mpris2Client *mpris2);
LoopStatus      mpris2_client_get_loop_status           (Mpris2Client *mpris2);
void            mpris2_client_set_loop_status           (Mpris2Client *mpris2, LoopStatus loop_status);

gboolean        mpris2_client_player_has_shuffle        (Mpris2Client *mpris2);
gboolean        mpris2_client_get_shuffle               (Mpris2Client *mpris2);
void            mpris2_client_set_shuffle               (Mpris2Client *mpris2, gboolean shuffle);

/*
 * Library.
 */

gboolean        mpris2_client_is_connected              (Mpris2Client *mpris2);

const gchar    *mpris2_client_get_player                (Mpris2Client *mpris2);
void            mpris2_client_set_player                (Mpris2Client *mpris2, const gchar *player);
gchar          *mpris2_client_auto_set_player           (Mpris2Client *mpris2);

GType           mpris2_client_get_type (void) G_GNUC_CONST;

Mpris2Client   *mpris2_client_new (void);

#endif