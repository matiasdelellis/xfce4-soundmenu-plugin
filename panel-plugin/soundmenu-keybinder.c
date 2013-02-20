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

#include <keybinder.h>

#include "soundmenu-plugin.h"
#include "soundmenu-mpris2.h"

static void
keybind_play_handler (const char *keystring, SoundmenuPlugin *soundmenu)
{
	mpris2_send_message (soundmenu, "PlayPause");
}
static void
keybind_stop_handler (const char *keystring, SoundmenuPlugin *soundmenu)
{
	mpris2_send_message (soundmenu, "Stop");
}

static void
keybind_prev_handler (const char *keystring, SoundmenuPlugin *soundmenu)
{
	mpris2_send_message (soundmenu, "Previous");
}

static void
keybind_next_handler (const char *keystring, SoundmenuPlugin *soundmenu)
{
	mpris2_send_message (soundmenu, "Next");
}

void
keybinder_bind_keys(SoundmenuPlugin *soundmenu)
{
	keybinder_bind("XF86AudioPlay", (KeybinderHandler) keybind_play_handler, soundmenu);
	keybinder_bind("XF86AudioStop", (KeybinderHandler) keybind_stop_handler, soundmenu);
	keybinder_bind("XF86AudioPrev", (KeybinderHandler) keybind_prev_handler, soundmenu);
	keybinder_bind("XF86AudioNext", (KeybinderHandler) keybind_next_handler, soundmenu);
}

void
keybinder_unbind_keys(SoundmenuPlugin *soundmenu)
{
	keybinder_unbind("XF86AudioPlay", (KeybinderHandler) keybind_play_handler);
	keybinder_unbind("XF86AudioStop", (KeybinderHandler) keybind_stop_handler);
	keybinder_unbind("XF86AudioPrev", (KeybinderHandler) keybind_prev_handler);
	keybinder_unbind("XF86AudioNext", (KeybinderHandler) keybind_next_handler);
}

void
soundmenu_init_keybinder(void)
{
	keybinder_init ();
}