/* SPDX-FileCopyrightText: 2016 - David Rabel <david.rabel@noresoft.com>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TEPL_FOLD_REGION_H
#define TEPL_FOLD_REGION_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include <tepl/tepl-macros.h>

G_BEGIN_DECLS

#define TEPL_TYPE_FOLD_REGION (tepl_fold_region_get_type ())
_TEPL_EXTERN
G_DECLARE_DERIVABLE_TYPE (TeplFoldRegion, tepl_fold_region,
			  TEPL, FOLD_REGION,
			  GObject)

struct _TeplFoldRegionClass
{
	GObjectClass parent_class;

	gpointer padding[12];
};

_TEPL_EXTERN
TeplFoldRegion *
		tepl_fold_region_new		(GtkTextBuffer     *buffer,
						 const GtkTextIter *start,
						 const GtkTextIter *end);

_TEPL_EXTERN
GtkTextBuffer * tepl_fold_region_get_buffer	(TeplFoldRegion    *fold_region);

_TEPL_EXTERN
gboolean	tepl_fold_region_get_folded 	(TeplFoldRegion    *fold_region);

_TEPL_EXTERN
void		tepl_fold_region_set_folded 	(TeplFoldRegion    *fold_region,
						 gboolean           folded);

_TEPL_EXTERN
gboolean	tepl_fold_region_get_bounds	(TeplFoldRegion    *fold_region,
						 GtkTextIter       *start,
						 GtkTextIter       *end);

_TEPL_EXTERN
void		tepl_fold_region_set_bounds	(TeplFoldRegion    *fold_region,
						 const GtkTextIter *start,
						 const GtkTextIter *end);

G_END_DECLS

#endif /* TEPL_FOLD_REGION_H */
