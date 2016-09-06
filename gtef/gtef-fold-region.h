/*
 * This file is part of Gtef, a text editor library.
 *
 * Copyright 2016 - David Rabel <david.rabel@noresoft.com>
 *
 * Gtef is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * Gtef is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GTEF_FOLD_REGION_H
#define GTEF_FOLD_REGION_H

#if !defined (GTEF_H_INSIDE) && !defined (GTEF_COMPILATION)
#error "Only <gtef/gtef.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include <gtef/gtef-types.h>

G_BEGIN_DECLS

#define GTEF_TYPE_FOLD_REGION (gtef_fold_region_get_type ())
G_DECLARE_DERIVABLE_TYPE (GtefFoldRegion, gtef_fold_region,
			  GTEF, FOLD_REGION,
			  GObject)

struct _GtefFoldRegionClass
{
	GObjectClass parent_class;

	gpointer padding[12];
};

GtefFoldRegion *
		gtef_fold_region_new		(GtkTextBuffer     *buffer,
						 const GtkTextIter *start,
						 const GtkTextIter *end);

GtkTextBuffer * gtef_fold_region_get_buffer	(GtefFoldRegion    *fold_region);

gboolean	gtef_fold_region_get_folded 	(GtefFoldRegion    *fold_region);

void		gtef_fold_region_set_folded 	(GtefFoldRegion    *fold_region,
						 gboolean           folded);

gboolean	gtef_fold_region_get_bounds	(GtefFoldRegion    *fold_region,
						 GtkTextIter       *start,
						 GtkTextIter       *end);

void		gtef_fold_region_set_bounds	(GtefFoldRegion    *fold_region,
						 const GtkTextIter *start,
						 const GtkTextIter *end);

G_END_DECLS

#endif /* GTEF_FOLD_REGION_H */
