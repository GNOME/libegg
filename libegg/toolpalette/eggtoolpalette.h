#ifndef __EGG_TOOL_PALETTE_H__
#define __EGG_TOOL_PALETTE_H__

#include <gtk/gtkcontainer.h>
#include <gtk/gtktoolitem.h>

G_BEGIN_DECLS

#define EGG_TYPE_TOOL_PALETTE           (egg_tool_palette_get_type())
#define EGG_TOOL_PALETTE(obj)           (G_TYPE_CHECK_INSTANCE_CAST(obj, EGG_TYPE_TOOL_PALETTE, EggToolPalette))
#define EGG_TOOL_PALETTE_CLASS(cls)     (G_TYPE_CHECK_CLASS_CAST(cls, EGG_TYPE_TOOL_PALETTE, EggToolPaletteClass))
#define EGG_IS_TOOL_PALETTE(obj)        (G_TYPE_CHECK_INSTANCE_TYPE(obj, EGG_TYPE_TOOL_PALETTE))
#define EGG_IS_TOOL_PALETTE_CLASS(obj)  (G_TYPE_CHECK_CLASS_TYPE(obj, EGG_TYPE_TOOL_PALETTE))
#define EGG_TOOL_PALETTE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), EGG_TYPE_TOOL_PALETTE, EggToolPaletteClass))

typedef struct _EggToolPalette        EggToolPalette;
typedef struct _EggToolPaletteClass   EggToolPaletteClass;
typedef struct _EggToolPalettePrivate EggToolPalettePrivate;

typedef void (*EggToolPaletteCallback) (EggToolPalette *palette,
                                        GtkToolItem    *item,
                                        gpointer        data);

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

/* ===== instance handling ===== */

GType                 egg_tool_palette_get_type              (void) G_GNUC_CONST;
GtkWidget*            egg_tool_palette_new                   (void);

/* ===== category settings ===== */

void                  egg_tool_palette_set_category_name     (EggToolPalette *palette,
                                                              const gchar    *category,
                                                              const gchar    *name);
void                  egg_tool_palette_set_category_position (EggToolPalette *palette,
                                                              const gchar    *category,
                                                              gint            position);
void                  egg_tool_palette_set_category_expanded (EggToolPalette *palette,
                                                              const gchar    *category,
                                                              gboolean        expanded);

G_CONST_RETURN gchar* egg_tool_palette_get_category_name     (EggToolPalette *palette,
                                                              const gchar    *category);
gint                  egg_tool_palette_get_category_position (EggToolPalette *palette,
                                                              const gchar    *category);
gboolean              egg_tool_palette_get_category_expanded (EggToolPalette *palette,
                                                              const gchar    *category);

/* ===== item packing ===== */

void                  egg_tool_palette_insert            (EggToolPalette *palette,
                                                          const gchar    *category,
                                                          GtkToolItem    *item,
                                                          gint            position);

gint                  egg_tool_palette_get_n_items       (EggToolPalette *palette,
                                                          const gchar    *category);
GtkToolItem*          egg_tool_palette_get_nth_item      (EggToolPalette *palette,
                                                          const gchar    *category,
                                                          gint            index);
GtkToolItem*          egg_tool_palette_get_drop_item     (EggToolPalette *palette,
                                                          gint            x,
                                                          gint            y);

/* ===== item settings ===== */

void                  egg_tool_palette_set_item_category (EggToolPalette *palette,
                                                          GtkToolItem    *item,
                                                          const gchar    *category);
void                  egg_tool_palette_set_item_position (EggToolPalette *palette,
                                                          GtkToolItem    *item,
                                                          gint            position);

G_CONST_RETURN gchar* egg_tool_palette_get_item_category (EggToolPalette *palette,
                                                          GtkToolItem    *item);
gint                  egg_tool_palette_get_item_position (EggToolPalette *palette,
                                                          GtkToolItem    *item);

/* ===== drag-and-drop ===== */

void                  egg_tool_palette_set_drag_source   (EggToolPalette         *palette);
void                  egg_tool_palette_add_drag_dest     (EggToolPalette         *palette,
                                                          GtkWidget              *widget,
                                                          EggToolPaletteCallback  callback,
                                                          gpointer                user_data);

G_END_DECLS

#endif /* __EGG_TOOL_PALETTE_H__ */ 
