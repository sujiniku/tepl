/*
 * This file is part of Gtef, a text editor library.
 *
 * Copyright 2016 - Sébastien Wilmet <swilmet@gnome.org>,
 *                  David Rabel <david.rabel@noresoft.com>
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

#ifndef GTEF_GUTTER_RENDERER_FOLDS_H
#define GTEF_GUTTER_RENDERER_FOLDS_H

#if !defined (GTEF_H_INSIDE) && !defined (GTEF_COMPILATION)
#error "Only <gtef/gtef.h> can be included directly."
#endif

#include <gtksourceview/gtksource.h>
#include <gtef/gtef-types.h>

G_BEGIN_DECLS

#define GTEF_TYPE_GUTTER_RENDERER_FOLDS	(gtef_gutter_renderer_folds_get_type ())
G_DECLARE_DERIVABLE_TYPE		(GtefGutterRendererFolds,
					 gtef_gutter_renderer_folds,
					 GTEF, GUTTER_RENDERER_FOLDS,
					 GtkSourceGutterRenderer)

struct _GtefGutterRendererFoldsClass
{
	GtkSourceGutterRendererClass parent_class;
	/* Padding to allow adding up to 12 new virtual functions without
	* breaking ABI. */
	gpointer padding[12];
};

/**
 * GtefGutterRendererFoldsState:
 * @FOLDING_NONE: No code folding here.
 * @FOLDING_START_FOLDED: Start of currently folded fold region.
 * @FOLDING_START_OPENED: Start of currently opened fold region.
 * @FOLDING_CONTINUE: Fold region continues.
 * @FOLDING_END: End of fold region.
 *
 * The folding state at a certain line in the text buffer.
 * Some states can be combined. For example, %FOLDING_END and %FOLDING_CONTINUE.
 *
 * Since: 1.0
 */
typedef enum
{
	FOLDING_NONE		= 0,
	FOLDING_START_FOLDED	= 1 << 0,
	FOLDING_START_OPENED	= 1 << 1,
	FOLDING_CONTINUE	= 1 << 2,
	FOLDING_END		= 1 << 3
} GtefGutterRendererFoldsState;

GtkSourceGutterRenderer	*gtef_gutter_renderer_folds_new	      (void);

G_GNUC_INTERNAL
void			 gtef_gutter_renderer_folds_set_state (GtefGutterRendererFolds      *self,
							       GtefGutterRendererFoldsState  state);

G_END_DECLS

#endif /* GTEF_GUTTER_RENDERER_FOLDS_H */
