
#include <gtk/gtk.h>
#include "egg-pixbuf-thumbnail.h"

static GtkWidget *image = NULL;
static GtkWidget *label = NULL;

static void
update_preview_cb (GtkFileChooser *dialog)
{
  gchar *filename;
  GdkPixbuf *thumb;
  
  filename = gtk_file_chooser_get_preview_filename (dialog);

  if (filename != NULL)
    {
      thumb = egg_pixbuf_get_thumbnail_for_file (filename, EGG_PIXBUF_THUMBNAIL_NORMAL, NULL);
      g_free (filename);
    }
  else
    {
      thumb = NULL;
    }

  if (thumb != NULL)
    {
      gchar *text;

      gtk_image_set_from_pixbuf (GTK_IMAGE (image), thumb);
      g_object_unref (thumb);

      text = g_strdup_printf ("Width:\t\t%d\n"
			      "Height:\t\t%d\n"
			      "Mime-Type:\t%s\n"
			      "Description:\t%s\n"
			      "Pages:\t\t%d\n"
			      "Length:\t\t%ld\n"
			      "File Size:\t\t%" G_GSSIZE_FORMAT "\n"
			      "Last Modified:\t%ld\n",
			      egg_pixbuf_get_thumbnail_image_width (thumb),
			      egg_pixbuf_get_thumbnail_image_height (thumb),
			      egg_pixbuf_get_thumbnail_mime_type (thumb),
			      egg_pixbuf_get_thumbnail_description (thumb),
			      egg_pixbuf_get_thumbnail_document_pages (thumb),
			      egg_pixbuf_get_thumbnail_movie_length (thumb),
			      egg_pixbuf_get_thumbnail_filesize (thumb),
			      egg_pixbuf_get_thumbnail_mtime (thumb));
      gtk_label_set_text (GTK_LABEL (label), text);
      g_free (text);
      gtk_file_chooser_set_preview_widget_active (dialog, TRUE);
    }
  else
    {
      gtk_file_chooser_set_preview_widget_active (dialog, FALSE);
    }
}


int
main (int   argc,
      char *argv[])
{
  GtkWidget *window, *box;

  gtk_init (&argc, &argv);

  window = gtk_file_chooser_dialog_new ("Image FileChooser", NULL, GTK_FILE_CHOOSER_ACTION_OPEN,
					GTK_STOCK_QUIT, GTK_RESPONSE_CANCEL, NULL);
  g_signal_connect (window, "response", G_CALLBACK (gtk_main_quit), NULL);
  g_signal_connect (window, "delete-event", G_CALLBACK (gtk_main_quit), NULL);

  box = gtk_vbox_new (FALSE, 6);

  image = gtk_image_new ();
  gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 0);

  label = gtk_label_new (NULL);
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);

  gtk_widget_show_all (box);
  gtk_file_chooser_set_preview_widget (GTK_FILE_CHOOSER (window), box);
  g_signal_connect (window, "update-preview", G_CALLBACK (update_preview_cb), NULL);

  gtk_window_present (GTK_WINDOW (window));

  gtk_main ();

  return 0;
}
