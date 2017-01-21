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

#ifndef GTEF_ACTION_INFO_H
#define GTEF_ACTION_INFO_H

#if !defined (GTEF_H_INSIDE) && !defined (GTEF_COMPILATION)
#error "Only <gtef/gtef.h> can be included directly."
#endif

#include <glib-object.h>
#include <gtef/gtef-types.h>

G_BEGIN_DECLS

#define GTEF_TYPE_ACTION_INFO (gtef_action_info_get_type ())

/**
 * GtefActionInfoEntry:
 * @action_name: the action name.
 * @icon_name: the icon name, or %NULL.
 * @label: the label (i.e. a short description), or %NULL.
 * @accel: the accelerator, in the format understood by gtk_accelerator_parse().
 * Or %NULL.
 * @tooltip: the tooltip (i.e. a long description), or %NULL.
 *
 * This struct defines a set of information for a single action. It is for use
 * with gtef_action_info_store_add_entries().
 *
 * Like #GActionEntry, it is permissible to use an incomplete initialiser in
 * order to leave some of the later values as %NULL. Additional optional fields
 * may be added in the future.
 *
 * Since: 2.0
 */
struct _GtefActionInfoEntry
{
	const gchar *action_name;
	const gchar *icon_name;
	const gchar *label;
	const gchar *accel;
	const gchar *tooltip;

	/*< private >*/
	gpointer padding[3];
};

GType			gtef_action_info_get_type		(void) G_GNUC_CONST;

GtefActionInfo *	gtef_action_info_new			(void);

GtefActionInfo *	gtef_action_info_new_from_entry		(const GtefActionInfoEntry *info_entry,
								 const gchar               *translation_domain);

GtefActionInfo *	gtef_action_info_copy			(const GtefActionInfo *info);

void			gtef_action_info_free			(GtefActionInfo *info);

const gchar *		gtef_action_info_get_action_name	(const GtefActionInfo *info);

void			gtef_action_info_set_action_name	(GtefActionInfo *info,
								 const gchar    *action_name);

const gchar *		gtef_action_info_get_icon_name		(const GtefActionInfo *info);

void			gtef_action_info_set_icon_name		(GtefActionInfo *info,
								 const gchar    *icon_name);

const gchar *		gtef_action_info_get_label		(const GtefActionInfo *info);

void			gtef_action_info_set_label		(GtefActionInfo *info,
								 const gchar    *label);

const gchar *		gtef_action_info_get_tooltip		(const GtefActionInfo *info);

void			gtef_action_info_set_tooltip		(GtefActionInfo *info,
								 const gchar    *tooltip);

const gchar * const *	gtef_action_info_get_accels		(const GtefActionInfo *info);

void			gtef_action_info_set_accels		(GtefActionInfo      *info,
								 const gchar * const *accels);

G_END_DECLS

#endif  /* GTEF_ACTION_INFO_H */
