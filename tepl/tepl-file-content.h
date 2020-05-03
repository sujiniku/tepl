/* Copyright 2017 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TEPL_FILE_CONTENT_H
#define TEPL_FILE_CONTENT_H

#include <glib-object.h>
#include "tepl-encoding.h"
#include "tepl-encoding-converter.h"

G_BEGIN_DECLS

#define TEPL_TYPE_FILE_CONTENT             (_tepl_file_content_get_type ())
#define TEPL_FILE_CONTENT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEPL_TYPE_FILE_CONTENT, TeplFileContent))
#define TEPL_FILE_CONTENT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TEPL_TYPE_FILE_CONTENT, TeplFileContentClass))
#define TEPL_IS_FILE_CONTENT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEPL_TYPE_FILE_CONTENT))
#define TEPL_IS_FILE_CONTENT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TEPL_TYPE_FILE_CONTENT))
#define TEPL_FILE_CONTENT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TEPL_TYPE_FILE_CONTENT, TeplFileContentClass))

typedef struct _TeplFileContent         TeplFileContent;
typedef struct _TeplFileContentClass    TeplFileContentClass;
typedef struct _TeplFileContentPrivate  TeplFileContentPrivate;

struct _TeplFileContent
{
	GObject parent;

	TeplFileContentPrivate *priv;
};

struct _TeplFileContentClass
{
	GObjectClass parent_class;
};

G_GNUC_INTERNAL
GType			_tepl_file_content_get_type		(void);

G_GNUC_INTERNAL
TeplFileContent *	_tepl_file_content_new			(void);

G_GNUC_INTERNAL
void			_tepl_file_content_add_chunk		(TeplFileContent *content,
								 GBytes          *chunk);

G_GNUC_INTERNAL
TeplEncoding *		_tepl_file_content_determine_encoding	(TeplFileContent *content);

G_GNUC_INTERNAL
gboolean		_tepl_file_content_convert_to_utf8	(TeplFileContent                 *content,
								 TeplEncoding                    *from_encoding,
								 TeplEncodingConversionCallback   callback,
								 gpointer                         callback_user_data,
								 GError                         **error);

/* For unit tests */

G_GNUC_INTERNAL
TeplEncoding *		_tepl_file_content_determine_encoding_with_fallback_mode	(TeplFileContent *content,
											 GSList          *candidate_encodings);

G_GNUC_INTERNAL
gint64			_tepl_file_content_get_encoding_converter_buffer_size		(void);

G_END_DECLS

#endif /* TEPL_FILE_CONTENT_H */
