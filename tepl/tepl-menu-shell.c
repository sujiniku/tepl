/* SPDX-FileCopyrightText: 2017 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "tepl-menu-shell.h"
#include <amtk/amtk.h>

/**
 * SECTION:menu-shell
 * @Short_description: #GtkMenuShell functions
 * @Title: TeplMenuShell
 *
 * #GtkMenuShell functions.
 */

/**
 * tepl_menu_shell_append_edit_actions:
 * @menu_shell: a #GtkMenuShell.
 *
 * Appends #GtkMenuItem's to @menu_shell for the following #GAction's:
 * - `"win.tepl-cut"`
 * - `"win.tepl-copy"`
 * - `"win.tepl-paste"`
 * - `"win.tepl-delete"`
 * - `"win.tepl-select-all"`
 *
 * See the [list of GActions implemented in
 * TeplApplicationWindow][tepl-application-window-gactions]. This function
 * correctly uses the %AMTK_FACTORY_IGNORE_ACCELS_FOR_APP flag to create the
 * #GtkMenuItem's.
 *
 * Since: 3.0
 */
void
tepl_menu_shell_append_edit_actions (GtkMenuShell *menu_shell)
{
	AmtkFactory *factory;

	g_return_if_fail (GTK_IS_MENU_SHELL (menu_shell));

	factory = amtk_factory_new (NULL);
	amtk_factory_set_default_flags (factory, AMTK_FACTORY_IGNORE_ACCELS_FOR_APP);

	gtk_menu_shell_append (menu_shell, amtk_factory_create_menu_item (factory, "win.tepl-cut"));
	gtk_menu_shell_append (menu_shell, amtk_factory_create_menu_item (factory, "win.tepl-copy"));
	gtk_menu_shell_append (menu_shell, amtk_factory_create_menu_item (factory, "win.tepl-paste"));
	gtk_menu_shell_append (menu_shell, amtk_factory_create_menu_item (factory, "win.tepl-delete"));
	gtk_menu_shell_append (menu_shell, amtk_factory_create_menu_item (factory, "win.tepl-select-all"));

	g_object_unref (factory);
}
