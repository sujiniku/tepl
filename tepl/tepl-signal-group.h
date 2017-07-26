/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2017 - Sébastien Wilmet <swilmet@gnome.org>
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

#ifndef TEPL_SIGNAL_GROUP_H
#define TEPL_SIGNAL_GROUP_H

#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _TeplSignalGroup TeplSignalGroup;

G_GNUC_INTERNAL
TeplSignalGroup *	_tepl_signal_group_new		(GObject *object);

G_GNUC_INTERNAL
void			_tepl_signal_group_free		(TeplSignalGroup *group);

G_GNUC_INTERNAL
void			_tepl_signal_group_add		(TeplSignalGroup *group,
							 gulong           signal_handler_id);

G_END_DECLS

#endif /* TEPL_SIGNAL_GROUP_H */
