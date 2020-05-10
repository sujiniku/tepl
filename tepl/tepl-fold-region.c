/* SPDX-FileCopyrightText: 2016 - David Rabel <david.rabel@noresoft.com>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "tepl-fold-region.h"

/**
 * SECTION:fold-region
 * @Short_description: Foldable region in a #GtkTextBuffer
 * @Title: TeplFoldRegion
 *
 * #TeplFoldRegion represents a region in a #GtkTextBuffer that can be folded.
 *
 * When a region is being folded, a #GtkTextTag with the #GtkTextTag:invisible
 * property is applied to the folded region. The actual start and end position
 * of this #GtkTextTag is respectively at the next new line after the start and
 * end position of the bounds handed over to tepl_fold_region_set_bounds().
 */

enum
{
	PROP_0,
	PROP_BUFFER,
	PROP_FOLDED,
	N_PROPERTIES
};

typedef struct _TeplFoldRegionPrivate TeplFoldRegionPrivate;

struct _TeplFoldRegionPrivate
{
	GtkTextBuffer *buffer;

	/* A reference to the tag table where the tag is added. The sole
	 * purpose is to remove the tag in dispose(). We can not rely on
	 * 'buffer' since it is a weak reference.
	 */
	GtkTextTagTable *tag_table;
	GtkTextTag *tag;

	GtkTextMark *start_mark;
	GtkTextMark *end_mark;
};

static GParamSpec *properties[N_PROPERTIES];

G_DEFINE_TYPE_WITH_PRIVATE (TeplFoldRegion, tepl_fold_region, G_TYPE_OBJECT)

static void
apply_tag (TeplFoldRegion *fold_region)
{
	TeplFoldRegionPrivate *priv = tepl_fold_region_get_instance_private (fold_region);
	GtkTextIter start_iter;
	GtkTextIter end_iter;

	g_assert (priv->tag == NULL);
	g_assert (priv->tag_table == NULL);
	g_assert (priv->start_mark != NULL);
	g_assert (priv->end_mark != NULL);
	g_assert (priv->buffer != NULL);

	priv->tag = gtk_text_buffer_create_tag (priv->buffer,
						NULL,
						"invisible", TRUE,
						NULL);

	priv->tag_table = gtk_text_buffer_get_tag_table (priv->buffer);

	g_object_ref (priv->tag);
	g_object_ref (priv->tag_table);

	gtk_text_buffer_get_iter_at_mark (priv->buffer,
					  &start_iter,
					  priv->start_mark);

	gtk_text_buffer_get_iter_at_mark (priv->buffer,
					  &end_iter,
					  priv->end_mark);

	gtk_text_iter_forward_line (&start_iter);
	gtk_text_iter_forward_line (&end_iter);

	gtk_text_buffer_apply_tag (priv->buffer,
				   priv->tag,
				   &start_iter,
				   &end_iter);
}

static void
destroy_tag (TeplFoldRegion *fold_region)
{
	TeplFoldRegionPrivate *priv = tepl_fold_region_get_instance_private (fold_region);

	gtk_text_tag_table_remove (priv->tag_table, priv->tag);

	g_clear_object (&priv->tag);
	g_clear_object (&priv->tag_table);
}

static void
tepl_fold_region_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
	TeplFoldRegion *fold_region = TEPL_FOLD_REGION (object);

	switch (prop_id)
	{
		case PROP_BUFFER:
			g_value_set_object (value, tepl_fold_region_get_buffer (fold_region));
			break;

		case PROP_FOLDED:
			g_value_set_boolean (value, tepl_fold_region_get_folded (fold_region));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
tepl_fold_region_set_property (GObject       *object,
                               guint          prop_id,
                               const GValue  *value,
                               GParamSpec    *pspec)
{
	TeplFoldRegion *fold_region = TEPL_FOLD_REGION (object);
	TeplFoldRegionPrivate *priv = tepl_fold_region_get_instance_private (fold_region);

	switch (prop_id)
	{
		case PROP_BUFFER:
			g_assert (priv->buffer == NULL);
			g_set_weak_pointer (&priv->buffer, g_value_get_object (value));
			break;

		case PROP_FOLDED:
			tepl_fold_region_set_folded (fold_region, g_value_get_boolean (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
tepl_fold_region_dispose (GObject *object)
{
	TeplFoldRegion *fold_region = TEPL_FOLD_REGION (object);
	TeplFoldRegionPrivate *priv = tepl_fold_region_get_instance_private (fold_region);

	if (priv->tag != NULL &&
	    priv->tag_table != NULL)
	{
		gtk_text_tag_table_remove (priv->tag_table, priv->tag);

		g_clear_object (&priv->tag);
		g_clear_object (&priv->tag_table);
	}

	if (priv->buffer != NULL)
	{
		if (priv->start_mark != NULL)
		{
			gtk_text_buffer_delete_mark (priv->buffer, priv->start_mark);
			priv->start_mark = NULL;
		}
		if (priv->end_mark != NULL)
		{
			gtk_text_buffer_delete_mark (priv->buffer, priv->end_mark);
			priv->end_mark = NULL;
		}

		g_clear_weak_pointer (&priv->buffer);
	}

	priv->start_mark = NULL;
	priv->end_mark = NULL;

	G_OBJECT_CLASS (tepl_fold_region_parent_class)->dispose (object);
}

static void
tepl_fold_region_class_init (TeplFoldRegionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = tepl_fold_region_get_property;
	object_class->set_property = tepl_fold_region_set_property;
	object_class->dispose = tepl_fold_region_dispose;

	/**
	 * TeplFoldRegion:buffer:
	 *
	 * The #GtkTextBuffer where the fold region is applied. The
	 * #TeplFoldRegion object has a weak reference to the buffer.
	 *
	 * Since: 1.0
	 */
	properties[PROP_BUFFER] =
		g_param_spec_object ("buffer",
				     "Text Buffer",
				     "",
				     GTK_TYPE_TEXT_BUFFER,
				     G_PARAM_READWRITE |
				     G_PARAM_CONSTRUCT_ONLY |
				     G_PARAM_STATIC_STRINGS);

	/**
	 * TeplFoldRegion:folded:
	 *
	 * Whether the #TeplFoldRegion is folded or not.
	 *
	 * Since: 1.0
	 */
	properties[PROP_FOLDED] =
		g_param_spec_boolean ("folded",
				      "Folded",
				      "",
				      FALSE,
				      G_PARAM_READWRITE |
				      G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (object_class, N_PROPERTIES, properties);
}

static void
tepl_fold_region_init (TeplFoldRegion *fold_region)
{
}

/**
 * tepl_fold_region_new:
 * @buffer: a #GtkTextBuffer.
 * @start: a #GtkTextIter.
 * @end: a #GtkTextIter.
 *
 * Returns: a new #TeplFoldRegion.
 * Since: 1.0
 */
TeplFoldRegion *
tepl_fold_region_new (GtkTextBuffer     *buffer,
                      const GtkTextIter *start,
                      const GtkTextIter *end)
{
	TeplFoldRegion *fold_region;

	g_return_val_if_fail (GTK_IS_TEXT_BUFFER (buffer), NULL);
	g_return_val_if_fail (start != NULL, NULL);
	g_return_val_if_fail (end != NULL, NULL);

	fold_region = g_object_new (TEPL_TYPE_FOLD_REGION,
				    "buffer", buffer,
				    NULL);

	tepl_fold_region_set_bounds (fold_region, start, end);

	return fold_region;
}

/**
 * tepl_fold_region_get_buffer:
 * @fold_region: a #TeplFoldRegion.
 *
 * Returns: (transfer none) (nullable): the #GtkTextBuffer where the fold region
 *   is applied.
 * Since: 1.0
 */
GtkTextBuffer *
tepl_fold_region_get_buffer (TeplFoldRegion *fold_region)
{
	TeplFoldRegionPrivate *priv;

	g_return_val_if_fail (TEPL_IS_FOLD_REGION (fold_region), NULL);

	priv = tepl_fold_region_get_instance_private (fold_region);

	return priv->buffer;
}

/**
 * tepl_fold_region_get_folded:
 * @fold_region: a #TeplFoldRegion.
 *
 * Returns: whether the #TeplFoldRegion is folded.
 * Since: 1.0
 */
gboolean
tepl_fold_region_get_folded (TeplFoldRegion *fold_region)
{
	TeplFoldRegionPrivate *priv;

	g_return_val_if_fail (TEPL_IS_FOLD_REGION (fold_region), FALSE);

	priv = tepl_fold_region_get_instance_private (fold_region);

	return priv->tag != NULL;
}

/**
 * tepl_fold_region_set_folded:
 * @fold_region: a #TeplFoldRegion.
 * @folded: the new value.
 *
 * Folds or unfolds the region.
 *
 * Since: 1.0
 */
void
tepl_fold_region_set_folded (TeplFoldRegion *fold_region,
			     gboolean        folded)
{
	TeplFoldRegionPrivate *priv;

	g_return_if_fail (TEPL_IS_FOLD_REGION (fold_region));

	priv = tepl_fold_region_get_instance_private (fold_region);

	if (priv->buffer == NULL)
	{
		return;
	}

	if (priv->start_mark == NULL || priv->end_mark == NULL)
	{
		return;
	}

	folded = folded != FALSE;

	if (folded == tepl_fold_region_get_folded (fold_region))
	{
		return;
	}

	if (folded)
	{
		apply_tag (fold_region);
	}
	else
	{
		destroy_tag (fold_region);
	}

	g_object_notify_by_pspec (G_OBJECT (fold_region), properties[PROP_FOLDED]);
}

/**
 * tepl_fold_region_get_bounds:
 * @fold_region: a #TeplFoldRegion.
 * @start: (out): iterator to initialize.
 * @end: (out): iterator to initialize.
 *
 * Obtains iterators pointing to the start and end of the #TeplFoldRegion.
 *
 * Returns: %TRUE on success, %FALSE otherwise.
 * Since: 1.0
 */
gboolean
tepl_fold_region_get_bounds (TeplFoldRegion *fold_region,
			     GtkTextIter    *start,
			     GtkTextIter    *end)
{
	TeplFoldRegionPrivate *priv;

	g_return_val_if_fail (TEPL_IS_FOLD_REGION (fold_region), FALSE);
	g_return_val_if_fail (start != NULL, FALSE);
	g_return_val_if_fail (end != NULL, FALSE);

	priv = tepl_fold_region_get_instance_private (fold_region);

	if (priv->buffer == NULL)
	{
		return FALSE;
	}

	if (priv->start_mark == NULL || priv->end_mark == NULL)
	{
		return FALSE;
	}

	gtk_text_buffer_get_iter_at_mark (priv->buffer,
					  start,
					  priv->start_mark);

	gtk_text_buffer_get_iter_at_mark (priv->buffer,
					  end,
					  priv->end_mark);
	return TRUE;
}

/**
 * tepl_fold_region_set_bounds:
 * @fold_region: a #TeplFoldRegion.
 * @start: a #GtkTextIter.
 * @end: a #GtkTextIter.
 *
 * Sets the start and end of the #TeplFoldRegion.
 *
 * Since: 1.0
 */
void
tepl_fold_region_set_bounds (TeplFoldRegion    *fold_region,
			     const GtkTextIter *start,
			     const GtkTextIter *end)
{
	TeplFoldRegionPrivate *priv;

	g_return_if_fail (TEPL_IS_FOLD_REGION (fold_region));
	g_return_if_fail (start != NULL);
	g_return_if_fail (end != NULL);
	g_return_if_fail (gtk_text_iter_get_line (start) < gtk_text_iter_get_line (end));

	priv = tepl_fold_region_get_instance_private (fold_region);

	if (priv->buffer == NULL)
	{
		return;
	}

	if (priv->start_mark != NULL)
	{
		gtk_text_buffer_move_mark (priv->buffer, priv->start_mark, start);
	}
	else
	{
		priv->start_mark = gtk_text_buffer_create_mark (priv->buffer, NULL, start, TRUE);
	}

	if (priv->end_mark != NULL)
	{
		gtk_text_buffer_move_mark (priv->buffer, priv->end_mark, end);
	}
	else
	{
		priv->end_mark = gtk_text_buffer_create_mark (priv->buffer, NULL, end, FALSE);
	}

	if (priv->tag != NULL &&
	    priv->tag_table != NULL)
	{
		destroy_tag (fold_region);
		apply_tag (fold_region);
	}
}
