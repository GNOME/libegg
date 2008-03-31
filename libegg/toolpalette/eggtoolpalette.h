/* EggToolPalette -- A tool palette with categories and DnD support
 * Copyright (C) 2008  Openismus GmbH
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Authors:
 *      Mathias Hasselmann
 */

#ifndef __EGG_TOOL_PALETTE_H__
#define __EGG_TOOL_PALETTE_H__

#include <gtk/gtkcontainer.h>
#include <gtk/gtkdnd.h>
#include <gtk/gtktoolitem.h>

G_BEGIN_DECLS

#define EGG_TYPE_TOOL_PALETTE           (egg_tool_palette_get_type())
#define EGG_TOOL_PALETTE(obj)           (G_TYPE_CHECK_INSTANCE_CAST(obj, EGG_TYPE_TOOL_PALETTE, EggToolPalette))
#define EGG_TOOL_PALETTE_CLASS(cls)     (G_TYPE_CHECK_CLASS_CAST(cls, EGG_TYPE_TOOL_PALETTE, EggToolPaletteClass))
#define EGG_IS_TOOL_PALETTE(obj)        (G_TYPE_CHECK_INSTANCE_TYPE(obj, EGG_TYPE_TOOL_PALETTE))
#define EGG_IS_TOOL_PALETTE_CLASS(obj)  (G_TYPE_CHECK_CLASS_TYPE(obj, EGG_TYPE_TOOL_PALETTE))
#define EGG_TOOL_PALETTE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), EGG_TYPE_TOOL_PALETTE, EggToolPaletteClass))

typedef struct _EggToolPalette           EggToolPalette;
typedef struct _EggToolPaletteClass      EggToolPaletteClass;
typedef struct _EggToolPalettePrivate    EggToolPalettePrivate;

typedef enum /*< flags >*/
{
  EGG_TOOL_PALETTE_DRAG_ITEMS = (1 << 0),
  EGG_TOOL_PALETTE_DRAG_GROUPS = (1 << 1),
}
EggToolPaletteDragTargets;

struct _EggToolPalette
{
  GtkContainer parent_instance;
  EggToolPalettePrivate *priv;
};

struct _EggToolPaletteClass
{
  GtkContainerClass parent_class;

  void (*set_scroll_adjustments) (GtkWidget     *widget,
                                  GtkAdjustment *hadjustment,
                                  GtkAdjustment *vadjustment);
};

GType                          egg_tool_palette_get_type              (void) G_GNUC_CONST;
GtkWidget*                     egg_tool_palette_new                   (void);

void                           egg_tool_palette_set_group_position    (EggToolPalette            *palette,
                                                                       GtkWidget                 *group,
                                                                       gint                       position);
void                           egg_tool_palette_set_exclusive         (EggToolPalette            *palette,
                                                                       GtkWidget                 *group,
                                                                       gboolean                   exclusive);
void                           egg_tool_palette_set_expand            (EggToolPalette            *palette,
                                                                       GtkWidget                 *group,
                                                                       gboolean                   expand);

gint                           egg_tool_palette_get_group_position    (EggToolPalette            *palette,
                                                                       GtkWidget                 *group);
gboolean                       egg_tool_palette_get_exclusive         (EggToolPalette            *palette,
                                                                       GtkWidget                 *group);
gboolean                       egg_tool_palette_get_expand            (EggToolPalette            *palette,
                                                                       GtkWidget                 *group);

void                           egg_tool_palette_set_icon_size         (EggToolPalette            *palette,
                                                                       GtkIconSize                icon_size);
void                           egg_tool_palette_set_orientation       (EggToolPalette            *palette,
                                                                       GtkOrientation             orientation);
void                           egg_tool_palette_set_style             (EggToolPalette            *palette,
                                                                       GtkToolbarStyle            style);

GtkIconSize                    egg_tool_palette_get_icon_size         (EggToolPalette            *palette);
GtkOrientation                 egg_tool_palette_get_orientation       (EggToolPalette            *palette);
GtkToolbarStyle                egg_tool_palette_get_style             (EggToolPalette            *palette);

GtkToolItem*                   egg_tool_palette_get_drop_item         (EggToolPalette            *palette,
                                                                       gint                       x,
                                                                       gint                       y);
GtkWidget*                     egg_tool_palette_get_drop_group        (EggToolPalette            *palette,
                                                                       gint                       x,
                                                                       gint                       y);
GtkWidget*                     egg_tool_palette_get_drag_item         (EggToolPalette            *palette,
                                                                       const GtkSelectionData    *selection);

void                           egg_tool_palette_set_drag_source       (EggToolPalette            *palette);
void                           egg_tool_palette_add_drag_dest         (EggToolPalette            *palette,
                                                                       GtkWidget                 *widget,
                                                                       GtkDestDefaults            flags,
                                                                       EggToolPaletteDragTargets  targets,
                                                                       GdkDragAction              actions);

G_CONST_RETURN GtkTargetEntry* egg_tool_palette_get_drag_target_item  (void) G_GNUC_CONST;
G_CONST_RETURN GtkTargetEntry* egg_tool_palette_get_drag_target_group (void) G_GNUC_CONST;


G_END_DECLS

#endif /* __EGG_TOOL_PALETTE_H__ */ 
