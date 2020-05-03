/* Copyright 2017-2020 - Sébastien Wilmet <swilmet@gnome.org>
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

#include "config.h"
#include "tepl-application.h"
#include <glib/gi18n-lib.h>
#include "tepl-abstract-factory.h"
#include "tepl-application-window.h"
#include "tepl-metadata-manager.h"

/**
 * SECTION:application
 * @Short_description: An extension of GtkApplication
 * @Title: TeplApplication
 *
 * #TeplApplication extends the #GtkApplication class.
 *
 * For some features, the Tepl framework gets the default #GtkApplication with
 * g_application_get_default(), for example to call g_application_hold(),
 * g_application_mark_busy(), etc. Normally a GTK application has only one
 * #GApplication per process, so this shouldn't cause any problem.
 *
 * Note that #TeplApplication extends the #GtkApplication class but without
 * subclassing it, because several libraries might want to extend
 * #GtkApplication and an application needs to be able to use all those
 * extensions at the same time.
 *
 * # GActions # {#tepl-application-gactions}
 *
 * This class adds the following #GAction's to the #GtkApplication.
 * Corresponding #AmtkActionInfo's are available with
 * tepl_application_get_tepl_action_info_store().
 *
 * ## For the File menu
 *
 * - `"app.tepl-new-window"`: creates a new main window with
 *   tepl_abstract_factory_create_main_window().
 */

struct _TeplApplicationPrivate
{
	GtkApplication *gtk_app;
	AmtkActionInfoStore *app_action_info_store;
	AmtkActionInfoStore *tepl_action_info_store;

	guint handle_activate : 1;
	guint handle_open : 1;
	guint handle_metadata : 1;
};

enum
{
	PROP_0,
	PROP_APPLICATION,
	N_PROPERTIES
};

#define TEPL_APPLICATION_KEY "tepl-application-key"

static GParamSpec *properties[N_PROPERTIES];

G_DEFINE_TYPE_WITH_PRIVATE (TeplApplication, tepl_application, G_TYPE_OBJECT)

static void
init_tepl_action_info_store (TeplApplication *tepl_app)
{
	const AmtkActionInfoEntry entries[] =
	{
		/* action, icon, label, accel, tooltip */

		/* File menu */

		// Why "file" and not "document"? "Document" is not the best
		// word because the action is not always to create a new
		// document. For example a LaTeX document can be composed of
		// several _files_. Or for source code we do not really create a
		// new "document".
		{ "win.tepl-new-file", "document-new", N_("_New"), "<Control>n",
		  N_("New file") },

		{ "app.tepl-new-window", NULL, N_("New _Window"), NULL,
		  N_("Create a new window") },

		{ "win.tepl-open", "document-open", N_("_Open"), "<Control>o",
		  N_("Open a file") },

		{ "win.tepl-save", "document-save", N_("_Save"), "<Control>s",
		  N_("Save the current file") },

		{ "win.tepl-save-as", "document-save-as", N_("Save _As"), "<Shift><Control>s",
		  N_("Save the current file to a different location") },

		/* Edit menu */

		{ "win.tepl-undo", "edit-undo", N_("_Undo"), "<Control>z",
		  N_("Undo the last action") },

		{ "win.tepl-redo", "edit-redo", N_("_Redo"), "<Shift><Control>z",
		  N_("Redo the last undone action") },

		{ "win.tepl-cut", "edit-cut", N_("Cu_t"), "<Control>x",
		  N_("Cut the selection") },

		{ "win.tepl-copy", "edit-copy", N_("_Copy"), "<Control>c",
		  N_("Copy the selection") },

		{ "win.tepl-paste", "edit-paste", N_("_Paste"), "<Control>v",
		  N_("Paste the clipboard") },

		{ "win.tepl-delete", "edit-delete", N_("_Delete"), NULL,
		  N_("Delete the selected text") },

		{ "win.tepl-select-all", "edit-select-all", N_("Select _All"), "<Control>a",
		  N_("Select all the text") },

		{ "win.tepl-indent", "format-indent-more", N_("_Indent"), "Tab",
		  N_("Indent the selected lines") },

		{ "win.tepl-unindent", "format-indent-less", N_("_Unindent"), "<Shift>Tab",
		  N_("Unindent the selected lines") },

		/* Search menu */

		{ "win.tepl-goto-line", "go-jump", N_("_Go to Line…"), "<Control>l",
		  N_("Go to a specific line") },
	};

	g_assert (tepl_app->priv->tepl_action_info_store == NULL);
	tepl_app->priv->tepl_action_info_store = amtk_action_info_store_new ();

	amtk_action_info_store_add_entries (tepl_app->priv->tepl_action_info_store,
					    entries,
					    G_N_ELEMENTS (entries),
					    GETTEXT_PACKAGE);
}

static void
new_window_cb (GSimpleAction *action,
	       GVariant      *parameter,
	       gpointer       user_data)
{
	TeplApplication *tepl_app = TEPL_APPLICATION (user_data);
	TeplAbstractFactory *factory;
	GtkApplicationWindow *main_window;

	factory = tepl_abstract_factory_get_singleton ();
	main_window = tepl_abstract_factory_create_main_window (factory, tepl_app->priv->gtk_app);
	g_return_if_fail (main_window != NULL);

	gtk_widget_show (GTK_WIDGET (main_window));
}

static void
add_actions (TeplApplication *tepl_app)
{
	/* The actions need to be namespaced, to not conflict with the
	 * application or other libraries.
	 *
	 * Do not forget to document each action in the TeplApplication class
	 * description, and to add the corresponding AmtkActionInfoEntry in
	 * init_tepl_action_info_store().
	 */
	const GActionEntry entries[] = {
		/* File menu */
		{ "tepl-new-window", new_window_cb },
	};

	amtk_action_map_add_action_entries_check_dups (G_ACTION_MAP (tepl_app->priv->gtk_app),
						       entries,
						       G_N_ELEMENTS (entries),
						       tepl_app);
}

static void
tepl_application_get_property (GObject    *object,
			       guint       prop_id,
			       GValue     *value,
			       GParamSpec *pspec)
{
	TeplApplication *tepl_app = TEPL_APPLICATION (object);

	switch (prop_id)
	{
		case PROP_APPLICATION:
			g_value_set_object (value, tepl_application_get_application (tepl_app));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
tepl_application_set_property (GObject      *object,
			       guint         prop_id,
			       const GValue *value,
			       GParamSpec   *pspec)
{
	TeplApplication *tepl_app = TEPL_APPLICATION (object);

	switch (prop_id)
	{
		case PROP_APPLICATION:
			g_assert (tepl_app->priv->gtk_app == NULL);
			tepl_app->priv->gtk_app = g_value_get_object (value);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
tepl_application_constructed (GObject *object)
{
	TeplApplication *tepl_app = TEPL_APPLICATION (object);

	if (G_OBJECT_CLASS (tepl_application_parent_class)->constructed != NULL)
	{
		G_OBJECT_CLASS (tepl_application_parent_class)->constructed (object);
	}

	add_actions (tepl_app);
}

static void
tepl_application_dispose (GObject *object)
{
	TeplApplication *tepl_app = TEPL_APPLICATION (object);

	tepl_app->priv->gtk_app = NULL;
	g_clear_object (&tepl_app->priv->app_action_info_store);
	g_clear_object (&tepl_app->priv->tepl_action_info_store);

	G_OBJECT_CLASS (tepl_application_parent_class)->dispose (object);
}

static void
tepl_application_class_init (TeplApplicationClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = tepl_application_get_property;
	object_class->set_property = tepl_application_set_property;
	object_class->constructed = tepl_application_constructed;
	object_class->dispose = tepl_application_dispose;

	/**
	 * TeplApplication:application:
	 *
	 * The #GtkApplication.
	 *
	 * Since: 2.0
	 */
	properties[PROP_APPLICATION] =
		g_param_spec_object ("application",
				     "GtkApplication",
				     "",
				     GTK_TYPE_APPLICATION,
				     G_PARAM_READWRITE |
				     G_PARAM_CONSTRUCT_ONLY |
				     G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (object_class, N_PROPERTIES, properties);
}

static void
tepl_application_init (TeplApplication *tepl_app)
{
	tepl_app->priv = tepl_application_get_instance_private (tepl_app);

	tepl_app->priv->app_action_info_store = amtk_action_info_store_new ();
	init_tepl_action_info_store (tepl_app);
}

/**
 * tepl_application_get_from_gtk_application:
 * @gtk_app: a #GtkApplication.
 *
 * Returns the #TeplApplication of @gtk_app. The returned object is guaranteed
 * to be the same for the lifetime of @gtk_app.
 *
 * Returns: (transfer none): the #TeplApplication of @gtk_app.
 * Since: 2.0
 */
TeplApplication *
tepl_application_get_from_gtk_application (GtkApplication *gtk_app)
{
	TeplApplication *tepl_app;

	g_return_val_if_fail (GTK_IS_APPLICATION (gtk_app), NULL);

	tepl_app = g_object_get_data (G_OBJECT (gtk_app), TEPL_APPLICATION_KEY);

	if (tepl_app == NULL)
	{
		tepl_app = g_object_new (TEPL_TYPE_APPLICATION,
					 "application", gtk_app,
					 NULL);

		g_object_set_data_full (G_OBJECT (gtk_app),
					TEPL_APPLICATION_KEY,
					tepl_app,
					g_object_unref);
	}

	g_return_val_if_fail (TEPL_IS_APPLICATION (tepl_app), NULL);
	return tepl_app;
}

/**
 * tepl_application_get_default:
 *
 * Convenience function that calls g_application_get_default() followed by
 * tepl_application_get_from_gtk_application(). The object returned by
 * g_application_get_default() must be a #GtkApplication.
 *
 * Returns: (transfer none): the default #TeplApplication.
 * Since: 2.0
 */
TeplApplication *
tepl_application_get_default (void)
{
	GApplication *g_app;

	g_app = g_application_get_default ();
	g_return_val_if_fail (GTK_IS_APPLICATION (g_app), NULL);

	return tepl_application_get_from_gtk_application (GTK_APPLICATION (g_app));
}

/**
 * tepl_application_get_application:
 * @tepl_app: a #TeplApplication.
 *
 * Returns: (transfer none): the #GtkApplication of @tepl_app.
 * Since: 2.0
 */
GtkApplication *
tepl_application_get_application (TeplApplication *tepl_app)
{
	g_return_val_if_fail (TEPL_IS_APPLICATION (tepl_app), NULL);

	return tepl_app->priv->gtk_app;
}

/**
 * tepl_application_get_app_action_info_store:
 * @tepl_app: a #TeplApplication.
 *
 * Returns an initially empty #AmtkActionInfoStore reserved for the
 * application-specific actions. Libraries should not add #AmtkActionInfo's to
 * this store. Libraries should provide their own store if they want to share
 * #AmtkActionInfo's.
 *
 * Returns: (transfer none): the #AmtkActionInfoStore reserved for the
 * application.
 * Since: 2.0
 */
AmtkActionInfoStore *
tepl_application_get_app_action_info_store (TeplApplication *tepl_app)
{
	g_return_val_if_fail (TEPL_IS_APPLICATION (tepl_app), NULL);

	return tepl_app->priv->app_action_info_store;
}

/**
 * tepl_application_get_tepl_action_info_store:
 * @tepl_app: a #TeplApplication.
 *
 * The returned #AmtkActionInfoStore contains #AmtkActionInfo's for all the
 * #GAction's listed in the [class description of
 * TeplApplicationWindow][tepl-application-window-gactions] and the [class
 * description of TeplApplication][tepl-application-gactions].
 *
 * Returns: (transfer none): the #AmtkActionInfoStore of the Tepl library.
 * Since: 3.0
 */
AmtkActionInfoStore *
tepl_application_get_tepl_action_info_store (TeplApplication *tepl_app)
{
	g_return_val_if_fail (TEPL_IS_APPLICATION (tepl_app), NULL);

	return tepl_app->priv->tepl_action_info_store;
}

/**
 * tepl_application_get_active_main_window:
 * @tepl_app: a #TeplApplication.
 *
 * Like gtk_application_get_active_window(), but returns the main window in the
 * sense of tepl_application_window_is_main_window().
 *
 * Returns: (transfer none) (nullable): the active main #GtkApplicationWindow,
 * or %NULL.
 * Since: 4.0
 */
GtkApplicationWindow *
tepl_application_get_active_main_window (TeplApplication *tepl_app)
{
	GList *windows;
	GList *l;

	g_return_val_if_fail (TEPL_IS_APPLICATION (tepl_app), NULL);

	windows = gtk_application_get_windows (tepl_app->priv->gtk_app);

	for (l = windows; l != NULL; l = l->next)
	{
		GtkWindow *window = l->data;

		if (GTK_IS_APPLICATION_WINDOW (window) &&
		    tepl_application_window_is_main_window (GTK_APPLICATION_WINDOW (window)))
		{
			return GTK_APPLICATION_WINDOW (window);
		}
	}

	return NULL;
}

/**
 * tepl_application_open_simple:
 * @tepl_app: a #TeplApplication.
 * @file: a #GFile.
 *
 * Calls g_application_open() with a single file and an empty hint.
 *
 * Since: 2.0
 */
void
tepl_application_open_simple (TeplApplication *tepl_app,
			      GFile           *file)
{
	GFile *files[1];

	g_return_if_fail (TEPL_IS_APPLICATION (tepl_app));
	g_return_if_fail (G_IS_FILE (file));

	files[0] = file;
	g_application_open (G_APPLICATION (tepl_app->priv->gtk_app), files, 1, "");
}

static void
activate_cb (GApplication    *g_app,
	     TeplApplication *tepl_app)
{
	GtkApplicationWindow *main_window;

	g_application_hold (g_app);

	main_window = tepl_application_get_active_main_window (tepl_app);

	if (main_window == NULL)
	{
		TeplAbstractFactory *factory;

		factory = tepl_abstract_factory_get_singleton ();
		main_window = tepl_abstract_factory_create_main_window (factory, tepl_app->priv->gtk_app);
		gtk_widget_show (GTK_WIDGET (main_window));
	}
	else
	{
		GtkWindow *active_window;

		active_window = gtk_application_get_active_window (tepl_app->priv->gtk_app);
		gtk_window_present (active_window);
	}

	g_application_release (g_app);
}

/**
 * tepl_application_handle_activate:
 * @tepl_app: a #TeplApplication.
 *
 * Connects a generic function handler for the #GApplication::activate signal.
 *
 * If no main windows exist, it creates one with
 * tepl_abstract_factory_create_main_window(). If a main window already exists,
 * it calls gtk_window_present() on the most recently focused window of the
 * application.
 *
 * Since: 4.0
 */
void
tepl_application_handle_activate (TeplApplication *tepl_app)
{
	g_return_if_fail (TEPL_IS_APPLICATION (tepl_app));

	if (!tepl_app->priv->handle_activate)
	{
		g_signal_connect_object (tepl_app->priv->gtk_app,
					 "activate",
					 G_CALLBACK (activate_cb),
					 tepl_app,
					 0);

		tepl_app->priv->handle_activate = TRUE;
	}
}

static void
open_cb (GApplication     *g_app,
	 GFile           **files,
	 gint              n_files,
	 const gchar      *hint,
	 TeplApplication  *tepl_app)
{
	GtkApplicationWindow *main_window;
	TeplApplicationWindow *tepl_window;
	gint file_num;

	if (n_files < 1)
	{
		return;
	}

	g_application_hold (g_app);

	main_window = tepl_application_get_active_main_window (tepl_app);

	if (main_window == NULL)
	{
		TeplAbstractFactory *factory;

		factory = tepl_abstract_factory_get_singleton ();
		main_window = tepl_abstract_factory_create_main_window (factory, tepl_app->priv->gtk_app);

		if (main_window == NULL)
		{
			g_warn_if_reached ();
			goto out;
		}

		gtk_widget_show (GTK_WIDGET (main_window));
	}

	tepl_window = tepl_application_window_get_from_gtk_application_window (main_window);

	/* TODO: improve this, currently all the files are open at the same time
	 * in parallel, it would be better to open them sequentially. Maybe by
	 * writing a MultiFileLoader class:
	 * 1. Create all the tabs, jump only to the first one.
	 * 2. Set locations.
	 * 3. Set editable=FALSE on all those views (+ set tab state/locking?).
	 * 4. Load the files one by one. Needs an async/finish API to load one
	 *    file.
	 */
	for (file_num = 0; file_num < n_files; file_num++)
	{
		GFile *cur_file = files[file_num];
		gboolean jump_to = file_num == 0;

		tepl_application_window_open_file (tepl_window, cur_file, jump_to);
	}

out:
	g_application_release (g_app);
}

/**
 * tepl_application_handle_open:
 * @tepl_app: a #TeplApplication.
 *
 * Connects a generic function handler for the #GApplication::open signal.
 *
 * It calls tepl_application_window_open_file() for each #GFile to open, on the
 * active main window as returned by tepl_application_get_active_main_window().
 * If the active main window is %NULL, it creates one with
 * tepl_abstract_factory_create_main_window().
 *
 * Since: 4.0
 */
void
tepl_application_handle_open (TeplApplication *tepl_app)
{
	g_return_if_fail (TEPL_IS_APPLICATION (tepl_app));

	if (!tepl_app->priv->handle_open)
	{
		g_signal_connect_object (tepl_app->priv->gtk_app,
					 "open",
					 G_CALLBACK (open_cb),
					 tepl_app,
					 0);

		tepl_app->priv->handle_open = TRUE;
	}
}

static void
handle_metadata__startup_cb (GtkApplication  *gtk_app,
			     TeplApplication *tepl_app)
{
	TeplAbstractFactory *factory = tepl_abstract_factory_get_singleton ();
	TeplMetadataManager *manager = tepl_metadata_manager_get_singleton ();
	GFile *file;
	GError *error = NULL;

	file = tepl_abstract_factory_create_metadata_manager_file (factory);
	if (file == NULL)
	{
		return;
	}

	tepl_metadata_manager_load_from_disk (manager, file, &error);
	if (error != NULL)
	{
		g_warning ("Failed to load metadata: %s", error->message);
		g_clear_error (&error);
	}

	g_object_unref (file);
}

static void
handle_metadata__shutdown_after_cb (GtkApplication  *gtk_app,
				    TeplApplication *tepl_app)
{
	TeplAbstractFactory *factory = tepl_abstract_factory_get_singleton ();
	TeplMetadataManager *manager = tepl_metadata_manager_get_singleton ();
	GFile *file;
	GError *error = NULL;

	file = tepl_abstract_factory_create_metadata_manager_file (factory);
	if (file == NULL)
	{
		return;
	}

	tepl_metadata_manager_save_to_disk (manager, file, TRUE, &error);
	if (error != NULL)
	{
		g_warning ("Failed to save metadata: %s", error->message);
		g_clear_error (&error);
	}

	g_object_unref (file);
}

/**
 * tepl_application_handle_metadata:
 * @tepl_app: a #TeplApplication.
 *
 * This function:
 * - Connects to the #GApplication::startup signal to call
 *   tepl_metadata_manager_load_from_disk().
 * - Connects to the #GApplication::shutdown signal to call
 *   tepl_metadata_manager_save_to_disk() with @trim set to %TRUE.
 *
 * It gets the #GFile by calling
 * tepl_abstract_factory_create_metadata_manager_file().
 *
 * Since: 5.0
 */
void
tepl_application_handle_metadata (TeplApplication *tepl_app)
{
	g_return_if_fail (TEPL_IS_APPLICATION (tepl_app));

	if (!tepl_app->priv->handle_metadata)
	{
		g_signal_connect_object (tepl_app->priv->gtk_app,
					 "startup",
					 G_CALLBACK (handle_metadata__startup_cb),
					 tepl_app,
					 0);

		/* Connect with G_CONNECT_AFTER, so that GTK is properly
		 * shutdown. Saving metadata should be done last.
		 */
		g_signal_connect_object (tepl_app->priv->gtk_app,
					 "shutdown",
					 G_CALLBACK (handle_metadata__shutdown_after_cb),
					 tepl_app,
					 G_CONNECT_AFTER);

		tepl_app->priv->handle_metadata = TRUE;
	}
}
