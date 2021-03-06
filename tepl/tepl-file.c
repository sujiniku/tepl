/* SPDX-FileCopyrightText: 2016-2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "config.h"
#include "tepl-file.h"
#include <glib/gi18n-lib.h>
#include "tepl-utils.h"
#include "tepl-enum-types.h"

/**
 * SECTION:file
 * @Title: TeplFile
 * @Short_description: On-disk representation of a #TeplBuffer
 * @See_also: #TeplFileLoader, #TeplFileSaver
 *
 * A #TeplFile object is the on-disk representation of a #TeplBuffer.
 *
 * With a #TeplFile, you can create and configure a #TeplFileLoader and
 * #TeplFileSaver which take by default the values of the #TeplFile properties
 * (except for the file loader which auto-detects some properties). On a
 * successful load or save operation, the #TeplFile properties are updated. If
 * an operation fails, the #TeplFile properties have still the previous valid
 * values.
 *
 * It is possible to use #TeplFile without using #TeplFileLoader and
 * #TeplFileSaver. #TeplFile alone offers several features useful for a text
 * editor.
 *
 * When using #TeplFile alone, #TeplFile does the I/O operations (if any)
 * asynchronously.
 */

struct _TeplFilePrivate
{
	GFile *location;
	TeplNewlineType newline_type;

	/* For the short-name. */
	gint untitled_number;
	gchar *display_name;

	TeplMountOperationFactory mount_operation_factory;
	gpointer mount_operation_userdata;
	GDestroyNotify mount_operation_notify;

	/* Last known entity tag of 'location'. The value is updated on a file
	 * loading or file saving.
	 */
	gchar *etag;
};

enum
{
	PROP_0,
	PROP_LOCATION,
	PROP_NEWLINE_TYPE,
	PROP_SHORT_NAME,
	N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES];

/* The list is sorted. */
static GSList *allocated_untitled_numbers;

G_DEFINE_TYPE_WITH_PRIVATE (TeplFile, tepl_file, G_TYPE_OBJECT)

static gint
compare_untitled_numbers (gconstpointer a,
			  gconstpointer b)
{
	gint num_a = GPOINTER_TO_INT (a);
	gint num_b = GPOINTER_TO_INT (b);

	return num_a - num_b;
}

/* Starts at 1. O(n). But n is normally always very small. */
static gint
allocate_first_available_untitled_number (void)
{
	gint num = 1;
	GSList *l;

	for (l = allocated_untitled_numbers; l != NULL; l = l->next)
	{
		gint cur_num = GPOINTER_TO_INT (l->data);

		if (num != cur_num)
		{
			g_assert_cmpint (num, <, cur_num);
			break;
		}

		num++;
	}

	g_assert (g_slist_find (allocated_untitled_numbers, GINT_TO_POINTER (num)) == NULL);

	allocated_untitled_numbers = g_slist_insert_sorted (allocated_untitled_numbers,
							    GINT_TO_POINTER (num),
							    compare_untitled_numbers);

	return num;
}

static void
release_untitled_number (gint num)
{
	g_assert (g_slist_find (allocated_untitled_numbers, GINT_TO_POINTER (num)) != NULL);

	allocated_untitled_numbers = g_slist_remove (allocated_untitled_numbers,
						     GINT_TO_POINTER (num));

	g_assert (g_slist_find (allocated_untitled_numbers, GINT_TO_POINTER (num)) == NULL);
}

static void
tepl_file_get_property (GObject    *object,
			guint       prop_id,
			GValue     *value,
			GParamSpec *pspec)
{
	TeplFile *file = TEPL_FILE (object);

	switch (prop_id)
	{
		case PROP_LOCATION:
			g_value_set_object (value, tepl_file_get_location (file));
			break;

		case PROP_NEWLINE_TYPE:
			g_value_set_enum (value, tepl_file_get_newline_type (file));
			break;

		case PROP_SHORT_NAME:
			g_value_take_string (value, tepl_file_get_short_name (file));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
tepl_file_set_property (GObject      *object,
			guint         prop_id,
			const GValue *value,
			GParamSpec   *pspec)
{
	TeplFile *file = TEPL_FILE (object);

	switch (prop_id)
	{
		case PROP_LOCATION:
			tepl_file_set_location (file, g_value_get_object (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
tepl_file_dispose (GObject *object)
{
	TeplFile *file = TEPL_FILE (object);

	g_clear_object (&file->priv->location);

	if (file->priv->mount_operation_notify != NULL)
	{
		file->priv->mount_operation_notify (file->priv->mount_operation_userdata);
		file->priv->mount_operation_notify = NULL;
	}

	G_OBJECT_CLASS (tepl_file_parent_class)->dispose (object);
}

static void
tepl_file_finalize (GObject *object)
{
	TeplFile *file = TEPL_FILE (object);

	if (file->priv->untitled_number > 0)
	{
		release_untitled_number (file->priv->untitled_number);
	}

	g_free (file->priv->display_name);
	g_free (file->priv->etag);

	G_OBJECT_CLASS (tepl_file_parent_class)->finalize (object);
}

static void
tepl_file_class_init (TeplFileClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = tepl_file_get_property;
	object_class->set_property = tepl_file_set_property;
	object_class->dispose = tepl_file_dispose;
	object_class->finalize = tepl_file_finalize;

	/**
	 * TeplFile:location:
	 *
	 * The location.
	 *
	 * Since: 1.0
	 */
	properties[PROP_LOCATION] =
		g_param_spec_object ("location",
				     "location",
				     "",
				     G_TYPE_FILE,
				     G_PARAM_READWRITE |
				     G_PARAM_CONSTRUCT |
				     G_PARAM_STATIC_STRINGS);

	/**
	 * TeplFile:newline-type:
	 *
	 * The line ending type.
	 *
	 * Since: 1.0
	 */
	properties[PROP_NEWLINE_TYPE] =
		g_param_spec_enum ("newline-type",
				   "newline-type",
				   "",
				   TEPL_TYPE_NEWLINE_TYPE,
				   TEPL_NEWLINE_TYPE_DEFAULT,
				   G_PARAM_READABLE |
				   G_PARAM_STATIC_STRINGS);

	/**
	 * TeplFile:short-name:
	 *
	 * The file short name.
	 *
	 * When the #TeplFile:location is %NULL, this property contains
	 * "Untitled File N", with N the Nth untitled file of the application,
	 * starting at 1. When an untitled file is closed (when the #TeplFile is
	 * freed) or its #TeplFile:location is set, its untitled number is
	 * released and can be used by a later file.
	 *
	 * When the #TeplFile:location is not %NULL, this property contains the
	 * display-name (see #G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME). However,
	 * requesting the display-name can take some time (for example for a
	 * remote file with a slow network connection). When the
	 * #TeplFile:location property is set, the display-name is fetched
	 * asynchronously. When the display-name is available, this property is
	 * notified. In the meantime – after the #TeplFile:location is set but
	 * before receiving the display-name – a fallback implementation is used
	 * that does no blocking I/O (but it may return a different result
	 * compared to the real display-name).
	 *
	 * Since: 1.0
	 */
	properties[PROP_SHORT_NAME] =
		g_param_spec_string ("short-name",
				     "short-name",
				     "",
				     NULL,
				     G_PARAM_READABLE |
				     G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (object_class, N_PROPERTIES, properties);
}

static void
query_display_name_cb (GObject      *source_object,
		       GAsyncResult *result,
		       gpointer      user_data)
{
	GFile *location = G_FILE (source_object);
	TeplFile *file = TEPL_FILE (user_data);
	GFileInfo *info;

	info = g_file_query_info_finish (location, result, NULL);

	/* Note: we ignore errors here, because there is no GError to report it.
	 * The same error will probably occur when the user will load or save
	 * the file, and in that case the FileLoader or FileSaver can report a
	 * GError which can be displayed at an appropriate place in the UI.
	 *
	 * When querying the display name fails, the fallback short-name is used
	 * instead.
	 */

	if (info != NULL)
	{
		g_free (file->priv->display_name);
		file->priv->display_name = g_strdup (g_file_info_get_display_name (info));

		g_object_unref (info);
	}

	g_object_notify_by_pspec (G_OBJECT (file), properties[PROP_SHORT_NAME]);

	/* Async operation finished. */
	g_object_unref (file);
}

static void
update_short_name (TeplFile *file)
{
	if (file->priv->location == NULL)
	{
		g_free (file->priv->display_name);
		file->priv->display_name = NULL;

		if (file->priv->untitled_number == 0)
		{
			file->priv->untitled_number = allocate_first_available_untitled_number ();
		}

		g_object_notify_by_pspec (G_OBJECT (file), properties[PROP_SHORT_NAME]);
		return;
	}

	if (file->priv->untitled_number > 0)
	{
		release_untitled_number (file->priv->untitled_number);
		file->priv->untitled_number = 0;
	}

	/* Special case for URIs like "https://example.net". Querying the
	 * display-name for those URIs return "/", which can be confused with
	 * the local root directory.
	 */
	if (!g_file_has_uri_scheme (file->priv->location, "file") &&
	    !g_file_has_parent (file->priv->location, NULL))
	{
		g_free (file->priv->display_name);
		file->priv->display_name = NULL;

		g_object_notify_by_pspec (G_OBJECT (file), properties[PROP_SHORT_NAME]);
		return;
	}

	/* Note: fetching the display name can take a lot of time, for example
	 * for a remote location with a slow response time.
	 * Tested with Slowwly, a test service to mock a slow http response:
	 * http://slowwly.robertomurray.co.uk/
	 *
	 * Note 2: the GFile location needs to really exist, if the file is not
	 * found, then querying the display name fails.
	 */
	g_file_query_info_async (file->priv->location,
				 G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME,
				 G_FILE_QUERY_INFO_NONE,
				 G_PRIORITY_DEFAULT,
				 NULL,
				 query_display_name_cb,
				 g_object_ref (file));
}

static void
tepl_file_init (TeplFile *file)
{
	file->priv = tepl_file_get_instance_private (file);

	file->priv->newline_type = TEPL_NEWLINE_TYPE_DEFAULT;
	update_short_name (file);
}

/**
 * tepl_file_new:
 *
 * Returns: a new #TeplFile object.
 * Since: 1.0
 */
TeplFile *
tepl_file_new (void)
{
	return g_object_new (TEPL_TYPE_FILE, NULL);
}

/**
 * tepl_file_set_location:
 * @file: a #TeplFile.
 * @location: (nullable): the new #GFile, or %NULL.
 *
 * Sets the #TeplFile:location property.
 *
 * Since: 1.0
 */
void
tepl_file_set_location (TeplFile *file,
			GFile    *location)
{
	g_return_if_fail (TEPL_IS_FILE (file));
	g_return_if_fail (location == NULL || G_IS_FILE (location));

	if (g_set_object (&file->priv->location, location))
	{
		/* The etag is for the old location. */
		g_free (file->priv->etag);
		file->priv->etag = NULL;

		g_free (file->priv->display_name);
		file->priv->display_name = NULL;
		update_short_name (file);

		g_object_notify_by_pspec (G_OBJECT (file), properties[PROP_LOCATION]);
	}
}

/**
 * tepl_file_get_location:
 * @file: a #TeplFile.
 *
 * Returns: (transfer none): the value of the #TeplFile:location property.
 * Since: 1.0
 */
GFile *
tepl_file_get_location (TeplFile *file)
{
	g_return_val_if_fail (TEPL_IS_FILE (file), NULL);

	return file->priv->location;
}

/**
 * tepl_file_get_short_name:
 * @file: a #TeplFile.
 *
 * Returns: the value of the #TeplFile:short-name property. Free with g_free()
 *   when no longer needed.
 * Since: 5.0
 */
gchar *
tepl_file_get_short_name (TeplFile *file)
{
	g_return_val_if_fail (TEPL_IS_FILE (file), NULL);

	if (file->priv->untitled_number > 0)
	{
		return g_strdup_printf (_("Untitled File %d"), file->priv->untitled_number);
	}

	if (file->priv->display_name != NULL)
	{
		return g_strdup (file->priv->display_name);
	}

	return _tepl_utils_get_fallback_basename_for_display (file->priv->location);
}

void
_tepl_file_set_newline_type (TeplFile        *file,
			     TeplNewlineType  newline_type)
{
	g_return_if_fail (TEPL_IS_FILE (file));

	if (file->priv->newline_type != newline_type)
	{
		file->priv->newline_type = newline_type;
		g_object_notify_by_pspec (G_OBJECT (file), properties[PROP_NEWLINE_TYPE]);
	}
}

/**
 * tepl_file_get_newline_type:
 * @file: a #TeplFile.
 *
 * Returns: the value of the #TeplFile:newline-type property.
 * Since: 1.0
 */
TeplNewlineType
tepl_file_get_newline_type (TeplFile *file)
{
	g_return_val_if_fail (TEPL_IS_FILE (file), TEPL_NEWLINE_TYPE_DEFAULT);

	return file->priv->newline_type;
}

/**
 * tepl_file_set_mount_operation_factory:
 * @file: a #TeplFile.
 * @callback: (scope notified): a #TeplMountOperationFactory to call when a
 *   #GMountOperation is needed.
 * @user_data: (closure): the data to pass to the @callback function.
 * @notify: (nullable): function to call on @user_data when the @callback is no
 *   longer needed, or %NULL.
 *
 * Sets a #TeplMountOperationFactory function that will be called when a
 * #GMountOperation must be created. This is useful for creating a
 * #GtkMountOperation with the parent #GtkWindow.
 *
 * If a mount operation factory isn't set, g_mount_operation_new() will be
 * called.
 *
 * Since: 1.0
 */
void
tepl_file_set_mount_operation_factory (TeplFile                  *file,
				       TeplMountOperationFactory  callback,
				       gpointer                   user_data,
				       GDestroyNotify             notify)
{
	g_return_if_fail (TEPL_IS_FILE (file));

	if (file->priv->mount_operation_notify != NULL)
	{
		file->priv->mount_operation_notify (file->priv->mount_operation_userdata);
	}

	file->priv->mount_operation_factory = callback;
	file->priv->mount_operation_userdata = user_data;
	file->priv->mount_operation_notify = notify;
}

GMountOperation *
_tepl_file_create_mount_operation (TeplFile *file)
{
	if (file == NULL)
	{
		goto fallback;
	}

	g_return_val_if_fail (TEPL_IS_FILE (file), NULL);

	if (file->priv->mount_operation_factory != NULL)
	{
		return file->priv->mount_operation_factory (file, file->priv->mount_operation_userdata);
	}

fallback:
	return g_mount_operation_new ();
}

/* Notify @file that its location has been mounted. */
void
_tepl_file_set_mounted (TeplFile *file)
{
	g_return_if_fail (TEPL_IS_FILE (file));

	/* Querying the display-name should work now. */
	update_short_name (file);
}

const gchar *
_tepl_file_get_etag (TeplFile *file)
{
	if (file == NULL)
	{
		return NULL;
	}

	g_return_val_if_fail (TEPL_IS_FILE (file), NULL);

	return file->priv->etag;
}

void
_tepl_file_set_etag (TeplFile    *file,
		     const gchar *etag)
{
	if (file == NULL)
	{
		return;
	}

	g_return_if_fail (TEPL_IS_FILE (file));

	g_free (file->priv->etag);
	file->priv->etag = g_strdup (etag);
}

/**
 * tepl_file_add_uri_to_recent_manager:
 * @file: a #TeplFile.
 *
 * If the #TeplFile:location isn't %NULL, adds its URI to the default
 * #GtkRecentManager with gtk_recent_manager_add_item().
 *
 * Since: 4.0
 */

/* In the future a vfunc could be added for this function if it is desirable to
 * customize it in an application.
 */
void
tepl_file_add_uri_to_recent_manager (TeplFile *file)
{
	GtkRecentManager *recent_manager;
	gchar *uri;

	g_return_if_fail (TEPL_IS_FILE (file));

	if (file->priv->location == NULL)
	{
		return;
	}

	recent_manager = gtk_recent_manager_get_default ();

	uri = g_file_get_uri (file->priv->location);
	gtk_recent_manager_add_item (recent_manager, uri);
	g_free (uri);
}
