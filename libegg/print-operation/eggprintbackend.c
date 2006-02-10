/* GTK - The GIMP Toolkit
 * eggprintbackend.h: Abstract printer backend interfaces
 * Copyright (C) 2003, Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <config.h>
#include <gmodule.h>

#include "eggprintbackend.h"

static void egg_print_backend_base_init (gpointer g_class);

GQuark
egg_print_backend_error_quark (void)
{
  static GQuark quark = 0;
  if (quark == 0)
    quark = g_quark_from_static_string ("egg-print-backend-error-quark");
  return quark;
}

/*****************************************
 *             EggPrintBackend           *
 *****************************************/
GType
egg_print_backend_get_type (void)
{
  static GType print_backend_type = 0;

  if (!print_backend_type)
    {
      static const GTypeInfo print_backend_info =
      {
	sizeof (EggPrintBackendIface),  /* class_size */
	egg_print_backend_base_init,    /* base_init */
	NULL,                         /* base_finalize */
      };

      print_backend_type = g_type_register_static (G_TYPE_INTERFACE,
						 "EggPrintBackend",
						 &print_backend_info, 0);

      g_type_interface_add_prerequisite (print_backend_type, G_TYPE_OBJECT);
    }

  return print_backend_type;
}

static void
egg_print_backend_base_init (gpointer g_class)
{
  static gboolean initialized = FALSE;
  
  if (!initialized)
    {
      GType iface_type = G_TYPE_FROM_INTERFACE (g_class);

      g_signal_new ("printer-added",
		    iface_type,
		    G_SIGNAL_RUN_LAST,
		    G_STRUCT_OFFSET (EggPrintBackendIface, printer_added),
		    NULL, NULL,
		    g_cclosure_marshal_VOID__OBJECT,
		    G_TYPE_NONE, 1, G_TYPE_OBJECT);
      g_signal_new ("printer-removed",
		    iface_type,
		    G_SIGNAL_RUN_LAST,
		    G_STRUCT_OFFSET (EggPrintBackendIface, printer_removed),
		    NULL, NULL,
		    g_cclosure_marshal_VOID__OBJECT,
		    G_TYPE_NONE, 1, G_TYPE_OBJECT);
      g_signal_new ("printer-status-changed",
		    iface_type,
		    G_SIGNAL_RUN_LAST,
		    G_STRUCT_OFFSET (EggPrintBackendIface, printer_status_changed),
		    NULL, NULL,
		    g_cclosure_marshal_VOID__OBJECT,
		    G_TYPE_NONE, 1, G_TYPE_OBJECT);

      initialized = TRUE;
    }
}

/* TODO: setup loading of backends */
EggPrintBackend *
_egg_print_backend_create (const char *backend_name)
{
#if 0
  GSList *l;
  char *module_path;
  EggPrintBackendModule *pb_module;
  EggPrintBackend *pb;

  /* TODO: make module loading code work */
  for (l = loaded_print_backends; l != NULL; l = l->next)
    {
      fs_module = l->data;
      
      if (strcmp (G_TYPE_MODULE (pb_module)->name, print_backend_name) == 0)
	return _egg_print_backend_module_create (pb_module);
    }

  pb = NULL;
  if (g_module_supported ())
    {
      module_path = _egg_find_module (print_backend_name, "printbackends");

      if (module_path)
	{
	  pb_module = g_object_new (EGG_TYPE_PRINT_BACKEND_MODULE, NULL);

	  g_type_module_set_name (G_TYPE_MODULE (pb_module), print_backend_name);
	  pb_module->path = g_strdup (module_path);

	  loaded_print_backends = g_slist_prepend (loaded_print_backends,
						   pb_module);

	  pb = _egg_print_backend_module_create (pb_module);
	}
      
      g_free (module_path);
    }

  return pb;
#endif 

  return NULL;
}

cairo_surface_t *
egg_print_backend_printer_create_cairo_surface (EggPrintBackend *print_backend,
                                                EggPrinter *printer,
                                                gdouble width, 
                                                gdouble height,
						gint cache_fd)
{
  g_return_val_if_fail (EGG_IS_PRINT_BACKEND (print_backend), NULL);

  return EGG_PRINT_BACKEND_GET_IFACE (print_backend)->printer_create_cairo_surface (print_backend, printer, width, height, cache_fd);
}

EggPrinter *
egg_print_backend_find_printer (EggPrintBackend *print_backend,
                                const gchar *printer_name)
{
  g_return_val_if_fail (EGG_IS_PRINT_BACKEND (print_backend), NULL);

  return EGG_PRINT_BACKEND_GET_IFACE (print_backend)->find_printer (print_backend, printer_name);

}

void
egg_print_backend_print_stream (EggPrintBackend *print_backend,
                                EggPrinter *printer,
                                const gchar *title,
                                gint data_fd,
                                EggPrinterSendCompleteFunc callback,
                                gpointer user_data)
{
  g_return_if_fail (EGG_IS_PRINT_BACKEND (print_backend));

  return EGG_PRINT_BACKEND_GET_IFACE (print_backend)->print_stream (print_backend,
                                                                    printer,
                                                                    title,
                                                                    data_fd,
                                                                    callback,
                                                                    user_data);

}

