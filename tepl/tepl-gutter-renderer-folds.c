/* Copyright 2013, 2016 - Sébastien Wilmet <swilmet@gnome.org>
 * Copyright 2016 - David Rabel <david.rabel@noresoft.com>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "tepl-gutter-renderer-folds.h"

/**
 * SECTION:gutter-renderer-folds
 * @Short_description: Basic gutter renderer for code folding
 * @Title: TeplGutterRendererFolds
 *
 * #TeplGutterRendererFolds is a basic gutter renderer for code folding. It
 * handles only the drawing, and has a flat view of the folding tree.
 */

/* The square size for drawing the box around the minus and plus signs. To be
 * able to draw the sign in the middle of the square, the square size must be an
 * odd number, since the line width is 1.0.
 */
#define SQUARE_SIZE 9

typedef struct _TeplGutterRendererFoldsPrivate TeplGutterRendererFoldsPrivate;

struct _TeplGutterRendererFoldsPrivate
{
	TeplGutterRendererFoldsState folding_state;
};

G_DEFINE_TYPE_WITH_PRIVATE (TeplGutterRendererFolds,
			    tepl_gutter_renderer_folds,
			    GTK_SOURCE_TYPE_GUTTER_RENDERER)

/* Draw a minus or a plus surrounded by a square. */
static void
draw_sign (cairo_t      *cr,
	   GdkRectangle *cell_area,
	   gboolean      folded)
{
	gint sign_width = SQUARE_SIZE - 4;
	gint left_margin;
	gint top_margin;
	gint x;
	gint y;

	/* Integer division */
	left_margin = (cell_area->width - SQUARE_SIZE) / 2;
	top_margin = (cell_area->height - SQUARE_SIZE) / 2;

	x = cell_area->x + left_margin;
	y = cell_area->y + top_margin;

	cairo_rectangle (cr,
			 x + 0.5,
			 y + 0.5,
			 SQUARE_SIZE - 1.0,
			 SQUARE_SIZE - 1.0);

	cairo_move_to (cr,
		       x + 2.5,
		       y + SQUARE_SIZE / 2.0);

	cairo_rel_line_to (cr, sign_width - 1.0, 0);

	if (folded)
	{
		cairo_move_to (cr,
			       x + SQUARE_SIZE / 2.0,
			       y + 2.5);

		cairo_rel_line_to (cr, 0, sign_width - 1.0);
	}
}

static void
draw_vertical_line (cairo_t      *cr,
		    GdkRectangle *cell_area)
{
	/* Integer division */
	gint x = cell_area->x + cell_area->width / 2;

	cairo_move_to (cr,
		       x + 0.5,
		       cell_area->y + 0.5);

	cairo_rel_line_to (cr,
			   0,
			   cell_area->height - 1.0);
}

static void
draw_end (cairo_t      *cr,
	  GdkRectangle *cell_area)
{
	/* Integer division */
	gint x = cell_area->x + cell_area->width / 2;
	gint height = cell_area->height / 2 + 1;

	cairo_move_to (cr,
		       x + 0.5,
		       cell_area->y + 0.5);

	cairo_rel_line_to (cr,
			   0,
			   height - 1.0);

	cairo_line_to (cr,
		       cell_area->x + cell_area->width - 0.5,
		       cell_area->y + height - 0.5);
}

/* To draw the folding states (that can be combined), the cell_area is split in
 * three parts. The middle_area can contain the minus or plus sign, surrounded
 * by a square. It can also contain a vertical bar, or a small horizontal bar to
 * mark a fold end, etc. The top_area and bottom_area can just contain a
 * vertical bar.
 *
 * Returns: %TRUE on success, %FALSE if the @cell_area is too small.
 */
static gboolean
split_cell_area (const GdkRectangle *cell_area,
		 GdkRectangle       *top_area,
		 GdkRectangle       *middle_area,
		 GdkRectangle       *bottom_area)
{
	if (cell_area->height < SQUARE_SIZE ||
	    cell_area->width < SQUARE_SIZE)
	{
		return FALSE;
	}

	top_area->x = cell_area->x;
	top_area->y = cell_area->y;
	top_area->width = cell_area->width;
	top_area->height = (cell_area->height - SQUARE_SIZE) / 2;

	middle_area->x = cell_area->x;
	middle_area->y = top_area->y + top_area->height;
	middle_area->width = cell_area->width;
	middle_area->height = SQUARE_SIZE;

	bottom_area->x = cell_area->x;
	bottom_area->y = middle_area->y + middle_area->height;
	bottom_area->width = cell_area->width;
	bottom_area->height = cell_area->height - top_area->height - middle_area->height;

	return TRUE;
}

static void
tepl_gutter_renderer_folds_draw (GtkSourceGutterRenderer      *renderer,
			         cairo_t                      *cr,
			         GdkRectangle                 *background_area,
			         GdkRectangle                 *cell_area,
			         GtkTextIter                  *start,
			         GtkTextIter                  *end,
			         GtkSourceGutterRendererState  state)
{
	TeplGutterRendererFolds *self;
	TeplGutterRendererFoldsPrivate *priv;
	TeplGutterRendererFoldsState folding_state;
	GdkRectangle top_area;
	GdkRectangle middle_area;
	GdkRectangle bottom_area;

	self = TEPL_GUTTER_RENDERER_FOLDS (renderer);
	priv = tepl_gutter_renderer_folds_get_instance_private (self);

	/* Chain up to draw background */
	if (GTK_SOURCE_GUTTER_RENDERER_CLASS (tepl_gutter_renderer_folds_parent_class)->draw != NULL)
	{
		GTK_SOURCE_GUTTER_RENDERER_CLASS (tepl_gutter_renderer_folds_parent_class)->draw (renderer,
												  cr,
												  background_area,
												  cell_area,
												  start,
												  end,
												  state);
	}

	if (!split_cell_area (cell_area,
			      &top_area,
			      &middle_area,
			      &bottom_area))
	{
		return;
	}

	cairo_save (cr);
	cairo_set_line_cap (cr, CAIRO_LINE_CAP_SQUARE);
	cairo_set_line_width (cr, 1.0);

	folding_state = priv->folding_state;

	/* Top area */

	if (folding_state & TEPL_GUTTER_RENDERER_FOLDS_STATE_CONTINUE ||
	    folding_state & TEPL_GUTTER_RENDERER_FOLDS_STATE_END)
	{
		draw_vertical_line (cr, &top_area);
	}

	/* Middle area */

	if (folding_state & TEPL_GUTTER_RENDERER_FOLDS_STATE_START_FOLDED)
	{
		draw_sign (cr, &middle_area, TRUE);
	}
	else if (folding_state & TEPL_GUTTER_RENDERER_FOLDS_STATE_START_OPENED)
	{
		draw_sign (cr, &middle_area, FALSE);
	}
	else
	{
		if (folding_state & TEPL_GUTTER_RENDERER_FOLDS_STATE_CONTINUE)
		{
			draw_vertical_line (cr, &middle_area);
		}

		if (folding_state & TEPL_GUTTER_RENDERER_FOLDS_STATE_END)
		{
			draw_end (cr, &middle_area);
		}
	}

	/* Bottom area */

	if (folding_state & TEPL_GUTTER_RENDERER_FOLDS_STATE_START_OPENED ||
	    folding_state & TEPL_GUTTER_RENDERER_FOLDS_STATE_CONTINUE)
	{
		draw_vertical_line (cr, &bottom_area);
	}

	cairo_stroke (cr);
	cairo_restore (cr);
}

static void
tepl_gutter_renderer_folds_constructed (GObject *object)
{
	GtkSourceGutterRenderer *renderer = GTK_SOURCE_GUTTER_RENDERER (object);

	G_OBJECT_CLASS (tepl_gutter_renderer_folds_parent_class)->constructed (object);

	gtk_source_gutter_renderer_set_size (renderer, SQUARE_SIZE);
	gtk_source_gutter_renderer_set_padding (renderer, 2, -1);
}

static void
tepl_gutter_renderer_folds_class_init (TeplGutterRendererFoldsClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkSourceGutterRendererClass *renderer_class = GTK_SOURCE_GUTTER_RENDERER_CLASS (klass);

	object_class->constructed = tepl_gutter_renderer_folds_constructed;

	renderer_class->draw = tepl_gutter_renderer_folds_draw;
}

static void
tepl_gutter_renderer_folds_init (TeplGutterRendererFolds *self)
{
}

/**
 * tepl_gutter_renderer_folds_new:
 *
 * Returns: a new #TeplGutterRendererFolds.
 * Since: 1.0
 */
GtkSourceGutterRenderer *
tepl_gutter_renderer_folds_new (void)
{
	return g_object_new (TEPL_TYPE_GUTTER_RENDERER_FOLDS, NULL);
}

/**
 * tepl_gutter_renderer_folds_set_state:
 * @self: a #TeplGutterRendererFolds.
 * @state: a #TeplGutterRendererFoldsState.
 *
 * Sets the folding state of the next cell to be drawn.
 *
 * This function is intended to be called from a subclass' draw method before
 * chaining-up to its parent's draw method.
 *
 * Since: 1.0
 */
void
tepl_gutter_renderer_folds_set_state (TeplGutterRendererFolds 	   *self,
                                      TeplGutterRendererFoldsState  state)
{
	TeplGutterRendererFoldsPrivate *priv;

	g_return_if_fail (TEPL_IS_GUTTER_RENDERER_FOLDS (self));

	priv = tepl_gutter_renderer_folds_get_instance_private (self);
	priv->folding_state = state;
}
