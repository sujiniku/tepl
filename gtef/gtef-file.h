/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2014, 2015, 2016, 2017 - Sébastien Wilmet <swilmet@gnome.org>
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

#ifndef TEPL_FILE_H
#define TEPL_FILE_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gtksourceview/gtksource.h>
#include <tepl/tepl-types.h>

G_BEGIN_DECLS

#define TEPL_TYPE_FILE (tepl_file_get_type ())
G_DECLARE_DERIVABLE_TYPE (TeplFile, tepl_file,
			  TEPL, FILE,
			  GObject)

struct _TeplFileClass
{
	GObjectClass parent_class;

	gpointer padding[12];
};

/**
 * TeplMountOperationFactory:
 * @file: a #TeplFile.
 * @userdata: user data
 *
 * Type definition for a function that will be called to create a
 * #GMountOperation. This is useful for creating a #GtkMountOperation.
 *
 * Since: 1.0
 */
typedef GMountOperation *(*TeplMountOperationFactory) (TeplFile *file,
						       gpointer  userdata);

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

/**
 * TeplCompressionType:
 * @TEPL_COMPRESSION_TYPE_NONE: plain text.
 * @TEPL_COMPRESSION_TYPE_GZIP: gzip compression.
 *
 * Since: 1.0
 */
typedef enum
{
	TEPL_COMPRESSION_TYPE_NONE,
	TEPL_COMPRESSION_TYPE_GZIP
} TeplCompressionType;

TeplFile *		tepl_file_new				(void);

TeplFileMetadata *	tepl_file_get_file_metadata		(TeplFile *file);

GFile *			tepl_file_get_location			(TeplFile *file);

void			tepl_file_set_location			(TeplFile *file,
								 GFile    *location);

const gchar *		tepl_file_get_short_name		(TeplFile *file);

const TeplEncoding *	tepl_file_get_encoding			(TeplFile *file);

TeplNewlineType		tepl_file_get_newline_type		(TeplFile *file);

TeplCompressionType	tepl_file_get_compression_type		(TeplFile *file);

void		 	tepl_file_set_mount_operation_factory	(TeplFile                  *file,
								 TeplMountOperationFactory  callback,
								 gpointer                   user_data,
								 GDestroyNotify             notify);

void		 	tepl_file_check_file_on_disk		(TeplFile *file);

gboolean	 	tepl_file_is_local			(TeplFile *file);

gboolean	 	tepl_file_is_externally_modified	(TeplFile *file);

gboolean	 	tepl_file_is_deleted			(TeplFile *file);

gboolean	 	tepl_file_is_readonly			(TeplFile *file);

G_GNUC_INTERNAL
void			_tepl_file_set_encoding			(TeplFile           *file,
								 const TeplEncoding *encoding);

G_GNUC_INTERNAL
void			_tepl_file_set_newline_type		(TeplFile        *file,
								 TeplNewlineType  newline_type);

G_GNUC_INTERNAL
void			_tepl_file_set_compression_type		(TeplFile            *file,
								 TeplCompressionType  compression_type);

G_GNUC_INTERNAL
GMountOperation *	_tepl_file_create_mount_operation	(TeplFile *file);

G_GNUC_INTERNAL
void			_tepl_file_set_mounted			(TeplFile *file);

G_GNUC_INTERNAL
const gchar *		_tepl_file_get_etag			(TeplFile *file);

G_GNUC_INTERNAL
void			_tepl_file_set_etag			(TeplFile    *file,
								 const gchar *etag);

G_GNUC_INTERNAL
void			_tepl_file_set_externally_modified	(TeplFile *file,
								 gboolean  externally_modified);

G_GNUC_INTERNAL
void			_tepl_file_set_deleted			(TeplFile *file,
								 gboolean  deleted);

G_GNUC_INTERNAL
void			_tepl_file_set_readonly			(TeplFile *file,
								 gboolean  readonly);

G_END_DECLS

#endif /* TEPL_FILE_H */
