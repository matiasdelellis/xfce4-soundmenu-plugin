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
void soundmenu_search_lyric_dialog (GtkWidget *widget, SoundmenuPlugin *soundmenu)
{
	GtkWidget *dialog;
	GtkWidget *header, *view, *frame, *scrolled;
	GtkTextBuffer *buffer;
	gchar *subtitle_header = NULL, *title_header = NULL;
	GlyrQuery q;
	GLYR_ERROR err;

	if(soundmenu->state == ST_STOPPED)
		return;

	if ((strlen(soundmenu->metadata->artist) == 0) ||
	    (strlen(soundmenu->metadata->title) == 0))
		return;

	glyr_query_init(&q);
	glyr_opt_type(&q, GLYR_GET_LYRICS);

	glyr_opt_artist(&q, soundmenu->metadata->artist);
	glyr_opt_title(&q, soundmenu->metadata->title);

	GlyrMemCache *head = glyr_get(&q, &err, NULL);

	if(head == NULL) {
		g_warning("Error searching lyrics: %s", glyr_strerror(err));
		goto bad;
	}

	view = gtk_text_view_new ();
	gtk_text_view_set_editable (GTK_TEXT_VIEW (view), FALSE);
	gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (view), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW (view), GTK_WRAP_WORD);

	frame = gtk_frame_new (NULL);
	scrolled = gtk_scrolled_window_new (NULL, NULL);

	gtk_container_add (GTK_CONTAINER (scrolled), view);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
					GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	gtk_container_set_border_width (GTK_CONTAINER (frame), 8);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
	gtk_container_add (GTK_CONTAINER (frame), scrolled);

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

	gtk_text_buffer_set_text (buffer, head->data, -1);

	title_header = g_strdup_printf(_("%s by %s"), soundmenu->metadata->title, soundmenu->metadata->artist);

	dialog = xfce_titled_dialog_new_with_buttons (title_header,
						      GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (soundmenu->plugin))),
						      GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR,
						      GTK_STOCK_CLOSE, GTK_RESPONSE_OK,
						      NULL);

	gtk_window_set_default_size(GTK_WINDOW (dialog), 450, 350);

	gtk_window_set_icon_name (GTK_WINDOW (dialog), "xfce4-soundmenu-plugin");

	subtitle_header = g_strdup_printf(_("Lyrics thanks to %s"), head->prov);
	xfce_titled_dialog_set_subtitle (dialog, (const gchar *)subtitle_header);

	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), frame, TRUE, TRUE, 0);

	gtk_widget_show_all(dialog);

	gdk_window_set_cursor(GDK_WINDOW(soundmenu->hvbox->window), NULL);

	gtk_dialog_run(GTK_DIALOG(dialog));

	gtk_widget_destroy(dialog);

	glyr_free_list(head);
	g_free(subtitle_header);
	g_free(title_header);

bad:
	glyr_query_destroy(&q);

	return;
}

void soundmenu_search_artistinfo_dialog (GtkWidget *widget, SoundmenuPlugin *soundmenu)
{
	GtkWidget *dialog;
	GtkWidget *header, *view, *frame, *scrolled;
	GtkTextBuffer *buffer;
	gchar *subtitle_header = NULL;
	GlyrQuery q;
	GLYR_ERROR err;

	if(soundmenu->state == ST_STOPPED)
		return;
	
	glyr_query_init(&q);
	glyr_opt_type(&q, GLYR_GET_ARTISTBIO);

	glyr_opt_artist(&q, soundmenu->metadata->artist);

	glyr_opt_lang (&q, ISO_639_1);
	glyr_opt_lang_aware_only (&q, TRUE);

	GlyrMemCache *head = glyr_get(&q, &err, NULL);

	if(head == NULL) {
		g_warning("Error searching artist info: %s", glyr_strerror(err));
		goto bad;
	}

	view = gtk_text_view_new ();
	gtk_text_view_set_editable (GTK_TEXT_VIEW (view), FALSE);
	gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (view), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW (view), GTK_WRAP_WORD);
	gtk_text_view_set_accepts_tab (GTK_TEXT_VIEW (view), FALSE);

	frame = gtk_frame_new (NULL);
	scrolled = gtk_scrolled_window_new (NULL, NULL);

	gtk_container_add (GTK_CONTAINER (scrolled), view);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
					GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	gtk_container_set_border_width (GTK_CONTAINER (frame), 8);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
	gtk_container_add (GTK_CONTAINER (frame), scrolled);

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

	gtk_text_buffer_set_text (buffer, head->data, -1);

	dialog = xfce_titled_dialog_new_with_buttons (soundmenu->metadata->artist,
						      GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (soundmenu->plugin))),
						      GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR,
						      GTK_STOCK_CLOSE, GTK_RESPONSE_OK,
						      NULL);

	gtk_window_set_default_size(GTK_WINDOW (dialog), 450, 350);

	gtk_window_set_icon_name (GTK_WINDOW (dialog), "xfce4-soundmenu-plugin");

	subtitle_header = g_strdup_printf(_("Artist information thanks to %s"), head->prov);
	xfce_titled_dialog_set_subtitle (dialog, (const gchar *)subtitle_header);

	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), frame, TRUE, TRUE, 0);

	gtk_widget_show_all(dialog);

	gtk_dialog_run(GTK_DIALOG(dialog));

	gtk_widget_destroy(dialog);

	glyr_free_list(head);

bad:
	glyr_query_destroy(&q);

	return;
}

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
