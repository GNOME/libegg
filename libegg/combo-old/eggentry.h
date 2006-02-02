/* GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
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

/*
 * Modified by the GTK+ Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/. 
 */

#ifndef __EGG_ENTRY_H__
#define __EGG_ENTRY_H__


#include <gdk/gdk.h>
#include <gtk/gtkeditable.h>
#include <gtk/gtkimcontext.h>
#include <gtk/gtkliststore.h>
#include <gtk/gtktreeviewcolumn.h>
#include <gtk/gtkmenu.h>
#include <pango/pango.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define EGG_TYPE_ENTRY                  (egg_entry_get_type ())
#define EGG_ENTRY(obj)                  (GTK_CHECK_CAST ((obj), EGG_TYPE_ENTRY, EggEntry))
#define EGG_ENTRY_CLASS(klass)          (GTK_CHECK_CLASS_CAST ((klass), EGG_TYPE_ENTRY, EggEntryClass))
#define EGG_IS_ENTRY(obj)               (GTK_CHECK_TYPE ((obj), EGG_TYPE_ENTRY))
#define EGG_IS_ENTRY_CLASS(klass)       (GTK_CHECK_CLASS_TYPE ((klass), EGG_TYPE_ENTRY))
#define EGG_ENTRY_GET_CLASS(obj)        (GTK_CHECK_GET_CLASS ((obj), EGG_TYPE_ENTRY, EggEntryClass))


typedef struct _EggEntry       EggEntry;
typedef struct _EggEntryClass  EggEntryClass;

struct _EggEntry
{
  GtkWidget  widget;

  gchar     *text;

  guint      editable : 1;
  guint      visible  : 1;
  guint      overwrite_mode : 1;
  guint      in_drag : 1;	/* Dragging within the selection */

  guint16 text_length;	/* length in use, in chars */
  guint16 text_max_length;

  /*< private >*/
  GdkWindow *text_area;
  GtkIMContext *im_context;
  GtkWidget   *popup_menu;
  
  gint         current_pos;
  gint         selection_bound;
  
  PangoLayout *cached_layout;
  guint        cache_includes_preedit : 1;

  guint        need_im_reset : 1;

  guint        has_frame : 1;

  guint        activates_default : 1;

  guint        cursor_visible : 1;

  guint        in_click : 1;	/* Flag so we don't select all when clicking in entry to focus in */

  guint        is_cell_renderer : 1;
  guint        editing_canceled : 1; /* Only used by GtkCellRendererText */

  guint        mouse_cursor_obscured : 1;
  
  guint   button;
  guint   blink_timeout;
  guint   recompute_idle;
  gint    scroll_offset;
  gint    ascent;	/* font ascent, in pango units  */
  gint    descent;	/* font descent, in pango units  */
  
  guint16 text_size;	/* allocated size, in bytes */
  guint16 n_bytes;	/* length in use, in bytes */

  guint16 preedit_length;	/* length of preedit string, in bytes */
  guint16 preedit_cursor;	/* offset of cursor within preedit string, in chars */

  gint dnd_position;		/* In chars, -1 == no DND cursor */

  gint drag_start_x;
  gint drag_start_y;
  
  gunichar invisible_char;

  gint width_chars;
};

struct _EggEntryClass
{
  GtkWidgetClass parent_class;

  /* Hook to customize right-click popup */
  void (* populate_popup)   (EggEntry       *entry,
                             GtkMenu        *menu);
  
  /* Action signals
   */
  void (* activate)           (EggEntry       *entry);
  void (* move_cursor)        (EggEntry       *entry,
			       GtkMovementStep step,
			       gint            count,
			       gboolean        extend_selection);
  void (* insert_at_cursor)   (EggEntry       *entry,
			       const gchar    *str);
  void (* delete_from_cursor) (EggEntry       *entry,
			       GtkDeleteType   type,
			       gint            count);
  void (* cut_clipboard)      (EggEntry       *entry);
  void (* copy_clipboard)     (EggEntry       *entry);
  void (* paste_clipboard)    (EggEntry       *entry);
  void (* toggle_overwrite)   (EggEntry       *entry);

  /* Padding for future expansion */
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
};

typedef gboolean (* EggCompletionFunc) (const gchar *key,
					const gchar *item,
					GtkTreeIter *iter,
					gpointer     user_data);


GtkType    egg_entry_get_type       		(void) G_GNUC_CONST;
GtkWidget* egg_entry_new            		(void);
void       egg_entry_set_visibility 		(EggEntry      *entry,
						 gboolean       visible);
gboolean   egg_entry_get_visibility             (EggEntry      *entry);
void       egg_entry_set_invisible_char         (EggEntry      *entry,
                                                 gunichar       ch);
gunichar   egg_entry_get_invisible_char         (EggEntry      *entry);
void       egg_entry_set_has_frame              (EggEntry      *entry,
                                                 gboolean       setting);
gboolean   egg_entry_get_has_frame              (EggEntry      *entry);
/* text is truncated if needed */
void       egg_entry_set_max_length 		(EggEntry      *entry,
						 gint           max);
gint       egg_entry_get_max_length             (EggEntry      *entry);
void       egg_entry_set_activates_default      (EggEntry      *entry,
                                                 gboolean       setting);
gboolean   egg_entry_get_activates_default      (EggEntry      *entry);

void       egg_entry_set_width_chars            (EggEntry      *entry,
                                                 gint           n_chars);
gint       egg_entry_get_width_chars            (EggEntry      *entry);

/* Somewhat more convenient than the GtkEditable generic functions
 */
void                  egg_entry_set_text        (EggEntry      *entry,
                                                 const gchar   *text);
/* returns a reference to the text */
G_CONST_RETURN gchar* egg_entry_get_text        (EggEntry      *entry);

PangoLayout* egg_entry_get_layout               (EggEntry      *entry);
void         egg_entry_get_layout_offsets       (EggEntry      *entry,
                                                 gint          *x,
                                                 gint          *y);

/* Completion
 */

void         egg_entry_enable_completion        (EggEntry          *entry,
                                                 GtkListStore      *model,
						 gint               list_column,
						 gint               entry_column,
						 EggCompletionFunc  func,
						 gpointer           func_data,
						 GtkDestroyNotify   func_destroy);
gboolean     egg_entry_completion_enabled       (EggEntry          *entry);


GtkTreeViewColumn *egg_entry_completion_get_column (EggEntry       *entry);
void               egg_entry_completion_get_model  (EggEntry       *entry,
		                                    GtkTreeModel  **model,
						    gint           *list_column);

/* History
 */

void          egg_entry_history_set_max         (EggEntry     *entry,
		                                 gint          max);
gint          egg_entry_history_get_max         (EggEntry     *entry);

/* Deprecated compatibility functions
 */

#ifndef GTK_DISABLE_DEPRECATED
GtkWidget* egg_entry_new_with_max_length	(gint           max);
void       egg_entry_append_text    		(EggEntry      *entry,
						 const gchar   *text);
void       egg_entry_prepend_text   		(EggEntry      *entry,
						 const gchar   *text);
void       egg_entry_set_position   		(EggEntry      *entry,
						 gint           position);
void       egg_entry_select_region  		(EggEntry      *entry,
						 gint           start,
						 gint           end);
void       egg_entry_set_editable   		(EggEntry      *entry,
						 gboolean       editable);
#endif /* GTK_DISABLE_DEPRECATED */

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __EGG_ENTRY_H__ */
