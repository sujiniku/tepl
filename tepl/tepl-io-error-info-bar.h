/* Copyright 2016 - Sébastien Wilmet <swilmet@gnome.org>
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

#ifndef TEPL_IO_ERROR_INFO_BAR_H
#define TEPL_IO_ERROR_INFO_BAR_H

#include <gtksourceview/gtksource.h>
#include "tepl-info-bar.h"

G_BEGIN_DECLS

#define TEPL_TYPE_IO_ERROR_INFO_BAR (_tepl_io_error_info_bar_get_type ())
G_DECLARE_DERIVABLE_TYPE (TeplIoErrorInfoBar, _tepl_io_error_info_bar,
			  TEPL, IO_ERROR_INFO_BAR,
			  TeplInfoBar)

struct _TeplIoErrorInfoBarClass
{
	TeplInfoBarClass parent_class;
};

G_GNUC_INTERNAL
TeplIoErrorInfoBar *	_tepl_io_error_info_bar_new				(void);

G_GNUC_INTERNAL
void			_tepl_io_error_info_bar_set_loading_error		(TeplIoErrorInfoBar  *info_bar,
										 GtkSourceFileLoader *loader,
										 const GError        *error);

G_END_DECLS

#endif /* TEPL_IO_ERROR_INFO_BAR_H */
