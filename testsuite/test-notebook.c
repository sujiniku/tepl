/* SPDX-FileCopyrightText: 2017 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
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

		g_assert_true (expected_data == received_data);
	}

	g_assert_true (l1 == NULL);
	g_assert_true (l2 == NULL);
}

static void
test_tab_group_basic (void)
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
	g_assert_true (tepl_tab_group_get_tabs (tab_group) == NULL);
	g_assert_true (tepl_tab_group_get_active_tab (tab_group) == NULL);

	/* One child, but not a TeplTab */
	other_widget = gtk_grid_new ();
	gtk_widget_show (other_widget);
	gtk_notebook_append_page (notebook, other_widget, NULL);

	g_assert_cmpint (gtk_notebook_get_n_pages (notebook), ==, 1);
	g_assert_true (tepl_tab_group_get_tabs (tab_group) == NULL);
	g_assert_true (tepl_tab_group_get_active_tab (tab_group) == NULL);

	/* Append one TeplTab */
	tab1 = tepl_tab_new ();
	gtk_widget_show (GTK_WIDGET (tab1));
	tepl_tab_group_append_tab (tab_group, tab1, TRUE);
	expected_list = g_list_append (NULL, tab1);

	g_assert_cmpint (gtk_notebook_get_n_pages (notebook), ==, 2);
	g_assert_true (tepl_tab_group_get_active_tab (tab_group) == tab1);

	received_list = tepl_tab_group_get_tabs (tab_group);
	check_list_equal (expected_list, received_list);
	g_list_free (received_list);
	received_list = NULL;

	gtk_notebook_set_current_page (notebook, 0);
	g_assert_true (tepl_tab_group_get_active_tab (tab_group) == NULL);

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
	g_assert_true (tepl_tab_group_get_active_tab (tab_group) == NULL);

	tepl_tab_group_set_active_tab (tab_group, tab1);
	g_assert_true (tepl_tab_group_get_active_tab (tab_group) == tab1);

	tepl_tab_group_set_active_tab (tab_group, tab2);
	g_assert_true (tepl_tab_group_get_active_tab (tab_group) == tab2);

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

typedef struct
{
	gint active_tab_notify_delta_count;

	/* Must be always equal to active_tab_notify_delta_count. */
	gint active_view_notify_delta_count;

	gint active_buffer_notify_delta_count;
} NotifyDeltaCounters;

static void
check_notify_delta_counters (NotifyDeltaCounters *delta_counters,
			     gint                 expected_tab_delta,
			     gint                 expected_buffer_delta)
{
	g_assert_cmpint (delta_counters->active_tab_notify_delta_count, ==, expected_tab_delta);
	g_assert_cmpint (delta_counters->active_view_notify_delta_count, ==, expected_tab_delta);
	g_assert_cmpint (delta_counters->active_buffer_notify_delta_count, ==, expected_buffer_delta);

	delta_counters->active_tab_notify_delta_count = 0;
	delta_counters->active_view_notify_delta_count = 0;
	delta_counters->active_buffer_notify_delta_count = 0;
}

static void
notify_cb (TeplTabGroup *tab_group,
	   GParamSpec   *pspec,
	   gint         *counter)
{
	(*counter)++;
}

static void
change_buffer (TeplTab *tab)
{
	TeplView *view;
	TeplBuffer *new_buffer;

	view = tepl_tab_get_view (tab);
	new_buffer = tepl_buffer_new ();
	gtk_text_view_set_buffer (GTK_TEXT_VIEW (view), GTK_TEXT_BUFFER (new_buffer));
	g_object_unref (new_buffer);
}

static void
test_tab_group_notify_signals (void)
{
	GtkNotebook *notebook;
	TeplTabGroup *tab_group;
	TeplTab *tab1;
	TeplTab *tab2;
	TeplTab *tab3;
	NotifyDeltaCounters delta_counters = { 0, 0, 0 };

	notebook = GTK_NOTEBOOK (tepl_notebook_new ());
	tab_group = TEPL_TAB_GROUP (notebook);
	g_object_ref_sink (notebook);

	gtk_widget_show (GTK_WIDGET (notebook));

	g_signal_connect (tab_group,
			  "notify::active-tab",
			  G_CALLBACK (notify_cb),
			  &(delta_counters.active_tab_notify_delta_count));

	g_signal_connect (tab_group,
			  "notify::active-view",
			  G_CALLBACK (notify_cb),
			  &(delta_counters.active_view_notify_delta_count));

	g_signal_connect (tab_group,
			  "notify::active-buffer",
			  G_CALLBACK (notify_cb),
			  &(delta_counters.active_buffer_notify_delta_count));

	/* Create first tab. */
	tab1 = tepl_tab_new ();
	gtk_widget_show (GTK_WIDGET (tab1));
	tepl_tab_group_append_tab (tab_group, tab1, FALSE);
	check_notify_delta_counters (&delta_counters, 1, 1);

	/* For the first tab, GtkNotebook has already set it as the active tab. */
	tepl_tab_group_set_active_tab (tab_group, tab1);
	check_notify_delta_counters (&delta_counters, 0, 0);

	/* Change buffer */
	change_buffer (tab1);
	check_notify_delta_counters (&delta_counters, 0, 1);

	/* Remove tab -> active-tab is NULL. */
	gtk_widget_destroy (GTK_WIDGET (tab1));
	check_notify_delta_counters (&delta_counters, 1, 1);
	g_assert_true (tepl_tab_group_get_tabs (tab_group) == NULL);
	g_assert_true (tepl_tab_group_get_active_tab (tab_group) == NULL);

	/* Re-create first tab. */
	tab1 = tepl_tab_new ();
	gtk_widget_show (GTK_WIDGET (tab1));
	// With jump_to = TRUE this time.
	tepl_tab_group_append_tab (tab_group, tab1, TRUE);
	check_notify_delta_counters (&delta_counters, 1, 1);

	/* Append a second tab. */
	tab2 = tepl_tab_new ();
	gtk_widget_show (GTK_WIDGET (tab2));
	tepl_tab_group_append_tab (tab_group, tab2, FALSE);
	check_notify_delta_counters (&delta_counters, 0, 0);

	tepl_tab_group_set_active_tab (tab_group, tab2);
	check_notify_delta_counters (&delta_counters, 1, 1);

	/* Change buffer of tab1. */
	change_buffer (tab1);
	check_notify_delta_counters (&delta_counters, 0, 0);

	/* Change buffer of tab2. */
	change_buffer (tab2);
	check_notify_delta_counters (&delta_counters, 0, 1);

	/* Switch tabs */
	tepl_tab_group_set_active_tab (tab_group, tab1);
	check_notify_delta_counters (&delta_counters, 1, 1);

	tepl_tab_group_set_active_tab (tab_group, tab2);
	check_notify_delta_counters (&delta_counters, 1, 1);

	/* Reorder non-active tab */
	gtk_notebook_reorder_child (notebook, GTK_WIDGET (tab1), 1);
	check_notify_delta_counters (&delta_counters, 0, 0);
	g_assert_true (tepl_tab_group_get_active_tab (tab_group) == tab2);

	/* Reorder active tab.
	 * The order is reset to tab1 -> tab2.
	 */
	gtk_notebook_reorder_child (notebook, GTK_WIDGET (tab2), 1);
	check_notify_delta_counters (&delta_counters, 0, 0);
	g_assert_true (tepl_tab_group_get_active_tab (tab_group) == tab2);

	/* Append a third tab. */
	tab3 = tepl_tab_new ();
	gtk_widget_show (GTK_WIDGET (tab3));
	tepl_tab_group_append_tab (tab_group, tab3, FALSE);
	check_notify_delta_counters (&delta_counters, 0, 0);
	g_assert_true (tepl_tab_group_get_active_tab (tab_group) == tab2);

	/* Remove a non-active tab. */
	gtk_widget_destroy (GTK_WIDGET (tab1));
	check_notify_delta_counters (&delta_counters, 0, 0);

	/* Remove active tab. */
	gtk_widget_destroy (GTK_WIDGET (tab2));
	check_notify_delta_counters (&delta_counters, 1, 1);
	g_assert_true (tepl_tab_group_get_active_tab (tab_group) == tab3);

	g_object_unref (notebook);
}

int
main (int    argc,
      char **argv)
{
	gtk_test_init (&argc, &argv, NULL);

	g_test_add_func ("/notebook/tab-group-basic", test_tab_group_basic);
	g_test_add_func ("/notebook/tab-group-notify-signals", test_tab_group_notify_signals);

	return g_test_run ();
}
