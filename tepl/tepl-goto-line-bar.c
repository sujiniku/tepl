/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "config.h"
#include "tepl-goto-line-bar.h"
#include <glib/gi18n-lib.h>
#include "tepl-utils.h"

/**
 * SECTION:goto-line-bar
 * @Title: TeplGotoLineBar
 * @Short_description: Horizontal bar for the 'Go to line' feature
 *
 * #TeplGotoLineBar is an horizontal bar containing among other things:
 * - A #GtkSearchEntry.
 * - A close button.
 *
 * When the #GtkSearchEntry's content changes, tepl_view_goto_line() is called
 * on the associated #TeplView.
 *
 * # High-level API
 *
 * #TeplGotoLineBar is integrated in the Tepl framework, see:
 * - The `"win.tepl-goto-line"` #GAction in #TeplApplicationWindow.
 * - The tepl_tab_get_goto_line_bar() function.
 */

struct _TeplGotoLineBarPrivate
{
	/* A GtkSearchEntry. */
	GtkEntry *entry;

	/* Owns a strong ref. */
	TeplView *view;

	guint bound_to_gaction_state : 1;
};

G_DEFINE_TYPE_WITH_PRIVATE (TeplGotoLineBar, tepl_goto_line_bar, GTK_TYPE_GRID)

static void
tepl_goto_line_bar_dispose (GObject *object)
{
	TeplGotoLineBar *bar = TEPL_GOTO_LINE_BAR (object);

	bar->priv->entry = NULL;
	g_clear_object (&bar->priv->view);

	G_OBJECT_CLASS (tepl_goto_line_bar_parent_class)->dispose (object);
}

static void
tepl_goto_line_bar_class_init (TeplGotoLineBarClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = tepl_goto_line_bar_dispose;
}

static void
set_success (GtkEntry *entry,
	     gboolean  success)
{
	GtkStyleContext *style_context;

	style_context = gtk_widget_get_style_context (GTK_WIDGET (entry));

	if (success)
	{
		gtk_style_context_remove_class (style_context, GTK_STYLE_CLASS_ERROR);
	}
	else
	{
		gtk_style_context_add_class (style_context, GTK_STYLE_CLASS_ERROR);
	}
}

static void
entry_search_changed_cb (GtkEntry        *entry,
			 TeplGotoLineBar *bar)
{
	const gchar *entry_text;
	gint64 line = 0;
	gboolean success = FALSE;

	if (bar->priv->view == NULL)
	{
		return;
	}

	entry_text = gtk_entry_get_text (entry);

	if (entry_text == NULL || entry_text[0] == '\0')
	{
		set_success (entry, TRUE);
		return;
	}

	if (g_ascii_string_to_signed (entry_text,
				      10,
				      0, G_MAXINT,
				      &line,
				      NULL))
	{
		/* When typing "0" in the search entry, treat it the same as 1. */
		success = tepl_view_goto_line (bar->priv->view, MAX (line - 1, 0));
	}

	set_success (entry, success);
}

static void
entry_activate_cb (GtkEntry        *entry,
		   TeplGotoLineBar *bar)
{
	gtk_widget_hide (GTK_WIDGET (bar));
}

static void
create_entry (TeplGotoLineBar *bar)
{
	g_assert (bar->priv->entry == NULL);

	bar->priv->entry = GTK_ENTRY (gtk_search_entry_new ());
	gtk_widget_show (GTK_WIDGET (bar->priv->entry));
	gtk_grid_attach (GTK_GRID (bar), GTK_WIDGET (bar->priv->entry), 1, 0, 1, 1);

	g_signal_connect (bar->priv->entry,
			  "search-changed",
			  G_CALLBACK (entry_search_changed_cb),
			  bar);

	g_signal_connect (bar->priv->entry,
			  "activate",
			  G_CALLBACK (entry_activate_cb),
			  bar);
}

static void
close_button_clicked_cb (GtkButton       *close_button,
			 TeplGotoLineBar *bar)
{
	gtk_widget_hide (GTK_WIDGET (bar));
}

static void
bar_hide_cb (TeplGotoLineBar *bar,
	     gpointer         user_data)
{
	/* A previous implementation for this callback was:
	 * gtk_entry_set_text (bar->priv->entry, "");
	 *
	 * But there was the following situation that was not working well wrt
	 * the styling applied to the GtkEntry:
	 *
	 * GtkEntry in error state (see set_success()) -> hide bar -> set entry
	 * text to "" -> the entry state is set to success but the bar is
	 * immediately hidden -> then some time later the bar is shown again.
	 * --> the error state (in red) is still visible during a short period
	 *  of time when the bar is shown again. The time that it transitions to
	 *  the normal state (the colors fade progressively).
	 *
	 * I haven't found a GtkStyleContext API to fix that problem (for
	 * example to apply the final state immediately).
	 *
	 * So the easiest is to destroy and re-create the widget. That way when
	 * it will be shown again, we are sure that it will have the initial
	 * style.
	 */
	if (bar->priv->entry != NULL)
	{
		gtk_widget_destroy (GTK_WIDGET (bar->priv->entry));
		bar->priv->entry = NULL;
	}

	create_entry (bar);
}

static void
tepl_goto_line_bar_init (TeplGotoLineBar *bar)
{
	GtkWidget *label;
	GtkWidget *close_button;

	bar->priv = tepl_goto_line_bar_get_instance_private (bar);

	gtk_grid_set_column_spacing (GTK_GRID (bar), 6);
	gtk_widget_set_margin_start (GTK_WIDGET (bar), 6);
	gtk_widget_set_margin_end (GTK_WIDGET (bar), 4);
	gtk_widget_set_margin_top (GTK_WIDGET (bar), 3);
	gtk_widget_set_margin_bottom (GTK_WIDGET (bar), 3);

	label = gtk_label_new (_("Go to line:"));
	gtk_widget_show (label);
	gtk_grid_attach (GTK_GRID (bar), label, 0, 0, 1, 1);

	create_entry (bar);

	close_button = tepl_utils_create_close_button ();
	gtk_widget_show (close_button);
	gtk_widget_set_tooltip_text (close_button, _("Close"));
	gtk_widget_set_hexpand (close_button, TRUE);
	gtk_widget_set_halign (close_button, GTK_ALIGN_END);
	gtk_grid_attach (GTK_GRID (bar), close_button, 2, 0, 1, 1);

	g_signal_connect (close_button,
			  "clicked",
			  G_CALLBACK (close_button_clicked_cb),
			  bar);

	g_signal_connect (bar,
			  "hide",
			  G_CALLBACK (bar_hide_cb),
			  NULL);
}

/**
 * tepl_goto_line_bar_new:
 *
 * Returns: (transfer floating): a new #TeplGotoLineBar widget.
 * Since: 5.0
 */
TeplGotoLineBar *
tepl_goto_line_bar_new (void)
{
	return g_object_new (TEPL_TYPE_GOTO_LINE_BAR, NULL);
}

/**
 * tepl_goto_line_bar_set_view:
 * @bar: a #TeplGotoLineBar.
 * @view: a #TeplView.
 *
 * Sets the #TeplView. tepl_view_goto_line() will be called on @view when the
 * user types a line number in the #GtkSearchEntry of @bar.
 *
 * Only one #TeplView can be associated per #TeplGotoLineBar.
 *
 * Since: 5.0
 */
void
tepl_goto_line_bar_set_view (TeplGotoLineBar *bar,
			     TeplView        *view)
{
	g_return_if_fail (TEPL_IS_GOTO_LINE_BAR (bar));
	g_return_if_fail (view == NULL || TEPL_IS_VIEW (view));

	if (bar->priv->view != view)
	{
		g_clear_object (&bar->priv->view);
		bar->priv->view = g_object_ref_sink (view);
	}
}

/**
 * tepl_goto_line_bar_grab_focus_to_entry:
 * @bar: a #TeplGotoLineBar.
 *
 * Calls gtk_widget_grab_focus() to the #GtkSearchEntry of @bar.
 *
 * Since: 5.0
 */
void
tepl_goto_line_bar_grab_focus_to_entry (TeplGotoLineBar *bar)
{
	g_return_if_fail (TEPL_IS_GOTO_LINE_BAR (bar));

	gtk_widget_grab_focus (GTK_WIDGET (bar->priv->entry));
}

void
_tepl_goto_line_bar_bind_to_gaction_state (TeplGotoLineBar *bar,
					   GAction         *action)
{
	g_return_if_fail (TEPL_IS_GOTO_LINE_BAR (bar));
	g_return_if_fail (G_IS_ACTION (action));

	if (!bar->priv->bound_to_gaction_state)
	{
		g_object_bind_property_full (action, "state",
					     bar, "visible",
					     G_BINDING_BIDIRECTIONAL |
					     G_BINDING_SYNC_CREATE,
					     tepl_utils_binding_transform_func_smart_bool,
					     tepl_utils_binding_transform_func_smart_bool,
					     NULL, NULL);

		bar->priv->bound_to_gaction_state = TRUE;
	}
}
