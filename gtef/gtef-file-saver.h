/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2005, 2007 - Paolo Maggi
 * Copyrhing 2007 - Steve Frécinaux
 * Copyright 2008 - Jesse van den Kieboom
 * Copyright 2014, 2016, 2017 - Sébastien Wilmet
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

#ifndef TEPL_FILE_SAVER_H
#define TEPL_FILE_SAVER_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gtksourceview/gtksource.h>
#include <tepl/tepl-types.h>
#include <tepl/tepl-file.h>

G_BEGIN_DECLS

#define TEPL_TYPE_FILE_SAVER              (tepl_file_saver_get_type())
#define TEPL_FILE_SAVER(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), TEPL_TYPE_FILE_SAVER, TeplFileSaver))
#define TEPL_FILE_SAVER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), TEPL_TYPE_FILE_SAVER, TeplFileSaverClass))
#define TEPL_IS_FILE_SAVER(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), TEPL_TYPE_FILE_SAVER))
#define TEPL_IS_FILE_SAVER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), TEPL_TYPE_FILE_SAVER))
#define TEPL_FILE_SAVER_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), TEPL_TYPE_FILE_SAVER, TeplFileSaverClass))

typedef struct _TeplFileSaverClass   TeplFileSaverClass;
typedef struct _TeplFileSaverPrivate TeplFileSaverPrivate;

#define TEPL_FILE_SAVER_ERROR tepl_file_saver_error_quark ()

/**
 * TeplFileSaverError:
 * @TEPL_FILE_SAVER_ERROR_INVALID_CHARS: The buffer contains invalid
 *   characters.
 * @TEPL_FILE_SAVER_ERROR_EXTERNALLY_MODIFIED: The file is externally
 *   modified.
 *
 * An error code used with the %TEPL_FILE_SAVER_ERROR domain.
 * Since: 1.0
 */
typedef enum
{
	TEPL_FILE_SAVER_ERROR_INVALID_CHARS,
	TEPL_FILE_SAVER_ERROR_EXTERNALLY_MODIFIED
} TeplFileSaverError;

/**
 * TeplFileSaverFlags:
 * @TEPL_FILE_SAVER_FLAGS_NONE: No flags.
 * @TEPL_FILE_SAVER_FLAGS_IGNORE_INVALID_CHARS: Ignore invalid characters.
 * @TEPL_FILE_SAVER_FLAGS_IGNORE_MODIFICATION_TIME: Save file despite external modifications.
 * @TEPL_FILE_SAVER_FLAGS_CREATE_BACKUP: Create a backup before saving the file.
 *
 * Flags to define the behavior of a #TeplFileSaver.
 * Since: 1.0
 */
typedef enum
{
	TEPL_FILE_SAVER_FLAGS_NONE			= 0,
	TEPL_FILE_SAVER_FLAGS_IGNORE_INVALID_CHARS	= 1 << 0,
	TEPL_FILE_SAVER_FLAGS_IGNORE_MODIFICATION_TIME	= 1 << 1,
	TEPL_FILE_SAVER_FLAGS_CREATE_BACKUP		= 1 << 2
} TeplFileSaverFlags;

struct _TeplFileSaver
{
	GObject object;

	TeplFileSaverPrivate *priv;
};

struct _TeplFileSaverClass
{
	GObjectClass parent_class;

	gpointer padding[10];
};

GType			 tepl_file_saver_get_type		(void);

GQuark			 tepl_file_saver_error_quark		(void);

TeplFileSaver *		 tepl_file_saver_new			(TeplBuffer *buffer,
								 TeplFile   *file);

TeplFileSaver *		 tepl_file_saver_new_with_target	(TeplBuffer *buffer,
								 TeplFile   *file,
								 GFile      *target_location);

TeplBuffer *		 tepl_file_saver_get_buffer		(TeplFileSaver *saver);

TeplFile *		 tepl_file_saver_get_file		(TeplFileSaver *saver);

GFile *			 tepl_file_saver_get_location		(TeplFileSaver *saver);

void			 tepl_file_saver_set_encoding		(TeplFileSaver      *saver,
								 const TeplEncoding *encoding);

const TeplEncoding *	 tepl_file_saver_get_encoding		(TeplFileSaver *saver);

void			 tepl_file_saver_set_newline_type	(TeplFileSaver   *saver,
								 TeplNewlineType  newline_type);

TeplNewlineType		 tepl_file_saver_get_newline_type	(TeplFileSaver *saver);

void			 tepl_file_saver_set_compression_type	(TeplFileSaver       *saver,
								 TeplCompressionType  compression_type);

TeplCompressionType	 tepl_file_saver_get_compression_type	(TeplFileSaver *saver);

void			 tepl_file_saver_set_flags		(TeplFileSaver      *saver,
								 TeplFileSaverFlags  flags);

TeplFileSaverFlags	 tepl_file_saver_get_flags		(TeplFileSaver *saver);

void			 tepl_file_saver_save_async		(TeplFileSaver         *saver,
								 gint                   io_priority,
								 GCancellable          *cancellable,
								 GFileProgressCallback  progress_callback,
								 gpointer               progress_callback_data,
								 GDestroyNotify         progress_callback_notify,
								 GAsyncReadyCallback    callback,
								 gpointer               user_data);

gboolean		 tepl_file_saver_save_finish		(TeplFileSaver  *saver,
								 GAsyncResult   *result,
								 GError        **error);

G_END_DECLS

#endif  /* TEPL_FILE_SAVER_H  */
