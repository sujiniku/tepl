/* Copyright 2013, 2016 - Sébastien Wilmet <swilmet@gnome.org>
 * Copyright 2016 - David Rabel <david.rabel@noresoft.com>
 *
 * Tepl is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * Tepl is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TEPL_GUTTER_RENDERER_FOLDS_H
#define TEPL_GUTTER_RENDERER_FOLDS_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gtksourceview/gtksource.h>
#include <tepl/tepl-macros.h>

G_BEGIN_DECLS

#define TEPL_TYPE_GUTTER_RENDERER_FOLDS (tepl_gutter_renderer_folds_get_type ())
_TEPL_EXTERN
G_DECLARE_DERIVABLE_TYPE (TeplGutterRendererFolds, tepl_gutter_renderer_folds,
			  TEPL, GUTTER_RENDERER_FOLDS,
			  GtkSourceGutterRenderer)

struct _TeplGutterRendererFoldsClass
{
	GtkSourceGutterRendererClass parent_class;

	gpointer padding[12];
};

/**
 * TeplGutterRendererFoldsState:
 * @TEPL_GUTTER_RENDERER_FOLDS_STATE_NONE: No code folding here.
 * @TEPL_GUTTER_RENDERER_FOLDS_STATE_START_FOLDED: Start of currently folded
 *   fold region.
 * @TEPL_GUTTER_RENDERER_FOLDS_STATE_START_OPENED: Start of currently opened
 *   fold region.
 * @TEPL_GUTTER_RENDERER_FOLDS_STATE_CONTINUE: Fold region continues.
 * @TEPL_GUTTER_RENDERER_FOLDS_STATE_END: End of fold region.
 *
 * The folding state at a certain line in the #GtkTextBuffer.
 *
 * Since #TeplGutterRendererFolds has a flat view of the folding tree, some
 * states can be combined; for example, %TEPL_GUTTER_RENDERER_FOLDS_STATE_END
 * and %TEPL_GUTTER_RENDERER_FOLDS_STATE_CONTINUE.
 *
 * Since: 1.0
 */
typedef enum
{
	TEPL_GUTTER_RENDERER_FOLDS_STATE_NONE		= 0,
	TEPL_GUTTER_RENDERER_FOLDS_STATE_START_FOLDED	= 1 << 0,
	TEPL_GUTTER_RENDERER_FOLDS_STATE_START_OPENED	= 1 << 1,
	TEPL_GUTTER_RENDERER_FOLDS_STATE_CONTINUE	= 1 << 2,
	TEPL_GUTTER_RENDERER_FOLDS_STATE_END		= 1 << 3
} TeplGutterRendererFoldsState;

_TEPL_EXTERN
GtkSourceGutterRenderer *
		tepl_gutter_renderer_folds_new			(void);

_TEPL_EXTERN
void		tepl_gutter_renderer_folds_set_state		(TeplGutterRendererFolds      *self,
								 TeplGutterRendererFoldsState  state);

G_END_DECLS

#endif /* TEPL_GUTTER_RENDERER_FOLDS_H */
