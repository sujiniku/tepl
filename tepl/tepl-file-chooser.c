/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "tepl-file-chooser.h"

/**
 * SECTION:file-chooser
 * @Title: TeplFileChooser
 * @Short_description: Extra functions for #GtkFileChooser
 *
 * Extra functions for #GtkFileChooser, to have common code between
 * #GtkFileChooserDialog and GtkFileChooserNative.
 */

/**
 * tepl_file_chooser_set_modal:
 * @chooser: a #GtkFileChooser.
 * @modal: the new value.
 *
 * Calls either gtk_native_dialog_set_modal() or gtk_window_set_modal()
 * depending on the @chooser type.
 *
 * Since: 5.0
 */
void
tepl_file_chooser_set_modal (GtkFileChooser *chooser,
			     gboolean        modal)
{
	if (GTK_IS_NATIVE_DIALOG (chooser))
	{
		gtk_native_dialog_set_modal (GTK_NATIVE_DIALOG (chooser), modal);
	}
	else if (GTK_IS_WINDOW (chooser))
	{
		gtk_window_set_modal (GTK_WINDOW (chooser), modal);
	}
	else
	{
		g_warn_if_reached ();
	}
}

/**
 * tepl_file_chooser_set_parent:
 * @chooser: a #GtkFileChooser.
 * @parent: (nullable): a #GtkWindow, or %NULL.
 *
 * Sets or unsets a parent #GtkWindow for the @chooser dialog. It calls the
 * right functions depending on the type of @chooser.
 *
 * Since: 5.0
 */
void
tepl_file_chooser_set_parent (GtkFileChooser *chooser,
			      GtkWindow      *parent)
{
	g_return_if_fail (parent == NULL || GTK_IS_WINDOW (parent));

	if (GTK_IS_NATIVE_DIALOG (chooser))
	{
		gtk_native_dialog_set_transient_for (GTK_NATIVE_DIALOG (chooser), parent);
	}
	else if (GTK_IS_WINDOW (chooser))
	{
		gtk_window_set_transient_for (GTK_WINDOW (chooser), parent);

		if (parent != NULL)
		{
			gtk_window_set_destroy_with_parent (GTK_WINDOW (chooser), TRUE);
		}
	}
	else
	{
		g_warn_if_reached ();
	}
}

/**
 * tepl_file_chooser_show:
 * @chooser: a #GtkFileChooser.
 *
 * Calls gtk_native_dialog_show() or gtk_window_present() depending on the type
 * of @chooser.
 *
 * Since: 5.0
 */
void
tepl_file_chooser_show (GtkFileChooser *chooser)
{
	if (GTK_IS_NATIVE_DIALOG (chooser))
	{
		gtk_native_dialog_show (GTK_NATIVE_DIALOG (chooser));
	}
	else if (GTK_IS_WINDOW (chooser))
	{
		gtk_window_present (GTK_WINDOW (chooser));
	}
	else
	{
		g_warn_if_reached ();
	}
}
