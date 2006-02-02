/*
 * egg-dock-object.h - Abstract base class for all dock related objects
 *
 * Copyright (C) 2002 Gustavo Giráldez <gustavo.giraldez@gmx.net>
 * Copyright (C) 2003 Biswapesh Chattopadhyay <biswapesh_chatterjee@tcscal.co.in>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.  
 */

#ifndef __EGG_DOCK_OBJECT_H__
#define __EGG_DOCK_OBJECT_H__

#include <gtk/gtkcontainer.h>

G_BEGIN_DECLS

/* standard macros */
#define EGG_TYPE_DOCK_OBJECT             (egg_dock_object_get_type ())
#define EGG_DOCK_OBJECT(obj)             (GTK_CHECK_CAST ((obj), EGG_TYPE_DOCK_OBJECT, EggDockObject))
#define EGG_DOCK_OBJECT_CLASS(klass)     (GTK_CHECK_CLASS_CAST ((klass), EGG_TYPE_DOCK_OBJECT, EggDockObjectClass))
#define EGG_IS_DOCK_OBJECT(obj)          (GTK_CHECK_TYPE ((obj), EGG_TYPE_DOCK_OBJECT))
#define EGG_IS_DOCK_OBJECT_CLASS(klass)  (GTK_CHECK_CLASS_TYPE ((klass), EGG_TYPE_DOCK_OBJECT))
#define EGG_DOCK_OBJECT_GET_CLASS(obj)   (GTK_CHECK_GET_CLASS ((obj), GTK_TYPE_DOCK_OBJECT, EggDockObjectClass))

/* data types & structures */
typedef enum {
    /* the parameter is to be exported for later layout rebuilding */
    EGG_DOCK_PARAM_EXPORT = 1 << G_PARAM_USER_SHIFT,
    /* the parameter must be set after adding the children objects */
    EGG_DOCK_PARAM_AFTER  = 1 << (G_PARAM_USER_SHIFT + 1)
} EggDockParamFlags;

#define EGG_DOCK_NAME_PROPERTY    "name"
#define EGG_DOCK_MASTER_PROPERTY  "master"

typedef enum {
    EGG_DOCK_AUTOMATIC  = 1 << 0,
    EGG_DOCK_ATTACHED   = 1 << 1,
    EGG_DOCK_IN_REFLOW  = 1 << 2,
    EGG_DOCK_IN_DETACH  = 1 << 3
} EggDockObjectFlags;

#define EGG_DOCK_OBJECT_FLAGS_SHIFT 8

typedef enum {
    EGG_DOCK_NONE = 0,
    EGG_DOCK_TOP,
    EGG_DOCK_BOTTOM,
    EGG_DOCK_RIGHT,
    EGG_DOCK_LEFT,
    EGG_DOCK_CENTER,
    EGG_DOCK_FLOATING
} EggDockPlacement;

typedef struct _EggDockObject      EggDockObject;
typedef struct _EggDockObjectClass EggDockObjectClass;
typedef struct _EggDockRequest     EggDockRequest;
typedef struct _EggDockObjectPrivate EggDockObjectPrivate;

struct _EggDockRequest {
    EggDockObject    *applicant;
    EggDockObject    *target;
    EggDockPlacement  position;
    GdkRectangle      rect;
    GValue            extra;
};

struct _EggDockObject {
    GtkContainer        container;

    EggDockObjectFlags  flags;
    gint                freeze_count;
    
    GObject            *master;
    gchar              *name;
    gchar              *long_name;
    
    gboolean            reduce_pending;
	EggDockObjectPrivate *_priv;
};

struct _EggDockObjectClass {
    GtkContainerClass parent_class;

    gboolean          is_compound;
    
    void     (* detach)          (EggDockObject    *object,
                                  gboolean          recursive);
    void     (* reduce)          (EggDockObject    *object);

    gboolean (* dock_request)    (EggDockObject    *object,
                                  gint              x,
                                  gint              y,
                                  EggDockRequest   *request);

    void     (* dock)            (EggDockObject    *object,
                                  EggDockObject    *requestor,
                                  EggDockPlacement  position,
                                  GValue           *other_data);
    
    gboolean (* reorder)         (EggDockObject    *object,
                                  EggDockObject    *child,
                                  EggDockPlacement  new_position,
                                  GValue           *other_data);

    void     (* present)         (EggDockObject    *object,
                                  EggDockObject    *child);

    gboolean (* child_placement) (EggDockObject    *object,
                                  EggDockObject    *child,
                                  EggDockPlacement *placement);

	gpointer unused[2];
};

/* additional macros */
#define EGG_DOCK_OBJECT_FLAGS(obj)  (EGG_DOCK_OBJECT (obj)->flags)
#define EGG_DOCK_OBJECT_AUTOMATIC(obj) \
    ((EGG_DOCK_OBJECT_FLAGS (obj) & EGG_DOCK_AUTOMATIC) != 0)
#define EGG_DOCK_OBJECT_ATTACHED(obj) \
    ((EGG_DOCK_OBJECT_FLAGS (obj) & EGG_DOCK_ATTACHED) != 0)
#define EGG_DOCK_OBJECT_IN_REFLOW(obj) \
    ((EGG_DOCK_OBJECT_FLAGS (obj) & EGG_DOCK_IN_REFLOW) != 0)
#define EGG_DOCK_OBJECT_IN_DETACH(obj) \
    ((EGG_DOCK_OBJECT_FLAGS (obj) & EGG_DOCK_IN_DETACH) != 0)

#define EGG_DOCK_OBJECT_SET_FLAGS(obj,flag) \
    G_STMT_START { (EGG_DOCK_OBJECT_FLAGS (obj) |= (flag)); } G_STMT_END
#define EGG_DOCK_OBJECT_UNSET_FLAGS(obj,flag) \
    G_STMT_START { (EGG_DOCK_OBJECT_FLAGS (obj) &= ~(flag)); } G_STMT_END
 
#define EGG_DOCK_OBJECT_FROZEN(obj) (EGG_DOCK_OBJECT (obj)->freeze_count > 0)


/* public interface */
 
GType          egg_dock_object_get_type          (void);

gboolean       egg_dock_object_is_compound       (EggDockObject    *object);

void           egg_dock_object_detach            (EggDockObject    *object,
                                                  gboolean          recursive);

EggDockObject *egg_dock_object_get_parent_object (EggDockObject    *object);

void           egg_dock_object_freeze            (EggDockObject    *object);
void           egg_dock_object_thaw              (EggDockObject    *object);

void           egg_dock_object_reduce            (EggDockObject    *object);

gboolean       egg_dock_object_dock_request      (EggDockObject    *object,
                                                  gint              x,
                                                  gint              y,
                                                  EggDockRequest   *request);
void           egg_dock_object_dock              (EggDockObject    *object,
                                                  EggDockObject    *requestor,
                                                  EggDockPlacement  position,
                                                  GValue           *other_data);

void           egg_dock_object_bind              (EggDockObject    *object,
                                                  GObject          *master);
void           egg_dock_object_unbind            (EggDockObject    *object);
gboolean       egg_dock_object_is_bound          (EggDockObject    *object);

gboolean       egg_dock_object_reorder           (EggDockObject    *object,
                                                  EggDockObject    *child,
                                                  EggDockPlacement  new_position,
                                                  GValue           *other_data);

void           egg_dock_object_present           (EggDockObject    *object,
                                                  EggDockObject    *child);

gboolean       egg_dock_object_child_placement   (EggDockObject    *object,
                                                  EggDockObject    *child,
                                                  EggDockPlacement *placement);

/* other types */

/* this type derives from G_TYPE_STRING and is meant to be the basic
   type for serializing object parameters which are exported
   (i.e. those that are needed for layout rebuilding) */
#define EGG_TYPE_DOCK_PARAM   (egg_dock_param_get_type ())

GType egg_dock_param_get_type (void);

/* functions for setting/retrieving nick names for serializing EggDockObject types */
G_CONST_RETURN gchar *egg_dock_object_nick_from_type    (GType        type);
GType                 egg_dock_object_type_from_nick    (const gchar *nick);
GType                 egg_dock_object_set_type_for_nick (const gchar *nick,
                                                         GType        type);


/* helper macros */
#define EGG_TRACE_OBJECT(object, format, args...) \
    G_STMT_START {                            \
    g_log (G_LOG_DOMAIN,                      \
	   G_LOG_LEVEL_DEBUG,                 \
           "%s:%d (%s) %s [%p %d%s:%d]: "format, \
	   __FILE__,                          \
	   __LINE__,                          \
	   __PRETTY_FUNCTION__,               \
           G_OBJECT_TYPE_NAME (object), object, \
           G_OBJECT (object)->ref_count, \
           (GTK_IS_OBJECT (object) && GTK_OBJECT_FLOATING (object)) ? "(float)" : "", \
           EGG_IS_DOCK_OBJECT (object) ? EGG_DOCK_OBJECT (object)->freeze_count : -1, \
	   ##args); } G_STMT_END                   
    


G_END_DECLS

#endif  /* __EGG_DOCK_OBJECT_H__ */
