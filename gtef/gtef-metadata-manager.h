/*
 * This file is part of Gtef, a text editor library.
 *
 * Copyright 2003 - Paolo Maggi
 * Copyright 2016 - Sébastien Wilmet <swilmet@gnome.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GTEF_METADATA_MANAGER_H
#define GTEF_METADATA_MANAGER_H

#include <gio/gio.h>

G_BEGIN_DECLS

void		gtef_metadata_manager_init		(const gchar *metadata_filename);

void		gtef_metadata_manager_shutdown		(void);

gchar *		gtef_metadata_manager_get 		(GFile       *location,
							 const gchar *key);

void		gtef_metadata_manager_set		(GFile       *location,
							 const gchar *key,
							 const gchar *value);

G_END_DECLS

#endif /* GTEF_METADATA_MANAGER_H */
