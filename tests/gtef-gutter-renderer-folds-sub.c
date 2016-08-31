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

#include "gtef-gutter-renderer-folds-sub.h"

G_DEFINE_TYPE(GtefGutterRendererFoldsSub,
              gtef_gutter_renderer_folds_sub,
              GTEF_TYPE_GUTTER_RENDERER_FOLDS)

static void
gtef_gutter_renderer_folds_sub_init (GtefGutterRendererFoldsSub *self)
{
}

static void
draw (GtkSourceGutterRenderer      *renderer,
      cairo_t                      *cr,
      GdkRectangle                 *background_area,
      GdkRectangle                 *cell_area,
      GtkTextIter                  *start,
      GtkTextIter                  *end,
      GtkSourceGutterRendererState  state)
{
	gint line_num;
	GtefGutterRendererFoldsState folding_state;

	line_num = gtk_text_iter_get_line (start);

	if (line_num == 0)
	{
		folding_state = FOLDING_START_FOLDED;
	}
	else if (line_num == 1)
	{
		folding_state = FOLDING_START_OPENED;
	}
	else if (line_num < 5)
	{
		folding_state = FOLDING_CONTINUE;
	}
	else if (line_num == 5)
	{
		folding_state = FOLDING_END;
	}
	else if (line_num == 6)
	{
		folding_state = FOLDING_NONE;
	}
	else if (line_num == 7)
	{
		folding_state = FOLDING_START_OPENED;
	}
	else if (line_num == 8)
	{
		folding_state = FOLDING_CONTINUE;
	}
	else if (line_num == 9)
	{
		folding_state = FOLDING_CONTINUE | FOLDING_START_OPENED;
	}
	else if (line_num < 12)
	{
		folding_state = FOLDING_CONTINUE;
	}
	else if (line_num == 12)
	{
		folding_state = FOLDING_CONTINUE | FOLDING_END;
	}
	else if (line_num == 13)
	{
		folding_state = FOLDING_CONTINUE;
	}
	else if (line_num == 14)
	{
		folding_state = FOLDING_CONTINUE | FOLDING_START_FOLDED;
	}
	else if (line_num == 15)
	{
		folding_state = FOLDING_CONTINUE;
	}
	else if (line_num == 16)
	{
		folding_state = FOLDING_END;
	}
	else
	{
		folding_state = FOLDING_NONE;
	}

	gtef_gutter_renderer_folds_set_state (GTEF_GUTTER_RENDERER_FOLDS (renderer), folding_state);


	if (GTK_SOURCE_GUTTER_RENDERER_CLASS (gtef_gutter_renderer_folds_sub_parent_class)->draw != NULL)
	{
		GTK_SOURCE_GUTTER_RENDERER_CLASS (gtef_gutter_renderer_folds_sub_parent_class)->draw (renderer,
												      cr,
												      background_area,
												      cell_area,
												      start,
												      end,
												      state);
	}
}

static void
gtef_gutter_renderer_folds_sub_class_init (GtefGutterRendererFoldsSubClass *klass)
{
	GtkSourceGutterRendererClass *renderer_class = GTK_SOURCE_GUTTER_RENDERER_CLASS (klass);

	renderer_class->draw = draw;
}

GtkSourceGutterRenderer *
gtef_gutter_renderer_folds_sub_new (void)
{
	return g_object_new (GTEF_TYPE_GUTTER_RENDERER_FOLDS_SUB,
			     "size", 9,
			     "xpad", 2,
			     NULL);
}
