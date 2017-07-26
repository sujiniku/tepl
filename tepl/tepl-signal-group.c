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

#include "tepl-signal-group.h"

/* Small utility to disconnect signal handlers without the need to keep around
 * the GObject instance where the signals were connected.
 *
 * It is similar to DzlSignalGroup from libdazzle. TeplSignalGroup is less
 * convenient to use, it contains less features, but this has the advantage that
 * the implementation is *much* simpler, and I can thus be more confident that
 * it is bug-free (which is more important to me). If one day DzlSignalGroup is
 * included in GObject or GIO (after being properly reviewed), then I'll
 * probably start using it. -- swilmet
 */

struct _TeplSignalGroup
{
	/* The GObject that the signal handlers are connected to.
	 * Weak ref.
	 */
	GObject *object;

	/* The IDs of the signal handlers. Element-type: gulong. */
	GArray *handler_ids;
};

TeplSignalGroup *
_tepl_signal_group_new (GObject *object)
{
	TeplSignalGroup *group;

	g_return_val_if_fail (G_IS_OBJECT (object), NULL);

	group = g_new0 (TeplSignalGroup, 1);

	group->object = object;
	g_object_add_weak_pointer (object, (gpointer *) &group->object);

	group->handler_ids = g_array_new (FALSE, TRUE, sizeof (gulong));

	return group;
}

static void
_tepl_signal_group_free (TeplSignalGroup *group)
{
	if (group == NULL)
	{
		return;
	}

	if (group->object != NULL)
	{
		guint i;

		/* Disconnect all signal handlers. */
		for (i = 0; i < group->handler_ids->len; i++)
		{
			gulong handler_id;

			handler_id = g_array_index (group->handler_ids, gulong, i);

			g_signal_handler_disconnect (group->object, handler_id);
		}

		g_object_remove_weak_pointer (group->object, (gpointer *) &group->object);
		group->object = NULL;
	}

	g_array_free (group->handler_ids, TRUE);
	g_free (group);
}

void
_tepl_signal_group_clear (TeplSignalGroup **group_pointer)
{
	g_return_if_fail (group_pointer != NULL);

	_tepl_signal_group_free (*group_pointer);
	*group_pointer = NULL;
}

void
_tepl_signal_group_add (TeplSignalGroup *group,
			gulong           signal_handler_id)
{
	g_return_if_fail (group != NULL);

	g_array_append_val (group->handler_ids, signal_handler_id);
}
