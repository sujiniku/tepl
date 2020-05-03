/* SPDX-FileCopyrightText: 2016 - David Rabel <david.rabel@noresoft.com>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "tepl-gutter-renderer-folds-sub.h"

G_DEFINE_TYPE (TeplGutterRendererFoldsSub,
	       tepl_gutter_renderer_folds_sub,
	       TEPL_TYPE_GUTTER_RENDERER_FOLDS)

static void
tepl_gutter_renderer_folds_sub_draw (GtkSourceGutterRenderer      *renderer,
				     cairo_t                      *cr,
				     GdkRectangle                 *background_area,
				     GdkRectangle                 *cell_area,
				     GtkTextIter                  *start,
				     GtkTextIter                  *end,
				     GtkSourceGutterRendererState  state)
{
	gint line_num;
	TeplGutterRendererFoldsState folding_state;

	line_num = gtk_text_iter_get_line (start);

	if (line_num == 0)
	{
		folding_state = TEPL_GUTTER_RENDERER_FOLDS_STATE_START_FOLDED;
	}
	else if (line_num == 1)
	{
		folding_state = TEPL_GUTTER_RENDERER_FOLDS_STATE_START_OPENED;
	}
	else if (line_num < 5)
	{
		folding_state = TEPL_GUTTER_RENDERER_FOLDS_STATE_CONTINUE;
	}
	else if (line_num == 5)
	{
		folding_state = TEPL_GUTTER_RENDERER_FOLDS_STATE_END;
	}
	else if (line_num == 6)
	{
		folding_state = TEPL_GUTTER_RENDERER_FOLDS_STATE_NONE;
	}
	else if (line_num == 7)
	{
		folding_state = TEPL_GUTTER_RENDERER_FOLDS_STATE_START_OPENED;
	}
	else if (line_num == 8)
	{
		folding_state = TEPL_GUTTER_RENDERER_FOLDS_STATE_CONTINUE;
	}
	else if (line_num == 9)
	{
		folding_state = (TEPL_GUTTER_RENDERER_FOLDS_STATE_CONTINUE |
				 TEPL_GUTTER_RENDERER_FOLDS_STATE_START_OPENED);
	}
	else if (line_num < 12)
	{
		folding_state = TEPL_GUTTER_RENDERER_FOLDS_STATE_CONTINUE;
	}
	else if (line_num == 12)
	{
		folding_state = (TEPL_GUTTER_RENDERER_FOLDS_STATE_CONTINUE |
				 TEPL_GUTTER_RENDERER_FOLDS_STATE_END);
	}
	else if (line_num == 13)
	{
		folding_state = TEPL_GUTTER_RENDERER_FOLDS_STATE_CONTINUE;
	}
	else if (line_num == 14)
	{
		folding_state = (TEPL_GUTTER_RENDERER_FOLDS_STATE_CONTINUE |
				 TEPL_GUTTER_RENDERER_FOLDS_STATE_START_FOLDED);
	}
	else if (line_num == 15)
	{
		folding_state = TEPL_GUTTER_RENDERER_FOLDS_STATE_CONTINUE;
	}
	else if (line_num == 16)
	{
		folding_state = TEPL_GUTTER_RENDERER_FOLDS_STATE_END;
	}
	else
	{
		folding_state = TEPL_GUTTER_RENDERER_FOLDS_STATE_NONE;
	}

	tepl_gutter_renderer_folds_set_state (TEPL_GUTTER_RENDERER_FOLDS (renderer), folding_state);

	if (GTK_SOURCE_GUTTER_RENDERER_CLASS (tepl_gutter_renderer_folds_sub_parent_class)->draw != NULL)
	{
		GTK_SOURCE_GUTTER_RENDERER_CLASS (tepl_gutter_renderer_folds_sub_parent_class)->draw (renderer,
												      cr,
												      background_area,
												      cell_area,
												      start,
												      end,
												      state);
	}
}

static void
tepl_gutter_renderer_folds_sub_class_init (TeplGutterRendererFoldsSubClass *klass)
{
	GtkSourceGutterRendererClass *renderer_class = GTK_SOURCE_GUTTER_RENDERER_CLASS (klass);

	renderer_class->draw = tepl_gutter_renderer_folds_sub_draw;
}

static void
tepl_gutter_renderer_folds_sub_init (TeplGutterRendererFoldsSub *self)
{
}

GtkSourceGutterRenderer *
tepl_gutter_renderer_folds_sub_new (void)
{
	return g_object_new (TEPL_TYPE_GUTTER_RENDERER_FOLDS_SUB, NULL);
}
