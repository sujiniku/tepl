/*
 * This file is part of Gtef, a text editor library.
 *
 * Copyright 2005, 2007 - Paolo Maggi
 * Copyrhing 2007 - Steve Frécinaux
 * Copyright 2008 - Jesse van den Kieboom
 * Copyright 2014, 2016 - Sébastien Wilmet
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

#ifndef GTEF_FILE_SAVER_H
#define GTEF_FILE_SAVER_H

#if !defined (GTEF_H_INSIDE) && !defined (GTEF_COMPILATION)
#error "Only <gtef/gtef.h> can be included directly."
#endif

#include <gtksourceview/gtksource.h>
#include <gtef/gtef-types.h>
#include <gtef/gtef-file.h>

G_BEGIN_DECLS

#define GTEF_TYPE_FILE_SAVER              (gtef_file_saver_get_type())
#define GTEF_FILE_SAVER(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GTEF_TYPE_FILE_SAVER, GtefFileSaver))
#define GTEF_FILE_SAVER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GTEF_TYPE_FILE_SAVER, GtefFileSaverClass))
#define GTEF_IS_FILE_SAVER(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GTEF_TYPE_FILE_SAVER))
#define GTEF_IS_FILE_SAVER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GTEF_TYPE_FILE_SAVER))
#define GTEF_FILE_SAVER_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GTEF_TYPE_FILE_SAVER, GtefFileSaverClass))

typedef struct _GtefFileSaverClass   GtefFileSaverClass;
typedef struct _GtefFileSaverPrivate GtefFileSaverPrivate;

#define GTEF_FILE_SAVER_ERROR gtef_file_saver_error_quark ()

/**
 * GtefFileSaverError:
 * @GTEF_FILE_SAVER_ERROR_INVALID_CHARS: The buffer contains invalid
 *   characters.
 * @GTEF_FILE_SAVER_ERROR_EXTERNALLY_MODIFIED: The file is externally
 *   modified.
 *
 * An error code used with the %GTEF_FILE_SAVER_ERROR domain.
 * Since: 1.0
 */
typedef enum
{
	GTEF_FILE_SAVER_ERROR_INVALID_CHARS,
	GTEF_FILE_SAVER_ERROR_EXTERNALLY_MODIFIED
} GtefFileSaverError;

/**
 * GtefFileSaverFlags:
 * @GTEF_FILE_SAVER_FLAGS_NONE: No flags.
 * @GTEF_FILE_SAVER_FLAGS_IGNORE_INVALID_CHARS: Ignore invalid characters.
 * @GTEF_FILE_SAVER_FLAGS_IGNORE_MODIFICATION_TIME: Save file despite external modifications.
 * @GTEF_FILE_SAVER_FLAGS_CREATE_BACKUP: Create a backup before saving the file.
 *
 * Flags to define the behavior of a #GtefFileSaver.
 * Since: 1.0
 */
typedef enum
{
	GTEF_FILE_SAVER_FLAGS_NONE			= 0,
	GTEF_FILE_SAVER_FLAGS_IGNORE_INVALID_CHARS	= 1 << 0,
	GTEF_FILE_SAVER_FLAGS_IGNORE_MODIFICATION_TIME	= 1 << 1,
	GTEF_FILE_SAVER_FLAGS_CREATE_BACKUP		= 1 << 2
} GtefFileSaverFlags;

struct _GtefFileSaver
{
	GObject object;

	GtefFileSaverPrivate *priv;
};

struct _GtefFileSaverClass
{
	GObjectClass parent_class;

	gpointer padding[10];
};

GType			 gtef_file_saver_get_type		(void);

GQuark			 gtef_file_saver_error_quark		(void);

GtefFileSaver *		 gtef_file_saver_new			(GtefBuffer *buffer,
								 GtefFile   *file);

GtefFileSaver *		 gtef_file_saver_new_with_target	(GtefBuffer *buffer,
								 GtefFile   *file,
								 GFile      *target_location);

GtefBuffer *		 gtef_file_saver_get_buffer		(GtefFileSaver *saver);

GtefFile *		 gtef_file_saver_get_file		(GtefFileSaver *saver);

GFile *			 gtef_file_saver_get_location		(GtefFileSaver *saver);

void			 gtef_file_saver_set_encoding		(GtefFileSaver           *saver,
								 const GtkSourceEncoding *encoding);

const GtkSourceEncoding *gtef_file_saver_get_encoding		(GtefFileSaver *saver);

void			 gtef_file_saver_set_newline_type	(GtefFileSaver   *saver,
								 GtefNewlineType  newline_type);

GtefNewlineType		 gtef_file_saver_get_newline_type	(GtefFileSaver *saver);

void			 gtef_file_saver_set_compression_type	(GtefFileSaver            *saver,
								 GtkSourceCompressionType  compression_type);

GtkSourceCompressionType gtef_file_saver_get_compression_type	(GtefFileSaver *saver);

void			 gtef_file_saver_set_flags		(GtefFileSaver      *saver,
								 GtefFileSaverFlags  flags);

GtefFileSaverFlags	 gtef_file_saver_get_flags		(GtefFileSaver *saver);

void			 gtef_file_saver_save_async		(GtefFileSaver         *saver,
								 gint                   io_priority,
								 GCancellable          *cancellable,
								 GFileProgressCallback  progress_callback,
								 gpointer               progress_callback_data,
								 GDestroyNotify         progress_callback_notify,
								 GAsyncReadyCallback    callback,
								 gpointer               user_data);

gboolean		 gtef_file_saver_save_finish		(GtefFileSaver  *saver,
								 GAsyncResult   *result,
								 GError        **error);

G_END_DECLS

#endif  /* GTEF_FILE_SAVER_H  */
