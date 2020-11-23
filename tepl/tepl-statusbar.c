/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "config.h"
#include "tepl-statusbar.h"
#include <glib/gi18n-lib.h>
#include "tepl-signal-group.h"

/**
 * SECTION:statusbar
 * @Title: TeplStatusbar
 * @Short_description: Subclass of #GtkStatusbar
 *
 * #TeplStatusbar is a subclass of #GtkStatusbar with a better look by default,
 * and with added functions useful for a text editor.
 */

struct _TeplStatusbarPrivate
{
	GtkLabel *label;
	TeplTabGroup *tab_group;
	TeplSignalGroup *buffer_signal_group;
};

G_DEFINE_TYPE_WITH_PRIVATE (TeplStatusbar, tepl_statusbar, GTK_TYPE_STATUSBAR)

static void
tepl_statusbar_dispose (GObject *object)
{
	TeplStatusbar *statusbar = TEPL_STATUSBAR (object);

	g_clear_object (&statusbar->priv->tab_group);
	tepl_signal_group_clear (&statusbar->priv->buffer_signal_group);

	statusbar->priv->label = NULL;

	G_OBJECT_CLASS (tepl_statusbar_parent_class)->dispose (object);
}

static void
tepl_statusbar_class_init (TeplStatusbarClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = tepl_statusbar_dispose;
}

static void
tepl_statusbar_init (TeplStatusbar *statusbar)
{
	statusbar->priv = tepl_statusbar_get_instance_private (statusbar);

	gtk_widget_set_margin_top (GTK_WIDGET (statusbar), 0);
	gtk_widget_set_margin_bottom (GTK_WIDGET (statusbar), 0);

	statusbar->priv->label = GTK_LABEL (gtk_label_new (NULL));
	gtk_widget_set_no_show_all (GTK_WIDGET (statusbar->priv->label), TRUE);

	gtk_box_pack_end (GTK_BOX (statusbar),
			  GTK_WIDGET (statusbar->priv->label),
			  FALSE, TRUE, 0);
}

/**
 * tepl_statusbar_new:
 *
 * Returns: (transfer floating): a new #TeplStatusbar.
 * Since: 5.0
 */
TeplStatusbar *
tepl_statusbar_new (void)
{
	return g_object_new (TEPL_TYPE_STATUSBAR, NULL);
}

/**
 * tepl_statusbar_show_cursor_position:
 * @statusbar: a #TeplStatusbar.
 * @line: the line number, must be >= 1.
 * @column: the column number, must be >= 1.
 *
 * Shows the line and column numbers on the right side of the @statusbar. (So
 * messages added with gtk_statusbar_push() are still visible after calling this
 * function).
 *
 * Since: 5.0
 */
void
tepl_statusbar_show_cursor_position (TeplStatusbar *statusbar,
				     gint           line,
				     gint           column)
{
	gchar *text;

	g_return_if_fail (TEPL_IS_STATUSBAR (statusbar));
	g_return_if_fail (line >= 1);
	g_return_if_fail (column >= 1);

	/* Translators: "Ln" is an abbreviation for "Line", Col is an
	 * abbreviation for "Column". Please, use abbreviations if possible.
	 */
	text = g_strdup_printf (_("Ln %d, Col %d"), line, column);

	gtk_label_set_text (statusbar->priv->label, text);
	gtk_widget_show (GTK_WIDGET (statusbar->priv->label));

	g_free (text);
}

/**
 * tepl_statusbar_hide_cursor_position:
 * @statusbar: a #TeplStatusbar.
 *
 * The reverse action of tepl_statusbar_show_cursor_position(). This function
 * hides the text used to show the line and column numbers.
 *
 * Since: 5.0
 */
void
tepl_statusbar_hide_cursor_position (TeplStatusbar *statusbar)
{
	g_return_if_fail (TEPL_IS_STATUSBAR (statusbar));

	gtk_widget_hide (GTK_WIDGET (statusbar->priv->label));
}

static void
update_cursor_position (TeplStatusbar *statusbar)
{
	TeplView *active_view;
	GtkTextBuffer *active_buffer;
	GtkTextIter iter;
	gint line;
	gint column;

	active_view = tepl_tab_group_get_active_view (statusbar->priv->tab_group);
	if (active_view == NULL)
	{
		tepl_statusbar_hide_cursor_position (statusbar);
		return;
	}

	active_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (active_view));
	gtk_text_buffer_get_iter_at_mark (active_buffer,
					  &iter,
					  gtk_text_buffer_get_insert (active_buffer));

	line = gtk_text_iter_get_line (&iter);
	column = gtk_source_view_get_visual_column (GTK_SOURCE_VIEW (active_view), &iter);
	tepl_statusbar_show_cursor_position (statusbar, line + 1, column + 1);
}

static void
active_buffer_cursor_moved_cb (TeplBuffer    *active_buffer,
			       TeplStatusbar *statusbar)
{
	update_cursor_position (statusbar);
}

static void
active_buffer_changed (TeplStatusbar *statusbar)
{
	TeplBuffer *active_buffer;

	tepl_signal_group_clear (&statusbar->priv->buffer_signal_group);

	active_buffer = tepl_tab_group_get_active_buffer (statusbar->priv->tab_group);
	if (active_buffer == NULL)
	{
		goto end;
	}

	statusbar->priv->buffer_signal_group = tepl_signal_group_new (G_OBJECT (active_buffer));

	tepl_signal_group_add (statusbar->priv->buffer_signal_group,
			       g_signal_connect (active_buffer,
						 "tepl-cursor-moved",
						 G_CALLBACK (active_buffer_cursor_moved_cb),
						 statusbar));

end:
	update_cursor_position (statusbar);
}

static void
active_buffer_notify_cb (TeplTabGroup  *tab_group,
			 GParamSpec    *pspec,
			 TeplStatusbar *statusbar)
{
	active_buffer_changed (statusbar);
}

/**
 * tepl_statusbar_set_tab_group:
 * @statusbar: a #TeplStatusbar.
 * @tab_group: a #TeplTabGroup.
 *
 * Calls tepl_statusbar_show_cursor_position() and
 * tepl_statusbar_hide_cursor_position() according to the
 * #TeplTabGroup:active-view of @tab_group, and the
 * #TeplBuffer::tepl-cursor-moved signal.
 *
 * For the column number it uses the gtk_source_view_get_visual_column()
 * function.
 *
 * This function can be called only once, it is not possible to change the
 * #TeplTabGroup afterwards (this restriction may be lifted in the future if
 * there is a compelling use-case).
 *
 * Since: 5.0
 */
void
tepl_statusbar_set_tab_group (TeplStatusbar *statusbar,
			      TeplTabGroup  *tab_group)
{
	g_return_if_fail (TEPL_IS_STATUSBAR (statusbar));
	g_return_if_fail (TEPL_IS_TAB_GROUP (tab_group));

	if (statusbar->priv->tab_group != NULL)
	{
		g_warning ("%s(): the TeplTabGroup has already been set, it can be set only once.",
			   G_STRFUNC);
		return;
	}

	statusbar->priv->tab_group = g_object_ref_sink (tab_group);

	g_signal_connect_object (tab_group,
				 "notify::active-buffer",
				 G_CALLBACK (active_buffer_notify_cb),
				 statusbar,
				 0);

	active_buffer_changed (statusbar);
}
