/* eggfilesystem-unix.h
 * Copyright (C) 2002  Anders Carlsson <andersca@gnu.org>
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
#ifndef __EGG_FILE_SYSTEM_UNIX_H__
#define __EGG_FILE_SYSTEM_UNIX_H__

#include "eggfilesystem.h"

G_BEGIN_DECLS

#define EGG_TYPE_FILE_SYSTEM_UNIX		(egg_file_system_unix_get_type ())
#define EGG_FILE_SYSTEM_UNIX(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_FILE_SYSTEM_UNIX, EggFileSystemUnix))
#define EGG_FILE_SYSTEM_UNIX_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), EGG_TYPE_FILE_SYSTEM_UNIX, EggFileSystemUnixClass))
#define EGG_IS_FILE_SYSTEM_UNIX(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_FILE_SYSTEM_UNIX))
#define EGG_IS_FILE_SYSTEM_UNIX_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), EGG_TYPE_FILE_SYSTEM_UNIX))
#define EGG_FILE_SYSTEM_UNIX_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_FILE_SYSTEM_UNIX, EggFileSystemUnixClass))

typedef struct _EggFileSystemUnix           EggFileSystemUnix;
typedef struct _EggFileSystemUnixClass      EggFileSystemUnixClass;

struct _EggFileSystemUnix
{
  GObject parent;
};

struct _EggFileSystemUnixClass
{
  GObjectClass parent_class;
};

GType      egg_file_system_unix_get_type      (void);

G_END_DECLS

#endif /* __EGG_FILE_SYSTEM_UNIX_H__ */
