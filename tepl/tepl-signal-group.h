/* Copyright 2017 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TEPL_SIGNAL_GROUP_H
#define TEPL_SIGNAL_GROUP_H

#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _TeplSignalGroup TeplSignalGroup;

G_GNUC_INTERNAL
TeplSignalGroup *	_tepl_signal_group_new		(GObject *object);

G_GNUC_INTERNAL
void			_tepl_signal_group_clear	(TeplSignalGroup **group_pointer);

G_GNUC_INTERNAL
void			_tepl_signal_group_add		(TeplSignalGroup *group,
							 gulong           signal_handler_id);

G_END_DECLS

#endif /* TEPL_SIGNAL_GROUP_H */
