/*
 * This file is part of Gtef, a text editor library.
 *
 * Copyright 2016 - David Rabel <david.rabel@noresoft.com>
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

#include "gtef-fold-region.h"

/**
 * SECTION:fold-region
 * @Short_description: Foldable region in a #GtkTextBuffer
 * @Title: GtefFoldRegion
 *
 * #GtefFoldRegion represents a region in a #GtkTextBuffer that can be folded.
 */

enum
{
	PROP_0,
	PROP_BUFFER,
	PROP_FOLDED,
	N_PROPERTIES
};

typedef struct _GtefFoldRegionPrivate GtefFoldRegionPrivate;

struct _GtefFoldRegionPrivate
{
	GtkTextBuffer *buffer;
	GtkTextTag *tag;
	GtkTextMark *start;
	GtkTextMark *end;

	/* FIXME: use tag != NULL to know if the region is folded.
	 * Here, the state is stored redundently, which is bad because there is
	 * a risk that they get out of sync.
	 */
	guint folded : 1;
};

static GParamSpec *properties[N_PROPERTIES];

G_DEFINE_TYPE_WITH_PRIVATE (GtefFoldRegion, gtef_fold_region, G_TYPE_OBJECT)

static void
gtef_fold_region_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
	GtefFoldRegion *fold_region = GTEF_FOLD_REGION (object);

	switch (prop_id)
	{
		case PROP_BUFFER:
			g_value_set_object (value, gtef_fold_region_get_buffer (fold_region));
			break;

		case PROP_FOLDED:
			g_value_set_boolean (value, gtef_fold_region_get_folded (fold_region));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gtef_fold_region_set_property (GObject       *object,
                               guint          prop_id,
                               const GValue  *value,
                               GParamSpec    *pspec)
{
	GtefFoldRegion *fold_region = GTEF_FOLD_REGION (object);
	GtefFoldRegionPrivate *priv = gtef_fold_region_get_instance_private (fold_region);

	switch (prop_id)
	{
		case PROP_BUFFER:
			g_assert (priv->buffer == NULL);
			priv->buffer = GTK_TEXT_BUFFER (g_value_get_object (value));
			g_object_add_weak_pointer (G_OBJECT (priv->buffer),
						   (gpointer *) &priv->buffer);
			break;

		case PROP_FOLDED:
			gtef_fold_region_set_folded (fold_region, g_value_get_boolean (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gtef_fold_region_dispose (GObject *object)
{
	GtefFoldRegion *fold_region = GTEF_FOLD_REGION (object);
	GtefFoldRegionPrivate *priv = gtef_fold_region_get_instance_private (fold_region);

	if (priv->buffer != NULL)
	{
		if (priv->start != NULL)
		{
			gtk_text_buffer_delete_mark (priv->buffer, priv->start);
			priv->start = NULL;
		}
		if (priv->end != NULL)
		{
			gtk_text_buffer_delete_mark (priv->buffer, priv->end);
			priv->end = NULL;
		}

		/* FIXME: remove the tag. */

		g_object_remove_weak_pointer (G_OBJECT (priv->buffer),
					      (gpointer *) &priv->buffer);
		priv->buffer = NULL;
	}

	priv->start = NULL;
	priv->end = NULL;

	G_OBJECT_CLASS (gtef_fold_region_parent_class)->dispose (object);
}

static void
gtef_fold_region_class_init (GtefFoldRegionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = gtef_fold_region_get_property;
	object_class->set_property = gtef_fold_region_set_property;
	object_class->dispose = gtef_fold_region_dispose;

	/**
	 * GtefFoldRegion:buffer:
	 *
	 * The #GtkTextBuffer where the fold region is applied. The
	 * #GtefFoldRegion object has a weak reference to the buffer.
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
	 * GtefFoldRegion:folded:
	 *
	 * Whether the #GtefFoldRegion is folded or not.
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
gtef_fold_region_init (GtefFoldRegion *fold_region)
{
}

/**
 * gtef_fold_region_new:
 * @buffer: a #GtkTextBuffer.
 * @start: a #GtkTextIter.
 * @end: a #GtkTextIter.
 *
 * Returns: a new #GtefFoldRegion.
 * Since: 1.0
 */
GtefFoldRegion *
gtef_fold_region_new (GtkTextBuffer     *buffer,
                      const GtkTextIter *start,
                      const GtkTextIter *end)
{
	GtefFoldRegion * fold_region;

	g_return_val_if_fail (GTK_IS_TEXT_BUFFER (buffer), NULL);
	g_return_val_if_fail (start != NULL, NULL);
	g_return_val_if_fail (end != NULL, NULL);

	fold_region = g_object_new (GTEF_TYPE_FOLD_REGION,
				    "buffer", buffer,
				    NULL);

	gtef_fold_region_set_bounds (fold_region, start, end);

	return fold_region;
}

/**
 * gtef_fold_region_get_buffer:
 * @fold_region: a #GtefFoldRegion.
 *
 * Returns: (transfer none) (nullable): the #GtkTextBuffer where the fold region
 *   is applied.
 * Since: 1.0
 */
GtkTextBuffer *
gtef_fold_region_get_buffer (GtefFoldRegion *fold_region)
{
	GtefFoldRegionPrivate *priv;

	g_return_val_if_fail (GTEF_IS_FOLD_REGION (fold_region), NULL);

	priv = gtef_fold_region_get_instance_private (fold_region);

	return priv->buffer;
}

/**
 * gtef_fold_region_get_folded:
 * @fold_region: a #GtefFoldRegion.
 *
 * Returns: whether the #GtefFoldRegion is folded.
 * Since: 1.0
 */
gboolean
gtef_fold_region_get_folded (GtefFoldRegion *fold_region)
{
	GtefFoldRegionPrivate *priv;

	g_return_val_if_fail (GTEF_IS_FOLD_REGION (fold_region), FALSE);

	priv = gtef_fold_region_get_instance_private (fold_region);

	return priv->folded;
}

/**
 * gtef_fold_region_set_folded:
 * @fold_region: a #GtefFoldRegion.
 * @folded: the new value.
 *
 * Folds or unfolds the region.
 *
 * Since: 1.0
 */
void
gtef_fold_region_set_folded (GtefFoldRegion *fold_region,
			     gboolean        folded)
{
	GtefFoldRegionPrivate *priv;

	g_return_if_fail (GTEF_IS_FOLD_REGION (fold_region));

	priv = gtef_fold_region_get_instance_private (fold_region);

	folded = folded != FALSE;

	if (priv->folded == folded)
	{
		return;
	}

	priv->folded = folded;

	if (priv->buffer != NULL)
	{
		GtkTextTagTable *tag_table;
		GtkTextIter start;
		GtkTextIter end;

		gtk_text_buffer_get_iter_at_mark (priv->buffer,
						  &start,
						  priv->start);

		gtk_text_buffer_get_iter_at_mark (priv->buffer,
						  &end,
						  priv->end);

		gtk_text_iter_forward_line (&start);
		gtk_text_iter_forward_line (&end);

		if (folded)
		{
			/* FIXME: own a ref to the tag. */
			priv->tag = gtk_text_buffer_create_tag (priv->buffer,
								NULL,
								"invisible", TRUE,
								NULL);

			gtk_text_buffer_apply_tag (priv->buffer,
						   priv->tag,
						   &start,
						   &end);
		}
		else
		{
			tag_table = gtk_text_buffer_get_tag_table (priv->buffer);
			gtk_text_tag_table_remove (tag_table, priv->tag);

			/* FIXME unref the tag and set to NULL */
		}
	}

	g_object_notify_by_pspec (G_OBJECT (fold_region), properties[PROP_FOLDED]);
}

/**
 * gtef_fold_region_get_bounds:
 * @fold_region: a #GtefFoldRegion.
 * @start: (out): iterator to initialize.
 * @end: (out): iterator to initialize.
 *
 * Obtains iterators pointing to the start and end of the #GtefFoldRegion.
 *
 * Returns: %TRUE on success, %FALSE otherwise.
 * Since: 1.0
 */
gboolean
gtef_fold_region_get_bounds (GtefFoldRegion *fold_region,
			     GtkTextIter    *start,
			     GtkTextIter    *end)
{
	GtefFoldRegionPrivate *priv;

	g_return_val_if_fail (GTEF_IS_FOLD_REGION (fold_region), FALSE);
	g_return_val_if_fail (start != NULL, FALSE);
	g_return_val_if_fail (end != NULL, FALSE);

	priv = gtef_fold_region_get_instance_private (fold_region);

	if (priv->buffer != NULL)
	{
		gtk_text_buffer_get_iter_at_mark (priv->buffer,
						  start,
						  priv->start);

		gtk_text_buffer_get_iter_at_mark (priv->buffer,
						  end,
						  priv->end);
		return TRUE;
	}

	return FALSE;
}

/**
 * gtef_fold_region_set_bounds:
 * @fold_region: a #GtefFoldRegion.
 * @start: a #GtkTextIter.
 * @end: a #GtkTextIter.
 *
 * Sets the start and end of the #GtefFoldRegion.
 *
 * Since: 1.0
 */
void
gtef_fold_region_set_bounds (GtefFoldRegion    *fold_region,
			     const GtkTextIter *start,
			     const GtkTextIter *end)
{
	GtefFoldRegionPrivate *priv;
	GtkTextIter start_iter;
	GtkTextIter end_iter;

	g_return_if_fail (GTEF_IS_FOLD_REGION (fold_region));
	g_return_if_fail (start != NULL);
	g_return_if_fail (end != NULL);
	g_return_if_fail (gtk_text_iter_get_line (start) < gtk_text_iter_get_line (end));

	priv = gtef_fold_region_get_instance_private (fold_region);

	if (priv->buffer == NULL)
	{
		return;
	}

	/* FIXME: use start and end directly. */
	start_iter = *start;
	end_iter = *end;

	if (priv->start != NULL)
	{
		gtk_text_buffer_move_mark (priv->buffer, priv->start, &start_iter);
	}
	else
	{
		priv->start = gtk_text_mark_new (NULL, FALSE);
		gtk_text_buffer_add_mark (priv->buffer, priv->start, &start_iter);
	}

	if (priv->end != NULL)
	{
		gtk_text_buffer_move_mark (priv->buffer, priv->end, &end_iter);
	}
	else
	{
		priv->end = gtk_text_mark_new (NULL, TRUE);
		gtk_text_buffer_add_mark (priv->buffer, priv->end, &end_iter);
	}

	/* FIXME: update tag if applied. */
}
