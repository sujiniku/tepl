/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2003 - Paolo Maggi
 * Copyright 2016 - Sébastien Wilmet <swilmet@gnome.org>
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

#ifndef TEPL_METADATA_MANAGER_H
#define TEPL_METADATA_MANAGER_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gio/gio.h>

G_BEGIN_DECLS

void		tepl_metadata_manager_init				(const gchar *metadata_path);

void		tepl_metadata_manager_shutdown				(void);

G_GNUC_INTERNAL
GFileInfo *	_tepl_metadata_manager_get_all_metadata_for_location	(GFile *location);

G_GNUC_INTERNAL
void		_tepl_metadata_manager_set_metadata_for_location	(GFile     *location,
									 GFileInfo *metadata);

G_GNUC_INTERNAL
void		_tepl_metadata_manager_set_unit_test_mode		(void);

G_END_DECLS

#endif /* TEPL_METADATA_MANAGER_H */
