#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "eggrecentmanager.h"

static void
show_usage (void)
{
  g_print (
"Usage: populate-recent <option>\n"
"\nOptions:\n"
"  --add <uri>   Add <uri> to the list of recently used resources\n"
"  --info <uri>  Shows informations about <uri>\n"
"  --list [n]    Lists at max [n] items from teh recently used resources list\n"
  );
}

static void
add_recent_item (EggRecentManager *recent_manager,
		 const gchar      *uri)
{
  GError *err = NULL;
  
  g_return_if_fail (EGG_IS_RECENT_MANAGER (recent_manager));
  g_return_if_fail (uri != NULL);

  egg_recent_manager_add_item (recent_manager, uri, &err);
  if (err)
    {
      g_warning ("Unable to add item `%s': %s", uri, err->message);
      g_error_free (err);
      
      return;
    }
}

static void
print_recent_info (EggRecentManager *recent_manager,
		   const gchar      *uri)
{
  EggRecentInfo *info;
  GError *err = NULL;
  time_t added, modified;
  gchar *added_str, *modified_str;
  
  g_return_if_fail (EGG_IS_RECENT_MANAGER (recent_manager));
  g_return_if_fail (uri != NULL);
  
  info = egg_recent_manager_lookup_item (recent_manager, uri, &err);
  if (err)
    {
      g_warning ("Unable to retrieve informations on item `%s': %s\n",
      		 uri,
      		 err->message);
      g_error_free (err);
      
      return;
    }
  
  added = egg_recent_info_get_added (info);
  added_str = g_strdup (ctime (&added));
  modified = egg_recent_info_get_modified (info);
  modified_str = g_strdup (ctime (&modified));
  
  g_print ("** `%s':       \n"
           "\tURI:       %s\n"
           "\tMIME Type: %s\n"
           "\tAdded:     %s"
           "\tModified:  %s",
           egg_recent_info_get_display_name (info),
           egg_recent_info_get_uri (info),
           egg_recent_info_get_mime_type (info),
           added_str,
           modified_str);
  
  g_free (added_str);
  g_free (modified_str);
  
  egg_recent_info_unref (info);
}

static void
print_recent_list (EggRecentManager *recent_manager)
{
  GList *items, *l;
  gint i;
  
  g_return_if_fail (EGG_IS_RECENT_MANAGER (recent_manager));
  
  egg_recent_manager_set_limit (recent_manager, -1);
  
  items = egg_recent_manager_get_items (recent_manager);
  for (l = items, i = 0; l != NULL; l = l->next, i++)
    {
      EggRecentInfo *info = (EggRecentInfo *) l->data;
      
      g_assert (info != NULL);
      
      g_print ("Item [%03d]: %s\n",
               i,
               egg_recent_info_get_uri (info));
      
      egg_recent_info_unref (info);
    }
  
  if (items)  
    g_list_free (items);
}

int
main (int argc, char *argv[])
{
  gint i;
  EggRecentManager *recent_manager;
  
  g_type_init ();
  
  if (argc == 1)
    {
      show_usage ();
      return -1;
    }
  
  /* since we are not using GTK, we must set this by ourselves */
  g_set_prgname ("populate-recent");
  g_set_application_name ("populate-recent");
  
  recent_manager = egg_recent_manager_new ();
  
  for (i = 1; i < argc; i++)
    {
      if (0 == strcmp (argv[i], "--add"))
        {
          gchar *uri;
          
          if (i + 1 > argc)
            {
              g_print ("** Argument 'add' requires an URI\n");
              show_usage ();
              return -1;
            }
          
          uri = argv[++i];
          
          g_print ("** Adding\n"
		   "**   `%s'\n"
		   "** to the recently used resources list...", uri);
          
          add_recent_item (recent_manager, uri);
          
	  g_print ("Done.\n");
	  
          break;
        }
      else if (0 == strcmp (argv[i], "--info"))
        {
          gchar *uri;
          
          if (i + 1 > argc)
            {
              g_print ("** Argument 'info' requires an URI\n");
              show_usage ();
              return -1;
            }
          
          uri = argv[++i];
          
          g_print ("** Showing informations about `%s'...\n", uri);
          
          print_recent_info (recent_manager, uri);
          
          break;
        }
      else if (0 == strcmp (argv[i], "--list"))
        {
          g_print ("** Getting item list...\n");
          
          print_recent_list (recent_manager);
          
          break;
        }
      else
        {
          show_usage ();
          return -1;
        }
    }
  
  g_object_unref (recent_manager);
  
  return 0;
}
