/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2020 - Sébastien Wilmet <swilmet@gnome.org>
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

#ifndef TEPL_METADATA_STORE_LOADER_H
#define TEPL_METADATA_STORE_LOADER_H

#include <gio/gio.h>

G_BEGIN_DECLS

G_GNUC_INTERNAL
gboolean	_tepl_metadata_store_loader	(GFile       *from_file,
						 GHashTable  *hash_table,
						 GError     **error);

G_END_DECLS

#endif /* TEPL_METADATA_STORE_LOADER_H */
