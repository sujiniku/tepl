/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "config.h"
#include "tepl-language-chooser-widget.h"
#include <glib/gi18n-lib.h>
#include "tepl-language-chooser.h"

/**
 * SECTION:language-chooser-widget
 * @Title: TeplLanguageChooserWidget
 * @Short_description: A widget for choosing a #GtkSourceLanguage
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
tepl_language_chooser_widget_class_init (TeplLanguageChooserWidgetClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = tepl_language_chooser_widget_dispose;
}

static void
tepl_language_chooser_widget_select_language (TeplLanguageChooser *chooser,
					      GtkSourceLanguage   *language)
{
	/* TODO */
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
search_changed_cb (GtkSearchEntry            *search_entry,
		   TeplLanguageChooserWidget *chooser_widget)
{
	gtk_list_box_invalidate_filter (chooser_widget->priv->list_box);
}

static void
list_box_row_activated_cb (GtkListBox                *list_box,
			   GtkListBoxRow             *list_box_row,
			   TeplLanguageChooserWidget *chooser_widget)
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
tepl_language_chooser_widget_init (TeplLanguageChooserWidget *chooser_widget)
{
	GtkWidget *scrolled_window;

	chooser_widget->priv = tepl_language_chooser_widget_get_instance_private (chooser_widget);

	gtk_orientable_set_orientation (GTK_ORIENTABLE (chooser_widget), GTK_ORIENTATION_VERTICAL);
	gtk_widget_set_size_request (GTK_WIDGET (chooser_widget), 300, 400);
	gtk_grid_set_row_spacing (GTK_GRID (chooser_widget), 3);
	gtk_container_set_border_width (GTK_CONTAINER (chooser_widget), 6);

	chooser_widget->priv->search_entry = GTK_SEARCH_ENTRY (gtk_search_entry_new ());
	gtk_entry_set_placeholder_text (GTK_ENTRY (chooser_widget->priv->search_entry),
					_("Search highlight mode…"));
	gtk_widget_show (GTK_WIDGET (chooser_widget->priv->search_entry));
	gtk_container_add (GTK_CONTAINER (chooser_widget), GTK_WIDGET (chooser_widget->priv->search_entry));

	chooser_widget->priv->list_box = GTK_LIST_BOX (gtk_list_box_new ());
	gtk_list_box_set_activate_on_single_click (chooser_widget->priv->list_box, FALSE);
	populate_list_box (chooser_widget);
	gtk_widget_set_hexpand (GTK_WIDGET (chooser_widget->priv->list_box), TRUE);
	gtk_widget_set_vexpand (GTK_WIDGET (chooser_widget->priv->list_box), TRUE);

	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window), GTK_SHADOW_IN);
	gtk_container_add (GTK_CONTAINER (scrolled_window), GTK_WIDGET (chooser_widget->priv->list_box));
	gtk_widget_show_all (scrolled_window);
	gtk_container_add (GTK_CONTAINER (chooser_widget), scrolled_window);

	gtk_list_box_set_filter_func (chooser_widget->priv->list_box,
				      filter_cb,
				      chooser_widget,
				      NULL);

	g_signal_connect (chooser_widget->priv->search_entry,
			  "search-changed",
			  G_CALLBACK (search_changed_cb),
			  chooser_widget);

	g_signal_connect (chooser_widget->priv->list_box,
			  "row-activated",
			  G_CALLBACK (list_box_row_activated_cb),
			  chooser_widget);
}

/**
 * tepl_language_chooser_widget_new:
 *
 * Returns: (transfer floating): a new #TeplLanguageChooserWidget.
 * Since: 5.2
 */
TeplLanguageChooserWidget *
tepl_language_chooser_widget_new (void)
{
	return g_object_new (TEPL_TYPE_LANGUAGE_CHOOSER_WIDGET, NULL);
}
