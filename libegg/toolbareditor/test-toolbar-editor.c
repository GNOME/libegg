#include <gtk/gtk.h>

#include "egg-editable-toolbar.h"
#include "egg-toolbars-model.h"
#include "egg-toolbar-editor.h"

#include <glib/gi18n.h>

static void
activate_action (GtkAction *action)
{
}

static GtkActionEntry entries[] = {
  {"StockEditMenuAction", NULL, N_("_Edit")},

  {"NewAction", GTK_STOCK_NEW, N_("New"), "<control>n", NULL,
   G_CALLBACK (activate_action)},
  {"New2Action", GTK_STOCK_NEW, N_("New2"), "<control>m", NULL,
   G_CALLBACK (activate_action)},
  {"OpenAction", GTK_STOCK_OPEN, N_("Open"), "<control>o", NULL,
   G_CALLBACK (activate_action)},
  {"QuitAction", GTK_STOCK_QUIT, N_("Quit"), "<control>q", NULL,
   G_CALLBACK (gtk_main_quit)},
  {"CutAction", GTK_STOCK_CUT, N_("Cut"), "<control>x", NULL,
   G_CALLBACK (activate_action)},
  {"CopyAction", GTK_STOCK_COPY, N_("Copy"), "<control>c", NULL,
   G_CALLBACK (activate_action)},
  {"PasteAction", GTK_STOCK_PASTE, N_("Paste"), "<control>v", NULL,
   G_CALLBACK (activate_action)},
};
static guint n_entries = G_N_ELEMENTS (entries);

static GtkUIManager *manager = NULL;
static EggToolbarsModel *model = NULL;

static void
editor_destroy_cb (GtkWidget *editor, EggEditableToolbar *toolbar)
{
  egg_editable_toolbar_set_edit_mode (toolbar, FALSE);
}

static void
edit (GtkWidget *button, EggEditableToolbar *toolbar)
{
  GtkWidget *dialog;
  GtkWidget *editor;

  dialog = gtk_dialog_new ();
  gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);
  gtk_window_set_title (GTK_WINDOW (dialog), "Toolbar editor");

  editor = egg_toolbar_editor_new (manager, model);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), editor);
  gtk_widget_show (editor);
  g_signal_connect (editor, "destroy", G_CALLBACK (editor_destroy_cb), toolbar);
  egg_editable_toolbar_set_edit_mode (toolbar, TRUE);

  gtk_widget_show (dialog);
}

int
main (int argc, char **argv)
{
  GtkActionGroup *action_group;
  GtkWidget *window, *vbox, *button;
  GtkWidget *toolbar;
  GtkAction *action;

  gtk_init (&argc, &argv);

  action_group = gtk_action_group_new ("TestActions");
  gtk_action_group_add_actions (action_group, entries, n_entries, NULL);

  action = gtk_action_group_get_action (action_group, "CopyAction");
  g_object_set (action, "sensitive", FALSE, NULL);
  action = gtk_action_group_get_action (action_group, "PasteAction");
  g_object_set (action, "sensitive", FALSE, NULL);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size (GTK_WINDOW (window), 400, -1);
  g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (window), vbox);
  gtk_widget_show (vbox);

  manager = gtk_ui_manager_new ();
  gtk_ui_manager_insert_action_group (manager, action_group, 0);
  g_object_unref (action_group);

  model = egg_toolbars_model_new ();
  egg_toolbars_model_load_toolbars (model, "test-toolbar.xml");
  egg_toolbars_model_load_names (model, "test-toolbar.xml");

  toolbar = egg_editable_toolbar_new_with_model (manager, model, NULL);
  gtk_box_pack_start (GTK_BOX (vbox), toolbar, FALSE, FALSE, 0);
  gtk_widget_show (toolbar);
  egg_editable_toolbar_show (EGG_EDITABLE_TOOLBAR (toolbar), "Toolbar1");
  egg_editable_toolbar_show (EGG_EDITABLE_TOOLBAR (toolbar), "Toolbar2");

  button = gtk_button_new_with_mnemonic ("_Edit");
  g_signal_connect (button, "clicked", G_CALLBACK (edit), toolbar);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  gtk_widget_show (button);

  gtk_window_add_accel_group (GTK_WINDOW (window),
			      gtk_ui_manager_get_accel_group (manager));

  gtk_widget_show (window);
  gtk_main ();

  return 0;
}
