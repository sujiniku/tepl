/* Copyright 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TEPL_METADATA_PARSER_H
#define TEPL_METADATA_PARSER_H

#include <gio/gio.h>

G_BEGIN_DECLS

G_GNUC_INTERNAL
gboolean	_tepl_metadata_parser_read_file		(GFile       *from_file,
							 GHashTable  *hash_table,
							 GError     **error);

G_END_DECLS

#endif /* TEPL_METADATA_PARSER_H */
