/*
 * This file is part of Amtk - Actions, Menus and Toolbars Kit
 *
 * Copyright 2017 - Sébastien Wilmet <swilmet@gnome.org>
 *
 * Amtk is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * Amtk is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include "amtk-utils.h"

/* Deep copy of @strv. */
gchar **
_amtk_utils_strv_copy (const gchar * const *strv)
{
	guint length;
	gchar **new_strv;
	guint i;

	if (strv == NULL)
	{
		return NULL;
	}

	length = g_strv_length ((gchar **)strv);

	new_strv = g_malloc ((length + 1) * sizeof (gchar *));

	for (i = 0; i < length; i++)
	{
		new_strv[i] = g_strdup (strv[i]);
	}

	new_strv[length] = NULL;

	return new_strv;
}
