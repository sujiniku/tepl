/* Copyright 2017-2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "tepl-init.h"
#include <amtk/amtk.h>
#include <gtksourceview/gtksource.h>
#include "tepl-abstract-factory.h"
#include "tepl-metadata-manager.h"

/**
 * tepl_init:
 *
 * Initializes the Tepl library (e.g. for the internationalization).
 *
 * This function can be called several times, but is meant to be called at the
 * beginning of main(), before any other Tepl function call.
 *
 * This function also calls amtk_init() and gtk_source_init().
 *
 * Since: 3.0
 */
void
tepl_init (void)
{
	amtk_init ();
	gtk_source_init ();
}

/**
 * tepl_finalize:
 *
 * Free the resources allocated by Tepl. For example it unrefs the singleton
 * objects.
 *
 * This function also calls amtk_finalize() and gtk_source_finalize().
 *
 * It is not mandatory to call this function, it's just to be friendlier to
 * memory debugging tools. This function is meant to be called at the end of
 * main(). It can be called several times.
 *
 * Since: 3.0
 */

/* Another way is to use a DSO destructor, see gconstructor.h in GLib.
 *
 * The advantage of calling tepl_finalize() at the end of main() is that
 * gobject-list [1] correctly reports that all Tepl* objects have been finalized
 * when quitting the application. On the other hand a DSO destructor runs after
 * the gobject-list's last output, so it's much less convenient, see:
 * https://git.gnome.org/browse/gtksourceview/commit/?id=e761de9c2bee90c232875bbc41e6e73e1f63e145
 *
 * [1] A tool for debugging the lifetime of GObjects:
 * https://github.com/danni/gobject-list
 */
void
tepl_finalize (void)
{
	static gboolean done = FALSE;

	if (!done)
	{
		_tepl_metadata_manager_unref_singleton ();
		_tepl_abstract_factory_unref_singleton ();

		/* Since Tepl depends on Amtk and GtkSourceView, it's better to
		 * first finalize Tepl stuff and then finalize the underlying
		 * libraries, in case the Tepl singletons depend on Amtk or
		 * GtkSourceView.
		 */
		gtk_source_finalize ();
		amtk_finalize ();

		done = TRUE;
	}
}
