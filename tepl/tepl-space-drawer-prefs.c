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

	GtkCheckButton *check_button_leading_tabs;
	GtkCheckButton *check_button_leading_spaces;
	GtkCheckButton *check_button_inside_text_tabs;
	GtkCheckButton *check_button_inside_text_spaces;
	GtkCheckButton *check_button_trailing_tabs;
	GtkCheckButton *check_button_trailing_spaces;
	GtkCheckButton *check_button_newlines;
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

static void
tepl_space_drawer_prefs_init (TeplSpaceDrawerPrefs *prefs)
{
	prefs->priv = tepl_space_drawer_prefs_get_instance_private (prefs);

	gtk_orientable_set_orientation (GTK_ORIENTABLE (prefs), GTK_ORIENTATION_HORIZONTAL);
	gtk_grid_set_column_spacing (GTK_GRID (prefs), 12);

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
