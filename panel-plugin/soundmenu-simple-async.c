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

#include "soundmenu-simple-async.h"
#include "soundmenu-plugin.h"
#include "soundmenu-utils.h"

/* Generic function to set a message when finished the async operation.
 * You need set 'soundmenu_async_set_idle_message' as finish_func
 * and then return a 'const gchar *' on worker_func. */

gboolean
soundmenu_async_set_idle_message (gpointer user_data)
{
	AsycMessageData *data = user_data;

	remove_watch_cursor (GTK_WIDGET(data->soundmenu->plugin));

	#ifdef HAVE_LIBNOTIFY
	if (data->message != NULL)
		soundmenu_notify_message(data->message);
	#endif

	return FALSE;
}

AsycMessageData *
soundmenu_async_finished_message_new(SoundmenuPlugin *soundmenu, const gchar *message)
{
	AsycMessageData *data;

	data = g_slice_new (AsycMessageData);

	data->message = message;
	data->soundmenu = soundmenu;

	return data;
}

/* Launch a asynchronous operation (worker_func), and when finished use another
 * function (finish_func) in the main loop using the information returned by
 * the asynchronous operation. */

gboolean
soundmenu_async_finished(gpointer data)
{
	AsyncSimple *as = data;

	as->func_f(as->finished_data);
	g_slice_free(AsyncSimple, as);

	return FALSE;
}

gpointer
soundmenu_async_worker(gpointer data)
{
	AsyncSimple *as = data;

	as->finished_data = as->func_w(as->userdata);

	g_idle_add_full(G_PRIORITY_HIGH_IDLE, soundmenu_async_finished, as, NULL);

	return NULL;
}

void
soundmenu_async_launch (GThreadFunc worker_func, GSourceFunc finish_func, gpointer user_data)
{
	AsyncSimple *as;

	as = g_slice_new0(AsyncSimple);
	as->func_w = worker_func;
	as->func_f = finish_func;
	as->userdata = user_data;
	as->finished_data = NULL;

	#if GLIB_CHECK_VERSION(2,31,0)
	g_thread_new("Launch async", soundmenu_async_worker, as);
	#else
	g_thread_create(soundmenu_async_worker, as, FALSE, NULL);
	#endif
}
