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

#ifndef LIB_MPRIS2_CONTROL_H
#define LIB_MPRIS2_CONTROL_H

#include <glib-object.h>
#include "mpris2-metadata.h"

#define MPRIS2_TYPE_CONTROL              (mpris2_control_get_type ())
#define MPRIS2_CONTROL(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), MPRIS2_TYPE_CONTROL, Mpris2Control))
#define MPRIS2_IS_CONTROL(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MPRIS2_TYPE_CONTROL))
#define MPRIS2_CONTROL_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MPRIS2_TYPE_CONTROL, Mpris2ControlClass))
#define MPRIS2_IS_CONTROL_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MPRIS2_TYPE_CONTROL))
#define MPRIS2_CONTROL_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MPRIS2_TYPE_CONTROL, Mpris2ControlClass))

typedef struct _Mpris2Control Mpris2Control;
typedef struct _Mpris2ControlClass Mpris2ControlClass;

struct _Mpris2ControlClass {
	GObjectClass parent_class;
	void (*connection)      (Mpris2Control *mpris2);
	void (*playback_status) (Mpris2Control *mpris2);
	void (*metadata)        (Mpris2Control *mpris2, Mpris2Metadata *metadata);
	void (*volume)          (Mpris2Control *mpris2);
};

typedef enum {
	PLAYING = 1,
	PAUSED,
	STOPPED
} PlaybackStatus;

/*
 * Methods
 */
void           mpris2_control_play_pause          (Mpris2Control *mpris2);
void           mpris2_control_stop                (Mpris2Control *mpris2);
void           mpris2_control_prev                (Mpris2Control *mpris2);
void           mpris2_control_next                (Mpris2Control *mpris2);

/*
 * Interface MediaPlayer2 Methods.
 */
void           mpris2_control_quit_player               (Mpris2Control *mpris2);
void           mpris2_control_raise_player              (Mpris2Control *mpris2);

/*
 * Interface MediaPlayer2 Properies.
 */
gboolean       mpris2_control_can_quit                  (Mpris2Control *mpris2);
gboolean       mpris2_control_can_raise                 (Mpris2Control *mpris2);
gboolean       mpris2_control_has_tracklist_support     (Mpris2Control *mpris2);
const gchar   *mpris2_control_get_player_identity       (Mpris2Control *mpris2);
gchar        **mpris2_control_get_supported_uri_schemes (Mpris2Control *mpris2);
gchar        **mpris2_control_get_supported_mime_types  (Mpris2Control *mpris2);

/*
 * Interface MediaPlayer2.Player properties.
 */
PlaybackStatus  mpris2_control_get_playback_status (Mpris2Control *mpris2);

Mpris2Metadata *mpris2_control_get_metadata        (Mpris2Control *mpris2);

gdouble         mpris2_control_get_volume          (Mpris2Control *mpris2);
void            mpris2_control_set_volume          (Mpris2Control *mpris2, gdouble volume);

/*
 * Library.
 */

gboolean       mpris2_control_is_connected        (Mpris2Control *mpris2);
void           mpris2_control_set_player          (Mpris2Control *mpris2, const gchar *player);
gchar         *mpris2_control_auto_set_player     (Mpris2Control *mpris2);

GType          mpris2_control_get_type (void) G_GNUC_CONST;
Mpris2Control *mpris2_control_new (void);

#endif