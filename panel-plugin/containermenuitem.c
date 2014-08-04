/* -*- c-basic-offset: 2 -*- vi:set ts=2 sts=2 sw=2:
 * * Copyright (C) 2014 Eric Koegel <eric@xfce.org>
 *
 * Licensed under the GNU General Public License Version 2
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
/*
 * Based on the scale menu item implementation of the indicator applet:
 * Authors:
 *    Cody Russell <crussell@canonical.com>
 * http://bazaar.launchpad.net/~indicator-applet-developers/ido/trunk.14.10/view/head:/src/idoscalemenuitem.h
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "containermenuitem.h"

#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>

/* for DBG/TRACE */
#include <libxfce4util/libxfce4util.h>

struct _ContainerMenuItemPrivate {
  GtkWidget            *widget;
  GtkWidget            *alling;
};

G_DEFINE_TYPE (ContainerMenuItem, container_menu_item, GTK_TYPE_IMAGE_MENU_ITEM)


#define GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), TYPE_CONTAINER_MENU_ITEM, ContainerMenuItemPrivate))

void
container_menu_item_add (ContainerMenuItem *container, GtkWidget *widget)
{
  ContainerMenuItemPrivate *priv = GET_PRIVATE (container);
 
  gtk_container_add(GTK_CONTAINER(priv->alling), widget);

  priv->widget = g_object_ref (widget);
}


static gboolean
container_menu_item_release_event (GtkWidget *menuitem,
                                   GdkEventButton *event)
{
  TRACE("entering");

  return TRUE; // No close menu..
}


static void
container_menu_item_class_init (ContainerMenuItemClass *item_class)
{
  GtkWidgetClass    *widget_class =    GTK_WIDGET_CLASS    (item_class);

  widget_class->button_release_event = container_menu_item_release_event;

  g_type_class_add_private (item_class, sizeof (ContainerMenuItemPrivate));
}


static void
container_menu_item_init (ContainerMenuItem *self)
{
}


GtkWidget*
container_menu_item_new (void)
{
  ContainerMenuItem *container_item;
  ContainerMenuItemPrivate *priv;

  TRACE("entering");

  container_item = CONTAINER_MENU_ITEM (g_object_new (TYPE_CONTAINER_MENU_ITEM, NULL));

  priv = GET_PRIVATE (container_item);

  priv->alling = gtk_alignment_new (0.5, 0.5, 1, 1);
  priv->widget = NULL;

  gtk_widget_set_size_request (priv->alling, 100, 32);
  gtk_container_add(GTK_CONTAINER(container_item), priv->alling);

  return GTK_WIDGET(container_item);
}
