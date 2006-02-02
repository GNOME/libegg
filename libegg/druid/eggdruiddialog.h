/* eggdruiddialog.h
 * Copyright (C) 1999  Red Hat, Inc.
 * Copyright (C) 2002  Anders Carlsson <andersca@gnu.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __EGG_DRUID_DIALOG_H__
#define __EGG_DRUID_DIALOG_H__

#include <gtk/gtkdialog.h>

G_BEGIN_DECLS

#define EGG_TYPE_DRUID_DIALOG             (egg_druid_dialog_get_type ())
#define EGG_DRUID_DIALOG(obj)	          (G_TYPE_CHECK_INSTANCE_CLASS ((obj), EGG_TYPE_DRUID_DIALOG, EggDruidDialog))
#define EGG_DRUID_DIALOG_CLASS(klass)	  (G_TYPE_CHECK_CLASS_CAST ((klass), EGG_TYPE_DRUID_DIALOG, EggDruidDialogClass))
#define EGG_IS_DRUID_DIALOG(obj)	  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_DRUID_DIALOG))
#define EGG_IS_DRUID_DIALOG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((obj), EGG_TYPE_DRUID_DIALOG))
#define EGG_DRUID_DIALOG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_DRUID_DIALOG, EggDruidDialogClass))

typedef struct _EggDruidDialog      EggDruidDialog;
typedef struct _EggDruidDialogClass EggDruidDialogClass;

struct _EggDruidDialog
{
  GtkDialog parent;

  GtkWidget *druid;
};

struct _EggDruidDialogClass
{
  GtkDialogClass parent_class;
};

GType egg_druid_dialog_get_type (void);

#endif /* __EGG_DRUID_DIALOG_H__ */
