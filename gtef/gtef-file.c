/*
 * This file is part of Gtef, a text editor library.
 *
 * Copyright 2016 - Sébastien Wilmet <swilmet@gnome.org>
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
#include "gtef-file.h"
#include <glib/gi18n-lib.h>
#include "gtef-file-metadata.h"

/**
 * SECTION:file
 * @Short_description: On-disk representation of a GtefBuffer
 * @Title: GtefFile
 *
 * #GtefFile extends #GtkSourceFile.
 */

typedef struct _GtefFilePrivate GtefFilePrivate;

struct _GtefFilePrivate
{
	GtefFileMetadata *metadata;

	gchar *short_name;
	gint untitled_number;
};

enum
{
	PROP_0,
	PROP_SHORT_NAME,
	N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES];

/* The list is sorted. */
static GSList *allocated_untitled_numbers;

G_DEFINE_TYPE_WITH_PRIVATE (GtefFile, gtef_file, GTK_SOURCE_TYPE_FILE)

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
gtef_file_get_property (GObject    *object,
			guint       prop_id,
			GValue     *value,
			GParamSpec *pspec)
{
	GtefFile *file = GTEF_FILE (object);

	switch (prop_id)
	{
		case PROP_SHORT_NAME:
			g_value_set_string (value, gtef_file_get_short_name (file));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gtef_file_set_property (GObject      *object,
			guint         prop_id,
			const GValue *value,
			GParamSpec   *pspec)
{
	switch (prop_id)
	{
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gtef_file_dispose (GObject *object)
{
	GtefFilePrivate *priv = gtef_file_get_instance_private (GTEF_FILE (object));

	g_clear_object (&priv->metadata);

	G_OBJECT_CLASS (gtef_file_parent_class)->dispose (object);
}

static void
gtef_file_finalize (GObject *object)
{
	GtefFilePrivate *priv = gtef_file_get_instance_private (GTEF_FILE (object));

	g_free (priv->short_name);

	if (priv->untitled_number > 0)
	{
		release_untitled_number (priv->untitled_number);
	}

	G_OBJECT_CLASS (gtef_file_parent_class)->finalize (object);
}

static void
gtef_file_class_init (GtefFileClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = gtef_file_get_property;
	object_class->set_property = gtef_file_set_property;
	object_class->dispose = gtef_file_dispose;
	object_class->finalize = gtef_file_finalize;

	/**
	 * GtefFile:short-name:
	 *
	 * The file short name. See gtef_file_get_short_name().
	 *
	 * Since: 1.0
	 */
	properties[PROP_SHORT_NAME] =
		g_param_spec_string ("short-name",
				     "Short Name",
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
	GtefFile *file = GTEF_FILE (user_data);
	GtefFilePrivate *priv;
	GFileInfo *info;
	GError *error = NULL;

	priv = gtef_file_get_instance_private (file);

	info = g_file_query_info_finish (location, result, &error);

	if (error != NULL)
	{
		/* TODO short-name fallback when the file doesn't exist. */
		if (!g_error_matches (error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND))
		{
			g_warning ("Error when querying file information: %s", error->message);
		}

		g_clear_error (&error);
		goto out;
	}

	g_free (priv->short_name);
	priv->short_name = g_strdup (g_file_info_get_display_name (info));

	if (priv->untitled_number > 0)
	{
		release_untitled_number (priv->untitled_number);
		priv->untitled_number = 0;
	}

	g_object_notify_by_pspec (G_OBJECT (file), properties[PROP_SHORT_NAME]);

out:
	g_clear_object (&info);

	/* Async operation finished */
	g_object_unref (file);
}

static void
update_short_name (GtefFile *file)
{
	GtefFilePrivate *priv;
	GFile *location;

	priv = gtef_file_get_instance_private (file);

	location = gtk_source_file_get_location (GTK_SOURCE_FILE (file));

	if (location == NULL)
	{
		if (priv->untitled_number == 0)
		{
			priv->untitled_number = allocate_first_available_untitled_number ();
		}

		g_free (priv->short_name);
		priv->short_name = g_strdup_printf (_("Untitled File %d"),
						    priv->untitled_number);

		g_object_notify_by_pspec (G_OBJECT (file), properties[PROP_SHORT_NAME]);
	}
	else
	{
		g_file_query_info_async (location,
					 G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME,
					 G_FILE_QUERY_INFO_NONE,
					 G_PRIORITY_DEFAULT,
					 NULL,
					 query_display_name_cb,
					 g_object_ref (file));
	}
}

static void
location_notify_cb (GtefFile   *file,
		    GParamSpec *pspec,
		    gpointer    user_data)
{
	update_short_name (file);
}

static void
gtef_file_init (GtefFile *file)
{
	GtefFilePrivate *priv = gtef_file_get_instance_private (file);

	priv->metadata = gtef_file_metadata_new (file);

	update_short_name (file);
	g_signal_connect (file,
			  "notify::location",
			  G_CALLBACK (location_notify_cb),
			  NULL);
}

/**
 * gtef_file_new:
 *
 * Returns: a new #GtefFile object.
 * Since: 1.0
 */
GtefFile *
gtef_file_new (void)
{
	return g_object_new (GTEF_TYPE_FILE, NULL);
}

/**
 * gtef_file_get_file_metadata:
 * @file: a #GtefFile.
 *
 * Returns: (transfer none): the associated #GtefFileMetadata.
 * Since: 1.0
 */
GtefFileMetadata *
gtef_file_get_file_metadata (GtefFile *file)
{
	GtefFilePrivate *priv;

	g_return_val_if_fail (GTEF_IS_FILE (file), NULL);

	priv = gtef_file_get_instance_private (file);
	return priv->metadata;
}

/**
 * gtef_file_get_short_name:
 * @file: a #GtefFile.
 *
 * Gets the @file short name. If the #GtkSourceFile:location isn't %NULL,
 * returns its display-name (see #G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME).
 * Otherwise returns "Untitled File N", with N the Nth untitled file of the
 * application, starting at 1. When an untitled file is closed, its number is
 * released and can be used by a later untitled file.
 *
 * Returns: the @file short name.
 * Since: 1.0
 */
const gchar *
gtef_file_get_short_name (GtefFile *file)
{
	GtefFilePrivate *priv;

	g_return_val_if_fail (GTEF_IS_FILE (file), NULL);

	priv = gtef_file_get_instance_private (file);
	return priv->short_name;
}
