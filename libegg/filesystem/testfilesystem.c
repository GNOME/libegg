#include <gtk/gtk.h>
#include "eggfilesystem-unix.h"
#include "eggfilefilter.h"

static void
dump_children (EggFileSystem *system, EggFileFilter *filter, EggFileSystemItem *root, int indent)
{
  int i;
  char *name;

  if (egg_file_filter_should_show (filter, root))
    {
      /* Print the root name first */
      for (i = 0; i < indent; i++)
	g_print (" ");

      name = egg_file_system_item_get_name (root);
      g_print ("%s\n", name);
      g_free (name);
    }
  
  if (egg_file_system_item_get_item_type (root) == EGG_FILE_SYSTEM_ITEM_FOLDER)
    {
      GList *p, *children;

      children = egg_file_system_item_get_children (root, NULL);

      p = children;
      while (p)
	{
	  EggFileSystemItem *child = p->data;
	  
	  dump_children (system, filter, child, indent + 1);
	  
	  p = p->next;
	}

      egg_file_system_item_list_free (children);
    }
}

gboolean
filter_func (EggFileFilter *filter, EggFileSystemItem *item, gpointer data)
{
  static GPatternSpec *pspec = NULL;
  gchar *name;
  gboolean retval;
  
  if (!pspec)
    pspec = g_pattern_spec_new ("*.jpg");

  name = egg_file_system_item_get_name (item);

  retval = g_pattern_match_string (pspec, name);

  g_free (name);
  
  return retval;
}

gint
main (gint argc, gchar **argv)
{
  EggFileSystem *system;
  EggFileSystemItem *item;
  EggFileFilter *filter;
  
  g_type_init ();

  /* Create a filter */
  filter = egg_file_filter_new ("Test Filter", filter_func, NULL, NULL);

  system = g_object_new (EGG_TYPE_FILE_SYSTEM_UNIX, NULL);
  item = egg_file_system_get_folder (system, EGG_FILE_SYSTEM_FOLDER_HOME, NULL);

  dump_children (system, filter, item, 0);
  
  return 0;
}
