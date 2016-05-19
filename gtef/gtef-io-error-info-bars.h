/*
 * This file is part of Gtef, a text editor library.
 *
 * Copyright 2005 - Paolo Maggi
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

#ifndef GTEF_IO_ERROR_INFO_BARS_H
#define GTEF_IO_ERROR_INFO_BARS_H

#include <gtksourceview/gtksource.h>

G_BEGIN_DECLS

G_GNUC_INTERNAL
GtkInfoBar *	_gtef_io_loading_error_info_bar_new			(GtkSourceFileLoader *loader,
									 const GError        *error);

G_END_DECLS

#endif /* GTEF_IO_ERROR_INFO_BARS_H */
