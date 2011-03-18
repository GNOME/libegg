/* eggthumbnailpreview.h
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

#ifndef __EGG_THUMBNAIL_PREVIEW_H__
#define __EGG_THUMBNAIL_PREVIEW_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS


#define EGG_TYPE_THUMBNAIL_PREVIEW             (egg_thumbnail_preview_get_type ())
#define EGG_THUMBNAIL_PREVIEW(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_THUMBNAIL_PREVIEW, EggThumbnailPreview))
#define EGG_IS_THUMBNAIL_PREVIEW(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_THUMBNAIL_PREVIEW))
#define EGG_THUMBNAIL_PREVIEW_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EGG_TYPE_THUMBNAIL_PREVIEW, EggThumbnailPreview))
#define EGG_IS_THUMBNAIL_PREVIEW_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EGG_TYPE_THUMBNAIL_PREVIEW))
#define EGG_THUMBNAIL_PREVIEW_GET_CLASS(klass) (G_TYPE_INSTANCE_GET_CLASS ((klass), EGG_TYPE_THUMBNAIL_PREVIEW, EggThumbnailPreviewClass))


typedef enum /* <flags,prefix=EGG_THUMBNAIL_PREVIEW> */
{
  EGG_THUMBNAIL_PREVIEW_NONE              = 0,
  EGG_THUMBNAIL_PREVIEW_FILENAME          = 1 << 0,
  EGG_THUMBNAIL_PREVIEW_DESCRIPTION       = 1 << 1,
  EGG_THUMBNAIL_PREVIEW_IMAGE_DIMENSIONS  = 1 << 2,
  EGG_THUMBNAIL_PREVIEW_MOVIE_LENGTH      = 1 << 3,
  EGG_THUMBNAIL_PREVIEW_DOCUMENT_PAGES    = 1 << 4,
  EGG_THUMBNAIL_PREVIEW_MIME_TYPE         = 1 << 5,
  EGG_THUMBNAIL_PREVIEW_MTIME             = 1 << 6,
  EGG_THUMBNAIL_PREVIEW_FILE_SIZE         = 1 << 7
}
EggThumbnailPreviewFlags;


typedef struct _EggThumbnailPreview EggThumbnailPreview;
typedef struct _EggThumbnailPreviewPrivate EggThumbnailPreviewPrivate;
typedef struct _EggThumbnailPreviewClass EggThumbnailPreviewClass;

struct _EggThumbnailPreview
{
  GtkVBox parent;

  EggThumbnailPreviewPrivate *priv;
};

struct _EggThumbnailPreviewClass
{
  GtkVBoxClass parent_class;

  void (*__egg_reserved1);
  void (*__egg_reserved2);
  void (*__egg_reserved3);
  void (*__egg_reserved4);
  void (*__egg_reserved5);
  void (*__egg_reserved6);
  void (*__egg_reserved7);
  void (*__egg_reserved8);
};

GType                    egg_thumbnail_preview_get_type      (void) G_GNUC_CONST;

GtkWidget               *egg_thumbnail_preview_new           (void);

GdkPixbuf               *egg_thumbnail_preview_get_thumbnail (EggThumbnailPreview     *preview);
void                     egg_thumbnail_preview_set_thumbnail (EggThumbnailPreview     *preview,
							      GdkPixbuf               *thumbnail);

EggThumbnailPreviewFlags egg_thumbnail_preview_get_visible   (EggThumbnailPreview     *preview);
void                     egg_thumbnail_preview_set_visible   (EggThumbnailPreview     *preview,
							      EggThumbnailPreviewFlags visible);

G_END_DECLS

#endif /* __EGG_THUMBNAIL_PREVIEW_H__ */
