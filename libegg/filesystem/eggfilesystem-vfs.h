/* eggfilesystem-vfs.h
 * Copyright (C) 2002  Bastien Nocera <hadess@hadess.net>
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
#ifndef __EGG_FILE_SYSTEM_VFS_H__
#define __EGG_FILE_SYSTEM_VFS_H__

#include "eggfilesystem.h"

G_BEGIN_DECLS

#define EGG_TYPE_FILE_SYSTEM_VFS		(egg_file_system_vfs_get_type ())
#define EGG_FILE_SYSTEM_VFS(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_FILE_SYSTEM_VFS, EggFileSystemVfs))
#define EGG_FILE_SYSTEM_VFS_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), EGG_TYPE_FILE_SYSTEM_VFS, EggFileSystemVfsClass))
#define EGG_IS_FILE_SYSTEM_VFS(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_FILE_SYSTEM_VFS))
#define EGG_IS_FILE_SYSTEM_VFS_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), EGG_TYPE_FILE_SYSTEM_VFS))
#define EGG_FILE_SYSTEM_VFS_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_FILE_SYSTEM_VFS, EggFileSystemVfsClass))

typedef struct _EggFileSystemVfs           EggFileSystemVfs;
typedef struct _EggFileSystemVfsPrivate    EggFileSystemVfsPrivate;
typedef struct _EggFileSystemVfsClass      EggFileSystemVfsClass;

struct _EggFileSystemVfs
{
  GObject parent;
  EggFileSystemVfsPrivate *priv;
};

struct _EggFileSystemVfsClass
{
  GObjectClass parent_class;
};

GType      egg_file_system_vfs_get_type      (void);

G_END_DECLS

#endif /* __EGG_FILE_SYSTEM_VFS_H__ */
