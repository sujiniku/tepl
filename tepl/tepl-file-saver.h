/* Copyright 2005, 2007 - Paolo Maggi
 * Copyrhing 2007 - Steve Frécinaux
 * Copyright 2008 - Jesse van den Kieboom
 * Copyright 2014, 2016, 2017 - Sébastien Wilmet
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TEPL_FILE_SAVER_H
#define TEPL_FILE_SAVER_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gtksourceview/gtksource.h>
#include <tepl/tepl-buffer.h>
#include <tepl/tepl-encoding.h>
#include <tepl/tepl-file.h>

G_BEGIN_DECLS

#define TEPL_TYPE_FILE_SAVER              (tepl_file_saver_get_type())
#define TEPL_FILE_SAVER(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), TEPL_TYPE_FILE_SAVER, TeplFileSaver))
#define TEPL_FILE_SAVER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), TEPL_TYPE_FILE_SAVER, TeplFileSaverClass))
#define TEPL_IS_FILE_SAVER(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), TEPL_TYPE_FILE_SAVER))
#define TEPL_IS_FILE_SAVER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), TEPL_TYPE_FILE_SAVER))
#define TEPL_FILE_SAVER_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), TEPL_TYPE_FILE_SAVER, TeplFileSaverClass))

typedef struct _TeplFileSaver        TeplFileSaver;
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

	gpointer padding[12];
};

_TEPL_EXTERN
GType			 tepl_file_saver_get_type		(void);

_TEPL_EXTERN
GQuark			 tepl_file_saver_error_quark		(void);

_TEPL_EXTERN
TeplFileSaver *		 tepl_file_saver_new			(TeplBuffer *buffer,
								 TeplFile   *file);

_TEPL_EXTERN
TeplFileSaver *		 tepl_file_saver_new_with_target	(TeplBuffer *buffer,
								 TeplFile   *file,
								 GFile      *target_location);

_TEPL_EXTERN
TeplBuffer *		 tepl_file_saver_get_buffer		(TeplFileSaver *saver);

_TEPL_EXTERN
TeplFile *		 tepl_file_saver_get_file		(TeplFileSaver *saver);

_TEPL_EXTERN
GFile *			 tepl_file_saver_get_location		(TeplFileSaver *saver);

_TEPL_EXTERN
void			 tepl_file_saver_set_encoding		(TeplFileSaver      *saver,
								 const TeplEncoding *encoding);

_TEPL_EXTERN
const TeplEncoding *	 tepl_file_saver_get_encoding		(TeplFileSaver *saver);

_TEPL_EXTERN
void			 tepl_file_saver_set_newline_type	(TeplFileSaver   *saver,
								 TeplNewlineType  newline_type);

_TEPL_EXTERN
TeplNewlineType		 tepl_file_saver_get_newline_type	(TeplFileSaver *saver);

_TEPL_EXTERN
void			 tepl_file_saver_set_compression_type	(TeplFileSaver       *saver,
								 TeplCompressionType  compression_type);

_TEPL_EXTERN
TeplCompressionType	 tepl_file_saver_get_compression_type	(TeplFileSaver *saver);

_TEPL_EXTERN
void			 tepl_file_saver_set_flags		(TeplFileSaver      *saver,
								 TeplFileSaverFlags  flags);

_TEPL_EXTERN
TeplFileSaverFlags	 tepl_file_saver_get_flags		(TeplFileSaver *saver);

_TEPL_EXTERN
void			 tepl_file_saver_save_async		(TeplFileSaver         *saver,
								 gint                   io_priority,
								 GCancellable          *cancellable,
								 GFileProgressCallback  progress_callback,
								 gpointer               progress_callback_data,
								 GDestroyNotify         progress_callback_notify,
								 GAsyncReadyCallback    callback,
								 gpointer               user_data);

_TEPL_EXTERN
gboolean		 tepl_file_saver_save_finish		(TeplFileSaver  *saver,
								 GAsyncResult   *result,
								 GError        **error);

G_END_DECLS

#endif  /* TEPL_FILE_SAVER_H  */
