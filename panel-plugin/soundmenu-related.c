/*
 *  Copyright (c) 2011-2012 matias <mati86dl@gmail.com>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "soundmenu-plugin.h"
#include "soundmenu-dbus.h"
#include "soundmenu-dialogs.h"
#include "soundmenu-lastfm.h"
#include "soundmenu-mpris2.h"
#include "soundmenu-utils.h"
#include "soundmenu-related.h"

#ifdef HAVE_LIBGLYR
typedef struct
{
	SoundmenuPlugin *soundmenu;
	GlyrQuery        query;
	GlyrMemCache     *head;
}
glyr_struct;

//FIXME_GLYR_CAST: drop discarding const when we switch to enough new glyr, see https://github.com/sahib/glyr/issues/29

/* Use the download info on glyr thread and show a dialog. */
   
static void
soundmenu_text_info_dialog_response(GtkDialog *dialog,
                                    gint response,
                                    SoundmenuPlugin *soundmenu)
{
    gtk_widget_destroy(GTK_WIDGET(dialog));
}

static void
soundmenu_show_related_text_info_dialog(glyr_struct *glyr_info,
                                        gchar *title_header,
                                        gchar *subtitle_header)
{
    GtkWidget *dialog, *header, *view, *scrolled;
    GtkTextBuffer *buffer;

    SoundmenuPlugin *soundmenu = glyr_info->soundmenu;

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
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled),
                                        GTK_SHADOW_IN);

    gtk_container_set_border_width (GTK_CONTAINER (scrolled), 8);

    dialog = xfce_titled_dialog_new_with_buttons (title_header,
                                                  GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(soundmenu->plugin))),
                                                  GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR,
                                                  GTK_STOCK_CLOSE,
                                                  GTK_RESPONSE_OK,
                                                  NULL);

    xfce_titled_dialog_set_subtitle (XFCE_TITLED_DIALOG(dialog), (const gchar *)subtitle_header);

    gtk_window_set_icon_name (GTK_WINDOW (dialog), "xfce4-soundmenu-plugin");
    gtk_window_set_default_size(GTK_WINDOW (dialog), 450, 350);

    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), scrolled, TRUE, TRUE, 0);

    g_signal_connect(G_OBJECT(dialog), "response",
                     G_CALLBACK(soundmenu_text_info_dialog_response),
                     soundmenu);

    gtk_widget_show_all(dialog);
}

/* Manages the results of glyr threads. */

static gboolean
glyr_finished_thread_update (gpointer data)
{
    glyr_struct *glyr_info = data;
    gchar *title_header = NULL, *subtitle_header = NULL;

    switch (glyr_info->head->type) {
    case GLYR_TYPE_LYRICS:
        title_header = g_strdup_printf(_("%s by %s"), glyr_info->query.title, glyr_info->query.artist);
        subtitle_header = g_strdup_printf(_("Lyrics thanks to %s"), glyr_info->head->prov);
        soundmenu_show_related_text_info_dialog(glyr_info, title_header, subtitle_header);
        break;
#if GLYR_CHECK_VERSION (1, 0, 0)
    case GLYR_TYPE_ARTIST_BIO:
#else
    case GLYR_TYPE_ARTISTBIO:
#endif
        title_header = g_strdup(glyr_info->query.artist);
        subtitle_header = g_strdup_printf(_("Artist information thanks to %s"), glyr_info->head->prov);
        soundmenu_show_related_text_info_dialog(glyr_info, title_header, subtitle_header);
        break;
    default:
        break;
    }

    glyr_free_list(glyr_info->head);
    glyr_query_destroy(&glyr_info->query);
    g_slice_free(glyr_struct, glyr_info);

    g_free(title_header);
    g_free(subtitle_header);

    return FALSE;
}

/* Get artist bio or lyric on a thread. */

static gpointer
get_related_text_info_idle_func (gpointer data)
{
    GlyrMemCache *head;
    GLYR_ERROR error;

    glyr_struct *glyr_info = data;

    set_watch_cursor_on_thread(glyr_info->soundmenu);

    head = glyr_get(&glyr_info->query, &error, NULL);

    if(head != NULL) {
        glyr_info->head = head;
        g_idle_add(glyr_finished_thread_update, glyr_info);

        remove_watch_cursor_on_thread(NULL, glyr_info->soundmenu);
    }
    else {
        remove_watch_cursor_on_thread((glyr_info->query.type == GLYR_GET_LYRICS) ?
                                       _("Lyrics not found.") :
                                       _("Artist information not found."),
                                       glyr_info->soundmenu);

        g_warning("Error searching song info: %s", glyr_strerror(error));

        glyr_query_destroy(&glyr_info->query);
        g_slice_free(glyr_struct, glyr_info);
    }

    return NULL;
}

/* Configure the thread to get the artist bio or lyric. */

static void
configure_and_launch_get_text_info_dialog(GLYR_GET_TYPE type,
                                          const gchar *artist,
                                          const gchar *title,
                                          SoundmenuPlugin *soundmenu)
{
    glyr_struct *glyr_info;
    glyr_info = g_slice_new0 (glyr_struct);

    glyr_query_init(&glyr_info->query);
    glyr_opt_type(&glyr_info->query, type);

    switch (type) {
#if GLYR_CHECK_VERSION (1, 0, 0)
    case GLYR_GET_ARTIST_BIO:
#else
    case GLYR_GET_ARTISTBIO:
#endif
        glyr_opt_artist(&glyr_info->query, (char*)artist); //FIXME_GLYR_CAST

        glyr_opt_lang (&glyr_info->query, "auto");
        glyr_opt_lang_aware_only (&glyr_info->query, TRUE);
        break;
    case GLYR_GET_LYRICS:
        glyr_opt_artist(&glyr_info->query, (char*)artist); //FIXME_GLYR_CAST
        glyr_opt_title(&glyr_info->query, (char*)title); //FIXME_GLYR_CAST
        break;
    default:
        break;
    }

    glyr_info->soundmenu = soundmenu;

    #if GLIB_CHECK_VERSION(2,31,0)
    g_thread_new("Glyr get text", get_related_text_info_idle_func, glyr_info);
    #else
    g_thread_create(get_related_text_info_idle_func, glyr_info, FALSE, NULL);
    #endif
}

/* Functions that respond to menu actions, set the querry and call the thread. */

void soundmenu_search_lyric_dialog (GtkWidget *widget, SoundmenuPlugin *soundmenu)
{
    const gchar *artist = NULL, *title = NULL;

    if(soundmenu->state == ST_STOPPED)
        return;

    if ((soundmenu->metadata->artist == NULL) || (soundmenu->metadata->title == NULL) ||
        (strlen(soundmenu->metadata->artist) == 0) || (strlen(soundmenu->metadata->title) == 0))
        return;

    artist = soundmenu->metadata->artist;
    title = soundmenu->metadata->title;

    configure_and_launch_get_text_info_dialog(GLYR_GET_LYRICS, artist, title, soundmenu);
}

void soundmenu_search_artistinfo_dialog (GtkWidget *widget, SoundmenuPlugin *soundmenu)
{
    const gchar *artist = NULL;

    if(soundmenu->state == ST_STOPPED)
        return;

    if ((soundmenu->metadata->artist == NULL) ||
        (strlen(soundmenu->metadata->artist) == 0))
        return;

    artist = soundmenu->metadata->artist;

    configure_and_launch_get_text_info_dialog(GLYR_GET_ARTISTBIO, artist, NULL, soundmenu);
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
