/*
 * This file is part of Gtef, a text editor library.
 *
 * Copyright 2016 - Sébastien Wilmet <swilmet@gnome.org>
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

#include "gtef-iter.h"

/**
 * SECTION:iter
 * @Short_description: GtkTextIter utility functions
 * @Title: GtefIter
 *
 * #GtkTextIter utility functions.
 */

/* Get the boundary, on @iter's line, between leading spaces (indentation) and
 * the text.
 *
 * Copied from gtksourceiter.c:
 * _gtk_source_iter_get_leading_spaces_end_boundary().
 */
static void
get_leading_spaces_end_boundary (const GtkTextIter *iter,
				 GtkTextIter       *leading_end)
{
	g_return_if_fail (iter != NULL);
	g_return_if_fail (leading_end != NULL);

	*leading_end = *iter;
	gtk_text_iter_set_line_offset (leading_end, 0);

	while (!gtk_text_iter_is_end (leading_end))
	{
		gunichar ch = gtk_text_iter_get_char (leading_end);

		if (!g_unichar_isspace (ch))
		{
			break;
		}

		gtk_text_iter_forward_char (leading_end);
	}
}

/**
 * gtef_iter_get_line_indentation:
 * @iter: a #GtkTextIter.
 *
 * Gets the indentation, as a string, of the line at @iter. @iter can be
 * anywhere in the line.
 *
 * Possible use-case: to implement an action that inserts some text in the
 * #GtkTextBuffer. If the text to insert spans multiple lines, it is usually
 * desired to keep the same indentation level.
 *
 * Returns: the line indentation at @iter. Free with g_free().
 * Since: 2.0
 */
gchar *
gtef_iter_get_line_indentation (const GtkTextIter *iter)
{
	GtkTextIter line_start;
	GtkTextIter leading_end;

	g_return_val_if_fail (iter != NULL, NULL);

	line_start = *iter;
	gtk_text_iter_set_line_offset (&line_start, 0);

	get_leading_spaces_end_boundary (iter, &leading_end);

	return gtk_text_iter_get_text (&line_start, &leading_end);
}
