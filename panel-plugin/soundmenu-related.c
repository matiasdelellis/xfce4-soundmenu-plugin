/*  $Id$
 *
 *  Copyright (c) 2011 John Doo <john@foo.org>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "soundmenu-plugin.h"

#define ISO_639_1 _("en")

#ifdef HAVE_LIBGLYR
typedef struct
{
	SoundmenuPlugin	*soundmenu;
	GlyrQuery	query;
	GlyrMemCache	*head;
}
glyr_struct;

/* Some generics functions to avoid dubplicate code. */

gboolean
show_generic_related_text_info_dialog (gpointer data)
{
	GtkWidget *dialog, *view, *scrolled;
	GtkTextBuffer *buffer;
	gchar *artist = NULL, *title = NULL, *provider = NULL;
	gchar *title_header = NULL, *subtitle_header = NULL;

	glyr_struct *glyr_info = data;

	artist = g_strdup(glyr_info->query.artist);
	title = g_strdup(glyr_info->query.title);
	provider = g_strdup(glyr_info->head->prov);

	if(glyr_info->head->type == GLYR_TYPE_LYRICS) {
		title_header = g_strdup_printf(_("%s by %s"), title, artist);
		subtitle_header = g_strdup_printf(_("Lyrics thanks to %s"), provider);
	}
	else {
		title_header =  g_strdup(artist);
		subtitle_header = g_strdup_printf(_("Artist information thanks to %s"), provider);
	}

	view = gtk_text_view_new ();
	gtk_text_view_set_editable (GTK_TEXT_VIEW (view), FALSE);
	gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (view), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW (view), GTK_WRAP_WORD);
	gtk_text_view_set_accepts_tab (GTK_TEXT_VIEW (view), FALSE);

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
	gtk_text_buffer_set_text (buffer, glyr_info->head->data, -1);

	scrolled = gtk_scrolled_window_new (NULL, NULL);

	gtk_container_add (GTK_CONTAINER (scrolled), view);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
					GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled), GTK_SHADOW_IN);

	gtk_container_set_border_width (GTK_CONTAINER (scrolled), 8);

	dialog = xfce_titled_dialog_new_with_buttons (title_header,
						      GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (glyr_info->soundmenu->plugin))),
						      GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR,
						      GTK_STOCK_CLOSE, GTK_RESPONSE_OK,
						      NULL);

	xfce_titled_dialog_set_subtitle (dialog, (const gchar *)subtitle_header);

	gtk_window_set_icon_name (GTK_WINDOW (dialog), "xfce4-soundmenu-plugin");
	gtk_window_set_default_size(GTK_WINDOW (dialog), 450, 350);

	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), scrolled, TRUE, TRUE, 0);

	gtk_widget_show_all(dialog);

	gtk_dialog_run(GTK_DIALOG(dialog));

	gtk_widget_destroy(dialog);

	glyr_free_list(glyr_info->head);
	glyr_query_destroy(&glyr_info->query);
	g_slice_free(glyr_struct, glyr_info);

	g_free(title_header);
	g_free(subtitle_header);
	g_free(artist);
	g_free(title);
	g_free(provider);

	return FALSE;
}

/* Thread to get info without frizze the panel. */

gpointer
get_related_info_idle_func (gpointer data)
{
	GlyrMemCache *head;
	GLYR_ERROR error;

	glyr_struct *glyr_info = data;

	set_watch_cursor_on_thread(glyr_info->soundmenu);

	head = glyr_get(&glyr_info->query, &error, NULL);

	remove_watch_cursor_on_thread(NULL, glyr_info->soundmenu);

	if(head != NULL) {
		glyr_info->head = head;
		gdk_threads_add_idle(show_generic_related_text_info_dialog, glyr_info);
	}
	else {
		g_warning("Error searching lyrics: %s", glyr_strerror(error));
		glyr_query_destroy(&glyr_info->query);
		g_slice_free(glyr_struct, glyr_info);
	}

	return NULL;
}

/* Functions that respond to menu actions, set the querry and call the thread. */

void soundmenu_search_lyric_dialog (GtkWidget *widget, SoundmenuPlugin *soundmenu)
{
	glyr_struct *glyr_info;
	glyr_info = g_slice_new0 (glyr_struct);

	gchar *artist = NULL, *title = NULL;

	if(soundmenu->state == ST_STOPPED)
		return;

	if ((strlen(soundmenu->metadata->artist) == 0) ||
	    (strlen(soundmenu->metadata->title) == 0))
		return;

	artist = g_strdup(soundmenu->metadata->artist);
	title = g_strdup(soundmenu->metadata->title);

	glyr_query_init(&glyr_info->query);
	glyr_opt_type(&glyr_info->query, GLYR_GET_LYRICS);

	glyr_opt_artist(&glyr_info->query, artist);
	glyr_opt_title(&glyr_info->query, title);

	glyr_info->soundmenu = soundmenu;

	g_thread_create(get_related_info_idle_func, glyr_info, FALSE, NULL);

	g_free(artist);
	g_free(title);
}

void soundmenu_search_artistinfo_dialog (GtkWidget *widget, SoundmenuPlugin *soundmenu)
{
	glyr_struct *glyr_info;
	glyr_info = g_slice_new0 (glyr_struct);

	gchar *artist = NULL;

	if(soundmenu->state == ST_STOPPED)
		return;

	if (strlen(soundmenu->metadata->artist) == 0)
		return;

	artist = g_strdup(soundmenu->metadata->artist);

	glyr_query_init(&glyr_info->query);

	glyr_opt_type(&glyr_info->query, GLYR_GET_ARTISTBIO);

	glyr_opt_artist(&glyr_info->query, artist);

	glyr_opt_lang (&glyr_info->query, ISO_639_1);
	glyr_opt_lang_aware_only (&glyr_info->query, TRUE);

	glyr_info->soundmenu = soundmenu;

	g_thread_create(get_related_info_idle_func, glyr_info, FALSE, NULL);

	g_free(artist);
}

/* Function to un/init libglyr */

int uninit_glyr_related (SoundmenuPlugin *soundmenu)
{
	glyr_cleanup ();

	return 0;
}

int init_glyr_related (SoundmenuPlugin *soundmenu)
{
	glyr_init();

	return 0;
}
#endif
