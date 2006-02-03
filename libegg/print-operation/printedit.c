#include <pango/pangocairo.h>
#include <gtk/gtk.h>
#include "eggprint.h"

static GtkWidget *main_window;
static char *filename = NULL;
static EggPrinterSettings *settings = NULL;
static gboolean file_changed = FALSE;
static GtkTextBuffer *buffer;
static GtkWidget *statusbar;

static void
update_title (void)
{
  char *basename;
  char *title;
  
  if (filename == NULL)
    basename = g_strdup ("Untitled");
  else
    basename = g_path_get_basename (filename);

  title = g_strdup_printf ("Simple Editor with printing - %s", basename);
  g_free (basename);
  
  gtk_window_set_title (GTK_WINDOW (main_window), title);
  g_free (title);
}

static void
update_statusbar (void)
{
  gchar *msg;
  gint row, col;
  GtkTextIter iter;

  gtk_statusbar_pop (GTK_STATUSBAR (statusbar), 0);
  
  gtk_text_buffer_get_iter_at_mark (buffer,
                                    &iter,
                                    gtk_text_buffer_get_insert (buffer));

  row = gtk_text_iter_get_line (&iter);
  col = gtk_text_iter_get_line_offset (&iter);

  msg = g_strdup_printf ("%d, %d%s",
                         row, col, file_changed?" - Modified":"");

  gtk_statusbar_push (GTK_STATUSBAR (statusbar), 0, msg);

  g_free (msg);
}

static void
update_ui (void)
{
  update_title ();
  update_statusbar ();
}

static char *
get_text (void)
{
  GtkTextIter start, end;

  gtk_text_buffer_get_start_iter (buffer, &start);
  gtk_text_buffer_get_end_iter (buffer, &end);
  return gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
}

static void
set_text (const char *text, gsize len)
{
  gtk_text_buffer_set_text (buffer, text, len);
  file_changed = FALSE;
  update_ui ();
}

static void
do_new (GtkAction *action)
{
  g_free (filename);
  filename = NULL;
  set_text ("", 0);
}

static void
load_file (const char *open_filename)
{
  GtkWidget *error_dialog;
  char *contents;
  GError *error;
  gsize len;
  
  error_dialog = NULL;
  error = NULL;
  if (g_file_get_contents (open_filename, &contents, &len, &error))
    {
      if (g_utf8_validate (contents, len, NULL))
	{
	  filename = g_strdup (open_filename);
	  set_text (contents, len);
	  g_free (contents);
	}
      else
	{
	  error_dialog = gtk_message_dialog_new (GTK_WINDOW (main_window),
						 GTK_DIALOG_DESTROY_WITH_PARENT,
						 GTK_MESSAGE_ERROR,
						 GTK_BUTTONS_CLOSE,
						 "Error loading file %s:\n%s",
						 open_filename,
						 "Not valid utf8");
	}
    }
  else
    {
      error_dialog = gtk_message_dialog_new (GTK_WINDOW (main_window),
					     GTK_DIALOG_DESTROY_WITH_PARENT,
					     GTK_MESSAGE_ERROR,
					     GTK_BUTTONS_CLOSE,
					     "Error loading file %s:\n%s",
					     open_filename,
					     error->message);
      
      g_error_free (error);
    }
  if (error_dialog)
    {
      g_signal_connect (error_dialog, "response", G_CALLBACK (gtk_widget_destroy), NULL);
      gtk_widget_show (error_dialog);
    }
}

static void
do_open (GtkAction *action)
{
  GtkWidget *dialog;
  gint response;
  char *open_filename;
  
  dialog = gtk_file_chooser_dialog_new ("Select file",
					GTK_WINDOW (main_window),
					GTK_FILE_CHOOSER_ACTION_OPEN,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					GTK_STOCK_OPEN, GTK_RESPONSE_OK,
					NULL);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
  response = gtk_dialog_run (GTK_DIALOG (dialog));

  if (response == GTK_RESPONSE_OK)
    {
      open_filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
      load_file (open_filename);
      g_free (open_filename);
    }

  gtk_widget_destroy (dialog);
}

static void
save_file (const char *save_filename)
{
  char *text = get_text ();
  GtkWidget *error_dialog;
  GError *error;

  error = NULL;
  if (g_file_set_contents (save_filename,
			   text, -1, &error))
    {
      if (save_filename != filename)
	{
	  g_free (filename);
	  filename = g_strdup (save_filename);
	}
      file_changed = FALSE;
      update_ui ();
    }
  else
    {
      error_dialog = gtk_message_dialog_new (GTK_WINDOW (main_window),
					     GTK_DIALOG_DESTROY_WITH_PARENT,
					     GTK_MESSAGE_ERROR,
					     GTK_BUTTONS_CLOSE,
					     "Error saving to file %s:\n%s",
					     filename,
					     error->message);
      
      g_signal_connect (error_dialog, "response", G_CALLBACK (gtk_widget_destroy), NULL);
      gtk_widget_show (error_dialog);
      
      g_error_free (error);
    }
}

static void
do_save_as (GtkAction *action)
{
  GtkWidget *dialog;
  gint response;
  char *save_filename;
  
  dialog = gtk_file_chooser_dialog_new ("Select file",
					GTK_WINDOW (main_window),
					GTK_FILE_CHOOSER_ACTION_SAVE,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					GTK_STOCK_SAVE, GTK_RESPONSE_OK,
					NULL);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
  response = gtk_dialog_run (GTK_DIALOG (dialog));

  if (response == GTK_RESPONSE_OK)
    {
      save_filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
      save_file (save_filename);
      g_free (save_filename);
    }
  
  gtk_widget_destroy (dialog);
}

static void
do_save (GtkAction *action)
{
  if (filename == NULL)
    do_save_as (action);
  else
    save_file (filename);
}

typedef struct {
  char *text;
  PangoLayout *layout;
  GList *page_breaks;
} PrintData;

static void
begin_print (EggPrintOperation *operation,
	     EggPrintContext *context,
	     PrintData *print_data)
{
  PangoFontDescription *desc;
  PangoLayoutLine *layout_line;
  double width, height;
  double page_height;
  GList *page_breaks;
  int num_lines;
  int line;

  width = egg_print_context_get_width (context);
  height = egg_print_context_get_height (context);

  print_data->layout = egg_print_context_create_layout (context);

  desc = pango_font_description_from_string ("Sans 12");
  pango_layout_set_font_description (print_data->layout, desc);
  pango_font_description_free (desc);

  pango_layout_set_width (print_data->layout, width * PANGO_SCALE);
  
  pango_layout_set_text (print_data->layout, print_data->text, -1);

  num_lines = pango_layout_get_line_count (print_data->layout);

  page_breaks = NULL;
  page_height = 0;

  for (line = 0; line < num_lines; line++)
    {
      PangoRectangle ink_rect, logical_rect;
      double line_height;
      
      layout_line = pango_layout_get_line (print_data->layout, line);
      pango_layout_line_get_extents (layout_line, &ink_rect, &logical_rect);

      line_height = logical_rect.height / 1024.0;

      if (page_height + line_height > height)
	{
	  page_breaks = g_list_prepend (page_breaks, GINT_TO_POINTER (line));
	  page_height = 0;
	}

      page_height += line_height;
    }

  page_breaks = g_list_reverse (page_breaks);
  egg_print_operation_set_nr_of_pages (operation, g_list_length (page_breaks) + 1);
  
  print_data->page_breaks = page_breaks;
  
}

static void
draw_page (EggPrintOperation *operation,
	   EggPrintContext *context,
	   int page_nr,
	   PrintData *print_data)
{
  cairo_t *cr;
  GList *pagebreak;
  int start, end, i;

  if (page_nr == 0)
    start = 0;
  else
    {
      pagebreak = g_list_nth (print_data->page_breaks, page_nr - 1);
      start = GPOINTER_TO_INT (pagebreak->data);
    }

  pagebreak = g_list_nth (print_data->page_breaks, page_nr);
  if (pagebreak == NULL)
    end = pango_layout_get_line_count (print_data->layout);
  else
    end = GPOINTER_TO_INT (pagebreak->data);
    
  cr = egg_print_context_get_cairo (context);

  cairo_move_to (cr, 0, 0);
  for (i = start; i < end; i++)
    {
      PangoLayoutLine *layout_line;
      PangoRectangle ink_rect, logical_rect;
      double line_height;
      
      layout_line = pango_layout_get_line (print_data->layout, i);
      pango_cairo_show_layout_line  (cr, layout_line);
      
      pango_layout_line_get_extents (layout_line, &ink_rect, &logical_rect);
      line_height = logical_rect.height / 1024.0;
      cairo_rel_move_to (cr, 0, line_height);
    }
}

static void
do_print (GtkAction *action)
{
  GtkWidget *error_dialog;
  EggPrintOperation *print;
  PrintData print_data;
  EggPrintOperationResult res;
  GError *error;

  print_data.text = get_text ();

  print = egg_print_operation_new ();

  if (settings != NULL)
    egg_print_operation_set_printer_settings (print, settings);
  
  g_signal_connect (print, "begin_print", G_CALLBACK (begin_print), &print_data);
  g_signal_connect (print, "draw_page", G_CALLBACK (draw_page), &print_data);

  error = NULL;
  res = egg_print_operation_run (print, GTK_WINDOW (main_window), &error);

  if (res == EGG_PRINT_OPERATION_RESULT_ERROR)
    {
      error_dialog = gtk_message_dialog_new (GTK_WINDOW (main_window),
					     GTK_DIALOG_DESTROY_WITH_PARENT,
					     GTK_MESSAGE_ERROR,
					     GTK_BUTTONS_CLOSE,
					     "Error printing file:\n%s",
					     error->message);
      g_signal_connect (error_dialog, "response", G_CALLBACK (gtk_widget_destroy), NULL);
      gtk_widget_show (error_dialog);
      g_error_free (error);
    }
  else if (res == EGG_PRINT_OPERATION_RESULT_APPLY)
    {
      if (settings != NULL)
	g_object_unref (settings);
      settings = egg_print_operation_get_printer_settings (print);
    }
}

static void
do_about (GtkAction *action)
{
  const gchar *authors[] = {
    "Alexander Larsson",
    NULL
  };
  gtk_show_about_dialog (GTK_WINDOW (main_window),
			 "name", "print test editor",
			 "version", "0.1",
			 "copyright", "(C) Red Hat, Inc",
			 "comments", "Program to demonstrate GTK+ printing.",
			 "authors", authors,
			 NULL);
}

static void
do_quit (GtkAction *action)
{
  gtk_main_quit ();
}

static GtkActionEntry entries[] = {
  { "FileMenu", NULL, "_File" },               /* name, stock id, label */
  { "HelpMenu", NULL, "_Help" },               /* name, stock id, label */
  { "New", GTK_STOCK_NEW,                      /* name, stock id */
    "_New", "<control>N",                      /* label, accelerator */
    "Create a new file",                       /* tooltip */ 
    G_CALLBACK (do_new) },      
  { "Open", GTK_STOCK_OPEN,                    /* name, stock id */
    "_Open","<control>O",                      /* label, accelerator */     
    "Open a file",                             /* tooltip */
    G_CALLBACK (do_open) }, 
  { "Save", GTK_STOCK_SAVE,                    /* name, stock id */
    "_Save","<control>S",                      /* label, accelerator */     
    "Save current file",                       /* tooltip */
    G_CALLBACK (do_save) },
  { "SaveAs", GTK_STOCK_SAVE,                  /* name, stock id */
    "Save _As...", NULL,                       /* label, accelerator */     
    "Save to a file",                          /* tooltip */
    G_CALLBACK (do_save_as) },
  { "Quit", GTK_STOCK_QUIT,                    /* name, stock id */
    "_Quit", "<control>Q",                     /* label, accelerator */     
    "Quit",                                    /* tooltip */
    G_CALLBACK (do_quit) },
  { "About", NULL,                             /* name, stock id */
    "_About", "<control>A",                    /* label, accelerator */     
    "About",                                   /* tooltip */  
    G_CALLBACK (do_about) },
  { "Print", GTK_STOCK_PRINT,                  /* name, stock id */
     NULL, NULL,                               /* label, accelerator */     
    "Print the document",                      /* tooltip */
    G_CALLBACK (do_print) },
};
static guint n_entries = G_N_ELEMENTS (entries);

static const gchar *ui_info = 
"<ui>"
"  <menubar name='MenuBar'>"
"    <menu action='FileMenu'>"
"      <menuitem action='New'/>"
"      <menuitem action='Open'/>"
"      <menuitem action='Save'/>"
"      <menuitem action='SaveAs'/>"
"      <menuitem action='Print'/>"
"      <separator/>"
"      <menuitem action='Quit'/>"
"    </menu>"
"    <menu action='HelpMenu'>"
"      <menuitem action='About'/>"
"    </menu>"
"  </menubar>"
"</ui>";

static void
buffer_changed_callback (GtkTextBuffer *buffer)
{
  file_changed = TRUE;
  update_statusbar ();
}

static void
mark_set_callback (GtkTextBuffer     *buffer,
                   const GtkTextIter *new_location,
                   GtkTextMark       *mark,
                   gpointer           data)
{
  update_statusbar ();
}

static void
update_resize_grip (GtkWidget           *widget,
		    GdkEventWindowState *event,
		    GtkStatusbar        *statusbar)
{
  if (event->changed_mask & (GDK_WINDOW_STATE_MAXIMIZED | 
			     GDK_WINDOW_STATE_FULLSCREEN))
    {
      gboolean maximized;

      maximized = event->new_window_state & (GDK_WINDOW_STATE_MAXIMIZED | 
					     GDK_WINDOW_STATE_FULLSCREEN);
      gtk_statusbar_set_has_resize_grip (statusbar, !maximized);
    }
}

static void
create_window (void)
{
  GtkWidget *bar;
  GtkWidget *table;
  GtkWidget *contents;
  GtkUIManager *ui;
  GtkWidget *sw;
  GtkActionGroup *actions;
  GError *error;
  
  main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  gtk_window_set_default_size (GTK_WINDOW (main_window),
			       400, 600);
  
  g_signal_connect (main_window, "delete-event",
		    G_CALLBACK (gtk_main_quit), NULL);
  
  actions = gtk_action_group_new ("Actions");
  gtk_action_group_add_actions (actions, entries, n_entries, NULL);
  
  ui = gtk_ui_manager_new ();
  gtk_ui_manager_insert_action_group (ui, actions, 0);
  gtk_window_add_accel_group (GTK_WINDOW (main_window), 
			      gtk_ui_manager_get_accel_group (ui));
  gtk_container_set_border_width (GTK_CONTAINER (main_window), 0);

  error = NULL;
  if (!gtk_ui_manager_add_ui_from_string (ui, ui_info, -1, &error))
    {
      g_message ("building menus failed: %s", error->message);
      g_error_free (error);
    }

  table = gtk_table_new (1, 3, FALSE);
  gtk_container_add (GTK_CONTAINER (main_window), table);

  bar = gtk_ui_manager_get_widget (ui, "/MenuBar");
  gtk_widget_show (bar);
  gtk_table_attach (GTK_TABLE (table),
		    bar, 
		    /* X direction */          /* Y direction */
		    0, 1,                      0, 1,
		    GTK_EXPAND | GTK_FILL,     0,
		    0,                         0);

  /* Create document  */
  sw = gtk_scrolled_window_new (NULL, NULL);

  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);
  
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
				       GTK_SHADOW_IN);
  
  gtk_table_attach (GTK_TABLE (table),
		    sw,
		    /* X direction */       /* Y direction */
		    0, 1,                   1, 2,
		    GTK_EXPAND | GTK_FILL,  GTK_EXPAND | GTK_FILL,
		    0,                      0);
  
  contents = gtk_text_view_new ();
  gtk_widget_grab_focus (contents);
      
  gtk_container_add (GTK_CONTAINER (sw),
		     contents);
  
  /* Create statusbar */
  
  statusbar = gtk_statusbar_new ();
  gtk_table_attach (GTK_TABLE (table),
		    statusbar,
		    /* X direction */       /* Y direction */
		    0, 1,                   2, 3,
		    GTK_EXPAND | GTK_FILL,  0,
		    0,                      0);

  /* Show text widget info in the statusbar */
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (contents));
  
  g_signal_connect_object (buffer,
			   "changed",
			   G_CALLBACK (buffer_changed_callback),
			   NULL,
			   0);
  
  g_signal_connect_object (buffer,
			   "mark_set", /* cursor moved */
			   G_CALLBACK (mark_set_callback),
			   NULL,
			   0);
  
  g_signal_connect_object (main_window, 
			   "window_state_event", 
			   G_CALLBACK (update_resize_grip),
			   statusbar,
			   0);
  
  update_ui ();
  
  gtk_widget_show_all (main_window);
}

int
main (int argc, char **argv)
{
  gtk_init (&argc, &argv);

  create_window ();

  if (argc == 2)
    load_file (argv[1]);
  
  gtk_main ();
  return 0;
}
