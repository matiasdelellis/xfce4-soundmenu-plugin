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

#ifndef SIMPLE_ASYNC_H
#define SIMPLE_ASYNC_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
typedef struct _SoundmenuPlugin SoundmenuPlugin;
/*
 * #include "soundmenu-plugin.h" ???????????????????? Why no work!!!!??
 */

G_BEGIN_DECLS

typedef struct _AsycMessageData AsycMessageData;

#if GLIB_CHECK_VERSION (2, 32, 0)
#define SOUNDMENU_MUTEX(mtx) GMutex mtx
#define soundmenu_mutex_free(mtx) g_mutex_clear (&(mtx))
#define soundmenu_mutex_lock(mtx) g_mutex_lock (&(mtx))
#define soundmenu_mutex_unlock(mtx) g_mutex_unlock (&(mtx))
#define soundmenu_mutex_create(mtx) g_mutex_init (&(mtx))
#else
#define SOUNDMENU_MUTEX(mtx) GMutex *mtx
#define soundmenu_mutex_free(mtx) g_mutex_free (mtx)
#define soundmenu_mutex_lock(mtx) g_mutex_lock (mtx)
#define soundmenu_mutex_unlock(mtx) g_mutex_unlock (mtx)
#define soundmenu_mutex_create(mtx) (mtx) = g_mutex_new ()
#endif

gboolean
soundmenu_async_set_idle_message (gpointer user_data);
AsycMessageData *
soundmenu_async_finished_message_new(SoundmenuPlugin *soundmenu, const gchar *message);

void
soundmenu_async_launch (GThreadFunc worker_func, GSourceFunc finish_func, gpointer user_data);

G_END_DECLS

#endif /* SIMPLE_ASYNC_H */