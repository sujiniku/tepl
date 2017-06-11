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

#ifndef TEPL_ACTION_INFO_H
#define TEPL_ACTION_INFO_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <glib-object.h>
#include <tepl/tepl-types.h>

G_BEGIN_DECLS

#define TEPL_TYPE_ACTION_INFO (tepl_action_info_get_type ())

/**
 * TeplActionInfoEntry:
 * @action_name: the action name.
 * @icon_name: the icon name, or %NULL.
 * @label: the label (i.e. a short description), or %NULL.
 * @accel: the accelerator, in the format understood by gtk_accelerator_parse().
 * Or %NULL.
 * @tooltip: the tooltip (i.e. a long description), or %NULL.
 *
 * This struct defines a set of information for a single action. It is for use
 * with tepl_action_info_store_add_entries().
 *
 * Like #GActionEntry, it is permissible to use an incomplete initialiser in
 * order to leave some of the later values as %NULL. Additional optional fields
 * may be added in the future.
 *
 * Since: 2.0
 */
struct _TeplActionInfoEntry
{
	const gchar *action_name;
	const gchar *icon_name;
	const gchar *label;
	const gchar *accel;
	const gchar *tooltip;

	/*< private >*/
	gpointer padding[3];
};

GType			tepl_action_info_get_type		(void) G_GNUC_CONST;

TeplActionInfo *	tepl_action_info_new			(void);

TeplActionInfo *	tepl_action_info_new_from_entry		(const TeplActionInfoEntry *info_entry,
								 const gchar               *translation_domain);

TeplActionInfo *	tepl_action_info_ref			(TeplActionInfo *info);

void			tepl_action_info_unref			(TeplActionInfo *info);

TeplActionInfo *	tepl_action_info_copy			(const TeplActionInfo *info);

const gchar *		tepl_action_info_get_action_name	(const TeplActionInfo *info);

void			tepl_action_info_set_action_name	(TeplActionInfo *info,
								 const gchar    *action_name);

const gchar *		tepl_action_info_get_icon_name		(const TeplActionInfo *info);

void			tepl_action_info_set_icon_name		(TeplActionInfo *info,
								 const gchar    *icon_name);

const gchar *		tepl_action_info_get_label		(const TeplActionInfo *info);

void			tepl_action_info_set_label		(TeplActionInfo *info,
								 const gchar    *label);

const gchar *		tepl_action_info_get_tooltip		(const TeplActionInfo *info);

void			tepl_action_info_set_tooltip		(TeplActionInfo *info,
								 const gchar    *tooltip);

const gchar * const *	tepl_action_info_get_accels		(const TeplActionInfo *info);

void			tepl_action_info_set_accels		(TeplActionInfo      *info,
								 const gchar * const *accels);

G_GNUC_INTERNAL
gboolean		_tepl_action_info_get_used		(const TeplActionInfo *info);

G_GNUC_INTERNAL
void			_tepl_action_info_set_used		(TeplActionInfo *info);

G_END_DECLS

#endif  /* TEPL_ACTION_INFO_H */
