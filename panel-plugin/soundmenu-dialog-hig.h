/*
 *  Copyright (c) 2012-2013 matias <mati86dl@gmail.com>
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

#ifndef SOUNDMENU_HIG_H
#define SOUNDMENU_HIG_H

#include <gtk/gtk.h>

void
gtk_label_set_attribute_bold(GtkLabel *label);

GtkWidget *
soundmenu_hig_workarea_table_add_section_title(GtkWidget *table, guint *row, const char *section_title);

void
soundmenu_hig_workarea_table_add_wide_control(GtkWidget *table, guint *row, GtkWidget *widget);

void
soundmenu_hig_workarea_table_add_wide_tall_control(GtkWidget *table, guint *row, GtkWidget *widget);

void
soundmenu_hig_workarea_table_add_row(GtkWidget *table, guint *row, GtkWidget *label, GtkWidget *control);

GtkWidget *
soundmenu_hig_workarea_table_new();

void
soundmenu_hig_workarea_table_finish(GtkWidget *table, guint *row);

#endif
