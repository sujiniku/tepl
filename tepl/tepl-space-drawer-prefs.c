/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "config.h"
#include "tepl-space-drawer-prefs.h"
#include <glib/gi18n-lib.h>

/**
 * SECTION:space-drawer-prefs
 * @Short_description: Preferences widget for #GtkSourceSpaceDrawer
 * @Title: TeplSpaceDrawerPrefs
 *
 * #TeplSpaceDrawerPrefs is a #GtkWidget for configuring the preferences about
 * white space drawing with #GtkSourceSpaceDrawer.
 *
 * The configuration is stored in the #GtkSourceSpaceDrawer:matrix property of
 * the associated #GtkSourceSpaceDrawer object.
 */

struct _TeplSpaceDrawerPrefsPrivate
{
	/* Owned */
	GtkSourceSpaceDrawer *space_drawer;

	/* First column */
	GtkCheckButton *check_button_leading_tabs;
	GtkCheckButton *check_button_leading_spaces;
	GtkCheckButton *check_button_inside_text_tabs;
	GtkCheckButton *check_button_inside_text_spaces;
	GtkCheckButton *check_button_trailing_tabs;
	GtkCheckButton *check_button_trailing_spaces;
	GtkCheckButton *check_button_newlines;

	/* Second column */
	GtkGrid *second_column_vgrid;
};

G_DEFINE_TYPE_WITH_PRIVATE (TeplSpaceDrawerPrefs, tepl_space_drawer_prefs, GTK_TYPE_GRID)

static void matrix_notify_cb (GtkSourceSpaceDrawer *space_drawer,
			      GParamSpec           *pspec,
			      TeplSpaceDrawerPrefs *prefs);

static void
tepl_space_drawer_prefs_dispose (GObject *object)
{
	TeplSpaceDrawerPrefs *prefs = TEPL_SPACE_DRAWER_PREFS (object);

	g_clear_object (&prefs->priv->space_drawer);

	G_OBJECT_CLASS (tepl_space_drawer_prefs_parent_class)->dispose (object);
}

static void
tepl_space_drawer_prefs_class_init (TeplSpaceDrawerPrefsClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = tepl_space_drawer_prefs_dispose;
}

static void
set_matrix_state_according_to_check_buttons (TeplSpaceDrawerPrefs *prefs)
{
	GtkSourceSpaceTypeFlags space_types;

	g_signal_handlers_block_by_func (prefs->priv->space_drawer, matrix_notify_cb, prefs);

	space_types = GTK_SOURCE_SPACE_TYPE_NBSP;
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (prefs->priv->check_button_leading_tabs)))
	{
		space_types |= GTK_SOURCE_SPACE_TYPE_TAB;
	}
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (prefs->priv->check_button_leading_spaces)))
	{
		space_types |= GTK_SOURCE_SPACE_TYPE_SPACE;
	}
	gtk_source_space_drawer_set_types_for_locations (prefs->priv->space_drawer,
							 GTK_SOURCE_SPACE_LOCATION_LEADING,
							 space_types);

	space_types = GTK_SOURCE_SPACE_TYPE_NBSP;
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (prefs->priv->check_button_inside_text_tabs)))
	{
		space_types |= GTK_SOURCE_SPACE_TYPE_TAB;
	}
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (prefs->priv->check_button_inside_text_spaces)))
	{
		space_types |= GTK_SOURCE_SPACE_TYPE_SPACE;
	}
	gtk_source_space_drawer_set_types_for_locations (prefs->priv->space_drawer,
							 GTK_SOURCE_SPACE_LOCATION_INSIDE_TEXT,
							 space_types);

	space_types = GTK_SOURCE_SPACE_TYPE_NBSP;
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (prefs->priv->check_button_trailing_tabs)))
	{
		space_types |= GTK_SOURCE_SPACE_TYPE_TAB;
	}
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (prefs->priv->check_button_trailing_spaces)))
	{
		space_types |= GTK_SOURCE_SPACE_TYPE_SPACE;
	}
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (prefs->priv->check_button_newlines)))
	{
		space_types |= GTK_SOURCE_SPACE_TYPE_NEWLINE;
	}
	gtk_source_space_drawer_set_types_for_locations (prefs->priv->space_drawer,
							 GTK_SOURCE_SPACE_LOCATION_TRAILING,
							 space_types);

	g_signal_handlers_unblock_by_func (prefs->priv->space_drawer, matrix_notify_cb, prefs);
}

static void
check_button_toggled_cb (GtkCheckButton       *check_button,
			 TeplSpaceDrawerPrefs *prefs)
{
	set_matrix_state_according_to_check_buttons (prefs);
}

static void
set_check_button_state (TeplSpaceDrawerPrefs *prefs,
			GtkCheckButton       *check_button,
			gboolean              is_active)
{
	g_signal_handlers_block_by_func (check_button, check_button_toggled_cb, prefs);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button), is_active);
	g_signal_handlers_unblock_by_func (check_button, check_button_toggled_cb, prefs);
}

static void
set_check_buttons_state_according_to_matrix (TeplSpaceDrawerPrefs *prefs)
{
	GtkSourceSpaceTypeFlags space_types;

	space_types = gtk_source_space_drawer_get_types_for_locations (prefs->priv->space_drawer,
								       GTK_SOURCE_SPACE_LOCATION_LEADING);
	set_check_button_state (prefs,
				prefs->priv->check_button_leading_tabs,
				space_types & GTK_SOURCE_SPACE_TYPE_TAB);
	set_check_button_state (prefs,
				prefs->priv->check_button_leading_spaces,
				space_types & GTK_SOURCE_SPACE_TYPE_SPACE);

	space_types = gtk_source_space_drawer_get_types_for_locations (prefs->priv->space_drawer,
								       GTK_SOURCE_SPACE_LOCATION_INSIDE_TEXT);
	set_check_button_state (prefs,
				prefs->priv->check_button_inside_text_tabs,
				space_types & GTK_SOURCE_SPACE_TYPE_TAB);
	set_check_button_state (prefs,
				prefs->priv->check_button_inside_text_spaces,
				space_types & GTK_SOURCE_SPACE_TYPE_SPACE);

	space_types = gtk_source_space_drawer_get_types_for_locations (prefs->priv->space_drawer,
								       GTK_SOURCE_SPACE_LOCATION_TRAILING);
	set_check_button_state (prefs,
				prefs->priv->check_button_trailing_tabs,
				space_types & GTK_SOURCE_SPACE_TYPE_TAB);
	set_check_button_state (prefs,
				prefs->priv->check_button_trailing_spaces,
				space_types & GTK_SOURCE_SPACE_TYPE_SPACE);
	set_check_button_state (prefs,
				prefs->priv->check_button_newlines,
				space_types & GTK_SOURCE_SPACE_TYPE_NEWLINE);
}

static GtkCheckButton *
create_check_button (TeplSpaceDrawerPrefs *prefs,
		     const gchar          *label)
{
	GtkCheckButton *check_button;

	check_button = GTK_CHECK_BUTTON (gtk_check_button_new_with_label (label));
	gtk_widget_set_margin_start (GTK_WIDGET (check_button), 12);

	g_signal_connect (check_button,
			  "toggled",
			  G_CALLBACK (check_button_toggled_cb),
			  prefs);

	return check_button;
}

static void
init_check_buttons (TeplSpaceDrawerPrefs *prefs)
{
	prefs->priv->check_button_leading_tabs = create_check_button (prefs, _("Draw tabs"));
	prefs->priv->check_button_leading_spaces = create_check_button (prefs, _("Draw spaces"));
	prefs->priv->check_button_inside_text_tabs = create_check_button (prefs, _("Draw tabs"));
	prefs->priv->check_button_inside_text_spaces = create_check_button (prefs, _("Draw spaces"));
	prefs->priv->check_button_trailing_tabs = create_check_button (prefs, _("Draw tabs"));
	prefs->priv->check_button_trailing_spaces = create_check_button (prefs, _("Draw spaces"));
	prefs->priv->check_button_newlines = create_check_button (prefs, _("Draw new lines"));

	set_check_buttons_state_according_to_matrix (prefs);
}

static GtkWidget *
create_subtitle_label (const gchar *str)
{
	gchar *str_escaped;
	gchar *str_in_bold;
	GtkWidget *label;

	str_escaped = g_markup_escape_text (str, -1);
	str_in_bold = g_strdup_printf ("<b>%s</b>", str_escaped);

	label = gtk_label_new (str_in_bold);
	gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
	gtk_widget_set_halign (label, GTK_ALIGN_START);

	g_free (str_escaped);
	g_free (str_in_bold);

	return label;
}

static void
add_check_buttons (TeplSpaceDrawerPrefs *prefs)
{
	GtkContainer *vgrid;

	init_check_buttons (prefs);

	vgrid = GTK_CONTAINER (gtk_grid_new ());
	gtk_orientable_set_orientation (GTK_ORIENTABLE (vgrid), GTK_ORIENTATION_VERTICAL);
	gtk_grid_set_row_spacing (GTK_GRID (vgrid), 6);

	gtk_container_add (vgrid, create_subtitle_label (_("Leading Spaces")));
	gtk_container_add (vgrid, GTK_WIDGET (prefs->priv->check_button_leading_tabs));
	gtk_container_add (vgrid, GTK_WIDGET (prefs->priv->check_button_leading_spaces));

	gtk_container_add (vgrid, create_subtitle_label (_("Spaces Inside Text")));
	gtk_container_add (vgrid, GTK_WIDGET (prefs->priv->check_button_inside_text_tabs));
	gtk_container_add (vgrid, GTK_WIDGET (prefs->priv->check_button_inside_text_spaces));

	gtk_container_add (vgrid, create_subtitle_label (_("Trailing Spaces")));
	gtk_container_add (vgrid, GTK_WIDGET (prefs->priv->check_button_trailing_tabs));
	gtk_container_add (vgrid, GTK_WIDGET (prefs->priv->check_button_trailing_spaces));
	gtk_container_add (vgrid, GTK_WIDGET (prefs->priv->check_button_newlines));

	gtk_widget_show_all (GTK_WIDGET (vgrid));
	gtk_container_add (GTK_CONTAINER (prefs), GTK_WIDGET (vgrid));
}

static void
matrix_notify_cb (GtkSourceSpaceDrawer *space_drawer,
		  GParamSpec           *pspec,
		  TeplSpaceDrawerPrefs *prefs)
{
	set_check_buttons_state_according_to_matrix (prefs);
}

static gchar *
result_viewer_get_buffer_content (void)
{
	const gchar *tab_desc = _("Tab");
	const gchar *space_desc = _("Space");
	const gchar *nbsp_desc = _("No-Break Space");
	const gchar *narrow_nbsp_desc = _("Narrow No-Break Space");

	return g_strconcat ("\t", tab_desc, "\t", tab_desc, "\t\n",
			    " ", space_desc, " ", space_desc, " \n",
			    "\xC2\xA0", nbsp_desc, "\xC2\xA0", nbsp_desc, "\xC2\xA0\n",
			    "\xE2\x80\xAF", narrow_nbsp_desc, "\xE2\x80\xAF", narrow_nbsp_desc, "\xE2\x80\xAF",
			    NULL);
}

static void
add_result_viewer (TeplSpaceDrawerPrefs *prefs)
{
	GtkSourceView *view;
	GtkTextBuffer *buffer;
	gchar *buffer_content;
	GtkSourceSpaceDrawer *space_drawer;
	GtkWidget *scrolled_window;

	gtk_container_add (GTK_CONTAINER (prefs->priv->second_column_vgrid),
			   create_subtitle_label (_("Result")));

	view = GTK_SOURCE_VIEW (gtk_source_view_new ());
	gtk_source_view_set_show_line_numbers (view, TRUE);
	gtk_text_view_set_editable (GTK_TEXT_VIEW (view), FALSE);
	gtk_text_view_set_monospace (GTK_TEXT_VIEW (view), TRUE);

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
	buffer_content = result_viewer_get_buffer_content ();
	gtk_text_buffer_set_text (buffer, buffer_content, -1);
	g_free (buffer_content);

	space_drawer = gtk_source_view_get_space_drawer (view);
	gtk_source_space_drawer_set_enable_matrix (space_drawer, TRUE);
	g_object_bind_property (prefs->priv->space_drawer, "matrix",
				space_drawer, "matrix",
				G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);

	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_set_size_request (scrolled_window, 500, 120);
	gtk_widget_set_margin_start (scrolled_window, 12);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window), GTK_SHADOW_IN);
	gtk_scrolled_window_set_overlay_scrolling (GTK_SCROLLED_WINDOW (scrolled_window), FALSE);
	gtk_container_add (GTK_CONTAINER (scrolled_window), GTK_WIDGET (view));
	gtk_container_add (GTK_CONTAINER (prefs->priv->second_column_vgrid), scrolled_window);
}

static void
add_information (TeplSpaceDrawerPrefs *prefs)
{
	GtkLabel *label;

	gtk_container_add (GTK_CONTAINER (prefs->priv->second_column_vgrid),
			   create_subtitle_label (_("Information")));

	label = GTK_LABEL (gtk_label_new (_("When white space drawing is enabled, then non-breaking "
					    "spaces are always drawn at all locations, to distinguish "
					    "them from normal spaces.")));
	gtk_widget_set_margin_start (GTK_WIDGET (label), 12);
	gtk_widget_set_halign (GTK_WIDGET (label), GTK_ALIGN_START);
	gtk_label_set_xalign (label, 0.0);
	gtk_label_set_line_wrap (label, TRUE);
	gtk_label_set_selectable (label, TRUE);
	gtk_label_set_max_width_chars (label, 60);
	gtk_container_add (GTK_CONTAINER (prefs->priv->second_column_vgrid), GTK_WIDGET (label));
}

static void
tepl_space_drawer_prefs_init (TeplSpaceDrawerPrefs *prefs)
{
	prefs->priv = tepl_space_drawer_prefs_get_instance_private (prefs);

	gtk_orientable_set_orientation (GTK_ORIENTABLE (prefs), GTK_ORIENTATION_HORIZONTAL);
	gtk_grid_set_column_spacing (GTK_GRID (prefs), 24);

	g_object_set (prefs,
		      "margin", 6,
		      NULL);

	prefs->priv->space_drawer = gtk_source_space_drawer_new ();
	gtk_source_space_drawer_set_enable_matrix (prefs->priv->space_drawer, TRUE);
	gtk_source_space_drawer_set_types_for_locations (prefs->priv->space_drawer,
							 GTK_SOURCE_SPACE_LOCATION_ALL,
							 GTK_SOURCE_SPACE_TYPE_ALL &
							 ~GTK_SOURCE_SPACE_TYPE_NEWLINE);

	add_check_buttons (prefs);

	g_signal_connect_object (prefs->priv->space_drawer,
				 "notify::matrix",
				 G_CALLBACK (matrix_notify_cb),
				 prefs,
				 0);

	prefs->priv->second_column_vgrid = GTK_GRID (gtk_grid_new ());
	gtk_orientable_set_orientation (GTK_ORIENTABLE (prefs->priv->second_column_vgrid),
					GTK_ORIENTATION_VERTICAL);
	gtk_grid_set_row_spacing (prefs->priv->second_column_vgrid, 6);
	gtk_container_add (GTK_CONTAINER (prefs), GTK_WIDGET (prefs->priv->second_column_vgrid));
	add_result_viewer (prefs);
	add_information (prefs);
	gtk_widget_show_all (GTK_WIDGET (prefs->priv->second_column_vgrid));
}

/**
 * tepl_space_drawer_prefs_new:
 *
 * Returns: (transfer floating): a new #TeplSpaceDrawerPrefs.
 * Since: 5.2
 */
TeplSpaceDrawerPrefs *
tepl_space_drawer_prefs_new (void)
{
	return g_object_new (TEPL_TYPE_SPACE_DRAWER_PREFS, NULL);
}

/**
 * tepl_space_drawer_prefs_get_space_drawer:
 * @prefs: a #TeplSpaceDrawerPrefs.
 *
 * Gets the #GtkSourceSpaceDrawer associated with @prefs. The returned object is
 * guaranteed to be the same for the lifetime of @prefs. Each
 * #TeplSpaceDrawerPrefs object has a different #GtkSourceSpaceDrawer.
 *
 * Returns: (transfer none): the #GtkSourceSpaceDrawer associated with @prefs.
 * Since: 5.2
 */
GtkSourceSpaceDrawer *
tepl_space_drawer_prefs_get_space_drawer (TeplSpaceDrawerPrefs *prefs)
{
	g_return_val_if_fail (TEPL_IS_SPACE_DRAWER_PREFS (prefs), NULL);

	return prefs->priv->space_drawer;
}
