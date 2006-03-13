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

#include <string.h>
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
 *     EggPrintBackendModule modules     *
 *****************************************/

typedef struct _EggPrintBackendModule EggPrintBackendModule;
typedef struct _EggPrintBackendModuleClass EggPrintBackendModuleClass;

struct _EggPrintBackendModule
{
  GTypeModule parent_instance;
  
  GModule *library;

  void             (*init)     (GTypeModule    *module);
  void             (*exit)     (void);
  EggPrintBackend* (*create)   (void);

  gchar *path;
};

struct _EggPrintBackendModuleClass
{
  GTypeModuleClass parent_class;
};

G_DEFINE_TYPE (EggPrintBackendModule, _egg_print_backend_module, G_TYPE_TYPE_MODULE);
#define EGG_TYPE_PRINT_BACKEND_MODULE       (_egg_print_backend_module_get_type ())
#define EGG_PRINT_BACKEND_MODULE(module)	  (G_TYPE_CHECK_INSTANCE_CAST ((module), EGG_TYPE_PRINT_BACKEND_MODULE, EggPrintBackendModule))

static GSList *loaded_backends;

static gboolean
egg_print_backend_module_load (GTypeModule *module)
{
  EggPrintBackendModule *pb_module = EGG_PRINT_BACKEND_MODULE (module);
  
  pb_module->library = g_module_open (pb_module->path, G_MODULE_BIND_LAZY | G_MODULE_BIND_LOCAL);
  if (!pb_module->library)
    {
      g_warning (g_module_error());
      return FALSE;
    }
  
  /* extract symbols from the lib */
  if (!g_module_symbol (pb_module->library, "pb_module_init",
			(gpointer *)&pb_module->init) ||
      !g_module_symbol (pb_module->library, "pb_module_exit", 
			(gpointer *)&pb_module->exit) ||
      !g_module_symbol (pb_module->library, "pb_module_create", 
			(gpointer *)&pb_module->create))
    {
      g_warning (g_module_error());
      g_module_close (pb_module->library);
      
      return FALSE;
    }
	    
  /* call the filesystems's init function to let it */
  /* setup anything it needs to set up. */
  pb_module->init (module);

  return TRUE;
}

static void
egg_print_backend_module_unload (GTypeModule *module)
{
  EggPrintBackendModule *pb_module = EGG_PRINT_BACKEND_MODULE (module);
  
  pb_module->exit();

  g_module_close (pb_module->library);
  pb_module->library = NULL;

  pb_module->init = NULL;
  pb_module->exit = NULL;
  pb_module->create = NULL;
}

/* This only will ever be called if an error occurs during
 * initialization
 */
static void
egg_print_backend_module_finalize (GObject *object)
{
  EggPrintBackendModule *module = EGG_PRINT_BACKEND_MODULE (object);

  g_free (module->path);

  G_OBJECT_CLASS (_egg_print_backend_module_parent_class)->finalize (object);
}

static void
_egg_print_backend_module_class_init (EggPrintBackendModuleClass *class)
{
  GTypeModuleClass *module_class = G_TYPE_MODULE_CLASS (class);
  GObjectClass *gobject_class = G_OBJECT_CLASS (class);
  
  module_class->load = egg_print_backend_module_load;
  module_class->unload = egg_print_backend_module_unload;

  gobject_class->finalize = egg_print_backend_module_finalize;
}

static void
_egg_print_backend_module_init (EggPrintBackendModule *pb_module)
{
}


static EggPrintBackend *
_egg_print_backend_module_create (EggPrintBackendModule *pb_module)
{
  EggPrintBackend *pb;
  
  if (g_type_module_use (G_TYPE_MODULE (pb_module)))
    {
      pb = pb_module->create ();
      g_type_module_unuse (G_TYPE_MODULE (pb_module));
      return pb;
    }
  return NULL;
}

/* Like g_module_path, but use .la as the suffix
 */
static gchar*
module_build_la_path (const gchar *directory,
		      const gchar *module_name)
{
  gchar *filename;
  gchar *result;
	
  if (strncmp (module_name, "lib", 3) == 0)
    filename = (gchar *)module_name;
  else
    filename =  g_strconcat ("lib", module_name, ".la", NULL);

  if (directory && *directory)
    result = g_build_filename (directory, filename, NULL);
  else
    result = g_strdup (filename);

  if (filename != module_name)
    g_free (filename);

  return result;
}


static gchar *
_egg_print_find_module (const gchar *name,
	   	        const gchar *type)
{
  gchar *path;
  gchar *result;

  result = NULL;

  path = g_strdup_printf ("modules/%s/", type); 

  result = module_build_la_path (path, name);

  g_message ("%s", result);
  g_free (path);  
 
  return result;
}

EggPrintBackend *
_egg_print_backend_create (const char *backend_name)
{
  GSList *l;
  char *module_path;
  EggPrintBackendModule *pb_module;
  EggPrintBackend *pb;

  /* TODO: make module loading code work */
  for (l = loaded_backends; l != NULL; l = l->next)
    {
      pb_module = l->data;
      
      if (strcmp (G_TYPE_MODULE (pb_module)->name, backend_name) == 0)
	return _egg_print_backend_module_create (pb_module);
    }

  pb = NULL;
  if (g_module_supported ())
    {
      /* TODO: make this _gtk_find_module from gtkmodules when we move to gtk */
      module_path = _egg_print_find_module (backend_name, "printbackends/cups");

      if (module_path)
	{
	  pb_module = g_object_new (EGG_TYPE_PRINT_BACKEND_MODULE, NULL);

	  g_type_module_set_name (G_TYPE_MODULE (pb_module), backend_name);
	  pb_module->path = g_strdup (module_path);

	  loaded_backends = g_slist_prepend (loaded_backends,
		   		             pb_module);

	  pb = _egg_print_backend_module_create (pb_module);
	}
      
      g_free (module_path);
    }

  return pb;

  return NULL;
}

GList *
egg_print_backend_load_modules ()
{
  GList *result;
  EggPrintBackend *backend;
   
  result = NULL;

  /* TODO: don't hardcode modules. 
     Figure out how to specify which modules to load */

  backend = _egg_print_backend_create ("eggprintbackendcups");
  
  if (backend)
    result = g_list_append (result, backend);

  return result;
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

EggPrinter *
egg_print_backend_find_printer (EggPrintBackend *print_backend,
                                const gchar *printer_name)
{
  g_return_val_if_fail (EGG_IS_PRINT_BACKEND (print_backend), NULL);

  return EGG_PRINT_BACKEND_GET_IFACE (print_backend)->find_printer (print_backend, printer_name);

}

void
egg_print_backend_print_stream (EggPrintBackend *print_backend,
                                EggPrintJob *job,
                                const gchar *title,
                                gint data_fd,
                                EggPrintJobCompleteFunc callback,
                                gpointer user_data)
{
  g_return_if_fail (EGG_IS_PRINT_BACKEND (print_backend));

  return EGG_PRINT_BACKEND_GET_IFACE (print_backend)->print_stream (print_backend,
                                                                    job,
                                                                    title,
                                                                    data_fd,
                                                                    callback,
                                                                    user_data);
}


