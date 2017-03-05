/*
 * This file is part of Gtef, a text editor library.
 *
 * Copyright 2017 - Sébastien Wilmet <swilmet@gnome.org>
 *
 * Gtef is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * Gtef is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "gtef-action-info.h"
#include <glib/gi18n-lib.h>
#include "gtef-utils.h"

/**
 * SECTION:action-info
 * @Short_description: GAction information
 * @Title: GtefActionInfo
 * @See_also: #GtefActionInfoStore
 *
 * A #GtefActionInfo instance contains a set of information about a #GAction.
 * Those pieces of information are useful to create UI elements that trigger the
 * #GAction, for example a menu item or a toolbar item.
 *
 * When writing an XML file to create a #GMenu, with the format understood by
 * #GtkBuilder (see the class description of #GtkApplicationWindow), the
 * information in the XML file can be used only to create a #GMenu. The initial
 * goal with #GtefActionInfo and its related classes is to encode the
 * information just once, and be able to create both a menu and a toolbar easily
 * (to have a traditional user interface).
 */

struct _GtefActionInfo
{
	gchar *action_name;
	gchar *icon_name;
	gchar *label;
	gchar *tooltip;

	/* Must never be NULL, must be a NULL-terminated array. This way, it
	 * can be used directly as an argument to
	 * gtk_application_set_accels_for_action().
	 */
	gchar **accels;

	gint ref_count;

	guint used : 1;
};

static void _gtef_action_info_free (GtefActionInfo *info);

G_DEFINE_BOXED_TYPE (GtefActionInfo, gtef_action_info,
		     gtef_action_info_copy,
		     _gtef_action_info_free)

static void
_gtef_action_info_free (GtefActionInfo *info)
{
	if (info != NULL)
	{
		g_free (info->action_name);
		g_free (info->icon_name);
		g_free (info->label);
		g_free (info->tooltip);
		g_strfreev (info->accels);

		g_free (info);
	}
}

/**
 * gtef_action_info_new:
 *
 * Returns: a new #GtefActionInfo.
 * Since: 2.0
 */
GtefActionInfo *
gtef_action_info_new (void)
{
	GtefActionInfo *info;

	info = g_new0 (GtefActionInfo, 1);
	info->accels = g_malloc0 (sizeof (gchar *));
	info->ref_count = 1;

	return info;
}

/**
 * gtef_action_info_new_from_entry:
 * @info_entry: a #GtefActionInfoEntry.
 * @translation_domain: (nullable): a gettext domain, or %NULL.
 *
 * Creates a new #GtefActionInfo from a #GtefActionInfoEntry.
 *
 * If @translation_domain is not %NULL, g_dgettext() is used to translate the
 * @label and @tooltip before setting them to the #GtefActionInfo.
 *
 * Returns: a new #GtefActionInfo.
 * Since: 2.0
 */
GtefActionInfo *
gtef_action_info_new_from_entry (const GtefActionInfoEntry *info_entry,
				 const gchar               *translation_domain)
{
	GtefActionInfo *info;

	info = gtef_action_info_new ();
	info->action_name = g_strdup (info_entry->action_name);
	info->icon_name = g_strdup (info_entry->icon_name);

	if (translation_domain != NULL)
	{
		info->label = g_strdup (g_dgettext (translation_domain, info_entry->label));
		info->tooltip = g_strdup (g_dgettext (translation_domain, info_entry->tooltip));
	}
	else
	{
		info->label = g_strdup (info_entry->label);
		info->tooltip = g_strdup (info_entry->tooltip);
	}

	if (info_entry->accel != NULL)
	{
		g_strfreev (info->accels);

		info->accels = g_malloc (2 * sizeof (gchar *));
		info->accels[0] = g_strdup (info_entry->accel);
		info->accels[1] = NULL;
	}

	return info;
}

/**
 * gtef_action_info_ref:
 * @info: a #GtefActionInfo.
 *
 * Increments the reference count of @info by one.
 *
 * Returns: the passed in @info.
 * Since: 2.0
 */
GtefActionInfo *
gtef_action_info_ref (GtefActionInfo *info)
{
	g_return_val_if_fail (info != NULL, NULL);

	info->ref_count++;

	return info;
}

/**
 * gtef_action_info_unref:
 * @info: a #GtefActionInfo.
 *
 * Decrements the reference count of @info by one. If the reference count drops
 * to 0, @info is freed.
 *
 * Since: 2.0
 */
void
gtef_action_info_unref (GtefActionInfo *info)
{
	g_return_if_fail (info != NULL);

	info->ref_count--;

	if (info->ref_count == 0)
	{
		_gtef_action_info_free (info);
	}
}

/**
 * gtef_action_info_copy:
 * @info: a #GtefActionInfo.
 *
 * Returns: (transfer full): a copy of @info. The copy will have a reference
 * count of one.
 * Since: 2.0
 */
GtefActionInfo *
gtef_action_info_copy (const GtefActionInfo *info)
{
	GtefActionInfo *new_info;

	g_return_val_if_fail (info != NULL, NULL);

	new_info = gtef_action_info_new ();

	new_info->action_name = g_strdup (info->action_name);
	new_info->icon_name = g_strdup (info->icon_name);
	new_info->label = g_strdup (info->label);
	new_info->tooltip = g_strdup (info->tooltip);

	gtef_action_info_set_accels (new_info, (const gchar * const *)info->accels);

	return new_info;
}

/**
 * gtef_action_info_get_action_name:
 * @info: a #GtefActionInfo.
 *
 * Returns: (nullable): the action name, or %NULL. Example: `"win.save"`.
 * Since: 2.0
 */
const gchar *
gtef_action_info_get_action_name (const GtefActionInfo *info)
{
	g_return_val_if_fail (info != NULL, NULL);

	return info->action_name;
}

/**
 * gtef_action_info_set_action_name:
 * @info: a #GtefActionInfo.
 * @action_name: the action name.
 *
 * Sets the action name, for example `"win.save"`.
 *
 * Since: 2.0
 */
void
gtef_action_info_set_action_name (GtefActionInfo *info,
				  const gchar    *action_name)
{
	g_return_if_fail (info != NULL);
	g_return_if_fail (action_name != NULL);

	g_free (info->action_name);
	info->action_name = g_strdup (action_name);
}

/**
 * gtef_action_info_get_icon_name:
 * @info: a #GtefActionInfo.
 *
 * Returns: (nullable): the icon name, or %NULL.
 * Since: 2.0
 */
const gchar *
gtef_action_info_get_icon_name (const GtefActionInfo *info)
{
	g_return_val_if_fail (info != NULL, NULL);

	return info->icon_name;
}

/**
 * gtef_action_info_set_icon_name:
 * @info: a #GtefActionInfo.
 * @icon_name: (nullable): the icon name, or %NULL.
 *
 * Since: 2.0
 */
void
gtef_action_info_set_icon_name (GtefActionInfo *info,
				const gchar    *icon_name)
{
	g_return_if_fail (info != NULL);

	g_free (info->icon_name);
	info->icon_name = g_strdup (icon_name);
}

/**
 * gtef_action_info_get_label:
 * @info: a #GtefActionInfo.
 *
 * Returns: (nullable): the label (i.e. a short description), or %NULL.
 * Since: 2.0
 */
const gchar *
gtef_action_info_get_label (const GtefActionInfo *info)
{
	g_return_val_if_fail (info != NULL, NULL);

	return info->label;
}

/**
 * gtef_action_info_set_label:
 * @info: a #GtefActionInfo.
 * @label: (nullable): the label (i.e. a short description), or %NULL.
 *
 * Since: 2.0
 */
void
gtef_action_info_set_label (GtefActionInfo *info,
			    const gchar    *label)
{
	g_return_if_fail (info != NULL);

	g_free (info->label);
	info->label = g_strdup (label);
}

/**
 * gtef_action_info_get_tooltip:
 * @info: a #GtefActionInfo.
 *
 * Returns: (nullable): the tooltip (i.e. a long description), or %NULL.
 * Since: 2.0
 */
const gchar *
gtef_action_info_get_tooltip (const GtefActionInfo *info)
{
	g_return_val_if_fail (info != NULL, NULL);

	return info->tooltip;
}

/**
 * gtef_action_info_set_tooltip:
 * @info: a #GtefActionInfo.
 * @tooltip: (nullable): the tooltip (i.e. a long description), or %NULL.
 *
 * Since: 2.0
 */
void
gtef_action_info_set_tooltip (GtefActionInfo *info,
			      const gchar    *tooltip)
{
	g_return_if_fail (info != NULL);

	g_free (info->tooltip);
	info->tooltip = g_strdup (tooltip);
}

/**
 * gtef_action_info_get_accels:
 * @info: a #GtefActionInfo.
 *
 * Returns the accelerators. This function never returns %NULL, it always
 * returns a %NULL-terminated array, to be suitable for
 * gtk_application_set_accels_for_action().
 *
 * Returns: (transfer none) (array zero-terminated=1): a %NULL-terminated array
 * of accelerators in the format understood by gtk_accelerator_parse().
 * Since: 2.0
 */
const gchar * const *
gtef_action_info_get_accels (const GtefActionInfo *info)
{
	g_return_val_if_fail (info != NULL, NULL);

	g_assert (info->accels != NULL);

	return (const gchar * const *)info->accels;
}

/**
 * gtef_action_info_set_accels:
 * @info: a #GtefActionInfo.
 * @accels: (array zero-terminated=1): a %NULL-terminated array of accelerators
 * in the format understood by gtk_accelerator_parse().
 *
 * A function similar to gtk_application_set_accels_for_action().
 *
 * @accels must not be %NULL, it must be a %NULL-terminated array, to be
 * consistent with gtk_application_set_accels_for_action().
 *
 * Since: 2.0
 */
void
gtef_action_info_set_accels (GtefActionInfo      *info,
			     const gchar * const *accels)
{
	g_return_if_fail (info != NULL);
	g_return_if_fail (accels != NULL);

	g_strfreev (info->accels);
	info->accels = _gtef_utils_strv_copy (accels);
}

gboolean
_gtef_action_info_get_used (const GtefActionInfo *info)
{
	g_return_val_if_fail (info != NULL, FALSE);

	return info->used;
}

void
_gtef_action_info_set_used (GtefActionInfo *info)
{
	g_return_if_fail (info != NULL);

	info->used = TRUE;
}
