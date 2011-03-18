/* eggthumbnailpreview.c
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

#include <time.h>

#include <glib/gi18n.h>

#include <gtk/gtk.h>

#include "egg-pixbuf-thumbnail.h"

#include "eggthumbnailpreviewtypebuiltins.h"
#include "eggthumbnailpreview.h"


enum
{
  PROP_0,
  PROP_THUMBNAIL,
  PROP_VISIBLE
};

struct _EggThumbnailPreviewPrivate
{
  GtkWidget *image;
  GtkWidget *label_box;
  GtkWidget *name_label;
  GtkWidget *description_label;
  GtkWidget *mimetype_label;
  GtkWidget *dimension_label;
  GtkWidget *mtime_label;
  GtkWidget *filesize_label;
  GtkWidget *length_label;
  GtkWidget *pages_label;

  EggThumbnailPreviewFlags visible:8;
};


#define EGG_THUMBNAIL_PREVIEW_GET_PRIVATE(object) (EGG_THUMBNAIL_PREVIEW (object)->priv)
#define EGG_THUMBNAIL_PREVIEW_DEFAULT (EGG_THUMBNAIL_PREVIEW_MIME_TYPE | \
				       EGG_THUMBNAIL_PREVIEW_IMAGE_DIMENSIONS | \
				       EGG_THUMBNAIL_PREVIEW_DOCUMENT_PAGES | \
				       EGG_THUMBNAIL_PREVIEW_MOVIE_LENGTH | \
				       EGG_THUMBNAIL_PREVIEW_FILE_SIZE)


/* ************ *
 *  Prototypes  *
 * ************ */

/* GObject */
static void   egg_thumbnail_preview_set_property (GObject               *object,
						  guint                  param_id,
						  const GValue          *value,
						  GParamSpec            *pspec);
static void   egg_thumbnail_preview_get_property (GObject               *object,
						  guint                  param_id,
						  GValue                *value,
						  GParamSpec            *pspec);

static void  label_style_set (GtkWidget *label,
			      GtkStyle  *old_style,
			      gpointer   user_data);

static gchar *egg_time_to_date_str     (const gchar *format,
					time_t       timeval);
static gchar *egg_time_to_interval_str (time_t       timeval);
static gchar *egg_filesize_to_str      (gsize        sizeval);


#ifdef G_OS_WIN32

/* The gmtime() in Microsoft's C library *is* thread-safe. It has no gmtime_r(). */

static struct tm *
gmtime_r (const time_t *timep, struct tm *result)
{
  struct tm *tmp = gmtime (timep);

  if (tmp == NULL)
    return NULL;

  memcpy (result, tmp, sizeof (struct tm));

  return result;
}

#endif

G_DEFINE_TYPE (EggThumbnailPreview, egg_thumbnail_preview, GTK_TYPE_VBOX);


static void
egg_thumbnail_preview_class_init (EggThumbnailPreviewClass *class)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (class);

  gobject_class->set_property = egg_thumbnail_preview_set_property;
  gobject_class->get_property = egg_thumbnail_preview_get_property;

  g_object_class_install_property (gobject_class, PROP_THUMBNAIL,
				   g_param_spec_object ("thumbnail",
							_("Thumbnail"),
							_("The thumbnail currently being previewed."),
							GDK_TYPE_PIXBUF,
							G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, PROP_VISIBLE,
				   g_param_spec_flags ("visible",
						       _("Visible"),
						       _("What label items to show under the thumbnail."),
						       EGG_TYPE_THUMBNAIL_PREVIEW_FLAGS,
						       EGG_THUMBNAIL_PREVIEW_DEFAULT,
						       G_PARAM_READWRITE));

  g_type_class_add_private (class, sizeof (EggThumbnailPreviewPrivate));
}

static void
egg_thumbnail_preview_init (EggThumbnailPreview *preview)
{
  preview->priv = G_TYPE_INSTANCE_GET_PRIVATE (preview,
					       EGG_TYPE_THUMBNAIL_PREVIEW,
					       EggThumbnailPreviewPrivate);

  gtk_box_set_spacing (GTK_BOX (preview), 6);

  preview->priv->visible = EGG_THUMBNAIL_PREVIEW_DEFAULT;

  preview->priv->image = gtk_image_new ();
  gtk_box_pack_start (GTK_BOX (preview), preview->priv->image, FALSE, FALSE, 0);
  gtk_widget_show (preview->priv->image);

  preview->priv->label_box = gtk_vbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (preview), preview->priv->label_box,
		      FALSE, FALSE, 0);

  preview->priv->name_label = gtk_label_new (NULL);
  g_signal_connect (preview->priv->name_label, "style-set",
		    G_CALLBACK (label_style_set), NULL);
  gtk_box_pack_start (GTK_BOX (preview->priv->label_box),
		      preview->priv->name_label, FALSE, FALSE, 0);

  preview->priv->description_label = gtk_label_new (NULL);
  g_signal_connect (preview->priv->description_label, "style-set",
		    G_CALLBACK (label_style_set), NULL);
  gtk_box_pack_start (GTK_BOX (preview->priv->label_box),
		      preview->priv->description_label, FALSE, FALSE, 0);

  preview->priv->mimetype_label = gtk_label_new (NULL);
  g_signal_connect (preview->priv->mimetype_label, "style-set",
		    G_CALLBACK (label_style_set), NULL);
  gtk_box_pack_start (GTK_BOX (preview->priv->label_box),
		      preview->priv->mimetype_label, FALSE, FALSE, 0);

  preview->priv->dimension_label = gtk_label_new (NULL);
  g_signal_connect (preview->priv->dimension_label, "style-set",
		    G_CALLBACK (label_style_set), NULL);
  gtk_box_pack_start (GTK_BOX (preview->priv->label_box),
		      preview->priv->dimension_label, FALSE, FALSE, 0);

  preview->priv->mtime_label = gtk_label_new (NULL);
  g_signal_connect (preview->priv->mtime_label, "style-set",
		    G_CALLBACK (label_style_set), NULL);
  gtk_box_pack_start (GTK_BOX (preview->priv->label_box),
		      preview->priv->mtime_label, FALSE, FALSE, 0);

  preview->priv->filesize_label = gtk_label_new (NULL);
  g_signal_connect (preview->priv->filesize_label, "style-set",
		    G_CALLBACK (label_style_set), NULL);
  gtk_box_pack_start (GTK_BOX (preview->priv->label_box),
		      preview->priv->filesize_label, FALSE, FALSE, 0);

  preview->priv->length_label = gtk_label_new (NULL);
  g_signal_connect (preview->priv->length_label, "style-set",
		    G_CALLBACK (label_style_set), NULL);
  gtk_box_pack_start (GTK_BOX (preview->priv->label_box),
		      preview->priv->length_label, FALSE, FALSE, 0);

  preview->priv->pages_label = gtk_label_new (NULL);
  g_signal_connect (preview->priv->pages_label, "style-set",
		    G_CALLBACK (label_style_set), NULL);
  gtk_box_pack_start (GTK_BOX (preview->priv->label_box),
		      preview->priv->pages_label, FALSE, FALSE, 0);
}


static void
egg_thumbnail_preview_set_property (GObject      *object,
				    guint         param_id,
				    const GValue *value,
				    GParamSpec   *pspec)
{
  EggThumbnailPreview *preview;

  preview = EGG_THUMBNAIL_PREVIEW (object);

  switch (param_id)
    {
    case PROP_THUMBNAIL:
      egg_thumbnail_preview_set_thumbnail (preview, g_value_get_object (value));
      break;
    case PROP_VISIBLE:
      egg_thumbnail_preview_set_visible (preview, g_value_get_flags (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}

static void
egg_thumbnail_preview_get_property (GObject    *object,
				    guint       param_id,
				    GValue     *value,
				    GParamSpec *pspec)
{
  EggThumbnailPreviewPrivate *priv;

  priv = EGG_THUMBNAIL_PREVIEW_GET_PRIVATE (object);

  switch (param_id)
    {
    case PROP_THUMBNAIL:
      g_value_set_object (value,
			  gtk_image_get_pixbuf (GTK_IMAGE (priv->image)));
      break;
    case PROP_VISIBLE:
      g_value_set_flags (value, priv->visible);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}


GtkWidget *
egg_thumbnail_preview_new (void)
{
  return g_object_new (EGG_TYPE_THUMBNAIL_PREVIEW, NULL);
}


GdkPixbuf *
egg_thumbnail_preview_get_thumbnail (EggThumbnailPreview *preview)
{
  g_return_val_if_fail (EGG_IS_THUMBNAIL_PREVIEW (preview), NULL);

  return gtk_image_get_pixbuf (GTK_IMAGE (preview->priv->image));
}

void
egg_thumbnail_preview_set_thumbnail (EggThumbnailPreview *preview,
				     GdkPixbuf           *thumbnail)
{
  const gchar *str;
  gchar *tmp;
  gint width, height, pages;
  time_t timeval;
  gssize sizeval;

  g_return_if_fail (EGG_IS_THUMBNAIL_PREVIEW (preview));
  g_return_if_fail (thumbnail == NULL || GDK_IS_PIXBUF (thumbnail));

  gtk_image_set_from_pixbuf (GTK_IMAGE (preview->priv->image), thumbnail);

  if (thumbnail == NULL)
    {
      gtk_widget_hide (preview->priv->label_box);
      g_object_notify (G_OBJECT (preview), "thumbnail");
      return;
    }

  /* URI */
  str = egg_pixbuf_get_thumbnail_uri (thumbnail);
  tmp = g_filename_from_uri (str, NULL, NULL);
  gtk_label_set_text (GTK_LABEL (preview->priv->name_label), tmp);
  g_free (tmp);
  if (tmp != NULL && preview->priv->visible & EGG_THUMBNAIL_PREVIEW_FILENAME)
    gtk_widget_show (preview->priv->name_label);
  else
    gtk_widget_hide (preview->priv->name_label);

  /* Description */
  str = egg_pixbuf_get_thumbnail_description (thumbnail);
  gtk_label_set_text (GTK_LABEL (preview->priv->description_label), str);
  if (str != NULL && preview->priv->visible & EGG_THUMBNAIL_PREVIEW_DESCRIPTION)
    gtk_widget_show (preview->priv->description_label);
  else
    gtk_widget_hide (preview->priv->description_label);

  /* Dimensions */
  width = egg_pixbuf_get_thumbnail_image_width (thumbnail);
  height = egg_pixbuf_get_thumbnail_image_height (thumbnail);
  if (width > 0 && height > 0)
    {
      tmp = g_strdup_printf ("%dx%d", width, height);
      gtk_label_set_text (GTK_LABEL (preview->priv->dimension_label), tmp);
      g_free (tmp);

      if (preview->priv->visible & EGG_THUMBNAIL_PREVIEW_IMAGE_DIMENSIONS)
	gtk_widget_show (preview->priv->dimension_label);
      else
	gtk_widget_hide (preview->priv->dimension_label);
    }
  else
    gtk_widget_hide (preview->priv->dimension_label);

  /* Document Pages */
  pages = egg_pixbuf_get_thumbnail_document_pages (thumbnail);
  if (pages > 0)
    {
      tmp = g_strdup_printf ("%d pages", pages);
      gtk_label_set_text (GTK_LABEL (preview->priv->pages_label), tmp);
      g_free (tmp);

      if (preview->priv->visible & EGG_THUMBNAIL_PREVIEW_DOCUMENT_PAGES)
	gtk_widget_show (preview->priv->pages_label);
      else
	gtk_widget_hide (preview->priv->pages_label);
    }
  else
    gtk_widget_hide (preview->priv->pages_label);

  /* Movie Length */
  timeval = egg_pixbuf_get_thumbnail_movie_length (thumbnail);
  if (timeval > 0)
    {
      tmp = egg_time_to_interval_str (timeval);
      gtk_label_set_text (GTK_LABEL (preview->priv->length_label), tmp);
      g_free (tmp);

      if (preview->priv->visible & EGG_THUMBNAIL_PREVIEW_MOVIE_LENGTH)
	gtk_widget_show (preview->priv->length_label);
      else
	gtk_widget_hide (preview->priv->length_label);
    }
  else
    gtk_widget_hide (preview->priv->length_label);

  /* Mime Type */
  str = egg_pixbuf_get_thumbnail_mime_type (thumbnail);
  gtk_label_set_text (GTK_LABEL (preview->priv->mimetype_label), str);
  if (str != NULL && preview->priv->visible & EGG_THUMBNAIL_PREVIEW_MIME_TYPE)
    gtk_widget_show (preview->priv->mimetype_label);
  else
    gtk_widget_hide (preview->priv->mimetype_label);

  /* Modification Time */
  timeval = egg_pixbuf_get_thumbnail_mtime (thumbnail);
  if (timeval > 0)
    {
      tmp = egg_time_to_date_str ("%x", timeval);
      gtk_label_set_text (GTK_LABEL (preview->priv->mtime_label), tmp);
      g_free (tmp);

      if (preview->priv->visible & EGG_THUMBNAIL_PREVIEW_MTIME)
	gtk_widget_show (preview->priv->mtime_label);
      else
	gtk_widget_hide (preview->priv->mtime_label);
    }
  else
    gtk_widget_hide (preview->priv->mtime_label);

  /* File Size */
  sizeval = egg_pixbuf_get_thumbnail_filesize (thumbnail);
  if (sizeval > 0)
    {
      tmp = egg_filesize_to_str (sizeval);
      gtk_label_set_text (GTK_LABEL (preview->priv->filesize_label), tmp);
      g_free (tmp);

      if (preview->priv->visible & EGG_THUMBNAIL_PREVIEW_FILE_SIZE)
	gtk_widget_show (preview->priv->filesize_label);
      else
	gtk_widget_hide (preview->priv->filesize_label);
    }
  else
    gtk_widget_hide (preview->priv->filesize_label);

  gtk_widget_show (preview->priv->label_box);
  g_object_notify (G_OBJECT (preview), "thumbnail");
}

EggThumbnailPreviewFlags
egg_thumbnail_preview_get_visible (EggThumbnailPreview *preview)
{
  g_return_val_if_fail (EGG_IS_THUMBNAIL_PREVIEW (preview),
			EGG_THUMBNAIL_PREVIEW_NONE);

  return preview->priv->visible;
}

void
egg_thumbnail_preview_set_visible (EggThumbnailPreview     *preview,
				   EggThumbnailPreviewFlags visible)
{
  g_return_if_fail (EGG_IS_THUMBNAIL_PREVIEW (preview));
  // g_return_if_fail (G_ENUM_IS_VALID (EGG_TYPE_THUMBNAIL_PREVIEW_FLAGS, visible));

  preview->priv->visible = visible;

  if ((gtk_label_get_text (GTK_LABEL (preview->priv->name_label)) != NULL)
      && (visible & EGG_THUMBNAIL_PREVIEW_FILENAME))
    gtk_widget_show (preview->priv->name_label);
  else
    gtk_widget_hide (preview->priv->name_label);

  if ((gtk_label_get_text (GTK_LABEL (preview->priv->description_label)) != NULL)
      && (visible & EGG_THUMBNAIL_PREVIEW_DESCRIPTION))
    gtk_widget_show (preview->priv->description_label);
  else
    gtk_widget_hide (preview->priv->description_label);

  if ((gtk_label_get_text (GTK_LABEL (preview->priv->dimension_label)) != NULL)
      && (visible & EGG_THUMBNAIL_PREVIEW_IMAGE_DIMENSIONS))
    gtk_widget_show (preview->priv->dimension_label);
  else
    gtk_widget_hide (preview->priv->dimension_label);

  if ((gtk_label_get_text (GTK_LABEL (preview->priv->pages_label)) != NULL)
      && (visible & EGG_THUMBNAIL_PREVIEW_DOCUMENT_PAGES))
    gtk_widget_show (preview->priv->pages_label);
  else
    gtk_widget_hide (preview->priv->pages_label);

  if ((gtk_label_get_text (GTK_LABEL (preview->priv->length_label)) != NULL)
      && (visible & EGG_THUMBNAIL_PREVIEW_MOVIE_LENGTH))
    gtk_widget_show (preview->priv->length_label);
  else
    gtk_widget_hide (preview->priv->length_label);

  if ((gtk_label_get_text (GTK_LABEL (preview->priv->mimetype_label)) != NULL)
      && (visible & EGG_THUMBNAIL_PREVIEW_MIME_TYPE))
    gtk_widget_show (preview->priv->mimetype_label);
  else
    gtk_widget_hide (preview->priv->mimetype_label);

  if ((gtk_label_get_text (GTK_LABEL (preview->priv->mtime_label)) != NULL)
      && (visible & EGG_THUMBNAIL_PREVIEW_MTIME))
    gtk_widget_show (preview->priv->mtime_label);
  else
    gtk_widget_hide (preview->priv->mtime_label);

  if ((gtk_label_get_text (GTK_LABEL (preview->priv->filesize_label)) != NULL)
      && (visible & EGG_THUMBNAIL_PREVIEW_FILE_SIZE))
    gtk_widget_show (preview->priv->filesize_label);
  else
    gtk_widget_hide (preview->priv->filesize_label);
}


static void
label_style_set (GtkWidget *label,
		 GtkStyle  *old_style,
		 gpointer   user_data)
{
  PangoFontDescription *font_desc;

  GtkStyleContext *style = gtk_widget_get_style_context (label);
  font_desc = pango_font_description_copy (
				   gtk_style_context_get_font (style, GTK_STATE_FLAG_NORMAL));
  pango_font_description_set_size (font_desc,
				   pango_font_description_get_size (font_desc) * PANGO_SCALE_SMALL);
  g_signal_handlers_block_by_func (label, label_style_set, NULL);
  gtk_widget_modify_font (label, font_desc);
  g_signal_handlers_unblock_by_func (label, label_style_set, NULL);
  pango_font_description_free (font_desc);
}

static gchar *
egg_filesize_to_str (gsize sizeval)
{
  if (sizeval == 0)
    return NULL;

  /* GB */
  if (sizeval > 1073741824)
    return g_strdup_printf ("%0.2fGB", (gdouble) sizeval / 1073741824);

  /* MB */
  if (sizeval > 1048576)
    return g_strdup_printf ("%0.2fMB", (gdouble) sizeval / 1048576);
  
  /* KB */
  if (sizeval > 1024)
    return g_strdup_printf ("%0.2fKB", (gdouble) sizeval / 1024);

  return g_strdup_printf ("%" G_GSIZE_FORMAT " bytes", sizeval);
}

static gchar *
egg_time_to_date_str (const gchar *format,
		      time_t       timeval)
{
  gchar *retval;
  gsize retval_size, result;
  struct tm tp;

  g_return_val_if_fail (format != NULL, NULL);
  g_return_val_if_fail (timeval > 0, NULL);

  retval = NULL;
  retval_size = 32;

  gmtime_r (&timeval, &tp);

  do
    {
      g_free (retval);
      retval = g_new (gchar, retval_size);
      result = strftime (retval, retval_size, format, &tp);

      if (result == 0)
	retval_size = result;
    }
  while (result == 0);

  return retval;
}

static gchar *
egg_time_to_interval_str (time_t timeval)
{
  gint seconds, minutes, hours, days;

  seconds = -1;
  minutes = -1;
  hours = -1;
  days = -1;

  days = timeval / (3600 * 24);
  hours = timeval / 3600;
  minutes = timeval / 60;
  seconds = timeval % 60;

  return g_strdup_printf ("%d:%d:%d", hours, minutes, seconds);
}
