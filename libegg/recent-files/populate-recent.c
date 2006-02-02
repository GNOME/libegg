#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <glib-object.h>
#include <egg-recent.h>


static void
populate_from_directory (EggRecentModel *model, const gchar *dirname, int recur)
{
	GDir *dir;
	const gchar *name;

	if (recur == 0)
		return;

	dir = g_dir_open (dirname, 0, NULL);

	g_return_if_fail (dir != NULL);

	name = g_dir_read_name (dir);

	while (name) {
		gchar *full_path;
		gchar *uri;

		full_path = g_strdup_printf ("%s/%s", dirname, name);
		uri = g_strdup_printf ("file://%s", full_path);
		
		egg_recent_model_add (model, uri);

		if (g_file_test (full_path, G_FILE_TEST_IS_DIR))
			populate_from_directory (model, full_path, recur-1);

		g_free (uri);
		g_free (full_path);
		name = g_dir_read_name (dir);
	}

	g_dir_close (dir);
}


/* unfortunately, we need a main loop to trigger notifications, so any
 * modifications done through this tool won't be known to other apps
 */

int
main (int argc, char *argv[])
{
	EggRecentModel *model;

	g_type_init ();
	
	if (argc < 3) {
		g_print ("Usage:\n\n");
		g_print ("populate-recent --add <URI>\n");
		g_print ("populate-recent --delete <URI>\n");
		g_print ("populate-recent --recurse-dir <directory> <level>\n");
		return 1;
	}

	model = egg_recent_model_new (EGG_RECENT_MODEL_SORT_NONE);
	egg_recent_model_set_limit (model, 0);

	g_return_val_if_fail (model != NULL, 2);

	if (!strcmp (argv[1], "--add")) {
		g_print ("Adding: %s\n", argv[2]);
		egg_recent_model_add (model, argv[2]);
	} else if (!strcmp (argv[1], "--delete")) {
		g_print ("Deleting: %s\n", argv[2]);
		egg_recent_model_delete (model, argv[2]);
	} else if (!strcmp (argv[1], "--recurse-dir")) {
		int level;

		if (argc >= 4)
			level = atoi (argv[3]);
		else
			level = 1;

		g_print ("Adding files from: %s\n", argv[2]);
		populate_from_directory (model, argv[2], level);
	}
	

	return 0;
}
