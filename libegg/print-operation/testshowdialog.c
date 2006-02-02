#include <stdlib.h>

#include <eggprintunixdialog.h>

int
main (gint argc, gchar *argv[])
{
  GtkWidget *pd;

  gtk_init (&argc, &argv);

  pd = egg_print_unix_dialog_new ("Print Test", NULL, NULL);
  gtk_dialog_run (GTK_DIALOG (pd));

  return 0;
}
