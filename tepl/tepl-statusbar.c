/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2020 - Sébastien Wilmet <swilmet@gnome.org>
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
#include "tepl-statusbar.h"
#include <glib/gi18n-lib.h>

/**
 * SECTION:statusbar
 * @Title: TeplStatusbar
 * @Short_description: Subclass of #GtkStatusbar
 */

struct _TeplStatusbarPrivate
{
	GtkLabel *label;
};

G_DEFINE_TYPE_WITH_PRIVATE (TeplStatusbar, tepl_statusbar, GTK_TYPE_STATUSBAR)

static void
tepl_statusbar_class_init (TeplStatusbarClass *klass)
{
}

static void
tepl_statusbar_init (TeplStatusbar *statusbar)
{
	statusbar->priv = tepl_statusbar_get_instance_private (statusbar);

	/* FIXME: still needed? */
	/*
	gtk_widget_set_margin_top (GTK_WIDGET (statusbar), 0);
	gtk_widget_set_margin_bottom (GTK_WIDGET (statusbar), 0);
	*/

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
