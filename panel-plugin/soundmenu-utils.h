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

#define g_str_empty0(s) (!(s) || !(s)[0])
#define g_str_nempty0(s) ((s) && (s)[0])

gchar *
g_avariant_dup_string(GVariant * variant);

#if !GLIB_CHECK_VERSION(2,32,0)
#define NM_DBUS_SERVICE		"org.freedesktop.NetworkManager"
#define NM_DBUS_PATH		"/org/freedesktop/NetworkManager"
#define NM_DBUS_INTERFACE	"org.freedesktop.NetworkManager"

typedef enum {
        NM_STATE_UNKNOWN          = 0,
        NM_STATE_ASLEEP           = 10,
        NM_STATE_DISCONNECTED     = 20,
        NM_STATE_DISCONNECTING    = 30,
        NM_STATE_CONNECTING       = 40,
        NM_STATE_CONNECTED_LOCAL  = 50,
        NM_STATE_CONNECTED_SITE   = 60,
        NM_STATE_CONNECTED_GLOBAL = 70
} NMState;

static NMState dbus_check_nm_status (DBusConnection *connection);
gboolean nm_is_online ();
#endif

void set_watch_cursor_on_thread(SoundmenuPlugin *soundmenu);
void remove_watch_cursor_on_thread(gchar *message, SoundmenuPlugin *soundmenu);

gboolean
soundmenu_album_art_frame_press_callback (GtkWidget       *event_box,
                                          GdkEventButton  *event,
                                          SoundmenuPlugin *soundmenu);

gchar* convert_length_str(gint length);