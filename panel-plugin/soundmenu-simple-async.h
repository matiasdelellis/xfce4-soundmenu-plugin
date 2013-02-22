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

#include "soundmenu-plugin.h"

typedef struct {
	const gchar *message;
	SoundmenuPlugin *soundmenu;
} AsycMessageData;

typedef struct {
	gpointer userdata;
	gpointer finished_data;
	GThreadFunc func_w;
	GSourceFunc func_f;
} AsyncSimple;

gboolean
soundmenu_async_set_idle_message (gpointer user_data);
AsycMessageData *
soundmenu_async_finished_message_new(SoundmenuPlugin *soundmenu, const gchar *message);

gboolean
soundmenu_async_finished(gpointer data);

gpointer
soundmenu_async_worker(gpointer data);

void
soundmenu_async_launch (GThreadFunc worker_func, GSourceFunc finish_func, gpointer user_data);

GThread *
soundmenu_async_launch_full (GThreadFunc worker_func, GSourceFunc finish_func, gpointer userdata);

