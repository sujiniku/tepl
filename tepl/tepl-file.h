/* SPDX-FileCopyrightText: 2014-2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TEPL_FILE_H
#define TEPL_FILE_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <tepl/tepl-macros.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define TEPL_TYPE_FILE             (tepl_file_get_type ())
#define TEPL_FILE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEPL_TYPE_FILE, TeplFile))
#define TEPL_FILE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TEPL_TYPE_FILE, TeplFileClass))
#define TEPL_IS_FILE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEPL_TYPE_FILE))
#define TEPL_IS_FILE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TEPL_TYPE_FILE))
#define TEPL_FILE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TEPL_TYPE_FILE, TeplFileClass))

typedef struct _TeplFile         TeplFile;
typedef struct _TeplFileClass    TeplFileClass;
typedef struct _TeplFilePrivate  TeplFilePrivate;

struct _TeplFile
{
	GObject parent;

	TeplFilePrivate *priv;
};

struct _TeplFileClass
{
	GObjectClass parent_class;

	gpointer padding[12];
};

/**
 * TeplMountOperationFactory:
 * @file: a #TeplFile.
 * @user_data: user data.
 *
 * Type definition for a function that will be called to create a
 * #GMountOperation. This is useful for creating a #GtkMountOperation.
 *
 * Since: 1.0
 */
typedef GMountOperation *(*TeplMountOperationFactory) (TeplFile *file,
						       gpointer  user_data);

/**
 * TeplNewlineType:
 * @TEPL_NEWLINE_TYPE_LF: line feed, used on UNIX.
 * @TEPL_NEWLINE_TYPE_CR: carriage return, used on Mac.
 * @TEPL_NEWLINE_TYPE_CR_LF: carriage return followed by a line feed, used
 *   on Windows.
 *
 * Since: 1.0
 */
typedef enum
{
	TEPL_NEWLINE_TYPE_LF,
	TEPL_NEWLINE_TYPE_CR,
	TEPL_NEWLINE_TYPE_CR_LF
} TeplNewlineType;

/**
 * TEPL_NEWLINE_TYPE_DEFAULT:
 *
 * The default newline type on the current OS.
 *
 * Since: 1.0
 */
#ifdef G_OS_WIN32
#define TEPL_NEWLINE_TYPE_DEFAULT TEPL_NEWLINE_TYPE_CR_LF
#else
#define TEPL_NEWLINE_TYPE_DEFAULT TEPL_NEWLINE_TYPE_LF
#endif

_TEPL_EXTERN
GType			tepl_file_get_type			(void);

_TEPL_EXTERN
TeplFile *		tepl_file_new				(void);

_TEPL_EXTERN
GFile *			tepl_file_get_location			(TeplFile *file);

_TEPL_EXTERN
void			tepl_file_set_location			(TeplFile *file,
								 GFile    *location);

_TEPL_EXTERN
gchar *			tepl_file_get_short_name		(TeplFile *file);

_TEPL_EXTERN
TeplNewlineType		tepl_file_get_newline_type		(TeplFile *file);

_TEPL_EXTERN
void		 	tepl_file_set_mount_operation_factory	(TeplFile                  *file,
								 TeplMountOperationFactory  callback,
								 gpointer                   user_data,
								 GDestroyNotify             notify);

_TEPL_EXTERN
void			tepl_file_add_uri_to_recent_manager	(TeplFile *file);

G_GNUC_INTERNAL
void			_tepl_file_set_newline_type		(TeplFile        *file,
								 TeplNewlineType  newline_type);

G_GNUC_INTERNAL
GMountOperation *	_tepl_file_create_mount_operation	(TeplFile *file);

G_GNUC_INTERNAL
void			_tepl_file_set_mounted			(TeplFile *file);

G_GNUC_INTERNAL
const gchar *		_tepl_file_get_etag			(TeplFile *file);

G_GNUC_INTERNAL
void			_tepl_file_set_etag			(TeplFile    *file,
								 const gchar *etag);

G_END_DECLS

#endif /* TEPL_FILE_H */
