/*
 * Copyright (C) 2007 Novell, Inc.
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

#include "config.h"

#include <string.h>
#include <glib/gi18n.h>
#include <libgnomeui/gnome-ui-init.h>

#include "eggsmclient.h"
#include "eggsmclient-libgnomeui.h"

/**
 * egg_sm_client_module_info_get:
 *
 * Gets a #GnomeModuleInfo for #EggSMClient support.
 *
 * Return value: the #GnomeModuleInfo.
 **/
const GnomeModuleInfo *
egg_sm_client_module_info_get (void)
{
	static GnomeModuleInfo module_info = {
		"eggsmclient", VERSION, N_("Session management"),
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		egg_sm_client_get_option_group
	};

	return &module_info;
}

/**
 * egg_sm_client_libgnomeui_module_info_get:
 *
 * Copies %LIBGNOMEUI_MODULE, but replaces #GnomeClient support with
 * #EggSMClient support.
 *
 * Return value: the #GnomeModuleInfo.
 **/
const GnomeModuleInfo *
egg_sm_client_libgnomeui_module_info_get (void)
{
  static GnomeModuleInfo module_info = { NULL };

  if (!module_info.name)
    {
      int i;

      module_info = *libgnomeui_module_info_get ();
      module_info.name = "libgnomeui+eggsmclient";
      module_info.version = VERSION;
      module_info.description = _("GNOME GUI Library + EggSMClient");

      for (i = 0; module_info.requirements[i].module_info; i++)
	{
	  if (!strcmp (module_info.requirements[i].module_info->name, "gnome-client"))
	    {
	      module_info.requirements[i].required_version = VERSION;
	      module_info.requirements[i].module_info = egg_sm_client_module_info_get ();
	      break;
	    }
	}
    }

  return &module_info;
}
