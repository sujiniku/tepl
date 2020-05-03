/* SPDX-FileCopyrightText: 2016-2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TEPL_BUFFER_H
#define TEPL_BUFFER_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gtksourceview/gtksource.h>
#include <tepl/tepl-file.h>
#include <tepl/tepl-metadata.h>

G_BEGIN_DECLS

#define TEPL_TYPE_BUFFER (tepl_buffer_get_type ())
_TEPL_EXTERN
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

_TEPL_EXTERN
TeplBuffer *		tepl_buffer_new				(void);

_TEPL_EXTERN
TeplFile *		tepl_buffer_get_file			(TeplBuffer *buffer);

_TEPL_EXTERN
TeplMetadata *		tepl_buffer_get_metadata		(TeplBuffer *buffer);

_TEPL_EXTERN
void			tepl_buffer_load_metadata_from_metadata_manager
								(TeplBuffer *buffer);

_TEPL_EXTERN
void			tepl_buffer_save_metadata_into_metadata_manager
								(TeplBuffer *buffer);

_TEPL_EXTERN
gboolean		tepl_buffer_is_untouched		(TeplBuffer *buffer);

_TEPL_EXTERN
gchar *			tepl_buffer_get_short_title		(TeplBuffer *buffer);

_TEPL_EXTERN
gchar *			tepl_buffer_get_full_title		(TeplBuffer *buffer);

_TEPL_EXTERN
gchar *			tepl_buffer_get_style_scheme_id		(TeplBuffer *buffer);

_TEPL_EXTERN
void			tepl_buffer_set_style_scheme_id		(TeplBuffer  *buffer,
								 const gchar *style_scheme_id);

_TEPL_EXTERN
TeplSelectionType	tepl_buffer_get_selection_type		(TeplBuffer *buffer);

G_GNUC_INTERNAL
void			_tepl_buffer_set_as_invalid_character	(TeplBuffer        *buffer,
								 const GtkTextIter *start,
								 const GtkTextIter *end);

G_GNUC_INTERNAL
gboolean		_tepl_buffer_has_invalid_chars		(TeplBuffer *buffer);

G_END_DECLS

#endif /* TEPL_BUFFER_H */
