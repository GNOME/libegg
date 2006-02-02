/* eggdruiddialog.c
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

#include "eggdruiddialog.h"

static void egg_druid_dialog_class_init (EggDruidDialogClass *klass);
static void egg_druid_dialog_init       (EggDruidDialog      *druid);


GType
egg_druid_dialog_get_type (void)
{
  static GType our_type = 0;

  if (!our_type)
    {
      static const GTypeInfo our_type_info =
      {
	sizeof (EggDruidDialogClass),
	(GBaseInitFunc) NULL,
	(GBaseFinalizeFunc) NULL,
	(GClassInitFunc) egg_druid_dialog_class_init,
	(GClassFinalizeFunc) NULL,
	NULL,
	sizeof (EggDruidDialog),
	0, /* n_preallocs */
	(GInstanceInitFunc) egg_druid_dialog_init
      };

      our_type = g_type_register_static (GTK_TYPE_DIALOG, "EggDruidDialog",
					&our_type_info, 0);
    }

  return our_type;
}

static void
egg_druid_dialog_class_init (EggDruidDialogClass *klass)
{
}

static void
egg_druid_dialog_init (EggDruidDialog *druid)
{
}

