#include "eggtoolpalette.h"

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <string.h>

static void
not_implemented (GtkAction *action,
                 GtkWindow *parent)
{
  GtkWidget *dialog = gtk_message_dialog_new (parent,
                                              GTK_DIALOG_MODAL |
                                              GTK_DIALOG_DESTROY_WITH_PARENT,
                                              GTK_MESSAGE_INFO,
                                              GTK_BUTTONS_CLOSE,
                                              _("Not implemented yet."));
  gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
                                            _("Sorry, the '%s' action is not implemented."),
                                            gtk_action_get_name (action));

  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
}

static void
load_stock_items (EggToolPalette *palette)
{
  const gchar *group = NULL;
  GtkToolItem *item;
  GSList *stock_ids;
  GSList *iter;

  stock_ids = gtk_stock_list_ids ();
  stock_ids = g_slist_sort (stock_ids, (GCompareFunc) strcmp);

  for (iter = stock_ids; iter; iter = g_slist_next (iter))
    {
      gchar *id = iter->data;

      switch (id[4])
        {
          case 'a':
            group = "stock-icons-af";
            break;

          case 'g':
            group = "stock-icons-gn";
            break;

          case 'o':
            group = "stock-icons-or";
            break;

          case 's':
            group = "stock-icons-sz";
            break;
        }

      item = gtk_tool_button_new_from_stock (id);
      gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (item), id);
      egg_tool_palette_insert (palette, group, item, -1);

      g_free (id);
    }

  g_slist_free (stock_ids);
}

static void
contents_drop_cb (EggToolPalette *palette G_GNUC_UNUSED,
                  GtkToolItem    *item,
                  gpointer        data)
{
  GtkTextBuffer *buffer = gtk_text_view_get_buffer (data);
  const gchar *stock_id = gtk_tool_button_get_stock_id (GTK_TOOL_BUTTON (item));
  gtk_text_buffer_set_text (buffer, stock_id, -1);
}

static GtkWidget*
create_ui (void)
{
  static const gchar ui_spec[] = "              \
    <ui>                                        \
      <menubar>                                 \
        <menu action='FileMenu'>                \
          <menuitem action='FileNew' />         \
          <menuitem action='FileOpen' />        \
          <separator />                         \
          <menuitem action='FileSave' />        \
          <menuitem action='FileSaveAs' />      \
          <separator />                         \
          <menuitem action='FileClose' />       \
          <menuitem action='FileQuit' />        \
        </menu>                                 \
                                                \
        <menu action='HelpMenu'>                \
          <menuitem action='HelpAbout' />       \
        </menu>                                 \
      </menubar>                                \
                                                \
      <toolbar>                                 \
        <toolitem action='FileNew' />           \
        <toolitem action='FileOpen' />          \
        <toolitem action='FileSave' />          \
        <separator />                           \
        <toolitem action='HelpAbout' />         \
      </toolbar>                                \
    </ui>";

  static GtkActionEntry actions[] = {
    { "FileMenu",   NULL, N_("_File"),       NULL, NULL, NULL },
    { "FileNew",    GTK_STOCK_NEW, NULL,     NULL, NULL, G_CALLBACK (not_implemented) },
    { "FileOpen",   GTK_STOCK_OPEN, NULL,    NULL, NULL, G_CALLBACK (not_implemented) },
    { "FileSave",   GTK_STOCK_SAVE, NULL,    NULL, NULL, G_CALLBACK (not_implemented) },
    { "FileSaveAs", GTK_STOCK_SAVE_AS, NULL, NULL, NULL, G_CALLBACK (not_implemented) },
    { "FileClose",  GTK_STOCK_CLOSE, NULL,   NULL, NULL, G_CALLBACK (not_implemented) },
    { "FileQuit",   GTK_STOCK_QUIT, NULL,    NULL, NULL, G_CALLBACK (gtk_main_quit) },
    { "HelpMenu",   NULL, N_("_Help"),       NULL, NULL, NULL },
    { "HelpAbout",  GTK_STOCK_ABOUT, NULL,   NULL, NULL, G_CALLBACK (not_implemented) },
  };

  GtkActionGroup *group;
  GError *error = NULL;
  GtkUIManager *ui;

  GtkWidget *window, *vbox;
  GtkWidget *menubar, *toolbar, *hpaned;
  GtkWidget *palette, *palette_scroller;
  GtkWidget *contents, *contents_scroller;

  /* ===== menubar/toolbar ===== */

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  group = gtk_action_group_new ("");
  ui = gtk_ui_manager_new ();

  gtk_action_group_add_actions (group, actions, G_N_ELEMENTS (actions), window);
  gtk_ui_manager_insert_action_group (ui, group, -1);

  if (!gtk_ui_manager_add_ui_from_string (ui, ui_spec, sizeof ui_spec - 1, &error))
    {
      g_message ("building ui_spec failed: %s", error->message);
      g_clear_error (&error);
    }

  menubar = gtk_ui_manager_get_widget (ui, "/menubar");
  toolbar = gtk_ui_manager_get_widget (ui, "/toolbar");

  /* ===== palette ===== */

  palette = egg_tool_palette_new ();

  load_stock_items (EGG_TOOL_PALETTE (palette));

  egg_tool_palette_set_category_name (EGG_TOOL_PALETTE (palette),
                                      "stock-icons-af", _("Stock Icons (A-F)"));
  egg_tool_palette_set_category_name (EGG_TOOL_PALETTE (palette),
                                      "stock-icons-gn", _("Stock Icons (G-N)"));
  egg_tool_palette_set_category_name (EGG_TOOL_PALETTE (palette),
                                      "stock-icons-or", _("Stock Icons (O-R)"));
  egg_tool_palette_set_category_name (EGG_TOOL_PALETTE (palette),
                                      "stock-icons-sz", _("Stock Icons (S-Z)"));

  egg_tool_palette_set_drag_source (EGG_TOOL_PALETTE (palette));

  palette_scroller = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (palette_scroller),
                                  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_container_set_border_width (GTK_CONTAINER (palette_scroller), 6);
  gtk_container_add (GTK_CONTAINER (palette_scroller), palette);

  /* ===== contents ===== */

  contents = gtk_text_view_new ();
  gtk_widget_set_size_request (contents, 100, 400);
  egg_tool_palette_add_drag_dest (EGG_TOOL_PALETTE (palette),
                                  contents, contents_drop_cb,
                                  contents);

  contents_scroller = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (contents_scroller),
                                  GTK_POLICY_ALWAYS, GTK_POLICY_ALWAYS);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (contents_scroller),
                                       GTK_SHADOW_IN);
  gtk_container_add (GTK_CONTAINER (contents_scroller), contents);
  gtk_container_set_border_width (GTK_CONTAINER (contents_scroller), 6);

  /* ===== hpaned ===== */

  hpaned = gtk_hpaned_new ();
  gtk_paned_pack1 (GTK_PANED (hpaned), palette_scroller, FALSE, FALSE);
  gtk_paned_pack2 (GTK_PANED (hpaned), contents_scroller, TRUE, FALSE);

  /* ===== vbox ===== */

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), menubar, FALSE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), toolbar, FALSE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hpaned, TRUE, TRUE, 0);
  gtk_widget_show_all (vbox);

  /* ===== window ===== */

  gtk_container_add (GTK_CONTAINER (window), vbox);
  gtk_window_set_default_size (GTK_WINDOW (window), 600, 500);
  gtk_window_add_accel_group (GTK_WINDOW (window), gtk_ui_manager_get_accel_group (ui));
  g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

  g_object_unref (ui);
  return window;
}

int
main (int   argc,
      char *argv[])
{
  GtkWidget *ui;

  gtk_init (&argc, &argv);

  gtk_rc_parse_string ("                                \
    style 'egg-tool-palette-header' {                   \
      bg[NORMAL] = @selected_bg_color                   \
      fg[NORMAL] = @selected_fg_color                   \
      bg[PRELIGHT] = shade(1.04, @selected_bg_color)    \
      fg[PRELIGHT] = @selected_fg_color                 \
      bg[ACTIVE] = shade(0.9, @selected_bg_color)       \
      fg[ACTIVE] = shade(0.9, @selected_fg_color)       \
      font_name = 'Sans Serif Bold 10.'                 \
    }                                                   \
                                                        \
    widget_class '*<EggToolPalette>.GtkButton*'         \
    style 'egg-tool-palette-header'                     \
    ");

  ui = create_ui ();
  gtk_widget_show (ui);
  gtk_main ();

  return 0;
}
