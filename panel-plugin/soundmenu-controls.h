/*
 *  Copyright (c) 2011-2014 matias <mati86dl@gmail.com>
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

#ifndef __SOUNDMENU_CONTROLS_H__
#define __SOUNDMENU_CONTROLS_H__

#include <gtk/gtk.h>
#include <libmpris2client/libmpris2client.h>

GType mpris2_controls_get_type (void);

#define TYPE_MPRIS2_CONTROLS             (mpris2_controls_get_type())
#define MPRIS2_CONTROLS(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_MPRIS2_CONTROLS, Mpris2Controls))
#define MPRIS2_CONTROLS_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass),  TYPE_MPRIS2_CONTROLS, Mpris2ControlsClass))
#define IS_MPRIS2_CONTROLS(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_MPRIS2_CONTROLS))
#define IS_MPRIS2_CONTROLS_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass),  TYPE_MPRIS2_CONTROLS))
#define MPRIS2_CONTROLS_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj),  TYPE_MPRIS2_CONTROLS, Mpris2ControlsClass))

typedef struct _Mpris2Controls Mpris2Controls;
typedef struct _Mpris2ControlsClass Mpris2ControlsClass;


void mpris2_controls_set_show_stop_button (Mpris2Controls *controls, gboolean show);
void mpris2_controls_set_orientation (Mpris2Controls *controls, GtkOrientation orientation);
void mpris2_controls_set_size (Mpris2Controls *controls, gint size);


Mpris2Controls *mpris2_controls_new (Mpris2Client *mpris2);

#endif