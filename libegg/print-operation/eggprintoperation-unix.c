/* EGG - The GIMP Toolkit
 * eggprintoperation-unix.c: Print Operation Details for Unix and Unix like platforms
 * Copyright (C) 2006, Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "eggprintoperation-private.h"
#include "eggprintunixdialog.h"

gboolean
egg_print_operation_platfrom_backend_run_dialog (EggPrintOperation *op,
						 GtkWindow *parent,
						 gboolean *do_print)
{
  GtkWidget *pd;
  gboolean result;
  
  pd = egg_print_unix_dialog_new ("Print...", parent, NULL);

  result = FALSE;
  *do_print = FALSE; 
  if (gtk_dialog_run (GTK_DIALOG (pd)) == GTK_RESPONSE_ACCEPT)
    {
      do_print = TRUE;
      result = TRUE;
    }

  gtk_widget_destroy (pd);

  return result;
}

