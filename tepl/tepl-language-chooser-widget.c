/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "tepl-language-chooser-widget.h"
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

static void
populate_list_box (TeplLanguageChooserWidget *chooser_widget)
{
	GtkSourceLanguageManager *language_manager;
	const gchar * const *language_ids;
	gint i;

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
		GtkLabel *label;
		GtkListBoxRow *list_box_row;

		language = gtk_source_language_manager_get_language (language_manager, cur_language_id);
		if (gtk_source_language_get_hidden (language))
		{
			continue;
		}

		label = GTK_LABEL (gtk_label_new (gtk_source_language_get_name (language)));
		gtk_label_set_xalign (label, 0.0);

		list_box_row = GTK_LIST_BOX_ROW (gtk_list_box_row_new ());
		list_box_row_set_language (list_box_row, language);
		gtk_container_add (GTK_CONTAINER (list_box_row), GTK_WIDGET (label));
		gtk_container_add (GTK_CONTAINER (chooser_widget->priv->list_box), GTK_WIDGET (list_box_row));
	}
}

static gboolean
filter_cb (GtkListBoxRow *list_box_row,
	   gpointer       user_data)
{
	TeplLanguageChooserWidget *chooser_widget = user_data;
	const gchar *search_text;
	GtkSourceLanguage *language;
	const gchar *language_name;
	gchar *language_name_normalized;
	gchar *language_name_casefolded;
	gchar *search_text_normalized;
	gchar *search_text_casefolded;
	gboolean visible;

	search_text = gtk_entry_get_text (GTK_ENTRY (chooser_widget->priv->search_entry));
	if (search_text == NULL || search_text[0] == '\0')
	{
		return TRUE;
	}

	language = list_box_row_get_language (list_box_row);
	g_return_val_if_fail (language != NULL, FALSE);

	language_name = gtk_source_language_get_name (language);
	g_return_val_if_fail (language_name != NULL, FALSE);

	/* search_text is guaranteed to be UTF-8 (it comes from a GTK widget).
	 * language_name is not really, it can come from foreign input (*.lang
	 * file), so it's safer to check.
	 */
	g_return_val_if_fail (g_utf8_validate (language_name, -1, NULL), FALSE);

	language_name_normalized = g_utf8_normalize (language_name, -1, G_NORMALIZE_ALL);
	language_name_casefolded = g_utf8_casefold (language_name_normalized, -1);
	g_free (language_name_normalized);

	/* Note: we do not apply g_strstrip() on the search text, because a
	 * trailing space (or - to a less extent - a leading space) can
	 * differentiate several GtkSourceLanguages, for example:
	 * - "ERB"
	 * - "ERB (HTML)"
	 * - "ERB (JavaScript)"
	 */
	search_text_normalized = g_utf8_normalize (search_text, -1, G_NORMALIZE_ALL);
	search_text_casefolded = g_utf8_casefold (search_text_normalized, -1);
	g_free (search_text_normalized);

	visible = strstr (language_name_casefolded, search_text_casefolded) != NULL;

	g_free (language_name_casefolded);
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
	g_return_if_fail (language != NULL);

	g_object_ref (language);
	g_signal_emit_by_name (chooser_widget, "language-activated", language);
	g_object_unref (language);
}

static void
tepl_language_chooser_widget_init (TeplLanguageChooserWidget *chooser_widget)
{
	GtkWidget *scrolled_window;

	chooser_widget->priv = tepl_language_chooser_widget_get_instance_private (chooser_widget);

	gtk_orientable_set_orientation (GTK_ORIENTABLE (chooser_widget), GTK_ORIENTATION_VERTICAL);

	chooser_widget->priv->search_entry = GTK_SEARCH_ENTRY (gtk_search_entry_new ());
	gtk_widget_show (GTK_WIDGET (chooser_widget->priv->search_entry));
	gtk_container_add (GTK_CONTAINER (chooser_widget), GTK_WIDGET (chooser_widget->priv->search_entry));

	chooser_widget->priv->list_box = GTK_LIST_BOX (gtk_list_box_new ());
	gtk_list_box_set_activate_on_single_click (chooser_widget->priv->list_box, FALSE);
	populate_list_box (chooser_widget);
	gtk_widget_set_hexpand (GTK_WIDGET (chooser_widget->priv->list_box), TRUE);
	gtk_widget_set_vexpand (GTK_WIDGET (chooser_widget->priv->list_box), TRUE);

	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
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
