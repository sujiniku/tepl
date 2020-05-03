/* Copyright 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "tepl-style-scheme-chooser-widget.h"
#include <gtksourceview/gtksource.h>

/**
 * SECTION:style-scheme-chooser-widget
 * @Title: TeplStyleSchemeChooserWidget
 * @Short_description: A simple #GtkSourceStyleSchemeChooser
 *
 * #TeplStyleSchemeChooserWidget is a simple implementation of the
 * #GtkSourceStyleSchemeChooser interface. It already contains a
 * #GtkScrolledWindow internally.
 *
 * Additional features compared to #GtkSourceStyleSchemeChooserWidget:
 * - There is an additional convenience property:
 *   #TeplStyleSchemeChooserWidget:tepl-style-scheme-id.
 * - When the #GtkWidget::map signal is emitted, #TeplStyleSchemeChooserWidget
 *   scrolls to the selected row.
 */

struct _TeplStyleSchemeChooserWidgetPrivate
{
	GtkListBox *list_box;
};

enum
{
	PROP_0,
	PROP_STYLE_SCHEME,
	PROP_TEPL_STYLE_SCHEME_ID
};

#define LIST_BOX_ROW_STYLE_SCHEME_KEY "style-scheme-key"

static void gtk_source_style_scheme_chooser_interface_init (gpointer g_iface,
							    gpointer iface_data);

G_DEFINE_TYPE_WITH_CODE (TeplStyleSchemeChooserWidget,
			 tepl_style_scheme_chooser_widget,
			 GTK_TYPE_BIN,
			 G_ADD_PRIVATE (TeplStyleSchemeChooserWidget)
			 G_IMPLEMENT_INTERFACE (GTK_SOURCE_TYPE_STYLE_SCHEME_CHOOSER,
						gtk_source_style_scheme_chooser_interface_init))

static void
clear_list_box_foreach_cb (GtkWidget *child,
			   gpointer   user_data)
{
	gtk_widget_destroy (child);
}

static void
clear_list_box (GtkListBox *list_box)
{
	gtk_container_foreach (GTK_CONTAINER (list_box),
			       clear_list_box_foreach_cb,
			       NULL);
}

static void
list_box_row_set_style_scheme (GtkListBoxRow        *list_box_row,
			       GtkSourceStyleScheme *style_scheme)
{
	g_object_set_data_full (G_OBJECT (list_box_row),
				LIST_BOX_ROW_STYLE_SCHEME_KEY,
				g_object_ref (style_scheme),
				g_object_unref);
}

/* Returns: (transfer none). */
static GtkSourceStyleScheme *
list_box_row_get_style_scheme (GtkListBoxRow *list_box_row)
{
	return g_object_get_data (G_OBJECT (list_box_row), LIST_BOX_ROW_STYLE_SCHEME_KEY);
}

static gboolean
style_scheme_equal (GtkSourceStyleScheme *style_scheme1,
		    GtkSourceStyleScheme *style_scheme2)
{
	const gchar *id1;
	const gchar *id2;

	if (style_scheme1 == style_scheme2)
	{
		return TRUE;
	}

	if (style_scheme1 == NULL || style_scheme2 == NULL)
	{
		return FALSE;
	}

	id1 = gtk_source_style_scheme_get_id (style_scheme1);
	id2 = gtk_source_style_scheme_get_id (style_scheme2);
	return g_strcmp0 (id1, id2) == 0;
}

static void
tepl_style_scheme_chooser_widget_get_property (GObject    *object,
                                               guint       prop_id,
                                               GValue     *value,
                                               GParamSpec *pspec)
{
	GtkSourceStyleSchemeChooser *gsv_chooser = GTK_SOURCE_STYLE_SCHEME_CHOOSER (object);
	TeplStyleSchemeChooserWidget *tepl_chooser = TEPL_STYLE_SCHEME_CHOOSER_WIDGET (object);

	switch (prop_id)
	{
		case PROP_STYLE_SCHEME:
			g_value_set_object (value, gtk_source_style_scheme_chooser_get_style_scheme (gsv_chooser));
			break;

		case PROP_TEPL_STYLE_SCHEME_ID:
			g_value_take_string (value, tepl_style_scheme_chooser_widget_get_style_scheme_id (tepl_chooser));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
tepl_style_scheme_chooser_widget_set_property (GObject      *object,
                                               guint         prop_id,
                                               const GValue *value,
                                               GParamSpec   *pspec)
{
	GtkSourceStyleSchemeChooser *gsv_chooser = GTK_SOURCE_STYLE_SCHEME_CHOOSER (object);
	TeplStyleSchemeChooserWidget *tepl_chooser = TEPL_STYLE_SCHEME_CHOOSER_WIDGET (object);

	switch (prop_id)
	{
		case PROP_STYLE_SCHEME:
			gtk_source_style_scheme_chooser_set_style_scheme (gsv_chooser, g_value_get_object (value));
			break;

		case PROP_TEPL_STYLE_SCHEME_ID:
			tepl_style_scheme_chooser_widget_set_style_scheme_id (tepl_chooser, g_value_get_string (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
tepl_style_scheme_chooser_widget_dispose (GObject *object)
{
	TeplStyleSchemeChooserWidget *chooser = TEPL_STYLE_SCHEME_CHOOSER_WIDGET (object);

	chooser->priv->list_box = NULL;

	G_OBJECT_CLASS (tepl_style_scheme_chooser_widget_parent_class)->dispose (object);
}

static void
scroll_to_row (GtkListBox    *list_box,
	       GtkListBoxRow *row)
{
	/* See also the call to gtk_container_set_focus_vadjustment() below. */
	gtk_container_set_focus_child (GTK_CONTAINER (list_box), GTK_WIDGET (row));
}

static void
scroll_to_selected_row (GtkListBox *list_box)
{
	GtkListBoxRow *selected_row = gtk_list_box_get_selected_row (list_box);

	if (selected_row != NULL)
	{
		scroll_to_row (list_box, selected_row);
	}
}

static void
tepl_style_scheme_chooser_widget_map (GtkWidget *widget)
{
	TeplStyleSchemeChooserWidget *chooser = TEPL_STYLE_SCHEME_CHOOSER_WIDGET (widget);

	if (GTK_WIDGET_CLASS (tepl_style_scheme_chooser_widget_parent_class)->map != NULL)
	{
		GTK_WIDGET_CLASS (tepl_style_scheme_chooser_widget_parent_class)->map (widget);
	}

	scroll_to_selected_row (chooser->priv->list_box);
}

static void
tepl_style_scheme_chooser_widget_class_init (TeplStyleSchemeChooserWidgetClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	object_class->get_property = tepl_style_scheme_chooser_widget_get_property;
	object_class->set_property = tepl_style_scheme_chooser_widget_set_property;
	object_class->dispose = tepl_style_scheme_chooser_widget_dispose;

	widget_class->map = tepl_style_scheme_chooser_widget_map;

	g_object_class_override_property (object_class, PROP_STYLE_SCHEME, "style-scheme");

	/**
	 * TeplStyleSchemeChooserWidget:tepl-style-scheme-id:
	 *
	 * The #GtkSourceStyleSchemeChooser:style-scheme ID, as a string. This
	 * property is useful for binding it to a #GSettings key.
	 *
	 * When the #GtkSourceStyleSchemeChooser:style-scheme is %NULL, this
	 * property contains the empty string.
	 *
	 * Since: 5.0
	 */
	g_object_class_install_property (object_class,
					 PROP_TEPL_STYLE_SCHEME_ID,
					 g_param_spec_string ("tepl-style-scheme-id",
							      "Tepl Style Scheme ID",
							      "",
							      "",
							      G_PARAM_READWRITE |
							      G_PARAM_STATIC_STRINGS));
}

static GtkSourceStyleScheme *
tepl_style_scheme_chooser_widget_get_style_scheme (GtkSourceStyleSchemeChooser *gsv_chooser)
{
	TeplStyleSchemeChooserWidget *chooser = TEPL_STYLE_SCHEME_CHOOSER_WIDGET (gsv_chooser);
	GtkListBoxRow *selected_row;

	selected_row = gtk_list_box_get_selected_row (chooser->priv->list_box);
	if (selected_row != NULL)
	{
		return list_box_row_get_style_scheme (selected_row);
	}

	return NULL;
}

static void
tepl_style_scheme_chooser_widget_set_style_scheme (GtkSourceStyleSchemeChooser *gsv_chooser,
						   GtkSourceStyleScheme        *style_scheme)
{
	TeplStyleSchemeChooserWidget *chooser = TEPL_STYLE_SCHEME_CHOOSER_WIDGET (gsv_chooser);
	GList *all_list_box_rows;
	GList *l;

	if (style_scheme == NULL)
	{
		return;
	}

	all_list_box_rows = gtk_container_get_children (GTK_CONTAINER (chooser->priv->list_box));

	for (l = all_list_box_rows; l != NULL; l = l->next)
	{
		GtkListBoxRow *cur_list_box_row = GTK_LIST_BOX_ROW (l->data);
		GtkSourceStyleScheme *cur_style_scheme;

		cur_style_scheme = list_box_row_get_style_scheme (cur_list_box_row);
		if (style_scheme_equal (cur_style_scheme, style_scheme))
		{
			gtk_list_box_select_row (chooser->priv->list_box, cur_list_box_row);
			scroll_to_row (chooser->priv->list_box, cur_list_box_row);
			break;
		}
	}

	g_list_free (all_list_box_rows);
}

static void
gtk_source_style_scheme_chooser_interface_init (gpointer g_iface,
						gpointer iface_data)
{
	GtkSourceStyleSchemeChooserInterface *interface = g_iface;

	interface->get_style_scheme = tepl_style_scheme_chooser_widget_get_style_scheme;
	interface->set_style_scheme = tepl_style_scheme_chooser_widget_set_style_scheme;
}

static void
append_style_scheme_to_list_box (TeplStyleSchemeChooserWidget *chooser,
				 GtkSourceStyleScheme         *style_scheme)
{
	const gchar *name;
	const gchar *description;
	gchar *markup;
	GtkWidget *label;
	GtkWidget *list_box_row;

	name = gtk_source_style_scheme_get_name (style_scheme);
	g_return_if_fail (name != NULL);
	description = gtk_source_style_scheme_get_description (style_scheme);

	if (description != NULL)
	{
		markup = g_markup_printf_escaped ("<b>%s</b> - %s", name, description);
	}
	else
	{
		markup = g_markup_printf_escaped ("<b>%s</b>", name);
	}

	label = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL (label), markup);
	gtk_widget_set_halign (label, GTK_ALIGN_START);

	list_box_row = gtk_list_box_row_new ();
	gtk_container_add (GTK_CONTAINER (list_box_row), label);
	list_box_row_set_style_scheme (GTK_LIST_BOX_ROW (list_box_row), style_scheme);
	gtk_widget_show_all (list_box_row);

	gtk_list_box_insert (chooser->priv->list_box, list_box_row, -1);

	g_free (markup);
}

static void
populate_list_box (TeplStyleSchemeChooserWidget *chooser)
{
	GtkSourceStyleSchemeManager *manager;
	const gchar * const *scheme_ids;
	gint i;

	manager = gtk_source_style_scheme_manager_get_default ();
	scheme_ids = gtk_source_style_scheme_manager_get_scheme_ids (manager);

	if (scheme_ids == NULL)
	{
		return;
	}

	for (i = 0; scheme_ids[i] != NULL; i++)
	{
		const gchar *cur_scheme_id = scheme_ids[i];
		GtkSourceStyleScheme *style_scheme;

		style_scheme = gtk_source_style_scheme_manager_get_scheme (manager, cur_scheme_id);
		append_style_scheme_to_list_box (chooser, style_scheme);
	}
}

static void
notify_properties (TeplStyleSchemeChooserWidget *chooser)
{
	g_object_notify (G_OBJECT (chooser), "style-scheme");
	g_object_notify (G_OBJECT (chooser), "tepl-style-scheme-id");
}

static void
list_box_selected_rows_changed_cb (GtkListBox                   *list_box,
				   TeplStyleSchemeChooserWidget *chooser)
{
	notify_properties (chooser);
}

static void
style_scheme_manager_notify_scheme_ids_cb (GtkSourceStyleSchemeManager  *manager,
					   GParamSpec                   *pspec,
					   TeplStyleSchemeChooserWidget *chooser)
{
	gchar *style_scheme_id;

	g_signal_handlers_block_by_func (chooser->priv->list_box,
					 list_box_selected_rows_changed_cb,
					 chooser);

	style_scheme_id = tepl_style_scheme_chooser_widget_get_style_scheme_id (chooser);

	clear_list_box (chooser->priv->list_box);
	populate_list_box (chooser);

	/* Note that the style_scheme_id may no longer exist, in which case no
	 * rows will be selected.
	 */
	tepl_style_scheme_chooser_widget_set_style_scheme_id (chooser, style_scheme_id);

	scroll_to_selected_row (chooser->priv->list_box);

	g_signal_handlers_unblock_by_func (chooser->priv->list_box,
					   list_box_selected_rows_changed_cb,
					   chooser);

	/* Notify properies in all cases, even if no rows are selected. */
	notify_properties (chooser);

	g_free (style_scheme_id);
}

static void
listen_to_scheme_manager_changes (TeplStyleSchemeChooserWidget *chooser)
{
	GtkSourceStyleSchemeManager *manager;

	manager = gtk_source_style_scheme_manager_get_default ();

	g_signal_connect_object (manager,
				 "notify::scheme-ids",
				 G_CALLBACK (style_scheme_manager_notify_scheme_ids_cb),
				 chooser,
				 0);
}

static void
tepl_style_scheme_chooser_widget_init (TeplStyleSchemeChooserWidget *chooser)
{
	GtkWidget *scrolled_window;
	GtkAdjustment *vadjustment;

	chooser->priv = tepl_style_scheme_chooser_widget_get_instance_private (chooser);

	chooser->priv->list_box = GTK_LIST_BOX (gtk_list_box_new ());
	gtk_list_box_set_selection_mode (chooser->priv->list_box, GTK_SELECTION_BROWSE);

	populate_list_box (chooser);
	listen_to_scheme_manager_changes (chooser);

	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_set_hexpand (scrolled_window, TRUE);
	gtk_widget_set_vexpand (scrolled_window, TRUE);

	/* Overlay scrolling gets in the way when trying to select the last row. */
	gtk_scrolled_window_set_overlay_scrolling (GTK_SCROLLED_WINDOW (scrolled_window), FALSE);

	gtk_container_add (GTK_CONTAINER (scrolled_window),
			   GTK_WIDGET (chooser->priv->list_box));
	gtk_widget_show_all (scrolled_window);
	gtk_container_add (GTK_CONTAINER (chooser), scrolled_window);

	vadjustment = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (scrolled_window));
	gtk_container_set_focus_vadjustment (GTK_CONTAINER (chooser->priv->list_box), vadjustment);

	g_signal_connect (chooser->priv->list_box,
			  "selected-rows-changed",
			  G_CALLBACK (list_box_selected_rows_changed_cb),
			  chooser);
}

/**
 * tepl_style_scheme_chooser_widget_new:
 *
 * Returns: (transfer floating): a new #TeplStyleSchemeChooserWidget.
 * Since: 5.0
 */
TeplStyleSchemeChooserWidget *
tepl_style_scheme_chooser_widget_new (void)
{
	return g_object_new (TEPL_TYPE_STYLE_SCHEME_CHOOSER_WIDGET, NULL);
}

/**
 * tepl_style_scheme_chooser_widget_get_style_scheme_id:
 * @chooser: a #TeplStyleSchemeChooserWidget.
 *
 * Returns: the value of the #TeplStyleSchemeChooserWidget:tepl-style-scheme-id
 * property. Free with g_free() when no longer needed.
 * Since: 5.0
 */
gchar *
tepl_style_scheme_chooser_widget_get_style_scheme_id (TeplStyleSchemeChooserWidget *chooser)
{
	GtkSourceStyleSchemeChooser *gsv_chooser;
	GtkSourceStyleScheme *style_scheme;
	const gchar *id;

	g_return_val_if_fail (TEPL_IS_STYLE_SCHEME_CHOOSER_WIDGET (chooser), g_strdup (""));

	gsv_chooser = GTK_SOURCE_STYLE_SCHEME_CHOOSER (chooser);
	style_scheme = gtk_source_style_scheme_chooser_get_style_scheme (gsv_chooser);

	if (style_scheme == NULL)
	{
		return g_strdup ("");
	}

	id = gtk_source_style_scheme_get_id (style_scheme);

	return id != NULL ? g_strdup (id) : g_strdup ("");
}

/**
 * tepl_style_scheme_chooser_widget_set_style_scheme_id:
 * @chooser: a #TeplStyleSchemeChooserWidget.
 * @style_scheme_id: the new value.
 *
 * Sets the #TeplStyleSchemeChooserWidget:tepl-style-scheme-id property.
 *
 * The #GtkSourceStyleScheme is taken from the default
 * #GtkSourceStyleSchemeManager as returned by
 * gtk_source_style_scheme_manager_get_default().
 *
 * Since: 5.0
 */
void
tepl_style_scheme_chooser_widget_set_style_scheme_id (TeplStyleSchemeChooserWidget *chooser,
						      const gchar                  *style_scheme_id)
{
	GtkSourceStyleSchemeManager *manager;
	GtkSourceStyleScheme *style_scheme;

	g_return_if_fail (TEPL_IS_STYLE_SCHEME_CHOOSER_WIDGET (chooser));
	g_return_if_fail (style_scheme_id != NULL);

	manager = gtk_source_style_scheme_manager_get_default ();
	style_scheme = gtk_source_style_scheme_manager_get_scheme (manager, style_scheme_id);

	if (style_scheme != NULL)
	{
		gtk_source_style_scheme_chooser_set_style_scheme (GTK_SOURCE_STYLE_SCHEME_CHOOSER (chooser),
								  style_scheme);
	}
}
