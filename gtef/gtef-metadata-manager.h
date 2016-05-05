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

#if !defined (GTEF_H_INSIDE) && !defined (GTEF_COMPILATION)
#error "Only <gtef/gtef.h> can be included directly."
#endif

#include <gio/gio.h>

G_BEGIN_DECLS

void		gtef_metadata_manager_init				(const gchar *metadata_path);

void		gtef_metadata_manager_shutdown				(void);

G_GNUC_INTERNAL
GFileInfo *	_gtef_metadata_manager_get_all_metadata_for_location	(GFile *location);

G_GNUC_INTERNAL
void		_gtef_metadata_manager_set_metadata_for_location	(GFile     *location,
									 GFileInfo *metadata);

G_END_DECLS

#endif /* GTEF_METADATA_MANAGER_H */
