/* eggiconchooserdefault.c
 * Copyright (C) 2004  James M. Cape  <jcape@ignore-your.tv>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */

#include <glib/gi18n.h>

#include <string.h>

#include <gtk/gtk.h>

#include "eggiconchoosertypebuiltins.h"
#include "egg-pixbuf-thumbnail.h"
#include "eggthumbnailpreview.h"
#include "gtkfilechooserembed.h"
#include "gtkfilechooserprivate.h"
#include "eggiconchooserutils.h"
#include "eggiconchooserdefault.h"

#define MISSING_IMAGE_NAME		"gnome-dev-broken-image"
#define FALLBACK_COMBO_ICON_SIZE	16
#define FALLBACK_ICON_SIZE		48
#define MINIMUM_ICON_SIZE		16
#define SPINNER_TIMEOUT			100


/* ************** *
 *  Enumerations  *
 * ************** */

enum
{
  CONTEXT_ICON_COLUMN,
  CONTEXT_NAME_COLUMN,
  CONTEXT_ID_COLUMN,
  CONTEXT_SORT_GROUP_COLUMN,
  N_CONTEXT_COLUMNS
};

enum
{
  ICON_CONTEXT_COLUMN,
  ICON_HASH_COLUMN,
  ICON_NAME_COLUMN,
  ICON_DISPLAY_NAME_COLUMN,
  ICON_PIXBUF_COLUMN,
  ICON_LOADED_COLUMN,
  N_ICON_COLUMNS
};

enum
{
  CONTEXT_SORT_ALL,
  CONTEXT_SORT_ALL_SEPARATOR,
  CONTEXT_SORT_CONTEXTS,
  CONTEXT_SORT_UNCATEGORIZED,
  CONTEXT_SORT_CONTEXTS_SEPARATOR,
  CONTEXT_SORT_FILE
};


/* *************** *
 *  Private Types  *
 * *************** */

struct _EggIconChooserDefault
{
  GtkVBox parent;

  /* Context Combo */
  GtkWidget *context_combo;
  GtkCellRenderer *context_icon_cell;

  /* Container Notebook */
  GtkWidget *notebook;

  /* Progress Bar */
  GtkWidget *progress;
  guint n_items;
  guint items_loaded;
  guint load_id;

  /* Animation Overlay */
  GtkWidget *anim_box;
  GtkWidget *anim_image;
  GdkPixbuf *anim_pixbuf;
  gint frame_x;
  gint frame_y;
  guint anim_id;

  /* Themed Icon Page Contents */
  GtkWidget *icon_view;
  GtkListStore *store;
  GtkIconTheme *theme;
  GSList *selected_icons; /* So we don't loose our selection when reloading */
  gulong icon_view_selection_changed_id;
  gint icon_size;
  gint real_icon_size;

  /* "Custom" Page */
  GtkWidget *file_chooser;
  GtkWidget *preview;
  gchar *file_system;

  guint8 context:4;
  guint8 show_names:1;
  guint8 allow_custom:1;
};

typedef struct _EggIconChooserDefaultClass
{
  GtkVBoxClass parent_class;
}
EggIconChooserDefaultClass;


/* ************ *
 *  Prototypes  *
 * ************ */

/* GType */
static void egg_icon_chooser_default_class_init (EggIconChooserDefaultClass *class);
static void egg_icon_chooser_default_init       (EggIconChooserDefault      *chooser);

/* GObject */
static GObject *egg_icon_chooser_default_constructor  (GType                  type,
						       guint                  n_params,
						       GObjectConstructParam *params);
static void     egg_icon_chooser_default_set_property (GObject               *object,
						       guint                  param_id,
						       const GValue          *value,
						       GParamSpec            *pspec);
static void     egg_icon_chooser_default_get_property (GObject               *object,
						       guint                  param_id,
						       GValue                *value,
						       GParamSpec            *pspec);
static void     egg_icon_chooser_default_finalize     (GObject               *object);

/* GtkWidget */
static void egg_icon_chooser_default_style_set      (GtkWidget *widget,
						     GtkStyle  *old_style);
static void egg_icon_chooser_default_screen_changed (GtkWidget *widget,
						     GdkScreen *old_screen);

/* EggIconChooserIface */
static void           egg_icon_chooser_default_iface_init      (EggIconChooserIface  *iface);
static void           egg_icon_chooser_default_unselect_all    (EggIconChooser       *icon_chooser);
static gboolean       egg_icon_chooser_default_select_icon     (EggIconChooser       *icon_chooser,
								const gchar          *icon_name);
static void           egg_icon_chooser_default_unselect_icon   (EggIconChooser       *icon_chooser,
								const gchar          *icon_name);
static GSList        *egg_icon_chooser_default_get_icons       (EggIconChooser       *icon_chooser);
static GtkFileSystem *egg_icon_chooser_default_get_file_system (EggIconChooser       *icon_chooser);
static gboolean       egg_icon_chooser_default_select_path     (EggIconChooser       *icon_chooser,
								const GtkFilePath    *path,
								GError              **error);
static void           egg_icon_chooser_default_unselect_path   (EggIconChooser       *icon_chooser,
								const GtkFilePath    *path);
static GSList        *egg_icon_chooser_default_get_paths       (EggIconChooser       *icon_chooser);
static void           egg_icon_chooser_default_add_filter      (EggIconChooser       *icon_chooser,
								GtkFileFilter        *filter);
static void           egg_icon_chooser_default_remove_filter   (EggIconChooser       *icon_chooser,
								GtkFileFilter        *filter);
static GSList        *egg_icon_chooser_default_list_filters    (EggIconChooser       *icon_chooser);

/* GtkFileChooserEmbedIface */
static void     egg_icon_chooser_default_embed_iface_init    (GtkFileChooserEmbedIface *iface);
static void     egg_icon_chooser_default_get_default_size    (GtkFileChooserEmbed      *embed,
							      gint                     *width,
							      gint                     *height);
static void     egg_icon_chooser_default_get_resizable_hints (GtkFileChooserEmbed      *embed,
							      gboolean                 *horizontally,
							      gboolean                 *vertically);
static gboolean egg_icon_chooser_default_should_respond      (GtkFileChooserEmbed      *embed);
static void     egg_icon_chooser_default_initial_focus       (GtkFileChooserEmbed      *embed);

/* Context ComboBox */
static gboolean context_combo_row_is_separator (GtkTreeModel    *model,
						GtkTreeIter     *iter,
						gpointer         user_data);
static gint     context_combo_sort_func        (GtkTreeModel    *model,
						GtkTreeIter     *iter1,
						GtkTreeIter     *iter2,
						gpointer         user_data);
static void     context_combo_changed          (GtkComboBox     *combo,
						gpointer         user_data);

/* IconView */
static void     icon_view_style_set         (GtkWidget    *icon_view,
					     GtkStyle     *old_style,
					     gpointer      user_data);
static void     icon_view_selection_changed (GtkIconView  *icon_view,
					     gpointer      user_data);
static void     icon_view_item_activated    (GtkIconView  *icon_view,
					     GtkTreePath  *path,
					     gpointer      user_data);
static gboolean icon_item_is_visible        (GtkTreeModel *model,
					     GtkTreeIter  *iter,
					     gpointer      user_data);
static gint     icon_model_unsorted         (GtkTreeModel *model,
					     GtkTreeIter  *iter1,
					     GtkTreeIter  *iter2,
					     gpointer      user_data);

/* FileChooser */
static void file_chooser_selection_changed    (GtkFileChooser *file_chooser,
					       gpointer        user_data);
static void file_chooser_file_activated       (GtkFileChooser *file_chooser,
					       gpointer        user_data);
static void file_chooser_update_preview       (GtkFileChooser *file_chooser,
					       gpointer        user_data);
static void file_chooser_default_size_changed (GtkFileChooser *file_chooser,
					       gpointer        user_data);

/* Loading */
static void reload_icon_view (EggIconChooserDefault *chooser);

/* Misc */
static GtkIconTheme *get_icon_theme_for_widget (GtkWidget      *widget);
static gint          get_icon_size_for_widget  (GtkWidget      *widget);


static gpointer parent_class = NULL;


/* ***************** *
 *  GType Functions  *
 * ***************** */

GType
_egg_icon_chooser_default_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static GTypeInfo info =
	{
	  sizeof (EggIconChooserDefaultClass),
	  NULL,	/* base_init */
	  NULL,	/* base_finalize */
	  (GClassInitFunc) egg_icon_chooser_default_class_init,
	  NULL,	/* class_finalize */
	  NULL,	/* class_data */
	  sizeof (EggIconChooserDefault),
	  0	/* n_preallocs */,
	  (GInstanceInitFunc) egg_icon_chooser_default_init,
	  NULL	/* value_table */
	};
      static GInterfaceInfo icon_chooser_info =
	{
	  (GInterfaceInitFunc) egg_icon_chooser_default_iface_init,
	  NULL, /* iface_finalize */
	  NULL, /* iface_data */
	};
      static GInterfaceInfo file_chooser_embed_info =
	{
	  (GInterfaceInitFunc) egg_icon_chooser_default_embed_iface_init,
	  NULL, /* iface_finalize */
	  NULL, /* iface_data */
	};

      type = g_type_register_static (GTK_TYPE_VBOX, "EggIconChooserDefault", &info, 0);
      g_type_add_interface_static (type, EGG_TYPE_ICON_CHOOSER, &icon_chooser_info);
      g_type_add_interface_static (type, GTK_TYPE_FILE_CHOOSER_EMBED, &file_chooser_embed_info);
    }

  return type;
}


static void
egg_icon_chooser_default_class_init (EggIconChooserDefaultClass *class)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (class);

  parent_class = g_type_class_peek_parent (class);

  gobject_class->constructor = egg_icon_chooser_default_constructor;
  gobject_class->set_property = egg_icon_chooser_default_set_property;
  gobject_class->get_property = egg_icon_chooser_default_get_property;
  gobject_class->finalize = egg_icon_chooser_default_finalize;

  widget_class->style_set = egg_icon_chooser_default_style_set;
  widget_class->screen_changed = egg_icon_chooser_default_screen_changed;
  widget_class->show_all = gtk_widget_show;
  widget_class->hide_all = gtk_widget_hide;

  _egg_icon_chooser_install_properties (gobject_class);
}

static void
egg_icon_chooser_default_init (EggIconChooserDefault *chooser)
{
  GtkListStore *store;
  GtkWidget *box, *label, *table, *scrwin;
  GtkCellRenderer *cell;
  GtkTreeIter iter;

  gtk_box_set_spacing (GTK_BOX (chooser), 12);

  chooser->theme = NULL;
  chooser->icon_size = -1;
  chooser->real_icon_size = FALLBACK_ICON_SIZE;
  chooser->context = EGG_ICON_CONTEXT_ALL;
  chooser->allow_custom = TRUE;

  gtk_widget_push_composite_child ();

  /* Top Row */
  box = gtk_hbox_new (FALSE, 6);
  gtk_box_pack_start (GTK_BOX (chooser), box, FALSE, FALSE, 0);
  gtk_widget_show (box);

  /* Label + Combo */
  label = gtk_label_new (_("Select icon from:"));
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  chooser->context_combo = gtk_combo_box_new ();
  gtk_box_pack_start (GTK_BOX (box), chooser->context_combo,
		      TRUE, TRUE, 0);
  gtk_widget_show (chooser->context_combo);

  chooser->context_icon_cell = g_object_new (GTK_TYPE_CELL_RENDERER_PIXBUF,
					     "xalign", 0.5, NULL);
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (chooser->context_combo),
			      chooser->context_icon_cell, FALSE);
  gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (chooser->context_combo),
				 chooser->context_icon_cell,
				 "pixbuf", CONTEXT_ICON_COLUMN);

  cell = g_object_new (GTK_TYPE_CELL_RENDERER_TEXT, NULL);
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (chooser->context_combo), cell,
			      TRUE);
  gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (chooser->context_combo), cell,
				 "text", CONTEXT_NAME_COLUMN);

  /* Notebook */
  chooser->notebook = gtk_notebook_new ();
  gtk_notebook_set_show_border (GTK_NOTEBOOK (chooser->notebook), FALSE);
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (chooser->notebook), FALSE);
  gtk_container_add (GTK_CONTAINER (chooser), chooser->notebook);
  gtk_widget_show (chooser->notebook);

  /* Icon Page Contents */
  table = gtk_table_new (3, 1, FALSE);
  gtk_notebook_append_page (GTK_NOTEBOOK (chooser->notebook),
			    table, NULL);
  gtk_widget_show (table);

  /* Animation Widgets */
  chooser->anim_box = gtk_event_box_new ();
  gtk_table_attach (GTK_TABLE (table), chooser->anim_box,
		    0, 1, 1, 2, 0, 0, 0, 0);

  box = gtk_vbox_new (FALSE, 6);
  gtk_container_add (GTK_CONTAINER (chooser->anim_box), box);
  gtk_widget_show (box);

  chooser->anim_image = gtk_image_new ();
  gtk_container_add (GTK_CONTAINER (box), chooser->anim_image);
  gtk_widget_show (chooser->anim_image);

  /* Progress Bar */
  chooser->progress = gtk_progress_bar_new ();
  gtk_container_add (GTK_CONTAINER (box), chooser->progress);
  gtk_widget_show (chooser->progress);

  /* Icon View */
  scrwin = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrwin),
				       GTK_SHADOW_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrwin),
				  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_table_attach (GTK_TABLE (table), scrwin, 0, 1, 0, 3,
		    (GTK_FILL | GTK_EXPAND), (GTK_FILL | GTK_EXPAND), 0, 0);
  gtk_widget_show (scrwin);

  chooser->icon_view = gtk_icon_view_new ();
  gtk_icon_view_set_pixbuf_column (GTK_ICON_VIEW (chooser->icon_view),
				   ICON_PIXBUF_COLUMN);
  gtk_icon_view_set_text_column (GTK_ICON_VIEW (chooser->icon_view), -1);
  gtk_icon_view_set_row_spacing (GTK_ICON_VIEW (chooser->icon_view), 12);
  gtk_icon_view_set_column_spacing (GTK_ICON_VIEW (chooser->icon_view), 12);
  g_signal_connect (chooser->icon_view, "style-set",
		    G_CALLBACK (icon_view_style_set), chooser->anim_box);
  g_signal_connect (chooser->icon_view, "item-activated",
		    G_CALLBACK (icon_view_item_activated), chooser);
  chooser->icon_view_selection_changed_id =
    g_signal_connect (chooser->icon_view, "selection-changed",
		      G_CALLBACK (icon_view_selection_changed), chooser);
  gtk_container_add (GTK_CONTAINER (scrwin), chooser->icon_view);
  gtk_widget_show (chooser->icon_view);

  gtk_widget_pop_composite_child ();

  /* Groups Model */
  store = gtk_list_store_new (N_CONTEXT_COLUMNS,
			      GDK_TYPE_PIXBUF,	     /* Icon */
			      G_TYPE_STRING,	     /* Display Name */
			      EGG_TYPE_ICON_CONTEXT, /* Context */
			      G_TYPE_UINT            /* Sort Grouping */);

  /* All Icons */
  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
		      CONTEXT_ICON_COLUMN, NULL,
		      CONTEXT_NAME_COLUMN, _("All Icons"),
		      CONTEXT_ID_COLUMN, EGG_ICON_CONTEXT_ALL,
		      CONTEXT_SORT_GROUP_COLUMN, CONTEXT_SORT_ALL,
		      -1);

  /* Separator 1 */
  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
		      CONTEXT_ICON_COLUMN, NULL,
		      CONTEXT_NAME_COLUMN, NULL,
		      CONTEXT_ID_COLUMN, EGG_ICON_CONTEXT_INVALID,
		      CONTEXT_SORT_GROUP_COLUMN, CONTEXT_SORT_ALL_SEPARATOR,
		      -1);

  /* Contexts */
  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
		      CONTEXT_ICON_COLUMN, NULL,
		      CONTEXT_NAME_COLUMN, _("Applications"),
		      CONTEXT_ID_COLUMN, EGG_ICON_CONTEXT_APPLICATIONS,
		      CONTEXT_SORT_GROUP_COLUMN, CONTEXT_SORT_CONTEXTS,
		      -1);

  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
		      CONTEXT_ICON_COLUMN, NULL,
		      CONTEXT_NAME_COLUMN, _("Types of Files"),
		      CONTEXT_ID_COLUMN, EGG_ICON_CONTEXT_MIMETYPES,
		      CONTEXT_SORT_GROUP_COLUMN, CONTEXT_SORT_CONTEXTS,
		      -1);

  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
		      CONTEXT_ICON_COLUMN, NULL,
		      CONTEXT_NAME_COLUMN, _("Emblems"),
		      CONTEXT_ID_COLUMN, EGG_ICON_CONTEXT_EMBLEMS,
		      CONTEXT_SORT_GROUP_COLUMN, CONTEXT_SORT_CONTEXTS,
		      -1);

  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
		      CONTEXT_ICON_COLUMN, NULL,
		      CONTEXT_NAME_COLUMN, _("Locations and Media"),
		      CONTEXT_ID_COLUMN, EGG_ICON_CONTEXT_LOCATIONS,
		      CONTEXT_SORT_GROUP_COLUMN, CONTEXT_SORT_CONTEXTS,
		      -1);

  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
		      CONTEXT_ICON_COLUMN, NULL,
		      CONTEXT_NAME_COLUMN, _("Toolbars and Menus"),
		      CONTEXT_ID_COLUMN, EGG_ICON_CONTEXT_STOCK,
		      CONTEXT_SORT_GROUP_COLUMN, CONTEXT_SORT_CONTEXTS,
		      -1);

  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
		      CONTEXT_ICON_COLUMN, NULL,
		      CONTEXT_NAME_COLUMN, _("Uncategorized"),
		      CONTEXT_ID_COLUMN, EGG_ICON_CONTEXT_UNCATEGORIZED,
		      CONTEXT_SORT_GROUP_COLUMN, CONTEXT_SORT_UNCATEGORIZED,
		      -1);

  /* Separator 2 */
  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
		      CONTEXT_ICON_COLUMN, NULL,
		      CONTEXT_NAME_COLUMN, NULL,
		      CONTEXT_ID_COLUMN, EGG_ICON_CONTEXT_STOCK,
		      CONTEXT_SORT_GROUP_COLUMN, CONTEXT_SORT_CONTEXTS_SEPARATOR,
		      -1);

  /* Custom */
  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
		      CONTEXT_ICON_COLUMN, NULL,
		      CONTEXT_NAME_COLUMN, _("Other"),
		      CONTEXT_ID_COLUMN, EGG_ICON_CONTEXT_FILE,
		      CONTEXT_SORT_GROUP_COLUMN, CONTEXT_SORT_FILE, -1);

  gtk_tree_sortable_set_default_sort_func (GTK_TREE_SORTABLE (store),
					   context_combo_sort_func, NULL, NULL);
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (store),
					GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID,
					GTK_SORT_ASCENDING);

  gtk_combo_box_set_model (GTK_COMBO_BOX (chooser->context_combo),
			   GTK_TREE_MODEL (store));
  gtk_combo_box_set_row_separator_func (GTK_COMBO_BOX (chooser->context_combo),
					context_combo_row_is_separator,
					NULL, NULL);
  g_object_unref (store);
  gtk_combo_box_set_active (GTK_COMBO_BOX (chooser->context_combo), 0);
  g_signal_connect (chooser->context_combo, "changed",
		    G_CALLBACK (context_combo_changed), chooser);

  /* Icons Model */
  chooser->store = gtk_list_store_new (N_ICON_COLUMNS,
				       EGG_TYPE_ICON_CONTEXT, /* Context */
				       G_TYPE_UINT,           /* Name Hash */
				       G_TYPE_STRING,         /* Name */
				       G_TYPE_STRING,         /* Label */
				       GDK_TYPE_PIXBUF,       /* Pixbuf */
				       G_TYPE_BOOLEAN         /* Is Loaded */);
  gtk_tree_sortable_set_default_sort_func (GTK_TREE_SORTABLE (chooser->store),
					   icon_model_unsorted, NULL, NULL);
}


/* ******************* *
 *  GObject Functions  *
 * ******************* */

static GObject *
egg_icon_chooser_default_constructor (GType                  type,
				      guint                  n_params,
				      GObjectConstructParam *params)
{
  GObject *object;
  EggIconChooserDefault *chooser;
  GtkFileFilter *filter;
  GSList *formats;

  object = (*G_OBJECT_CLASS (parent_class)->constructor) (type, n_params,
							  params);
  chooser = EGG_ICON_CHOOSER_DEFAULT (object);

  gtk_widget_push_composite_child ();

  /* "Custom" GtkFileChooser Page -- created here to use custom GtkFileSystem */
  chooser->file_chooser =
    gtk_file_chooser_widget_new_with_backend (GTK_FILE_CHOOSER_ACTION_OPEN,
					      chooser->file_system);
  g_free (chooser->file_system);
  _gtk_file_chooser_embed_set_delegate (GTK_FILE_CHOOSER_EMBED (chooser),
					GTK_FILE_CHOOSER_EMBED (chooser->file_chooser));
  g_signal_connect (chooser->file_chooser, "selection-changed",
		    G_CALLBACK (file_chooser_selection_changed), chooser);
  g_signal_connect (chooser->file_chooser, "file-activated",
		    G_CALLBACK (file_chooser_file_activated), chooser);
  g_signal_connect (chooser->file_chooser, "default-size-changed",
		    G_CALLBACK (file_chooser_default_size_changed), chooser);
  g_free (chooser->file_system);
  chooser->file_system = NULL;
  gtk_notebook_append_page (GTK_NOTEBOOK (chooser->notebook),
			    chooser->file_chooser, NULL);
  gtk_widget_show (chooser->file_chooser);

  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, _("All Images"));
  for (formats = gdk_pixbuf_get_formats ();
       formats != NULL;
       formats = g_slist_remove_link (formats, formats))
    {
      gchar **mime_types;
      guint i;

      if (formats->data != NULL)
	mime_types = gdk_pixbuf_format_get_mime_types (formats->data);
      else
	mime_types = NULL;

      if (mime_types != NULL)
	{
	  for (i = 0; mime_types[i] != NULL; i++)
	    {
	      gtk_file_filter_add_mime_type (filter, mime_types[i]);
	      g_free (mime_types[i]);
	    }
	  g_free (mime_types);
	}
    }
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser->file_chooser),
			       filter);

  chooser->preview = egg_thumbnail_preview_new ();
  gtk_file_chooser_set_preview_widget (GTK_FILE_CHOOSER (chooser->file_chooser),
				       chooser->preview);
  gtk_file_chooser_set_preview_widget_active (GTK_FILE_CHOOSER (chooser->file_chooser),
					      FALSE);
  g_signal_connect (chooser->file_chooser, "update-preview",
		    G_CALLBACK (file_chooser_update_preview), chooser->preview);

  gtk_widget_pop_composite_child ();

  return object;
}

static void
egg_icon_chooser_default_set_property (GObject      *object,
				       guint         param_id,
				       const GValue *value,
				       GParamSpec   *pspec)
{
  EggIconChooserDefault *chooser;

  chooser = EGG_ICON_CHOOSER_DEFAULT (object);

  switch (param_id)
    {
    case EGG_ICON_CHOOSER_PROP_FILE_SYSTEM:
      chooser->file_system = g_value_dup_string (value);
      break;
    case EGG_ICON_CHOOSER_PROP_CONTEXT:
      {
	EggIconContext ctx;
	GtkTreeModel *model;
	GtkTreeIter iter;
 
	ctx = g_value_get_enum (value);
	if (ctx <= EGG_ICON_CONTEXT_INVALID ||
	    ctx >= EGG_ICON_CONTEXT_LAST)
	  {
	    return;
	  }

	model = gtk_combo_box_get_model (GTK_COMBO_BOX (chooser->context_combo));
	if (gtk_tree_model_get_iter_first (model, &iter))
	  {
	    do
	      {
		EggIconContext tmp_ctx;

		gtk_tree_model_get (model, &iter,
				    CONTEXT_ID_COLUMN, &tmp_ctx,
				    -1);

		if (ctx == tmp_ctx)
		  gtk_combo_box_set_active_iter (GTK_COMBO_BOX (chooser->context_combo),
						 &iter);
	      }
	    while (gtk_tree_model_iter_next (model, &iter));
	  }
      }
      break;
    case EGG_ICON_CHOOSER_PROP_SELECT_MULTIPLE:
      if (g_value_get_boolean (value))
	{
	  gtk_icon_view_set_selection_mode (GTK_ICON_VIEW (chooser->icon_view),
					    GTK_SELECTION_MULTIPLE);
	  gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (chooser->file_chooser),
						TRUE);
	}
      else
	{
	  gtk_icon_view_set_selection_mode (GTK_ICON_VIEW (chooser->icon_view),
					    GTK_SELECTION_SINGLE);
	  gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (chooser->file_chooser),
						FALSE);
	}
      break;

    case EGG_ICON_CHOOSER_PROP_ICON_SIZE:
      {
	gint real_icon_size;

	chooser->icon_size = g_value_get_int (value);

	if (!gtk_widget_has_screen (GTK_WIDGET (chooser)))
	  return;

	if (chooser->icon_size < 0)
	  real_icon_size = get_icon_size_for_widget (GTK_WIDGET (chooser));
	else
	  real_icon_size = MAX (chooser->icon_size, MINIMUM_ICON_SIZE);

	/* Do nothing if effective icon size is unchanged. */
	if (real_icon_size == chooser->real_icon_size)
	  return;

	chooser->real_icon_size = real_icon_size;
	reload_icon_view (chooser);
      }
      break;

    case EGG_ICON_CHOOSER_PROP_SHOW_ICON_NAME:
      chooser->show_names = !!g_value_get_boolean (value);
      if (chooser->show_names &&
	  gtk_icon_view_get_model (GTK_ICON_VIEW (chooser->icon_view)) != NULL)
	gtk_icon_view_set_text_column (GTK_ICON_VIEW (chooser->icon_view),
				       ICON_DISPLAY_NAME_COLUMN);
      else
	gtk_icon_view_set_text_column (GTK_ICON_VIEW (chooser->icon_view),
				       -1);
      break;

    case EGG_ICON_CHOOSER_PROP_ALLOW_CUSTOM:
      {
	gboolean val;
	GtkTreeModel *model;
	GtkTreeIter iter;

	val = g_value_get_boolean (value);
	model = gtk_combo_box_get_model (GTK_COMBO_BOX (chooser->context_combo));

	/* Want to disable, but custom context is in model */
	if (!val && chooser->allow_custom)
	  {
	    gint n_children;

	    /* Select another context if we are currently in the Custom context. */
	    if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (chooser->context_combo),
					       &iter))
	      {
		EggIconContext context;

		gtk_tree_model_get (model, &iter, CONTEXT_ID_COLUMN, &context, -1);

		if (context == EGG_ICON_CONTEXT_FILE)
		  gtk_combo_box_set_active (GTK_COMBO_BOX (chooser->context_combo),
					    0);
	      }

	    n_children = gtk_tree_model_iter_n_children (model, NULL);
	    if (gtk_tree_model_iter_nth_child (model, &iter, NULL, n_children - 1))
	      gtk_list_store_remove (GTK_LIST_STORE (model), &iter);

	    if (gtk_tree_model_iter_nth_child (model, &iter, NULL, n_children - 2))
	      gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
	  }
	/* Want to enable and custom context is not in model */
	else if (val && !chooser->allow_custom)
	  {
	    /* Separator 2 */
	    gtk_list_store_append (GTK_LIST_STORE (model), &iter);
	    gtk_list_store_set (GTK_LIST_STORE (model), &iter,
				CONTEXT_ICON_COLUMN, NULL,
				CONTEXT_NAME_COLUMN, NULL,
				CONTEXT_ID_COLUMN, EGG_ICON_CONTEXT_INVALID,
				CONTEXT_SORT_GROUP_COLUMN, CONTEXT_SORT_CONTEXTS_SEPARATOR,
				-1);

	    /* Custom */
	    gtk_list_store_append (GTK_LIST_STORE (model), &iter);
	    gtk_list_store_set (GTK_LIST_STORE (model), &iter,
				CONTEXT_ICON_COLUMN, NULL,
				CONTEXT_NAME_COLUMN, _("Other"),
				CONTEXT_ID_COLUMN, EGG_ICON_CONTEXT_FILE, 
				CONTEXT_SORT_GROUP_COLUMN, CONTEXT_SORT_FILE,
				-1);
	  }

	chooser->allow_custom = (val != FALSE);
      }
      break;

    case EGG_ICON_CHOOSER_PROP_CUSTOM_FILTER:
      g_object_set_property (G_OBJECT (chooser->file_chooser), "filter", value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}

static void
egg_icon_chooser_default_get_property (GObject    *object,
				       guint       param_id,
				       GValue     *value,
				       GParamSpec *pspec)
{
  EggIconChooserDefault *chooser;

  chooser = EGG_ICON_CHOOSER_DEFAULT (object);

  switch (param_id)
    {
    case EGG_ICON_CHOOSER_PROP_CONTEXT:
      {
	GtkTreeModel *model;
	GtkTreeIter iter;

	model = gtk_combo_box_get_model (GTK_COMBO_BOX (chooser->context_combo));
	if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (chooser->context_combo), &iter))
	  {
	    EggIconContext ctx;

	    gtk_tree_model_get (model, &iter, CONTEXT_ID_COLUMN, &ctx, -1);
	    g_value_set_enum (value, ctx);
	  }
	else
	  g_value_set_enum (value, EGG_ICON_CONTEXT_INVALID);
      }
      break;
    case EGG_ICON_CHOOSER_PROP_SELECT_MULTIPLE:
      g_object_get_property (G_OBJECT (chooser->file_chooser), "select-multiple", value);
      break;
    case EGG_ICON_CHOOSER_PROP_ICON_SIZE:
      g_value_set_int (value, chooser->icon_size);
      break;
    case EGG_ICON_CHOOSER_PROP_SHOW_ICON_NAME:
      g_value_set_boolean (value, chooser->show_names);
      break;
    case EGG_ICON_CHOOSER_PROP_ALLOW_CUSTOM:
      g_value_set_boolean (value, chooser->allow_custom);
      break;

    case EGG_ICON_CHOOSER_PROP_CUSTOM_FILTER:
      g_object_get_property (G_OBJECT (chooser->file_chooser), "filter", value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}

static void
egg_icon_chooser_default_finalize (GObject *object)
{
  EggIconChooserDefault *chooser;

  chooser = EGG_ICON_CHOOSER_DEFAULT (object);

  if (chooser->load_id != 0)
    g_source_remove (chooser->load_id);
  
  if (chooser->anim_id != 0)
    g_source_remove (chooser->anim_id);

  if (chooser->store != NULL)
    g_object_unref (chooser->store);
  
  if (chooser->theme != NULL)
    g_object_unref (chooser->theme);

  if (G_OBJECT_CLASS (parent_class)->finalize != NULL)
    (*G_OBJECT_CLASS (parent_class)->finalize) (object);
}


/* ********************* *
 *  GtkWidget Functions  *
 * ********************* */

static void
refresh_combo_icons (EggIconChooserDefault *chooser)
{
  GdkScreen *screen;
  GtkSettings *settings;
  GtkIconTheme *theme;
  GtkTreeModel *model;
  GtkTreeIter iter;
  gint width, height, icon_size;

  if (gtk_widget_has_screen (GTK_WIDGET (chooser)))
    screen = gtk_widget_get_screen (GTK_WIDGET (chooser));
  else
    screen = gdk_screen_get_default ();

  settings = gtk_settings_get_for_screen (screen);

  if (gtk_icon_size_lookup_for_settings (settings, GTK_ICON_SIZE_MENU,
					 &width, &height))
    icon_size = MAX (width, height);
  else
    icon_size = FALLBACK_COMBO_ICON_SIZE;

  theme = gtk_icon_theme_get_for_screen (screen);
  model = gtk_combo_box_get_model (GTK_COMBO_BOX (chooser->context_combo));

  if (gtk_tree_model_get_iter_first (model, &iter))
    {
      width = icon_size;

      do
	{
	  EggIconContext context;
	  const gchar *icon_name;
	  GdkPixbuf *pixbuf;

	  gtk_tree_model_get (model, &iter, CONTEXT_ID_COLUMN, &context, -1);

	  switch (context)
	    {
	    case EGG_ICON_CONTEXT_FILE:
	    case EGG_ICON_CONTEXT_INVALID:
	      icon_name = NULL;
	      break;
	    case EGG_ICON_CONTEXT_ALL:
	      icon_name = "stock_show-all";
	      break;
	    case EGG_ICON_CONTEXT_APPLICATIONS:
	      icon_name = "gnome-applications";
	      break;
	    case EGG_ICON_CONTEXT_LOCATIONS:
	      icon_name = "gnome-dev-cdrom-audio";
	      break;
	    case EGG_ICON_CONTEXT_MIMETYPES:
	      icon_name = "stock_insert_image";
	      break;
	    case EGG_ICON_CONTEXT_STOCK:
	      icon_name = "stock_about";
	      break;
	    case EGG_ICON_CONTEXT_EMBLEMS:
	      icon_name = "stock_3d-favourites";
	      break;
	    case EGG_ICON_CONTEXT_UNCATEGORIZED:
	      icon_name = "stock_unknown";
	      break;
	    default:
	      g_assert_not_reached ();
	      icon_name = NULL; /* quiet GCC */
	      break;
	    }

	  if (icon_name != NULL)
	    pixbuf = gtk_icon_theme_load_icon (theme, icon_name, icon_size, 0,
					       NULL);
	  else
	    pixbuf = NULL;

	  if (pixbuf)
	    width = MAX (width, gdk_pixbuf_get_width (pixbuf));

	  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
			      CONTEXT_ICON_COLUMN, pixbuf,
			      -1);
	}
      while (gtk_tree_model_iter_next (model, &iter));

      g_object_set (chooser->context_icon_cell, "width", width + 4, NULL);
    }

  gtk_widget_queue_draw (chooser->context_combo);
}

static void
egg_icon_chooser_default_style_set (GtkWidget *widget,
				    GtkStyle  *old_style)
{
  if (GTK_WIDGET_CLASS (parent_class)->style_set != NULL)
    (*GTK_WIDGET_CLASS (parent_class)->style_set) (widget, old_style);

  if (!gtk_widget_has_screen (widget))
    return;

  reload_icon_view (EGG_ICON_CHOOSER_DEFAULT (widget));
  refresh_combo_icons (EGG_ICON_CHOOSER_DEFAULT (widget));

  g_signal_emit_by_name (widget, "default-size-changed");
}

static void
egg_icon_chooser_default_screen_changed (GtkWidget *widget,
					 GdkScreen *old_screen)
{
  if (GTK_WIDGET_CLASS (parent_class)->screen_changed)
    (*GTK_WIDGET_CLASS (parent_class)->screen_changed) (widget, old_screen);

  if (!gtk_widget_has_screen (widget))
    return;

  reload_icon_view (EGG_ICON_CHOOSER_DEFAULT (widget));
  refresh_combo_icons (EGG_ICON_CHOOSER_DEFAULT (widget));

  g_signal_emit_by_name (widget, "default-size-changed");
}


/* ************************************ *
 *  EggIconChooserIface Implementation  *
 * ************************************ */

static void
egg_icon_chooser_default_iface_init (EggIconChooserIface *iface)
{
  iface->unselect_all = egg_icon_chooser_default_unselect_all;

  iface->select_icon = egg_icon_chooser_default_select_icon;
  iface->unselect_icon = egg_icon_chooser_default_unselect_icon;
  iface->get_icons = egg_icon_chooser_default_get_icons;

  iface->get_file_system = egg_icon_chooser_default_get_file_system;
  iface->select_path = egg_icon_chooser_default_select_path;
  iface->unselect_path = egg_icon_chooser_default_unselect_path;
  iface->get_paths = egg_icon_chooser_default_get_paths;

  iface->add_filter = egg_icon_chooser_default_add_filter;
  iface->remove_filter = egg_icon_chooser_default_remove_filter;
  iface->list_filters = egg_icon_chooser_default_list_filters;
}


static void
egg_icon_chooser_default_unselect_all (EggIconChooser *icon_chooser)
{
  EggIconChooserDefault *chooser;

  chooser = EGG_ICON_CHOOSER_DEFAULT (icon_chooser);

  if (chooser->selected_icons)
    {
      g_slist_foreach (chooser->selected_icons, (GFunc) g_free, NULL);
      g_slist_free (chooser->selected_icons);
      chooser->selected_icons = NULL;
    }
  else
    {
      gtk_icon_view_unselect_all (GTK_ICON_VIEW (chooser->icon_view));
    }
  gtk_file_chooser_unselect_all (GTK_FILE_CHOOSER (chooser->file_chooser));
}

static gboolean
egg_icon_chooser_default_select_icon (EggIconChooser *icon_chooser,
				      const gchar    *icon_name)
{
  EggIconChooserDefault *chooser;
  gboolean retval;

  chooser = EGG_ICON_CHOOSER_DEFAULT (icon_chooser);
  retval = FALSE;

  if (chooser->selected_icons)
    {
      GtkIconInfo *info;

      info = gtk_icon_theme_lookup_icon (get_icon_theme_for_widget (GTK_WIDGET (icon_chooser)),
					 icon_name, -1, 0);

      if (info != NULL)
	{
	  chooser->selected_icons =
	    g_slist_append (chooser->selected_icons, g_strdup (icon_name));
	  retval = TRUE;
	}
    }
  else
    {
      GtkTreeModel *model;
      GtkTreeIter iter;

      model = gtk_icon_view_get_model (GTK_ICON_VIEW (chooser->icon_view));

      /* Matching icon_name against a row in the icons model.
       * 
       * This should be redone using a custom GList*-based GtkTreeModel that also
       * keeps the li->data members in a hash table and has a "find" function.
       */
      if (gtk_tree_model_get_iter_first (model, &iter))
	{
	  gchar *row_name;
	  guint icon_hash, row_hash;

	  icon_hash = g_str_hash (icon_name);

	  do
	    {
	      row_hash = 0;
	      gtk_tree_model_get (model, &iter, ICON_HASH_COLUMN, &row_hash, -1);

	      if (row_hash != 0 && icon_hash == row_hash)
		{
		  row_name = NULL;
		  gtk_tree_model_get (model, &iter, ICON_NAME_COLUMN, &row_name, -1);

		  if (row_name != NULL && strcmp (icon_name, row_name) == 0)
		    {
		      GtkTreePath *path;

		      path = gtk_tree_model_get_path (model, &iter);

		      if (path != NULL)
			{
			  gtk_icon_view_select_path (GTK_ICON_VIEW (chooser->icon_view),
						     path);
			  gtk_tree_path_free (path);
			  retval = TRUE;
			}
		    }

		  g_free (row_name);
		}
	    }
	  while (!retval && gtk_tree_model_iter_next (model, &iter));
	}
    }

  return retval;
}

static void
egg_icon_chooser_default_unselect_icon (EggIconChooser *icon_chooser,
				        const gchar    *icon_name)
{
  EggIconChooserDefault *chooser;

  chooser = EGG_ICON_CHOOSER_DEFAULT (icon_chooser);

  if (chooser->selected_icons)
    {
      GSList *list;
    
      for (list = chooser->selected_icons; list != NULL; list = list->next)
	{
	  if (strcmp (icon_name, list->data) == 0)
	    {
	      g_free (list->data);
	      chooser->selected_icons =
		g_slist_remove_link (chooser->selected_icons, list);
	      break;
	    }
	}
    }
  else
    {
      GtkTreeModel *model;
      GList *items;
      guint target_hash;

      model = gtk_icon_view_get_model (GTK_ICON_VIEW (chooser->icon_view));
      items = gtk_icon_view_get_selected_items (GTK_ICON_VIEW (chooser->icon_view));

      target_hash = g_str_hash (icon_name);

      while (items != NULL)
	{
	  GtkTreeIter iter;
	  gchar *current_name;
	  guint current_hash;

	  gtk_tree_model_get_iter (model, &iter, items->data);

	  current_hash = 0;
	  gtk_tree_model_get (model, &iter, ICON_HASH_COLUMN, &current_hash, -1);

	  if (current_hash != 0 && current_hash == target_hash)
	    {
	      current_name = NULL;
	      gtk_tree_model_get (model, &iter,
				  ICON_NAME_COLUMN, &current_name, -1);

	      if (current_name != NULL && strcmp (icon_name, current_name) == 0)
		{
		  gtk_icon_view_unselect_path (GTK_ICON_VIEW (chooser->icon_view),
					       items->data);
		  g_free (current_name);
		  break;
		}

	      g_free (current_name);
	    }

	  gtk_tree_path_free (items->data);
	  items = g_list_remove_link (items, items);
	}

      g_list_foreach (items, (GFunc) gtk_tree_path_free, NULL);
      g_list_free (items);
    }
}

static GSList *
egg_icon_chooser_default_get_icons (EggIconChooser *icon_chooser)
{
  EggIconChooserDefault *chooser;
  GSList *retval;

  chooser = EGG_ICON_CHOOSER_DEFAULT (icon_chooser);
  retval = NULL;

  if (chooser->selected_icons)
    {
      GSList *list;

      for (list = chooser->selected_icons; list != NULL; list = list->next)
	retval = g_slist_prepend (list, g_strdup (list->data));
    }
  else
    {
      GtkTreeModel *model;
      GList *items;

      model = gtk_icon_view_get_model (GTK_ICON_VIEW (chooser->icon_view));
  
      for (items = gtk_icon_view_get_selected_items (GTK_ICON_VIEW (chooser->icon_view));
	   items != NULL;
	   items = g_list_remove_link (items, items))
	{
	  GtkTreeIter iter;
	  gchar *row_name;

	  gtk_tree_model_get_iter (model, &iter, items->data);

	  row_name = NULL;
	  gtk_tree_model_get (model, &iter, ICON_NAME_COLUMN, &row_name, -1);

	  if (row_name != NULL)
	    retval = g_slist_prepend (retval, row_name);

	  gtk_tree_path_free (items->data);
	}

       retval = g_slist_reverse (retval);
    }

  return retval;
}

static GtkFileSystem *
egg_icon_chooser_default_get_file_system (EggIconChooser *icon_chooser)
{
  EggIconChooserDefault *chooser;

  chooser = EGG_ICON_CHOOSER_DEFAULT (icon_chooser);

  return _gtk_file_chooser_get_file_system (GTK_FILE_CHOOSER (chooser->file_chooser));
}

static gboolean
egg_icon_chooser_default_select_path (EggIconChooser     *icon_chooser,
				      const GtkFilePath  *path,
				      GError            **error)
{
  EggIconChooserDefault *chooser;

  chooser = EGG_ICON_CHOOSER_DEFAULT (icon_chooser);

  return _gtk_file_chooser_select_path (GTK_FILE_CHOOSER (chooser->file_chooser),
					path, error);
}

static void
egg_icon_chooser_default_unselect_path (EggIconChooser     *icon_chooser,
					const GtkFilePath  *path)
{
  EggIconChooserDefault *chooser;

  chooser = EGG_ICON_CHOOSER_DEFAULT (icon_chooser);

  _gtk_file_chooser_unselect_path (GTK_FILE_CHOOSER (chooser->file_chooser),
				   path);
}

static GSList *
egg_icon_chooser_default_get_paths (EggIconChooser *icon_chooser)
{
  EggIconChooserDefault *chooser;

  chooser = EGG_ICON_CHOOSER_DEFAULT (icon_chooser);

  return _gtk_file_chooser_get_paths (GTK_FILE_CHOOSER (chooser->file_chooser));
}

static void
egg_icon_chooser_default_add_filter (EggIconChooser *icon_chooser,
				     GtkFileFilter  *filter)
{
  EggIconChooserDefault *chooser;

  chooser = EGG_ICON_CHOOSER_DEFAULT (icon_chooser);

  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser->file_chooser),
			       filter);
}

static void
egg_icon_chooser_default_remove_filter (EggIconChooser *icon_chooser,
					GtkFileFilter  *filter)
{
  EggIconChooserDefault *chooser;

  chooser = EGG_ICON_CHOOSER_DEFAULT (icon_chooser);

  gtk_file_chooser_remove_filter (GTK_FILE_CHOOSER (chooser->file_chooser),
				  filter);
}

static GSList *
egg_icon_chooser_default_list_filters (EggIconChooser *icon_chooser)
{
  EggIconChooserDefault *chooser;

  chooser = EGG_ICON_CHOOSER_DEFAULT (icon_chooser);

  return gtk_file_chooser_list_filters (GTK_FILE_CHOOSER (chooser->file_chooser));
}


/* ***************************************** *
 *  GtkFileChooserEmbedIface Implementation  *
 * ***************************************** */

static void
egg_icon_chooser_default_embed_iface_init (GtkFileChooserEmbedIface *iface)
{
  iface->get_default_size = egg_icon_chooser_default_get_default_size;
  iface->get_resizable_hints = egg_icon_chooser_default_get_resizable_hints;
  iface->should_respond = egg_icon_chooser_default_should_respond;
  iface->initial_focus = egg_icon_chooser_default_initial_focus;
}


#define MIN_COLUMNS	8
#define MIN_ROWS	6

static void
egg_icon_chooser_default_get_default_size (GtkFileChooserEmbed *embed,
					   gint                *width,
					   gint                *height)
{
  EggIconChooserDefault *chooser;
  gint filechooser_w, filechooser_h, font_size;

  chooser = EGG_ICON_CHOOSER_DEFAULT (embed);

  _gtk_file_chooser_embed_get_default_size (GTK_FILE_CHOOSER_EMBED (chooser->file_chooser),
					    &filechooser_w, &filechooser_h);

  font_size = pango_font_description_get_size (GTK_WIDGET (embed)->style->font_desc);
  font_size = PANGO_PIXELS (font_size);

  if (gtk_notebook_get_current_page (GTK_NOTEBOOK (chooser->notebook)) == 1)
    {
      *width = filechooser_w;
      *height = filechooser_h;
    }
  else
    {
      gint column_spacing;
      gint row_spacing;
      gint item_width;

      column_spacing =
	gtk_icon_view_get_column_spacing (GTK_ICON_VIEW (chooser->icon_view));
      row_spacing =
	gtk_icon_view_get_row_spacing (GTK_ICON_VIEW (chooser->icon_view));
      item_width =
	gtk_icon_view_get_item_width (GTK_ICON_VIEW (chooser->icon_view));

      *width = (item_width + column_spacing) * MIN_COLUMNS + column_spacing * 3;
      *height = (item_width + row_spacing) * MIN_ROWS + row_spacing * 3;
    }
}

static void
egg_icon_chooser_default_get_resizable_hints (GtkFileChooserEmbed *embed,
					      gboolean            *horizontally,
					      gboolean            *vertically)
{
  if (horizontally)
    *horizontally = TRUE;

  if (vertically)
    *vertically = TRUE;
}

static gboolean
egg_icon_chooser_default_should_respond (GtkFileChooserEmbed *embed)
{
  EggIconChooserDefault *chooser;

  chooser = EGG_ICON_CHOOSER_DEFAULT (embed);

  return ((gtk_notebook_get_current_page (GTK_NOTEBOOK (chooser->notebook)) == 0) ||
	  _gtk_file_chooser_embed_should_respond (GTK_FILE_CHOOSER_EMBED (chooser->file_chooser)));
}

static void
egg_icon_chooser_default_initial_focus (GtkFileChooserEmbed *embed)
{
  EggIconChooserDefault *chooser;
  GtkTreeIter iter;

  chooser = EGG_ICON_CHOOSER_DEFAULT (embed);

  if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (chooser->context_combo), &iter))
    {
      GtkTreeModel *model;
      EggIconContext context;

      model = gtk_combo_box_get_model (GTK_COMBO_BOX (chooser->context_combo));

      context = EGG_ICON_CONTEXT_INVALID;
      gtk_tree_model_get (model, &iter, CONTEXT_ID_COLUMN, &context, -1);

      if (context == EGG_ICON_CONTEXT_FILE)
	_gtk_file_chooser_embed_initial_focus (GTK_FILE_CHOOSER_EMBED (chooser->file_chooser));
      else
	gtk_widget_grab_focus (chooser->icon_view);
    }
}


/* ********************************* *
 *  Callbacks and Utility Functions  *
 * ********************************* */

static gboolean
context_combo_row_is_separator (GtkTreeModel *model,
				GtkTreeIter  *iter,
				gpointer      user_data)
{
  guint sort_group;

  gtk_tree_model_get (model, iter, CONTEXT_SORT_GROUP_COLUMN, &sort_group, -1);

  return (sort_group == CONTEXT_SORT_ALL_SEPARATOR ||
	  sort_group == CONTEXT_SORT_CONTEXTS_SEPARATOR);
}

static gint
context_combo_sort_func (GtkTreeModel *model,
			 GtkTreeIter  *iter1,
			 GtkTreeIter  *iter2,
			 gpointer      user_data)
{
  guint group1, group2;
  gint retval;

  group1 = 0;
  group2 = 0;

  gtk_tree_model_get (model, iter1, CONTEXT_SORT_GROUP_COLUMN, &group1, -1);
  gtk_tree_model_get (model, iter2, CONTEXT_SORT_GROUP_COLUMN, &group2, -1);

  if (group1 < group2)
    retval = -1;
  else if (group1 > group2)
    retval = 1;
  else
    {
      gchar *name1, *name2;

      name1 = NULL;
      name2 = NULL;
      gtk_tree_model_get (model, iter1, CONTEXT_NAME_COLUMN, &name1, -1);
      gtk_tree_model_get (model, iter2, CONTEXT_NAME_COLUMN, &name2, -1);

      if (name1 == NULL && name2 == NULL)
	retval = 0;
      else if (name1 == NULL && name2 != NULL)
	retval = 1;
      else if (name1 != NULL && name2 == NULL)
	retval = -1;
      else
	retval = g_utf8_collate (name1, name2);

      g_free (name1);
      g_free (name2);
    }

  return retval;
}

static void
context_combo_changed (GtkComboBox *combo_box,
		       gpointer     user_data)
{
  EggIconChooserDefault *chooser;
  GtkTreeIter iter;

  chooser = EGG_ICON_CHOOSER_DEFAULT (user_data);

  if (gtk_combo_box_get_active_iter (combo_box, &iter))
    {
      GtkTreeModel *model;
      EggIconContext context;

      model = gtk_combo_box_get_model (combo_box);
      context = EGG_ICON_CONTEXT_INVALID;

      gtk_tree_model_get (model, &iter, CONTEXT_ID_COLUMN, &context, -1);
      chooser->context = context;

      switch (context)
	{
	case EGG_ICON_CONTEXT_ALL:
	case EGG_ICON_CONTEXT_APPLICATIONS:
	case EGG_ICON_CONTEXT_LOCATIONS:
	case EGG_ICON_CONTEXT_EMBLEMS:
	case EGG_ICON_CONTEXT_MIMETYPES:
	case EGG_ICON_CONTEXT_STOCK:
	case EGG_ICON_CONTEXT_UNCATEGORIZED:
	  if (gtk_notebook_get_current_page (GTK_NOTEBOOK (chooser->notebook)) != 0)
	    {
	      gtk_notebook_set_current_page (GTK_NOTEBOOK (chooser->notebook),
					     0);
	      g_signal_emit_by_name (chooser, "default_size_changed", 0);
	    }
	  break;

	case EGG_ICON_CONTEXT_FILE:
	default:
	  if (gtk_notebook_get_current_page (GTK_NOTEBOOK (chooser->notebook)) != 1)
	    {
	      gtk_notebook_set_current_page (GTK_NOTEBOOK (chooser->notebook),
					     1);
	      g_signal_emit_by_name (chooser, "default_size_changed", 0);
	    }
	  return;
	  break;
	}

      if (chooser->load_id != 0)
	return;

      model = gtk_icon_view_get_model (GTK_ICON_VIEW (chooser->icon_view));

      if (model != NULL)
	{
	  g_object_ref (model);
	  gtk_icon_view_set_model (GTK_ICON_VIEW (chooser->icon_view), NULL);
	  gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER (model));
	  gtk_icon_view_set_model (GTK_ICON_VIEW (chooser->icon_view), model);
	  g_object_unref (model);
	}
    }
}


static void
icon_view_style_set (GtkWidget *icon_view,
		     GtkStyle  *old_style,
		     gpointer user_data)
{
#if GTK_CHECK_VERSION(2,20,0)
  if (gtk_widget_get_visible (icon_view))
#else
  if (GTK_WIDGET_VISIBLE (icon_view))
#endif
    gtk_widget_modify_bg (GTK_WIDGET (user_data), GTK_STATE_NORMAL,
			  &(icon_view->style->base[icon_view->state]));
}

static void
icon_view_selection_changed (GtkIconView *icon_view,
			     gpointer     user_data)
{
  _egg_icon_chooser_icon_selection_changed (user_data);
}

static void
icon_view_item_activated (GtkIconView *icon_view,
			  GtkTreePath *path,
			  gpointer     user_data)
{
  _egg_icon_chooser_icon_activated (user_data);
}

static gint
icon_model_unsorted (GtkTreeModel *model,
		     GtkTreeIter  *iter1,
		     GtkTreeIter *iter2,
		     gpointer user_data)
{
  return 0;
}


static void
file_chooser_default_size_changed (GtkFileChooser *file_chooser,
				   gpointer        user_data)
{
  g_signal_emit_by_name (user_data, "default-size-changed");
}

static void
file_chooser_update_preview (GtkFileChooser *file_chooser,
			     gpointer        user_data)
{
  gchar *filename;

  filename = gtk_file_chooser_get_preview_filename (file_chooser);

  if (filename != NULL)
    {
      GdkPixbuf *pixbuf;

      pixbuf = egg_pixbuf_get_thumbnail_for_file (filename,
						  EGG_PIXBUF_THUMBNAIL_NORMAL,
						  NULL);

      egg_thumbnail_preview_set_thumbnail (EGG_THUMBNAIL_PREVIEW (user_data),
					   pixbuf);

      gtk_file_chooser_set_preview_widget_active (file_chooser,
						  (pixbuf != NULL));
    }
  else
    {
      gtk_file_chooser_set_preview_widget_active (file_chooser, FALSE);
    }
}

static void
file_chooser_selection_changed (GtkFileChooser *file_chooser,
			        gpointer        user_data)
{
  _egg_icon_chooser_file_selection_changed (user_data);
}

static void
file_chooser_file_activated (GtkFileChooser *file_chooser,
			     gpointer        user_data)
{
  _egg_icon_chooser_file_activated (user_data);
}


static gboolean
icon_item_is_visible (GtkTreeModel *model,
		      GtkTreeIter  *iter,
		      gpointer      user_data)
{
  EggIconChooserDefault *chooser;
  EggIconContext context;

  chooser = EGG_ICON_CHOOSER_DEFAULT (user_data);

  if (chooser->context == EGG_ICON_CONTEXT_ALL)
    return TRUE;

  gtk_tree_model_get (model, iter, ICON_CONTEXT_COLUMN, &context, -1);

  return (context == chooser->context);
}


/* ******************* *
 *  Loading Functions  *
 * ******************* */

/* Getting The Icon Names */
static void
read_icon_name_into_store (gpointer key,
			   gpointer value,
			   gpointer user_data)
{
  GtkTreeIter iter;

  gtk_list_store_prepend (user_data, &iter);
  gtk_list_store_set (user_data, &iter,
		      ICON_CONTEXT_COLUMN, GPOINTER_TO_UINT (value),
		      ICON_HASH_COLUMN, g_str_hash (key),
		      ICON_NAME_COLUMN, key,
		      ICON_DISPLAY_NAME_COLUMN, key,
		      ICON_PIXBUF_COLUMN, NULL, 
		      ICON_LOADED_COLUMN, FALSE, -1);
}

static void
load_context_icon_names (GHashTable     *table,
			 GtkIconTheme   *theme,
			 const gchar    *context,
			 EggIconContext  ctx_id)
{
  GList *icons;

  for (icons = gtk_icon_theme_list_icons (theme, context);
       icons != NULL; icons = g_list_remove_link (icons, icons))
    {
      if (g_hash_table_lookup (table, icons->data) == NULL)
	g_hash_table_insert (table, icons->data, GUINT_TO_POINTER (ctx_id));
    }
}

static inline void
read_icon_names (EggIconChooserDefault *chooser)
{
  GHashTable *table;

  table = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

  if (chooser->theme == NULL)
    chooser->theme = get_icon_theme_for_widget (GTK_WIDGET (chooser));

  load_context_icon_names (table, chooser->theme,
			   "Applications", EGG_ICON_CONTEXT_APPLICATIONS);
  load_context_icon_names (table, chooser->theme,
			   "Devices", EGG_ICON_CONTEXT_LOCATIONS);
  load_context_icon_names (table, chooser->theme,
			   "FileSystems", EGG_ICON_CONTEXT_LOCATIONS);
  load_context_icon_names (table, chooser->theme,
			   "Emblems", EGG_ICON_CONTEXT_EMBLEMS);
  load_context_icon_names (table, chooser->theme,
			   "MimeTypes", EGG_ICON_CONTEXT_MIMETYPES);
  load_context_icon_names (table, chooser->theme,
			   "Actions", EGG_ICON_CONTEXT_STOCK);
  load_context_icon_names (table, chooser->theme,
			   "Stock", EGG_ICON_CONTEXT_STOCK);
  load_context_icon_names (table, chooser->theme,
			   NULL, EGG_ICON_CONTEXT_UNCATEGORIZED);

  chooser->n_items = g_hash_table_size (table);
  chooser->items_loaded = 0;

  g_hash_table_foreach (table, read_icon_name_into_store, chooser->store);
  g_hash_table_destroy (table);
}

/* Actually reading the icons */
static gboolean
load_icons (gpointer user_data)
{
  EggIconChooserDefault *chooser;
  GtkTreeModel *model;
  GtkTreeIter iter;
  gboolean loaded, retval;

  GDK_THREADS_ENTER ();

  chooser = EGG_ICON_CHOOSER_DEFAULT (user_data);
  model = GTK_TREE_MODEL (chooser->store);
  loaded = FALSE;

  gtk_tree_model_iter_nth_child (model, &iter, NULL, chooser->items_loaded);

  gtk_tree_model_get (model, &iter, ICON_LOADED_COLUMN, &loaded, -1);

  if (!loaded)
    {
      gchar *icon_name;
      GtkIconInfo *info;
      GdkPixbuf *pixbuf;
      const gchar *display_name;

      icon_name = NULL;

      gtk_tree_model_get (model, &iter, ICON_NAME_COLUMN, &icon_name, -1);

      info = gtk_icon_theme_lookup_icon (chooser->theme, icon_name,
					 chooser->real_icon_size, 0);

      display_name = gtk_icon_info_get_display_name (info);
      if (display_name == NULL)
	display_name = (const gchar *) icon_name;

      pixbuf = gtk_icon_info_load_icon (info, NULL);

      if (pixbuf == NULL)
	{
	  pixbuf = gtk_icon_theme_load_icon (chooser->theme,
					     GTK_STOCK_MISSING_IMAGE,
					     chooser->real_icon_size, 0, NULL);
	}
      else
	{
	  gint width, height;

	  width = gdk_pixbuf_get_width (pixbuf);
	  height = gdk_pixbuf_get_height (pixbuf);

	  if ((width > chooser->real_icon_size + 4) ||
	      (height > chooser->real_icon_size + 4))
	    {
	      GdkPixbuf *tmp;

	      if (width > height)
		{
		  height *= (gdouble) chooser->real_icon_size / width;
		  width = chooser->real_icon_size;
		}
	      else
		{
		  width *= (gdouble) chooser->real_icon_size / height;
		  height = chooser->real_icon_size;
		}

	      tmp = gdk_pixbuf_scale_simple (pixbuf, width, height,
					     GDK_INTERP_BILINEAR);
	      g_object_unref (pixbuf);
	      pixbuf = tmp;
	    }
	}

      gtk_list_store_set (chooser->store, &iter,
			  ICON_DISPLAY_NAME_COLUMN, display_name,
			  ICON_PIXBUF_COLUMN, pixbuf,
			  ICON_LOADED_COLUMN, TRUE, -1);
      g_free (icon_name);
      gtk_icon_info_free (info);

      if (pixbuf != NULL)
	g_object_unref (pixbuf);
    }
  else
    {
      gchar *name;
    
      name = NULL;
      gtk_tree_model_get (model, &iter, ICON_NAME_COLUMN, &name, -1);
      g_debug ("Icon for %s asked to get loaded twice.", name);
      g_free (name);
    }

  chooser->items_loaded++;

  if (chooser->items_loaded == chooser->n_items)
    {
      gtk_widget_hide (chooser->anim_box);
      gtk_widget_set_sensitive (chooser->context_combo, TRUE);

      gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (model),
					    ICON_DISPLAY_NAME_COLUMN,
					    GTK_SORT_ASCENDING);

      model = gtk_tree_model_filter_new (model, NULL);
      gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER (model),
					      icon_item_is_visible,
					      chooser, NULL);
      gtk_icon_view_set_model (GTK_ICON_VIEW (chooser->icon_view), model);
      g_object_unref (model);

      retval = FALSE;
    }
  else
    {
      gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (chooser->progress),
				     (gdouble) chooser->items_loaded / chooser->n_items);

      retval = TRUE;
    }

  GDK_THREADS_LEAVE ();

  return retval;
}

static void
cleanup_after_load (gpointer user_data)
{
  EggIconChooserDefault *chooser;

  GDK_THREADS_ENTER ();

  chooser = user_data;

  while (chooser->selected_icons)
    {
      egg_icon_chooser_default_select_icon (user_data,
					    chooser->selected_icons->data);
      g_free (chooser->selected_icons->data);
      chooser->selected_icons = g_slist_remove_link (chooser->selected_icons,
						     chooser->selected_icons);
    }

  g_signal_handler_unblock (chooser->icon_view,
			    chooser->icon_view_selection_changed_id);

  chooser->load_id = 0;

  if (chooser->anim_id != 0)
    g_source_remove (chooser->anim_id);

  GDK_THREADS_LEAVE ();
}

/* "Loading" Animation */
static gboolean
animate_spinner (gpointer user_data)
{
  EggIconChooserDefault *chooser;
  GdkPixbuf *frame;

  GDK_THREADS_ENTER ();

  chooser = user_data;

  frame = gdk_pixbuf_new_subpixbuf (chooser->anim_pixbuf,
				    chooser->frame_x,
				    chooser->frame_y, 48, 48);
  gtk_image_set_from_pixbuf (GTK_IMAGE (chooser->anim_image), frame);
  g_object_unref (frame);

  if (chooser->frame_x + 48 < gdk_pixbuf_get_width (chooser->anim_pixbuf))
    {
      chooser->frame_x += 48;
    }
  else if (chooser->frame_y + 48 < gdk_pixbuf_get_height (chooser->anim_pixbuf))
    {
      chooser->frame_x = 0;
      chooser->frame_y += 48;
    }
  else
    {
      chooser->frame_x = 0;
      chooser->frame_y = 0;
    }

  GDK_THREADS_LEAVE ();

  return TRUE;
}

static void
cleanup_after_anim (gpointer user_data)
{
  EggIconChooserDefault *chooser;

  GDK_THREADS_ENTER ();

  chooser = user_data;

  g_object_unref (chooser->anim_pixbuf);
  chooser->anim_pixbuf = NULL;
  chooser->frame_x = 0;
  chooser->frame_y = 0;
  chooser->anim_id = 0;

  if (chooser->load_id != 0)
    g_source_remove (chooser->load_id);

  GDK_THREADS_LEAVE ();
}

static inline void
start_load (EggIconChooserDefault *chooser)
{
  /* Animation Pixbuf */
  if (!chooser->theme)
    chooser->theme = get_icon_theme_for_widget (GTK_WIDGET (chooser));

  chooser->anim_pixbuf = gtk_icon_theme_load_icon (chooser->theme,
						   "gnome-spinner",
						   48, 0, NULL);

  /* Show the widgets */
  gtk_widget_show_all (chooser->anim_box);
  gtk_widget_set_sensitive (chooser->context_combo, FALSE);

  /* Install the load handlers */
  chooser->anim_id = g_timeout_add_full (G_PRIORITY_DEFAULT, SPINNER_TIMEOUT,
					 animate_spinner, chooser,
					 cleanup_after_anim);
  chooser->load_id = g_idle_add_full (G_PRIORITY_HIGH_IDLE + 30, load_icons,
				      chooser, cleanup_after_load);
}

static void
reload_icon_view (EggIconChooserDefault *chooser)
{
  GList *sel;

  if (chooser->load_id != 0)
    g_source_remove (chooser->load_id);

  g_assert (chooser->selected_icons == NULL);

  sel = gtk_icon_view_get_selected_items (GTK_ICON_VIEW (chooser->icon_view));
  if (sel)
    {
      GtkTreeModel *model;

      model = gtk_icon_view_get_model (GTK_ICON_VIEW (chooser->icon_view));

      while (sel)
	{
	  GtkTreeIter iter;
	  gchar *icon_name;

	  if (gtk_tree_model_get_iter (model, &iter, sel->data))
	    {
	      gtk_tree_model_get (model, &iter,
				  ICON_NAME_COLUMN, &icon_name,
				  -1);
	      if (icon_name)
		chooser->selected_icons = g_slist_prepend (chooser->selected_icons,
							   icon_name);

	      icon_name = NULL;
	    }

	  gtk_tree_path_free (sel->data);
	  sel = g_list_remove_link (sel, sel);
	}
    }

  g_signal_handler_block (chooser->icon_view,
			  chooser->icon_view_selection_changed_id);

  gtk_icon_view_set_model (GTK_ICON_VIEW (chooser->icon_view), NULL);
  gtk_list_store_clear (chooser->store);
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (chooser->store),
					GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID,
					GTK_SORT_ASCENDING);

  if (!gtk_widget_has_screen (GTK_WIDGET (chooser)))
    return;

  gtk_icon_view_set_item_width (GTK_ICON_VIEW (chooser->icon_view),
				chooser->real_icon_size + 4);
  /* Clear the icon-view widget */
  gtk_widget_queue_draw (chooser->icon_view);

  if (chooser->icon_size < 0)
    chooser->real_icon_size = get_icon_size_for_widget (GTK_WIDGET (chooser));
  else
    chooser->real_icon_size = MAX (chooser->icon_size, MINIMUM_ICON_SIZE);

  read_icon_names (chooser);
  start_load (chooser);
}


/* ************************* *
 *  Miscellaneous Utilities  *
 * ************************* */

static GtkIconTheme *
get_icon_theme_for_widget (GtkWidget *widget)
{
  if (gtk_widget_has_screen (widget))
    return gtk_icon_theme_get_for_screen (gtk_widget_get_screen (widget));

  return gtk_icon_theme_get_default ();
}

static gint
get_icon_size_for_widget (GtkWidget *widget)
{
  GtkSettings *settings;
  gint width, height;

  if (gtk_widget_has_screen (widget))
    settings = gtk_settings_get_for_screen (gtk_widget_get_screen (widget));
  else
    settings = gtk_settings_get_default ();

  if (gtk_icon_size_lookup_for_settings (settings, GTK_ICON_SIZE_DIALOG,
					 &width, &height))
    return MAX (width, height);

  return FALLBACK_ICON_SIZE;
}
