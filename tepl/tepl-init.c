/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2017-2020 - Sébastien Wilmet <swilmet@gnome.org>
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

#include "tepl-init.h"
#include <amtk/amtk.h>
#include <gtksourceview/gtksource.h>
#include "tepl-abstract-factory.h"
#include "tepl-metadata-manager.h"
#include "tepl-metadata-store.h"

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
 * objects. It also properly shutdowns the metadata manager by calling
 * tepl_metadata_manager_shutdown().
 *
 * This function also calls amtk_finalize() and gtk_source_finalize().
 *
 * It is not mandatory to call this function, it's just to be friendlier to
 * memory debugging tools (but if you don't call this function and you use the
 * metadata manager, you should call tepl_metadata_manager_shutdown()). This
 * function is meant to be called at the end of main(). It can be called several
 * times.
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
		tepl_metadata_manager_shutdown ();
		_tepl_metadata_store_unref_singleton ();
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
