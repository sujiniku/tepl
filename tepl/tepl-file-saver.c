/* SPDX-FileCopyrightText: 2005-2007 - Paolo Borelli and Paolo Maggi
 * SPDX-FileCopyrightText: 2007 - Steve Frécinaux
 * SPDX-FileCopyrightText: 2008 - Jesse van den Kieboom
 * SPDX-FileCopyrightText: 2014-2020 - Sébastien Wilmet
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "tepl-file-saver.h"
#include <string.h>
#include "tepl-enum-types.h"

/**
 * SECTION:file-saver
 * @Title: TeplFileSaver
 * @Short_description: Save a #TeplBuffer into a file
 * @See_also: #TeplFile, #TeplFileLoader
 *
 * A #TeplFileSaver object permits to save a #TeplBuffer into a #GFile.
 *
 * A file saver should be used only for one save operation, including errors
 * handling. If an error occurs, you can reconfigure the saver and relaunch the
 * operation with tepl_file_saver_save_async().
 *
 * # Things that need to be done at a higher level
 *
 * Make the #GtkTextView non-editable during the save operation. See
 * gtk_text_view_set_editable(). Because the save operation is asynchronous, and
 * gtk_text_buffer_set_modified() must be called (with a %FALSE value) only when
 * the file has been successfully saved. So that's why
 * gtk_text_buffer_set_modified() is called by tepl_file_saver_save_finish(),
 * not at the beginning of the save operation. If the view/buffer is editable
 * during the save operation, gtk_text_buffer_set_modified() may be called at
 * the wrong place in the undo/redo history.
 */

enum
{
	PROP_0,
	PROP_BUFFER,
	PROP_FILE,
	PROP_LOCATION,
	PROP_NEWLINE_TYPE,
	PROP_FLAGS,
	N_PROPERTIES
};

struct _TeplFileSaverPrivate
{
	/* Weak ref to the TeplBuffer. A strong ref could create a reference
	 * cycle in an application. For example a subclass of TeplBuffer can
	 * have a strong ref to the FileSaver.
	 */
	TeplBuffer *buffer;

	/* Weak ref to the TeplFile. A strong ref could create a reference
	 * cycle in an application. For example a subclass of TeplFile can
	 * have a strong ref to the FileSaver.
	 */
	TeplFile *file;

	GFile *location;

	TeplNewlineType newline_type;
	TeplFileSaverFlags flags;

	guint is_saving : 1;
};

typedef struct _TaskData TaskData;
struct _TaskData
{
	gchar *buffer_content;
};

static GParamSpec *properties[N_PROPERTIES];

G_DEFINE_TYPE_WITH_PRIVATE (TeplFileSaver, tepl_file_saver, G_TYPE_OBJECT)

static TaskData *
task_data_new (void)
{
	return g_new0 (TaskData, 1);
}

static void
task_data_free (TaskData *data)
{
	if (data != NULL)
	{
		g_free (data->buffer_content);
		g_free (data);
	}
}

static void
tepl_file_saver_set_property (GObject      *object,
			      guint         prop_id,
			      const GValue *value,
			      GParamSpec   *pspec)
{
	TeplFileSaver *saver = TEPL_FILE_SAVER (object);

	switch (prop_id)
	{
		case PROP_BUFFER:
			g_assert (saver->priv->buffer == NULL);
			g_set_weak_pointer (&saver->priv->buffer, g_value_get_object (value));
			break;

		case PROP_FILE:
			g_assert (saver->priv->file == NULL);
			g_set_weak_pointer (&saver->priv->file, g_value_get_object (value));
			break;

		case PROP_LOCATION:
			g_assert (saver->priv->location == NULL);
			saver->priv->location = g_value_dup_object (value);
			break;

		case PROP_NEWLINE_TYPE:
			tepl_file_saver_set_newline_type (saver, g_value_get_enum (value));
			break;

		case PROP_FLAGS:
			tepl_file_saver_set_flags (saver, g_value_get_flags (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
tepl_file_saver_get_property (GObject    *object,
			      guint       prop_id,
			      GValue     *value,
			      GParamSpec *pspec)
{
	TeplFileSaver *saver = TEPL_FILE_SAVER (object);

	switch (prop_id)
	{
		case PROP_BUFFER:
			g_value_set_object (value, saver->priv->buffer);
			break;

		case PROP_FILE:
			g_value_set_object (value, saver->priv->file);
			break;

		case PROP_LOCATION:
			g_value_set_object (value, saver->priv->location);
			break;

		case PROP_NEWLINE_TYPE:
			g_value_set_enum (value, saver->priv->newline_type);
			break;

		case PROP_FLAGS:
			g_value_set_flags (value, saver->priv->flags);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
tepl_file_saver_constructed (GObject *object)
{
	TeplFileSaver *saver = TEPL_FILE_SAVER (object);

	if (saver->priv->file != NULL)
	{
		TeplNewlineType newline_type;

		newline_type = tepl_file_get_newline_type (saver->priv->file);
		tepl_file_saver_set_newline_type (saver, newline_type);

		if (saver->priv->location == NULL)
		{
			saver->priv->location = tepl_file_get_location (saver->priv->file);

			if (saver->priv->location != NULL)
			{
				g_object_ref (saver->priv->location);
			}
			else
			{
				g_warning ("TeplFileSaver: the TeplFile's location is NULL. "
					   "Use tepl_file_saver_new_with_target().");
			}
		}
	}

	G_OBJECT_CLASS (tepl_file_saver_parent_class)->constructed (object);
}

static void
tepl_file_saver_dispose (GObject *object)
{
	TeplFileSaver *saver = TEPL_FILE_SAVER (object);

	g_clear_weak_pointer (&saver->priv->buffer);
	g_clear_weak_pointer (&saver->priv->file);
	g_clear_object (&saver->priv->location);

	G_OBJECT_CLASS (tepl_file_saver_parent_class)->dispose (object);
}

static void
tepl_file_saver_class_init (TeplFileSaverClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->set_property = tepl_file_saver_set_property;
	object_class->get_property = tepl_file_saver_get_property;
	object_class->constructed = tepl_file_saver_constructed;
	object_class->dispose = tepl_file_saver_dispose;

	/**
	 * TeplFileSaver:buffer:
	 *
	 * The #TeplBuffer to save. The #TeplFileSaver object has a weak
	 * reference to the buffer.
	 *
	 * Since: 1.0
	 */
	properties[PROP_BUFFER] =
		g_param_spec_object ("buffer",
				     "buffer",
				     "",
				     TEPL_TYPE_BUFFER,
				     G_PARAM_READWRITE |
				     G_PARAM_CONSTRUCT_ONLY |
				     G_PARAM_STATIC_STRINGS);

	/**
	 * TeplFileSaver:file:
	 *
	 * The #TeplFile. The #TeplFileSaver object has a weak reference to the
	 * file.
	 *
	 * Since: 1.0
	 */
	properties[PROP_FILE] =
		g_param_spec_object ("file",
				     "file",
				     "",
				     TEPL_TYPE_FILE,
				     G_PARAM_READWRITE |
				     G_PARAM_CONSTRUCT_ONLY |
				     G_PARAM_STATIC_STRINGS);

	/**
	 * TeplFileSaver:location:
	 *
	 * The #GFile where to save the buffer. By default the location is taken
	 * from the #TeplFile at construction time.
	 *
	 * Since: 1.0
	 */
	properties[PROP_LOCATION] =
		g_param_spec_object ("location",
				     "location",
				     "",
				     G_TYPE_FILE,
				     G_PARAM_READWRITE |
				     G_PARAM_CONSTRUCT_ONLY |
				     G_PARAM_STATIC_STRINGS);

	/**
	 * TeplFileSaver:newline-type:
	 *
	 * The newline type.
	 *
	 * Since: 1.0
	 */
	properties[PROP_NEWLINE_TYPE] =
		g_param_spec_enum ("newline-type",
				   "newline-type",
				   "",
				   TEPL_TYPE_NEWLINE_TYPE,
				   TEPL_NEWLINE_TYPE_DEFAULT,
				   G_PARAM_READWRITE |
				   G_PARAM_CONSTRUCT |
				   G_PARAM_STATIC_STRINGS);

	/**
	 * TeplFileSaver:flags:
	 *
	 * File saving flags.
	 *
	 * Since: 1.0
	 */
	properties[PROP_FLAGS] =
		g_param_spec_flags ("flags",
				    "flags",
				    "",
				    TEPL_TYPE_FILE_SAVER_FLAGS,
				    TEPL_FILE_SAVER_FLAGS_NONE,
				    G_PARAM_READWRITE |
				    G_PARAM_CONSTRUCT |
				    G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (object_class, N_PROPERTIES, properties);
}

static void
tepl_file_saver_init (TeplFileSaver *saver)
{
	saver->priv = tepl_file_saver_get_instance_private (saver);

	saver->priv->newline_type = TEPL_NEWLINE_TYPE_DEFAULT;
	saver->priv->flags = TEPL_FILE_SAVER_FLAGS_NONE;
}

/**
 * tepl_file_saver_new:
 * @buffer: the #TeplBuffer to save.
 * @file: the #TeplFile.
 *
 * Creates a new #TeplFileSaver object. The @buffer will be saved to the
 * #TeplFile's location.
 *
 * This constructor is suitable for a simple "save" operation, when the @file
 * already contains a non-%NULL #TeplFile:location.
 *
 * Returns: a new #TeplFileSaver object.
 * Since: 1.0
 */
TeplFileSaver *
tepl_file_saver_new (TeplBuffer *buffer,
		     TeplFile   *file)
{
	g_return_val_if_fail (TEPL_IS_BUFFER (buffer), NULL);
	g_return_val_if_fail (TEPL_IS_FILE (file), NULL);

	return g_object_new (TEPL_TYPE_FILE_SAVER,
			     "buffer", buffer,
			     "file", file,
			     NULL);
}

/**
 * tepl_file_saver_new_with_target:
 * @buffer: the #TeplBuffer to save.
 * @file: the #TeplFile.
 * @target_location: the #GFile where to save the buffer to.
 *
 * Creates a new #TeplFileSaver object with a target location. When the
 * file saving is finished successfully, @target_location is set to the @file's
 * #TeplFile:location property. If an error occurs, the previous valid
 * location is still available in #TeplFile.
 *
 * This constructor is suitable for a "save as" operation, or for saving a new
 * buffer for the first time.
 *
 * Returns: a new #TeplFileSaver object.
 * Since: 1.0
 */
TeplFileSaver *
tepl_file_saver_new_with_target (TeplBuffer *buffer,
				 TeplFile   *file,
				 GFile      *target_location)
{
	g_return_val_if_fail (TEPL_IS_BUFFER (buffer), NULL);
	g_return_val_if_fail (TEPL_IS_FILE (file), NULL);
	g_return_val_if_fail (G_IS_FILE (target_location), NULL);

	return g_object_new (TEPL_TYPE_FILE_SAVER,
			     "buffer", buffer,
			     "file", file,
			     "location", target_location,
			     NULL);
}

/**
 * tepl_file_saver_get_buffer:
 * @saver: a #TeplFileSaver.
 *
 * Returns: (transfer none): the #TeplBuffer to save.
 * Since: 1.0
 */
TeplBuffer *
tepl_file_saver_get_buffer (TeplFileSaver *saver)
{
	g_return_val_if_fail (TEPL_IS_FILE_SAVER (saver), NULL);

	return saver->priv->buffer;
}

/**
 * tepl_file_saver_get_file:
 * @saver: a #TeplFileSaver.
 *
 * Returns: (transfer none): the #TeplFile.
 * Since: 1.0
 */
TeplFile *
tepl_file_saver_get_file (TeplFileSaver *saver)
{
	g_return_val_if_fail (TEPL_IS_FILE_SAVER (saver), NULL);

	return saver->priv->file;
}

/**
 * tepl_file_saver_get_location:
 * @saver: a #TeplFileSaver.
 *
 * Returns: (transfer none): the #GFile where to save the buffer to.
 * Since: 1.0
 */
GFile *
tepl_file_saver_get_location (TeplFileSaver *saver)
{
	g_return_val_if_fail (TEPL_IS_FILE_SAVER (saver), NULL);

	return saver->priv->location;
}

/**
 * tepl_file_saver_set_newline_type:
 * @saver: a #TeplFileSaver.
 * @newline_type: the new newline type.
 *
 * Sets the newline type. By default the newline type is taken from the
 * #TeplFile.
 *
 * Since: 1.0
 */
void
tepl_file_saver_set_newline_type (TeplFileSaver   *saver,
				  TeplNewlineType  newline_type)
{
	g_return_if_fail (TEPL_IS_FILE_SAVER (saver));
	g_return_if_fail (!saver->priv->is_saving);

	if (saver->priv->newline_type != newline_type)
	{
		saver->priv->newline_type = newline_type;
		g_object_notify_by_pspec (G_OBJECT (saver), properties[PROP_NEWLINE_TYPE]);
	}
}

/**
 * tepl_file_saver_get_newline_type:
 * @saver: a #TeplFileSaver.
 *
 * Returns: the newline type.
 * Since: 1.0
 */
TeplNewlineType
tepl_file_saver_get_newline_type (TeplFileSaver *saver)
{
	g_return_val_if_fail (TEPL_IS_FILE_SAVER (saver), TEPL_NEWLINE_TYPE_DEFAULT);

	return saver->priv->newline_type;
}

/**
 * tepl_file_saver_set_flags:
 * @saver: a #TeplFileSaver.
 * @flags: the new flags.
 *
 * Since: 1.0
 */
void
tepl_file_saver_set_flags (TeplFileSaver      *saver,
			   TeplFileSaverFlags  flags)
{
	g_return_if_fail (TEPL_IS_FILE_SAVER (saver));
	g_return_if_fail (!saver->priv->is_saving);

	if (saver->priv->flags != flags)
	{
		saver->priv->flags = flags;
		g_object_notify_by_pspec (G_OBJECT (saver), properties[PROP_FLAGS]);
	}
}

/**
 * tepl_file_saver_get_flags:
 * @saver: a #TeplFileSaver.
 *
 * Returns: the flags.
 * Since: 1.0
 */
TeplFileSaverFlags
tepl_file_saver_get_flags (TeplFileSaver *saver)
{
	g_return_val_if_fail (TEPL_IS_FILE_SAVER (saver), TEPL_FILE_SAVER_FLAGS_NONE);

	return saver->priv->flags;
}

static void
get_all_buffer_content (GTask *task)
{
	TeplFileSaver *saver = g_task_get_source_object (task);
	TaskData *task_data = g_task_get_task_data (task);
	GtkTextIter start;
	GtkTextIter end;

	gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER (saver->priv->buffer), &start, &end);

	g_free (task_data->buffer_content);
	task_data->buffer_content = gtk_text_buffer_get_text (GTK_TEXT_BUFFER (saver->priv->buffer),
							      &start,
							      &end,
							      TRUE);
}

static void
save_all_buffer_content_cb (GObject      *source_object,
			    GAsyncResult *result,
			    gpointer      user_data)
{
	GFile *location = G_FILE (source_object);
	GTask *task = G_TASK (user_data);
	GError *error = NULL;

	g_file_replace_contents_finish (location, result, NULL, &error);

	if (error != NULL)
	{
		g_task_return_error (task, error);
		g_object_unref (task);
		return;
	}

	g_task_return_boolean (task, TRUE);
	g_object_unref (task);
}

static void
save_all_buffer_content (GTask *task)
{
	TeplFileSaver *saver = g_task_get_source_object (task);
	TaskData *task_data = g_task_get_task_data (task);
	gboolean make_backup;

	make_backup = (saver->priv->flags & TEPL_FILE_SAVER_FLAGS_CREATE_BACKUP) != 0;

	g_file_replace_contents_async (saver->priv->location,
				       task_data->buffer_content,
				       strlen (task_data->buffer_content),
				       NULL,
				       make_backup,
				       G_FILE_CREATE_NONE,
				       g_task_get_cancellable (task),
				       save_all_buffer_content_cb,
				       task);
}

/**
 * tepl_file_saver_save_async:
 * @saver: a #TeplFileSaver.
 * @io_priority: the I/O priority of the request. E.g. %G_PRIORITY_LOW,
 *   %G_PRIORITY_DEFAULT or %G_PRIORITY_HIGH.
 * @cancellable: (nullable): optional #GCancellable object, %NULL to ignore.
 * @callback: (scope async): a #GAsyncReadyCallback to call when the request is
 *   satisfied.
 * @user_data: user data to pass to @callback.
 *
 * Saves asynchronously the buffer into the file. See the #GAsyncResult
 * documentation to know how to use this function.
 *
 * Since: 5.0
 */
void
tepl_file_saver_save_async (TeplFileSaver       *saver,
			    gint                 io_priority,
			    GCancellable        *cancellable,
			    GAsyncReadyCallback  callback,
			    gpointer             user_data)
{
	GTask *task;
	TaskData *task_data;

	g_return_if_fail (TEPL_IS_FILE_SAVER (saver));
	g_return_if_fail (cancellable == NULL || G_IS_CANCELLABLE (cancellable));
	g_return_if_fail (!saver->priv->is_saving);

	saver->priv->is_saving = TRUE;

	task = g_task_new (saver, cancellable, callback, user_data);
	g_task_set_priority (task, io_priority);

	task_data = task_data_new ();
	g_task_set_task_data (task, task_data, (GDestroyNotify)task_data_free);

	if (saver->priv->buffer == NULL ||
	    saver->priv->file == NULL ||
	    saver->priv->location == NULL)
	{
		g_task_return_boolean (task, FALSE);
		g_object_unref (task);
		return;
	}

	get_all_buffer_content (task);
	save_all_buffer_content (task);
}

/**
 * tepl_file_saver_save_finish:
 * @saver: a #TeplFileSaver.
 * @result: a #GAsyncResult.
 * @error: a #GError, or %NULL.
 *
 * Finishes a file saving started with tepl_file_saver_save_async().
 *
 * If the file has been saved successfully, the following #TeplFile
 * properties will be updated: the location and the newline type.
 *
 * gtk_text_buffer_set_modified() is called with %FALSE if the file has been
 * saved successfully.
 *
 * Returns: whether the file was saved successfully.
 * Since: 1.0
 */
gboolean
tepl_file_saver_save_finish (TeplFileSaver  *saver,
			     GAsyncResult   *result,
			     GError        **error)
{
	gboolean ok;

	g_return_val_if_fail (TEPL_IS_FILE_SAVER (saver), FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
	g_return_val_if_fail (g_task_is_valid (result, saver), FALSE);

	ok = g_task_propagate_boolean (G_TASK (result), error);

	if (ok && saver->priv->file != NULL)
	{
		tepl_file_set_location (saver->priv->file,
					saver->priv->location);

		_tepl_file_set_newline_type (saver->priv->file,
					     saver->priv->newline_type);
	}

	if (ok && saver->priv->buffer != NULL)
	{
		gtk_text_buffer_set_modified (GTK_TEXT_BUFFER (saver->priv->buffer), FALSE);
	}

	saver->priv->is_saving = FALSE;
	return ok;
}
