/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2017 - Sébastien Wilmet <swilmet@gnome.org>
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

#include <tepl/tepl.h>

static void
check_list_equal (GList *expected_list,
		  GList *received_list)
{
	GList *l1;
	GList *l2;

	g_assert_cmpint (g_list_length (expected_list), ==, g_list_length (received_list));

	for (l1 = expected_list, l2 = received_list;
	     l1 != NULL && l2 != NULL;
	     l1 = l1->next, l2 = l2->next)
	{
		gpointer expected_data = l1->data;
		gpointer received_data = l2->data;

		g_assert (expected_data == received_data);
	}

	g_assert (l1 == NULL);
	g_assert (l2 == NULL);
}

static void
test_tab_group (void)
{
	GtkNotebook *notebook;
	TeplTabGroup *tab_group;
	GtkWidget *other_widget;
	TeplTab *tab1;
	TeplTab *tab2;
	GList *expected_list;
	GList *received_list;

	notebook = GTK_NOTEBOOK (tepl_notebook_new ());
	tab_group = TEPL_TAB_GROUP (notebook);
	g_object_ref_sink (notebook);

	gtk_widget_show (GTK_WIDGET (notebook));

	/* Empty */
	g_assert (tepl_tab_group_get_tabs (tab_group) == NULL);
	g_assert (tepl_tab_group_get_active_tab (tab_group) == NULL);

	/* One child, but not a TeplTab */
	other_widget = gtk_grid_new ();
	gtk_widget_show (other_widget);
	gtk_notebook_append_page (notebook, other_widget, NULL);

	g_assert_cmpint (gtk_notebook_get_n_pages (notebook), ==, 1);
	g_assert (tepl_tab_group_get_tabs (tab_group) == NULL);
	g_assert (tepl_tab_group_get_active_tab (tab_group) == NULL);

	/* Append one TeplTab */
	tab1 = tepl_tab_new ();
	gtk_widget_show (GTK_WIDGET (tab1));
	tepl_tab_group_append_tab (tab_group, tab1, TRUE);
	expected_list = g_list_append (NULL, tab1);

	g_assert_cmpint (gtk_notebook_get_n_pages (notebook), ==, 2);
	g_assert (tepl_tab_group_get_active_tab (tab_group) == tab1);

	received_list = tepl_tab_group_get_tabs (tab_group);
	check_list_equal (expected_list, received_list);
	g_list_free (received_list);
	received_list = NULL;

	gtk_notebook_set_current_page (notebook, 0);
	g_assert (tepl_tab_group_get_active_tab (tab_group) == NULL);

	received_list = tepl_tab_group_get_tabs (tab_group);
	check_list_equal (expected_list, received_list);
	g_list_free (received_list);
	received_list = NULL;

	/* Append a second TeplTab */
	tab2 = tepl_tab_new ();
	gtk_widget_show (GTK_WIDGET (tab2));
	tepl_tab_group_append_tab (tab_group, tab2, FALSE);
	expected_list = g_list_append (expected_list, tab2);

	gtk_notebook_set_current_page (notebook, gtk_notebook_page_num (notebook, other_widget));
	g_assert (tepl_tab_group_get_active_tab (tab_group) == NULL);

	tepl_tab_group_set_active_tab (tab_group, tab1);
	g_assert (tepl_tab_group_get_active_tab (tab_group) == tab1);

	tepl_tab_group_set_active_tab (tab_group, tab2);
	g_assert (tepl_tab_group_get_active_tab (tab_group) == tab2);

	received_list = tepl_tab_group_get_tabs (tab_group);
	check_list_equal (expected_list, received_list);
	g_list_free (received_list);
	received_list = NULL;

	/* Move tab2 before tab1 */
	gtk_notebook_reorder_child (notebook, GTK_WIDGET (tab2), 0);
	expected_list = g_list_reverse (expected_list);

	received_list = tepl_tab_group_get_tabs (tab_group);
	check_list_equal (expected_list, received_list);
	g_list_free (received_list);
	received_list = NULL;

	g_list_free (expected_list);
	g_object_unref (notebook);
}

int
main (int    argc,
      char **argv)
{
	gtk_test_init (&argc, &argv, NULL);

	g_test_add_func ("/notebook/tab-group", test_tab_group);

	return g_test_run ();
}
