/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2016, 2017 - Sébastien Wilmet <swilmet@gnome.org>
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

#ifndef TEPL_BUFFER_H
#define TEPL_BUFFER_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gtksourceview/gtksource.h>
#include <tepl/tepl-file.h>

G_BEGIN_DECLS

#define TEPL_TYPE_BUFFER (tepl_buffer_get_type ())
G_DECLARE_DERIVABLE_TYPE (TeplBuffer, tepl_buffer,
			  TEPL, BUFFER,
			  GtkSourceBuffer)

struct _TeplBufferClass
{
	GtkSourceBufferClass parent_class;

	/* Signals */
	void (* tepl_cursor_moved)	(TeplBuffer *buffer);

	gpointer padding[12];
};

/**
 * TeplSelectionType:
 * @TEPL_SELECTION_TYPE_NO_SELECTION: No selection.
 * @TEPL_SELECTION_TYPE_ON_SAME_LINE: The start and end selection bounds are on
 *   the same line.
 * @TEPL_SELECTION_TYPE_MULTIPLE_LINES: The selection spans multiple lines.
 *
 * Since: 1.0
 */
typedef enum _TeplSelectionType
{
	TEPL_SELECTION_TYPE_NO_SELECTION,
	TEPL_SELECTION_TYPE_ON_SAME_LINE,
	TEPL_SELECTION_TYPE_MULTIPLE_LINES
} TeplSelectionType;

TeplBuffer *		tepl_buffer_new				(void);

TeplFile *		tepl_buffer_get_file			(TeplBuffer *buffer);

gboolean		tepl_buffer_is_untouched		(TeplBuffer *buffer);

gchar *			tepl_buffer_get_short_title		(TeplBuffer *buffer);

gchar *			tepl_buffer_get_full_title		(TeplBuffer *buffer);

gchar *			tepl_buffer_get_style_scheme_id		(TeplBuffer *buffer);

void			tepl_buffer_set_style_scheme_id		(TeplBuffer  *buffer,
								 const gchar *style_scheme_id);

TeplSelectionType	tepl_buffer_get_selection_type		(TeplBuffer *buffer);

G_GNUC_INTERNAL
void			_tepl_buffer_set_as_invalid_character	(TeplBuffer        *buffer,
								 const GtkTextIter *start,
								 const GtkTextIter *end);

G_GNUC_INTERNAL
gboolean		_tepl_buffer_has_invalid_chars		(TeplBuffer *buffer);

G_END_DECLS

#endif /* TEPL_BUFFER_H */
