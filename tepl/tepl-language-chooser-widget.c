/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "config.h"
#include "tepl-language-chooser-widget.h"
#include <glib/gi18n-lib.h>
#include "tepl-language-chooser.h"
#include "tepl-utils.h"

/**
 * SECTION:language-chooser-widget
 * @Title: TeplLanguageChooserWidget
 * @Short_description: A widget for choosing a #GtkSourceLanguage
 *
 * #TeplLanguageChooserWidget is a #GtkWidget to choose a #GtkSourceLanguage.
 * #TeplLanguageChooserWidget implements the #TeplLanguageChooser interface.
 *
 * In addition to the list, it contains a #GtkSearchEntry to search the list.
 */

struct _TeplLanguageChooserWidgetPrivate
{
	GtkSearchEntry *search_entry;
	GtkListBox *list_box;
};

#define LIST_BOX_ROW_LANGUAGE_KEY "language-key"

static void tepl_language_chooser_interface_init (gpointer g_iface,
						  gpointer iface_data);

G_DEFINE_TYPE_WITH_CODE (TeplLanguageChooserWidget,
			 tepl_language_chooser_widget,
			 GTK_TYPE_GRID,
			 G_ADD_PRIVATE (TeplLanguageChooserWidget)
			 G_IMPLEMENT_INTERFACE (TEPL_TYPE_LANGUAGE_CHOOSER,
						tepl_language_chooser_interface_init))

static gboolean filter_cb (GtkListBoxRow *list_box_row,
			   gpointer       user_data);

/* Could be moved to tepl-utils, but it would require to make it more general,
 * for all GtkListBox scenarios ideally (with non-selectable rows, headers,
 * etc). Or by documenting the limitations.
 */
static void
list_box_select_first_row (GtkListBox           *list_box,
			   GtkListBoxFilterFunc  filter_func,
			   gpointer              user_data)
{
	GtkListBoxRow *row;

	row = tepl_utils_list_box_get_row_at_index_with_filter (list_box, 0, filter_func, user_data);
	gtk_list_box_select_row (list_box, row);

	if (row != NULL)
	{
		tepl_utils_list_box_scroll_to_row (list_box, row);
	}
}

static void
select_first_row (TeplLanguageChooserWidget *chooser_widget)
{
	list_box_select_first_row (chooser_widget->priv->list_box,
				   filter_cb,
				   chooser_widget);
}

static const gchar *
get_language_name (GtkSourceLanguage *language)
{
	if (language == NULL)
	{
		return _("Plain Text");
	}

	return gtk_source_language_get_name (language);
}

static void
list_box_row_set_language (GtkListBoxRow     *list_box_row,
			   GtkSourceLanguage *language)
{
	g_object_set_data_full (G_OBJECT (list_box_row),
				LIST_BOX_ROW_LANGUAGE_KEY,
				g_object_ref (language),
				g_object_unref);
}

/* Returns: (transfer none). */
static GtkSourceLanguage *
list_box_row_get_language (GtkListBoxRow *list_box_row)
{
	return g_object_get_data (G_OBJECT (list_box_row), LIST_BOX_ROW_LANGUAGE_KEY);
}

static void
tepl_language_chooser_widget_dispose (GObject *object)
{
	TeplLanguageChooserWidget *chooser_widget = TEPL_LANGUAGE_CHOOSER_WIDGET (object);

	chooser_widget->priv->search_entry = NULL;
	chooser_widget->priv->list_box = NULL;

	G_OBJECT_CLASS (tepl_language_chooser_widget_parent_class)->dispose (object);
}

static void
tepl_language_chooser_widget_map (GtkWidget *widget)
{
	TeplLanguageChooserWidget *chooser_widget = TEPL_LANGUAGE_CHOOSER_WIDGET (widget);

	if (GTK_WIDGET_CLASS (tepl_language_chooser_widget_parent_class)->map != NULL)
	{
		GTK_WIDGET_CLASS (tepl_language_chooser_widget_parent_class)->map (widget);
	}

	tepl_utils_list_box_scroll_to_selected_row (chooser_widget->priv->list_box);
}

static void
tepl_language_chooser_widget_class_init (TeplLanguageChooserWidgetClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	object_class->dispose = tepl_language_chooser_widget_dispose;

	widget_class->map = tepl_language_chooser_widget_map;
}

static void
tepl_language_chooser_widget_select_language (TeplLanguageChooser *chooser,
					      GtkSourceLanguage   *language)
{
	TeplLanguageChooserWidget *chooser_widget = TEPL_LANGUAGE_CHOOSER_WIDGET (chooser);
	GList *all_rows;
	GList *l;

	all_rows = gtk_container_get_children (GTK_CONTAINER (chooser_widget->priv->list_box));

	for (l = all_rows; l != NULL; l = l->next)
	{
		GtkListBoxRow *cur_row = GTK_LIST_BOX_ROW (l->data);
		GtkSourceLanguage *cur_language;

		cur_language = list_box_row_get_language (cur_row);
		if (cur_language == language)
		{
			gtk_list_box_select_row (chooser_widget->priv->list_box, cur_row);
			tepl_utils_list_box_scroll_to_row (chooser_widget->priv->list_box, cur_row);
			break;
		}
	}

	g_list_free (all_rows);
}

static void
tepl_language_chooser_interface_init (gpointer g_iface,
				      gpointer iface_data)
{
	TeplLanguageChooserInterface *interface = g_iface;

	interface->select_language = tepl_language_chooser_widget_select_language;
}

static GtkListBoxRow *
create_list_box_row (const gchar *label_text)
{
	GtkWidget *label;
	GtkListBoxRow *list_box_row;

	label = gtk_label_new (label_text);
	gtk_label_set_xalign (GTK_LABEL (label), 0.0);

	list_box_row = GTK_LIST_BOX_ROW (gtk_list_box_row_new ());
	gtk_container_add (GTK_CONTAINER (list_box_row), label);

	return list_box_row;
}

static void
append_plain_text_item_to_list_box (TeplLanguageChooserWidget *chooser_widget)
{
	/* NULL GtkSourceLanguage. */
	gtk_container_add (GTK_CONTAINER (chooser_widget->priv->list_box),
			   GTK_WIDGET (create_list_box_row (get_language_name (NULL))));
}

static void
append_language_to_list_box (TeplLanguageChooserWidget *chooser_widget,
			     GtkSourceLanguage         *language)
{
	GtkListBoxRow *list_box_row;

	g_return_if_fail (GTK_SOURCE_IS_LANGUAGE (language));

	list_box_row = create_list_box_row (gtk_source_language_get_name (language));
	list_box_row_set_language (list_box_row, language);
	gtk_container_add (GTK_CONTAINER (chooser_widget->priv->list_box),
			   GTK_WIDGET (list_box_row));
}

static void
populate_list_box (TeplLanguageChooserWidget *chooser_widget)
{
	GtkSourceLanguageManager *language_manager;
	const gchar * const *language_ids;
	gint i;

	// First item
	append_plain_text_item_to_list_box (chooser_widget);

	language_manager = gtk_source_language_manager_get_default ();
	language_ids = gtk_source_language_manager_get_language_ids (language_manager);

	if (language_ids == NULL)
	{
		return;
	}

	for (i = 0; language_ids[i] != NULL; i++)
	{
		const gchar *cur_language_id = language_ids[i];
		GtkSourceLanguage *language;

		language = gtk_source_language_manager_get_language (language_manager, cur_language_id);
		if (!gtk_source_language_get_hidden (language))
		{
			append_language_to_list_box (chooser_widget, language);
		}
	}
}

static gboolean
filter_cb (GtkListBoxRow *list_box_row,
	   gpointer       user_data)
{
	TeplLanguageChooserWidget *chooser_widget = user_data;
	const gchar *search_text;
	GtkSourceLanguage *language;
	const gchar *item_name;
	gchar *item_name_normalized;
	gchar *item_name_casefolded;
	gchar *search_text_normalized;
	gchar *search_text_casefolded;
	gboolean visible = FALSE;

	search_text = gtk_entry_get_text (GTK_ENTRY (chooser_widget->priv->search_entry));
	if (search_text == NULL || search_text[0] == '\0')
	{
		return TRUE;
	}

	language = list_box_row_get_language (list_box_row);
	item_name = get_language_name (language);
	g_return_val_if_fail (item_name != NULL, FALSE);

	/* Safer to check... (item_name can come from a *.lang file). */
	g_return_val_if_fail (g_utf8_validate (search_text, -1, NULL), FALSE);
	g_return_val_if_fail (g_utf8_validate (item_name, -1, NULL), FALSE);

	item_name_normalized = g_utf8_normalize (item_name, -1, G_NORMALIZE_ALL);
	item_name_casefolded = g_utf8_casefold (item_name_normalized, -1);
	g_free (item_name_normalized);

	/* Note: we do not apply g_strstrip() on the search text, because a
	 * trailing space (or - to a less extent - a leading space) can
	 * differentiate several items, for example:
	 * - "ERB"
	 * - "ERB (HTML)"
	 * - "ERB (JavaScript)"
	 */
	search_text_normalized = g_utf8_normalize (search_text, -1, G_NORMALIZE_ALL);
	search_text_casefolded = g_utf8_casefold (search_text_normalized, -1);
	g_free (search_text_normalized);

	if (item_name_casefolded != NULL && search_text_casefolded != NULL)
	{
		visible = strstr (item_name_casefolded, search_text_casefolded) != NULL;
	}

	g_free (item_name_casefolded);
	g_free (search_text_casefolded);
	return visible;
}

static void
search_entry_changed_cb (GtkEditable               *search_entry,
			 TeplLanguageChooserWidget *chooser_widget)
{
	/* Invalidate the filter directly, not in the
	 * GtkSearchEntry::search-changed signal because ::search-changed is
	 * emitted after a small delay, and the filter_cb() function is used
	 * elsewhere in the code. To avoid inconsistencies.
	 *
	 * The delay is anyway not necessary because the list is small and is
	 * updated quickly enough, after a quick test.
	 */
	gtk_list_box_invalidate_filter (chooser_widget->priv->list_box);
	select_first_row (chooser_widget);
}

static void
emit_language_activated_for_row (TeplLanguageChooserWidget *chooser_widget,
				 GtkListBoxRow             *list_box_row)
{
	GtkSourceLanguage *language;

	language = list_box_row_get_language (list_box_row);

	if (language != NULL)
	{
		g_object_ref (language);
	}

	g_signal_emit_by_name (chooser_widget, "language-activated", language);

	if (language != NULL)
	{
		g_object_unref (language);
	}
}

static void
search_entry_activate_cb (GtkEntry                  *entry,
			  TeplLanguageChooserWidget *chooser_widget)
{
	_tepl_language_chooser_widget_activate_selected_language (chooser_widget);
}

static void
list_box_row_activated_cb (GtkListBox                *list_box,
			   GtkListBoxRow             *list_box_row,
			   TeplLanguageChooserWidget *chooser_widget)
{
	emit_language_activated_for_row (chooser_widget, list_box_row);
}

static void
move_selection (TeplLanguageChooserWidget *chooser_widget,
		gint                       how_many)
{
	GtkListBoxRow *selected_row;
	GtkListBoxRow **filtered_children = NULL;
	gint n_filtered_children;
	gint pos;
	gint selected_row_pos = 0;
	gboolean found = FALSE;
	gint new_row_to_select_pos;
	GtkListBoxRow *new_row_to_select;

	selected_row = gtk_list_box_get_selected_row (chooser_widget->priv->list_box);
	if (selected_row == NULL || !filter_cb (selected_row, chooser_widget))
	{
		select_first_row (chooser_widget);
		return;
	}

	filtered_children = tepl_utils_list_box_get_filtered_children (chooser_widget->priv->list_box,
								       filter_cb,
								       chooser_widget,
								       &n_filtered_children);

	if (filtered_children == NULL)
	{
		goto out;
	}

	for (pos = 0; filtered_children[pos] != NULL; pos++)
	{
		GtkListBoxRow *cur_row = filtered_children[pos];

		if (cur_row == selected_row)
		{
			selected_row_pos = pos;
			found = TRUE;
			break;
		}
	}

	if (!found)
	{
		g_warn_if_reached ();
		goto out;
	}

	new_row_to_select_pos = selected_row_pos + how_many;
	new_row_to_select_pos = CLAMP (new_row_to_select_pos, 0, n_filtered_children - 1);
	new_row_to_select = filtered_children[new_row_to_select_pos];
	gtk_list_box_select_row (chooser_widget->priv->list_box, new_row_to_select);
	tepl_utils_list_box_scroll_to_row (chooser_widget->priv->list_box, new_row_to_select);

out:
	g_free (filtered_children);
}

static gboolean
search_entry_key_press_event_cb (GtkWidget                 *search_entry,
				 GdkEventKey               *event,
				 TeplLanguageChooserWidget *chooser_widget)
{
	if (event->keyval == GDK_KEY_Down)
	{
		move_selection (chooser_widget, 1);
		return GDK_EVENT_STOP;
	}
	else if (event->keyval == GDK_KEY_Up)
	{
		move_selection (chooser_widget, -1);
		return GDK_EVENT_STOP;
	}
	else if (event->keyval == GDK_KEY_Page_Down)
	{
		move_selection (chooser_widget, 5);
		return GDK_EVENT_STOP;
	}
	else if (event->keyval == GDK_KEY_Page_Up)
	{
		move_selection (chooser_widget, -5);
		return GDK_EVENT_STOP;
	}

	return GDK_EVENT_PROPAGATE;
}

static void
tepl_language_chooser_widget_init (TeplLanguageChooserWidget *chooser_widget)
{
	GtkScrolledWindow *scrolled_window;

	chooser_widget->priv = tepl_language_chooser_widget_get_instance_private (chooser_widget);

	/* chooser_widget config */
	gtk_orientable_set_orientation (GTK_ORIENTABLE (chooser_widget), GTK_ORIENTATION_VERTICAL);
	gtk_widget_set_size_request (GTK_WIDGET (chooser_widget), 300, 400);
	gtk_grid_set_row_spacing (GTK_GRID (chooser_widget), 3);
	gtk_container_set_border_width (GTK_CONTAINER (chooser_widget), 6);

	/* GtkSearchEntry */
	chooser_widget->priv->search_entry = GTK_SEARCH_ENTRY (gtk_search_entry_new ());
	gtk_entry_set_placeholder_text (GTK_ENTRY (chooser_widget->priv->search_entry),
					_("Search highlight mode…"));
	gtk_widget_show (GTK_WIDGET (chooser_widget->priv->search_entry));
	gtk_container_add (GTK_CONTAINER (chooser_widget), GTK_WIDGET (chooser_widget->priv->search_entry));

	/* GtkListBox */
	chooser_widget->priv->list_box = GTK_LIST_BOX (gtk_list_box_new ());
	gtk_list_box_set_activate_on_single_click (chooser_widget->priv->list_box, FALSE);
	gtk_widget_set_hexpand (GTK_WIDGET (chooser_widget->priv->list_box), TRUE);
	gtk_widget_set_vexpand (GTK_WIDGET (chooser_widget->priv->list_box), TRUE);
	populate_list_box (chooser_widget);

	/* GtkScrolledWindow */
	scrolled_window = GTK_SCROLLED_WINDOW (gtk_scrolled_window_new (NULL, NULL));
	gtk_scrolled_window_set_shadow_type (scrolled_window, GTK_SHADOW_IN);

	gtk_container_add (GTK_CONTAINER (scrolled_window), GTK_WIDGET (chooser_widget->priv->list_box));
	gtk_widget_show_all (GTK_WIDGET (scrolled_window));
	gtk_container_add (GTK_CONTAINER (chooser_widget), GTK_WIDGET (scrolled_window));

	tepl_utils_list_box_setup_scrolling (chooser_widget->priv->list_box, scrolled_window);

	gtk_list_box_set_filter_func (chooser_widget->priv->list_box,
				      filter_cb,
				      chooser_widget,
				      NULL);

	g_signal_connect (chooser_widget->priv->search_entry,
			  "changed",
			  G_CALLBACK (search_entry_changed_cb),
			  chooser_widget);

	g_signal_connect (chooser_widget->priv->search_entry,
			  "activate",
			  G_CALLBACK (search_entry_activate_cb),
			  chooser_widget);

	g_signal_connect (chooser_widget->priv->search_entry,
			  "key-press-event",
			  G_CALLBACK (search_entry_key_press_event_cb),
			  chooser_widget);

	g_signal_connect (chooser_widget->priv->list_box,
			  "row-activated",
			  G_CALLBACK (list_box_row_activated_cb),
			  chooser_widget);

	select_first_row (chooser_widget);
	gtk_widget_grab_focus (GTK_WIDGET (chooser_widget->priv->search_entry));
}

/**
 * tepl_language_chooser_widget_new:
 *
 * Returns: (transfer floating): a new #TeplLanguageChooserWidget.
 * Since: 6.0
 */
TeplLanguageChooserWidget *
tepl_language_chooser_widget_new (void)
{
	return g_object_new (TEPL_TYPE_LANGUAGE_CHOOSER_WIDGET, NULL);
}

void
_tepl_language_chooser_widget_activate_selected_language (TeplLanguageChooserWidget *chooser_widget)
{
	GtkListBoxRow *selected_row;

	g_return_if_fail (TEPL_IS_LANGUAGE_CHOOSER_WIDGET (chooser_widget));

	selected_row = gtk_list_box_get_selected_row (chooser_widget->priv->list_box);
	if (selected_row != NULL)
	{
		emit_language_activated_for_row (chooser_widget, selected_row);
	}
}
