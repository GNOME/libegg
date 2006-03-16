/* EGG - The GIMP Toolkit
 * eggpagesetup.h: Page Setup
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

#ifndef __EGG_PAGE_SETUP_H__
#define __EGG_PAGE_SETUP_H__

#include <glib-object.h>
#include <eggprintenums.h>
#include <eggpapersize.h>

G_BEGIN_DECLS

typedef struct _EggPageSetup EggPageSetup;

#define EGG_TYPE_PAGE_SETUP    (egg_page_setup_get_type ())
#define EGG_PAGE_SETUP(obj)    (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_PAGE_SETUP, EggPageSetup))
#define EGG_IS_PAGE_SETUP(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_PAGE_SETUP))


GType              egg_page_setup_get_type          (void);
EggPageSetup *     egg_page_setup_new               (void);
EggPageSetup *     egg_page_setup_copy              (EggPageSetup       *other);
EggPageOrientation egg_page_setup_get_orientation   (EggPageSetup       *setup);
void               egg_page_setup_set_orientation   (EggPageSetup       *setup,
						     EggPageOrientation  orientation);
EggPaperSize *     egg_page_setup_get_paper_size    (EggPageSetup       *setup);
void               egg_page_setup_set_paper_size    (EggPageSetup       *setup,
						     EggPaperSize       *size);
double             egg_page_setup_get_top_margin    (EggPageSetup       *setup,
						     EggUnit             unit);
void               egg_page_setup_set_top_margin    (EggPageSetup       *setup,
						     double              margin,
						     EggUnit             unit);
double             egg_page_setup_get_bottom_margin (EggPageSetup       *setup,
						     EggUnit             unit);
void               egg_page_setup_set_bottom_margin (EggPageSetup       *setup,
						     double              margin,
						     EggUnit             unit);
double             egg_page_setup_get_left_margin   (EggPageSetup       *setup,
						     EggUnit             unit);
void               egg_page_setup_set_left_margin   (EggPageSetup       *setup,
						     double              margin,
						     EggUnit             unit);
double             egg_page_setup_get_right_margin  (EggPageSetup       *setup,
						     EggUnit             unit);
void               egg_page_setup_set_right_margin  (EggPageSetup       *setup,
						     double              margin,
						     EggUnit             unit);

void               egg_page_setup_set_paper_size_and_default_margins (EggPageSetup *setup,
								      EggPaperSize *size);

/* These take orientation, but not margins into consideration */
double             egg_page_setup_get_paper_width   (EggPageSetup       *setup,
						     EggUnit             unit);
double             egg_page_setup_get_paper_height  (EggPageSetup       *setup,
						     EggUnit             unit);


/* These take orientation, and margins into consideration */
double             egg_page_setup_get_page_width    (EggPageSetup       *setup,
						     EggUnit             unit);
double             egg_page_setup_get_page_height   (EggPageSetup       *setup,
						     EggUnit             unit);


/* More in here?
   paper color? paper source? scale? */

G_END_DECLS

#endif /* __EGG_PAGE_SETUP_H__ */
