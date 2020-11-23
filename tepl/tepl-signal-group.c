/* SPDX-FileCopyrightText: 2017-2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "tepl-signal-group.h"

/* Small utility to disconnect signal handlers without the need to keep around
 * the GObject instance where the signals were connected.
 *
 * It was inspired by DzlSignalGroup from libdazzle. TeplSignalGroup has a much
 * simpler implementation, it applies the "worse is better" philosophy.
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
tepl_signal_group_new (GObject *object)
{
	TeplSignalGroup *group;

	g_return_val_if_fail (G_IS_OBJECT (object), NULL);

	group = g_new0 (TeplSignalGroup, 1);

	g_set_weak_pointer (&group->object, object);

	group->handler_ids = g_array_new (FALSE, TRUE, sizeof (gulong));

	return group;
}

static void
signal_group_free (TeplSignalGroup *group)
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

		g_clear_weak_pointer (&group->object);
	}

	g_array_free (group->handler_ids, TRUE);
	g_free (group);
}

void
tepl_signal_group_clear (TeplSignalGroup **group_pointer)
{
	g_return_if_fail (group_pointer != NULL);

	signal_group_free (*group_pointer);
	*group_pointer = NULL;
}

void
tepl_signal_group_add (TeplSignalGroup *group,
		       gulong           signal_handler_id)
{
	g_return_if_fail (group != NULL);
	g_return_if_fail (signal_handler_id != 0);

	g_array_append_val (group->handler_ids, signal_handler_id);
}
