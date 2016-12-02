/*
 * This file is part of Gtef, a text editor library.
 *
 * Copyright 2014, 2015, 2016 - Sébastien Wilmet <swilmet@gnome.org>
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

#ifndef GTEF_FILE_H
#define GTEF_FILE_H

#if !defined (GTEF_H_INSIDE) && !defined (GTEF_COMPILATION)
#error "Only <gtef/gtef.h> can be included directly."
#endif

#include <gtksourceview/gtksource.h>
#include <gtef/gtef-types.h>

G_BEGIN_DECLS

#define GTEF_TYPE_FILE (gtef_file_get_type ())
G_DECLARE_DERIVABLE_TYPE (GtefFile, gtef_file,
			  GTEF, FILE,
			  GObject)

struct _GtefFileClass
{
	GObjectClass parent_class;

	gpointer padding[12];
};

/**
 * GtefMountOperationFactory:
 * @file: a #GtefFile.
 * @userdata: user data
 *
 * Type definition for a function that will be called to create a
 * #GMountOperation. This is useful for creating a #GtkMountOperation.
 *
 * Since: 1.0
 */
typedef GMountOperation *(*GtefMountOperationFactory) (GtefFile *file,
						       gpointer  userdata);

/**
 * GtefNewlineType:
 * @GTEF_NEWLINE_TYPE_LF: line feed, used on UNIX.
 * @GTEF_NEWLINE_TYPE_CR: carriage return, used on Mac.
 * @GTEF_NEWLINE_TYPE_CR_LF: carriage return followed by a line feed, used
 *   on Windows.
 *
 * Since: 1.0
 */
typedef enum
{
	GTEF_NEWLINE_TYPE_LF,
	GTEF_NEWLINE_TYPE_CR,
	GTEF_NEWLINE_TYPE_CR_LF
} GtefNewlineType;

/**
 * GTEF_NEWLINE_TYPE_DEFAULT:
 *
 * The default newline type on the current OS.
 *
 * Since: 1.0
 */
#ifdef G_OS_WIN32
#define GTEF_NEWLINE_TYPE_DEFAULT GTEF_NEWLINE_TYPE_CR_LF
#else
#define GTEF_NEWLINE_TYPE_DEFAULT GTEF_NEWLINE_TYPE_LF
#endif

/**
 * GtefCompressionType:
 * @GTEF_COMPRESSION_TYPE_NONE: plain text.
 * @GTEF_COMPRESSION_TYPE_GZIP: gzip compression.
 *
 * Since: 1.0
 */
typedef enum
{
	GTEF_COMPRESSION_TYPE_NONE,
	GTEF_COMPRESSION_TYPE_GZIP
} GtefCompressionType;

GtefFile *		gtef_file_new				(void);

GtefFileMetadata *	gtef_file_get_file_metadata		(GtefFile *file);

GFile *			gtef_file_get_location			(GtefFile *file);

void			gtef_file_set_location			(GtefFile *file,
								 GFile    *location);

const gchar *		gtef_file_get_short_name		(GtefFile *file);

const GtkSourceEncoding * gtef_file_get_encoding		(GtefFile *file);

GtefNewlineType		gtef_file_get_newline_type		(GtefFile *file);

GtefCompressionType	gtef_file_get_compression_type		(GtefFile *file);

void		 	gtef_file_set_mount_operation_factory	(GtefFile                  *file,
								 GtefMountOperationFactory  callback,
								 gpointer                   user_data,
								 GDestroyNotify             notify);

void		 	gtef_file_check_file_on_disk		(GtefFile *file);

gboolean	 	gtef_file_is_local			(GtefFile *file);

gboolean	 	gtef_file_is_externally_modified	(GtefFile *file);

gboolean	 	gtef_file_is_deleted			(GtefFile *file);

gboolean	 	gtef_file_is_readonly			(GtefFile *file);

G_GNUC_INTERNAL
void			_gtef_file_set_encoding			(GtefFile                *file,
								 const GtkSourceEncoding *encoding);

G_GNUC_INTERNAL
void			_gtef_file_set_newline_type		(GtefFile        *file,
								 GtefNewlineType  newline_type);

G_GNUC_INTERNAL
void			_gtef_file_set_compression_type		(GtefFile            *file,
								 GtefCompressionType  compression_type);

G_GNUC_INTERNAL
GMountOperation *	_gtef_file_create_mount_operation	(GtefFile *file);

G_GNUC_INTERNAL
void			_gtef_file_set_mounted			(GtefFile *file);

G_GNUC_INTERNAL
const gchar *		_gtef_file_get_etag			(GtefFile *file);

G_GNUC_INTERNAL
void			_gtef_file_set_etag			(GtefFile    *file,
								 const gchar *etag);

G_GNUC_INTERNAL
void			_gtef_file_set_externally_modified	(GtefFile *file,
								 gboolean  externally_modified);

G_GNUC_INTERNAL
void			_gtef_file_set_deleted			(GtefFile *file,
								 gboolean  deleted);

G_GNUC_INTERNAL
void			_gtef_file_set_readonly			(GtefFile *file,
								 gboolean  readonly);

G_END_DECLS

#endif /* GTEF_FILE_H */
