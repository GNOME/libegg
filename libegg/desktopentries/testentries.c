#include <glib.h>
#include "eggdesktopentries.h"
#include <locale.h>

#define DESKTOP_FILE "testentries.desktop"

void
print_sample_desktop_file (void)
{
  EggDesktopEntries *entries;

  entries =
    egg_desktop_entries_new (EGG_DESKTOP_ENTRIES_GENERATE_LOOKUP_MAP, NULL);

  egg_desktop_entries_set_string (entries,
                                  egg_desktop_entries_get_start_group (entries),
                                  "Encoding", "UTF-8", NULL);
  gchar *mime_types[] = { "text/plain", "image/png", "audio/wav", NULL };
  egg_desktop_entries_set_string_list (entries,
                                  egg_desktop_entries_get_start_group (entries),
                                  "MimeType", mime_types, G_N_ELEMENTS (mime_types), 
                                  NULL);



  egg_desktop_entries_set_integer (entries,
                                   egg_desktop_entries_get_start_group (entries),
                                   "NumStuff", 10, NULL);

  egg_desktop_entries_set_boolean (entries,
                                   egg_desktop_entries_get_start_group (entries),
                                   "CanDoStuff", FALSE, NULL);

  g_print ("%s\n", egg_desktop_entries_to_data (entries, NULL, NULL));

  egg_desktop_entries_free (entries);
}

int
main (int argc, char **argv)
{
  GError *error;
  gchar **group, *group_name, **key, **categories, *category;
  EggDesktopEntries *entries;
  int i, j;
  gboolean *boolean_list;
  gint *integer_list;
  gsize length = 0;

  setlocale (LC_ALL, "");
  error = NULL;

  entries =
    egg_desktop_entries_new_from_file (DESKTOP_FILE, NULL,
				       EGG_DESKTOP_ENTRIES_GENERATE_LOOKUP_MAP,
				       NULL);

  if (!entries)
    return 1;

  group = egg_desktop_entries_get_groups (entries, NULL);

  i = 0;
  while ((group_name = group[i]))
    {
      char *text;
      char *key_name;
      key = egg_desktop_entries_get_keys (entries, group_name, NULL, NULL);

      text = g_strdup_printf ("%s:\n", group_name);
      g_print ("%s", text);
      g_free (text);

      j = 0;
      while ((key_name = key[j]))
	{

	  text = g_strdup_printf ("key: '%s', value: '%s'\n", key_name,
				  egg_desktop_entries_get_string (entries,
								  group_name,
								  key_name,
								  NULL));
	  g_print ("%s", text);
	  g_free (text);

	  j++;
	}

      i++;
    }

  g_print ("Translated Name: %s\n",
	   egg_desktop_entries_get_locale_string (entries,
						  egg_desktop_entries_get_start_group (entries),
						  "Name", "zu", NULL));

  categories = egg_desktop_entries_get_locale_string_list (entries,
							   egg_desktop_entries_get_start_group (entries),
							   "Categories",
							   NULL, NULL, NULL);

  i = 0;
  while ((category = categories[i]))
    {
      g_print ("Category: %s\n", category);
      i++;
    }

  boolean_list = egg_desktop_entries_get_boolean_list (entries,
						       egg_desktop_entries_get_start_group (entries),
						       "Bools", &length,
						       NULL);

  for (i = 0; i < length; i++)
    {
      g_print ("Bool: %s\n", boolean_list[i] ? "true" : "false");
    }

  integer_list = egg_desktop_entries_get_integer_list (entries,
						       egg_desktop_entries_get_start_group (entries),
						       "Ints", &length, NULL);

  for (i = 0; i < length; i++)
    {
      g_print ("Int: %d\n", integer_list[i]);
    }

  g_print ("%s", egg_desktop_entries_to_data (entries, NULL, NULL));

  egg_desktop_entries_free (entries);

  g_print ("\n\nSample Desktop File:\n");
  print_sample_desktop_file ();

  return 0;
}
