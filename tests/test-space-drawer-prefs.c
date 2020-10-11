/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <tepl/tepl.h>
#include <stdlib.h>

typedef struct
{
	GtkWindow *window;
	TeplSpaceDrawerPrefs *space_drawer_prefs;
	GtkDialog *dialog;
	TeplTab *tab;
} SmallTest;

static void
init_space_drawer_prefs (SmallTest *self)
{
	g_assert (self->space_drawer_prefs == NULL);

	self->space_drawer_prefs = tepl_space_drawer_prefs_new ();
	gtk_widget_show (GTK_WIDGET (self->space_drawer_prefs));
}

static void
init_dialog (SmallTest *self)
{
	GtkWidget *content_area;

	g_assert (self->dialog == NULL);
	g_assert (self->window != NULL);
	g_assert (self->space_drawer_prefs != NULL);

	self->dialog = GTK_DIALOG (gtk_dialog_new_with_buttons ("Space Drawing Preferences",
								self->window,
								GTK_DIALOG_DESTROY_WITH_PARENT |
								GTK_DIALOG_USE_HEADER_BAR,
								NULL, NULL));

	content_area = gtk_dialog_get_content_area (self->dialog);
	gtk_container_add (GTK_CONTAINER (content_area), GTK_WIDGET (self->space_drawer_prefs));

	g_signal_connect (self->dialog,
			  "delete-event",
			  G_CALLBACK (gtk_widget_hide_on_delete),
			  NULL);
}

static void
init_tab (SmallTest *self)
{
	GtkSourceView *view;
	GtkSourceSpaceDrawer *space_drawer;
	GtkTextBuffer *buffer;

	g_assert (self->tab == NULL);

	self->tab = tepl_tab_new ();
	gtk_widget_show (GTK_WIDGET (self->tab));

	view = GTK_SOURCE_VIEW (tepl_tab_get_view (self->tab));
	gtk_text_view_set_monospace (GTK_TEXT_VIEW (view), TRUE);

	space_drawer = gtk_source_view_get_space_drawer (view);
	gtk_source_space_drawer_set_enable_matrix (space_drawer, TRUE);

	buffer = GTK_TEXT_BUFFER (tepl_tab_get_buffer (self->tab));
	gtk_text_buffer_set_text (buffer,
				  "\tTab\tTab\t\n"
				  " Space Space \n"
				  "\xC2\xA0No-Break Space\xC2\xA0No-Break Space\xC2\xA0\n"
				  "\xE2\x80\xAFNarrow No-Break Space\xE2\x80\xAFNarrow No-Break Space\xE2\x80\xAF",
				  -1);
}

static void
button_clicked_cb (GtkButton *button,
		   SmallTest *self)
{
	gtk_widget_show (GTK_WIDGET (self->dialog));
}

static GtkWidget *
create_button (SmallTest *self)
{
	GtkWidget *button;

	button = gtk_button_new_with_label ("Space drawing preferences");

	g_signal_connect (button,
			  "clicked",
			  G_CALLBACK (button_clicked_cb),
			  self);

	return button;
}

static GtkGrid *
create_main_vgrid (SmallTest *self)
{
	GtkGrid *main_vgrid;

	main_vgrid = GTK_GRID (gtk_grid_new ());
	gtk_orientable_set_orientation (GTK_ORIENTABLE (main_vgrid), GTK_ORIENTATION_VERTICAL);
	gtk_grid_set_row_spacing (main_vgrid, 6);

	g_object_set (main_vgrid,
		      "margin", 6,
		      NULL);

	gtk_container_add (GTK_CONTAINER (main_vgrid), create_button (self));
	gtk_container_add (GTK_CONTAINER (main_vgrid), GTK_WIDGET (self->tab));

	return main_vgrid;
}

static void
bind_matrix_properties (SmallTest *self)
{
	GtkSourceSpaceDrawer *prefs_space_drawer;
	GtkSourceView *view;
	GtkSourceSpaceDrawer *view_space_drawer;

	prefs_space_drawer = tepl_space_drawer_prefs_get_space_drawer (self->space_drawer_prefs);

	view = GTK_SOURCE_VIEW (tepl_tab_get_view (self->tab));
	view_space_drawer = gtk_source_view_get_space_drawer (view);

	g_object_bind_property (prefs_space_drawer, "matrix",
				view_space_drawer, "matrix",
				G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
}

static void
init_window (SmallTest *self)
{
	self->window = GTK_WINDOW (gtk_window_new (GTK_WINDOW_TOPLEVEL));
	gtk_window_set_default_size (self->window, 500, 500);
	g_signal_connect (self->window, "destroy", gtk_main_quit, NULL);

	init_space_drawer_prefs (self);
	init_dialog (self);
	init_tab (self);

	bind_matrix_properties (self);

	gtk_container_add (GTK_CONTAINER (self->window), GTK_WIDGET (create_main_vgrid (self)));
	gtk_widget_show_all (GTK_WIDGET (self->window));
}

int
main (int    argc,
      char **argv)
{
	SmallTest self = { NULL };

	gtk_init (&argc, &argv);
	init_window (&self);
	gtk_main ();

	return EXIT_SUCCESS;
}
