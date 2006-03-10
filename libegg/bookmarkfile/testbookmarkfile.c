#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

#include "eggbookmarkfile.h"

int main (int argc, char *argv[])
{
	EggBookmarkFile *bookmark_file;
	const gchar *filename;
	gchar *title, *desc;
	GError *err = NULL;
#if 0
	gchar **uris;
	gsize len, i;
#endif

	if (argc > 1)
          filename = argv[1];
	else
          {
            g_print ("Usage: %s <xbel-file>\n", argv[0]);
	    exit (1);
	  }

	g_assert (filename != NULL);

	bookmark_file = egg_bookmark_file_new ();
	if (!bookmark_file) {
		fprintf (stderr, "Unable to create BookmarkFile\n");
		return EXIT_FAILURE;
	}

	egg_bookmark_file_load_from_file (bookmark_file,
					  filename,
					  &err);
	if (err) {
		fprintf (stderr, "! Unable to load '%s':\n"
				 "!   error: %s\n",
				 filename,
				 err->message);
		g_error_free (err);
		egg_bookmark_file_free (bookmark_file);

		return EXIT_FAILURE;
	}

	fprintf (stdout, "%d bookmarks loaded successfully.\n",
		 egg_bookmark_file_get_size (bookmark_file));


	title = egg_bookmark_file_get_title (bookmark_file, NULL, NULL);
	desc = egg_bookmark_file_get_description (bookmark_file, NULL, NULL);

	if (title || desc)
		fprintf (stdout, "File: %s (%s).\n", title, desc);

	g_free (title);
	g_free (desc);

#if 0
	uris = egg_bookmark_file_get_uris (bookmark_file, &len);
	for (i = 0; i < len; i++) {
		gchar *uri, *title;

		uri = uris[i];
		title = egg_bookmark_file_get_title (bookmark_file, uri, &err);
		if (err) {
			fprintf (stderr, "! Unable to get the title for URI '%s'\n"
				         "!   error: %s\n",
					 uri,
					 err->message);
			g_error_free (err);

			egg_bookmark_file_free (bookmark_file);
			
			return EXIT_FAILURE;
		}

		fprintf (stdout, "* %02d: %s := %s\n",
			 i,
			 uri,
			 title);
	}

	g_strfreev (uris);

	egg_bookmark_file_to_file (bookmark_file, "output.xbel", &err);
	if (err) {
		fprintf (stderr, "! Unable to save to file:\n"
				 "!   error: %s\n", err->message);
		g_error_free (err);

		egg_bookmark_file_free (bookmark_file);
		
		return EXIT_FAILURE;
	}
#endif

	egg_bookmark_file_free (bookmark_file);

	return EXIT_SUCCESS;
}
