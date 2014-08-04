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


#ifndef _CONTAINER_MENU_ITEM_H_
#define _CONTAINER_MENU_ITEM_H_

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define TYPE_CONTAINER_MENU_ITEM         (container_menu_item_get_type ())
#define CONTAINER_MENU_ITEM(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), TYPE_CONTAINER_MENU_ITEM, ContainerMenuItem))
#define CONTAINER_MENU_ITEM_CLASS(c)     (G_TYPE_CHECK_CLASS_CAST ((c), TYPE_CONTAINER_MENU_ITEM, ContainerMenuItemClass))
#define IS_CONTAINER_MENU_ITEM(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), TYPE_CONTAINER_MENU_ITEM))
#define IS_CONTAINER_MENU_ITEM_CLASS(c)  (G_TYPE_CHECK_CLASS_TYPE ((c), TYPE_CONTAINER_MENU_ITEM))
#define CONTAINER_MENU_ITEM_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), TYPE_CONTAINER_MENU_ITEM, ContainerMenuItemClass))


typedef struct _ContainerMenuItem        ContainerMenuItem;
typedef struct _ContainerMenuItemClass   ContainerMenuItemClass;
typedef struct _ContainerMenuItemPrivate ContainerMenuItemPrivate;

struct _ContainerMenuItem
{
  GtkMenuItem parent_instance;

  ContainerMenuItemPrivate *priv;
};

struct _ContainerMenuItemClass
{
  GtkMenuItemClass parent_class;
};

void container_menu_item_add (ContainerMenuItem *container, GtkWidget *widget);

GType        container_menu_item_get_type              (void) G_GNUC_CONST;

GtkWidget   *container_menu_item_new                   (void);

G_END_DECLS

#endif /* _CONTAINER_MENU_ITEM_H_ */
