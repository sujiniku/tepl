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

#ifndef TEPL_IO_ERROR_INFO_BARS_H
#define TEPL_IO_ERROR_INFO_BARS_H

#include <gio/gio.h>
#include <tepl/tepl-info-bar.h>

G_BEGIN_DECLS

TeplInfoBar *	tepl_io_error_info_bar_file_already_open		(GFile *location);

TeplInfoBar *	tepl_io_error_info_bar_cant_create_backup		(GFile        *location,
									 const GError *error);

TeplInfoBar *	tepl_io_error_info_bar_externally_modified		(GFile    *location,
									 gboolean  document_modified);

TeplInfoBar *	tepl_io_error_info_bar_invalid_characters		(GFile *location);

G_END_DECLS

#endif /* TEPL_IO_ERROR_INFO_BARS_H */
