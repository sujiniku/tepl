/* SPDX-FileCopyrightText: 2016 - David Rabel <david.rabel@noresoft.com>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <tepl/tepl.h>

static GtkTextBuffer *
test_create_and_fill_buffer (guint lines)
{
	GtkTextBuffer *buffer;
	guint i;

	buffer = gtk_text_buffer_new (NULL);
	for (i = 0; i < lines; i++)
	{
		gtk_text_buffer_insert_at_cursor (buffer, "Another Line...\n", -1);
	}

	return buffer;
}

static TeplFoldRegion *
test_create_fold_region (GtkTextBuffer *buffer,
			 guint 		start_line,
			 guint 		end_line)
{
	GtkTextIter start_iter;
	GtkTextIter end_iter;

	gtk_text_buffer_get_iter_at_line (buffer, &start_iter, start_line);
	gtk_text_buffer_get_iter_at_line (buffer, &end_iter, end_line);

	return tepl_fold_region_new (buffer, &start_iter, &end_iter);
}

static guint
test_next_visible_line (GtkTextBuffer *buffer,
			guint 	       line)
{
	GtkTextIter iter;

	gtk_text_buffer_get_iter_at_line (buffer, &iter, line);
	gtk_text_iter_forward_visible_line (&iter);

	return gtk_text_iter_get_line (&iter);
}

static void
test_set_bounds_with_line_number (TeplFoldRegion *fold_region,
				  guint           start_line,
				  guint		  end_line)
{
	GtkTextIter start_iter;
	GtkTextIter end_iter;
	GtkTextBuffer *buffer;

	buffer = tepl_fold_region_get_buffer (fold_region);

	gtk_text_buffer_get_iter_at_line (buffer, &start_iter, start_line);
	gtk_text_buffer_get_iter_at_line (buffer, &end_iter, end_line);

	tepl_fold_region_set_bounds (fold_region, &start_iter, &end_iter);
}

static void
test_fold (void)
{
	GtkTextBuffer *buffer;
	TeplFoldRegion *fold_region;

	buffer = test_create_and_fill_buffer (6);

	fold_region = test_create_fold_region (buffer, 1, 3);
	tepl_fold_region_set_folded (fold_region, TRUE);

	g_assert_true (test_next_visible_line (buffer, 1) == 4);

	g_object_unref (fold_region);
	g_object_unref (buffer);
}

static void
test_unfold (void)
{
	GtkTextBuffer *buffer;
	TeplFoldRegion *fold_region;

	buffer = test_create_and_fill_buffer (6);

	fold_region = test_create_fold_region (buffer, 1, 3);
	tepl_fold_region_set_folded (fold_region, TRUE);
	tepl_fold_region_set_folded (fold_region, FALSE);

	g_assert_true (test_next_visible_line (buffer, 1) == 2);

	g_object_unref (fold_region);
	g_object_unref (buffer);
}

static void
test_toggle (void)
{
	GtkTextBuffer *buffer;
	TeplFoldRegion *fold_region;
	gboolean folded;

	buffer = test_create_and_fill_buffer (6);

	fold_region = test_create_fold_region (buffer, 1, 3);

	folded = tepl_fold_region_get_folded (fold_region);
	g_assert_true (folded == FALSE);

	folded = !folded;
	tepl_fold_region_set_folded (fold_region, folded);
	folded = tepl_fold_region_get_folded (fold_region);
	g_assert_true (folded == TRUE);

	folded = !folded;
	tepl_fold_region_set_folded (fold_region, folded);
	folded = tepl_fold_region_get_folded (fold_region);
	g_assert_true (folded == FALSE);

	g_object_unref (fold_region);
	g_object_unref (buffer);
}

static void
test_set_bounds (void)
{
	GtkTextBuffer *buffer;
	TeplFoldRegion *fold_region;

	buffer = test_create_and_fill_buffer (6);

	fold_region = test_create_fold_region (buffer, 1, 3);
	test_set_bounds_with_line_number (fold_region, 2, 4);
	tepl_fold_region_set_folded (fold_region, TRUE);

	g_assert_true (test_next_visible_line (buffer, 1) == 2);
	g_assert_true (test_next_visible_line (buffer, 2) == 5);

	g_object_unref (fold_region);
	g_object_unref (buffer);
}

static void
test_set_bounds_while_folded (void)
{
	GtkTextBuffer *buffer;
	TeplFoldRegion *fold_region;

	buffer = test_create_and_fill_buffer (6);

	fold_region = test_create_fold_region (buffer, 1, 3);
	tepl_fold_region_set_folded (fold_region, TRUE);
	test_set_bounds_with_line_number (fold_region, 2, 4);

	g_assert_true (test_next_visible_line (buffer, 1) == 2);
	g_assert_true (test_next_visible_line (buffer, 2) == 5);

	g_object_unref (fold_region);
	g_object_unref (buffer);
}

static void
test_get_bounds (void)
{
	GtkTextBuffer *buffer;
	TeplFoldRegion *fold_region;
	GtkTextIter start_iter;
	GtkTextIter end_iter;
	guint start_line;
	guint end_line;

	buffer = test_create_and_fill_buffer (6);

	fold_region = test_create_fold_region (buffer, 1, 3);
	tepl_fold_region_set_folded (fold_region, TRUE);

	tepl_fold_region_get_bounds (fold_region, &start_iter, &end_iter);
	start_line = gtk_text_iter_get_line (&start_iter);
	end_line = gtk_text_iter_get_line (&end_iter);

	g_assert_true (test_next_visible_line (buffer, start_line) == end_line + 1);

	g_object_unref (fold_region);
	g_object_unref (buffer);
}

static void
test_unref_while_folded (void)
{
	GtkTextBuffer *buffer;
	TeplFoldRegion *fold_region;

	buffer = test_create_and_fill_buffer (6);

	fold_region = test_create_fold_region (buffer, 1, 3);
	tepl_fold_region_set_folded (fold_region, TRUE);
	g_clear_object (&fold_region);

	g_assert_true (test_next_visible_line (buffer, 1) == 2);

	g_object_unref (buffer);
}

static void
test_clear_buffer (void)
{
	GtkTextBuffer *buffer;
	TeplFoldRegion *fold_region;
	GtkTextIter start_iter;
	GtkTextIter end_iter;

	buffer = test_create_and_fill_buffer (6);

	fold_region = test_create_fold_region (buffer, 1, 3);

	gtk_text_buffer_set_text (buffer, "", -1);

	tepl_fold_region_get_bounds (fold_region, &start_iter, &end_iter);

	g_assert_true (gtk_text_iter_get_line (&start_iter) == 0);
	g_assert_true (gtk_text_iter_get_line (&end_iter) == 0);

	g_object_unref (fold_region);
	g_object_unref (buffer);
}

static void
test_bounds_at_middle_of_line (void)
{
	GtkTextBuffer *buffer;
	TeplFoldRegion *fold_region;
	GtkTextIter start_iter;
	GtkTextIter end_iter;

	buffer = test_create_and_fill_buffer (6);

	gtk_text_buffer_get_iter_at_line (buffer, &start_iter, 1);
	gtk_text_buffer_get_iter_at_line (buffer, &end_iter, 3);

	gtk_text_iter_forward_chars (&start_iter, 3);
	gtk_text_iter_forward_chars (&end_iter, 3);

	fold_region = tepl_fold_region_new (buffer, &start_iter, &end_iter);

	tepl_fold_region_set_folded (fold_region, TRUE);
	g_assert_true (test_next_visible_line (buffer, 1) == 4);

	g_object_unref (fold_region);
	g_object_unref (buffer);
}

static void
test_bounds_at_end_of_line (void)
{
	GtkTextBuffer *buffer;
	TeplFoldRegion *fold_region;
	GtkTextIter start_iter;
	GtkTextIter end_iter;

	buffer = test_create_and_fill_buffer (6);

	gtk_text_buffer_get_iter_at_line (buffer, &start_iter, 1);
	gtk_text_buffer_get_iter_at_line (buffer, &end_iter, 3);

	gtk_text_iter_forward_line (&start_iter);
	gtk_text_iter_backward_char (&start_iter);
	gtk_text_iter_forward_line (&end_iter);
	gtk_text_iter_backward_char (&end_iter);

	fold_region = tepl_fold_region_new (buffer, &start_iter, &end_iter);

	tepl_fold_region_set_folded (fold_region, TRUE);
	g_assert_true (test_next_visible_line (buffer, 1) == 4);

	g_object_unref (fold_region);
	g_object_unref (buffer);
}

static void
test_double_fold (void)
{
	GtkTextBuffer *buffer;
	TeplFoldRegion *fold_region;

	buffer = test_create_and_fill_buffer (6);

	fold_region = test_create_fold_region (buffer, 1, 3);
	tepl_fold_region_set_folded (fold_region, TRUE);
	tepl_fold_region_set_folded (fold_region, TRUE);

	g_assert_true (test_next_visible_line (buffer, 1) == 4);

	tepl_fold_region_set_folded (fold_region, FALSE);

	g_assert_true (test_next_visible_line (buffer, 1) == 2);

	g_object_unref (fold_region);
	g_object_unref (buffer);
}

static void
test_double_unfold (void)
{
	GtkTextBuffer *buffer;
	TeplFoldRegion *fold_region;

	buffer = test_create_and_fill_buffer (6);

	fold_region = test_create_fold_region (buffer, 1, 3);
	tepl_fold_region_set_folded (fold_region, TRUE);
	tepl_fold_region_set_folded (fold_region, FALSE);
	tepl_fold_region_set_folded (fold_region, FALSE);

	g_assert_true (test_next_visible_line (buffer, 1) == 2);

	tepl_fold_region_set_folded (fold_region, TRUE);

	g_assert_true (test_next_visible_line (buffer, 1) == 4);

	g_object_unref (fold_region);
	g_object_unref (buffer);
}

static void
test_overlapping_regions (void)
{
	GtkTextBuffer *buffer;
	TeplFoldRegion *fold_region1;
	TeplFoldRegion *fold_region2;

	buffer = test_create_and_fill_buffer (6);

	fold_region1 = test_create_fold_region (buffer, 1, 3);
	fold_region2 = test_create_fold_region (buffer, 2, 4);

	tepl_fold_region_set_folded (fold_region1, TRUE);
	g_assert_true (test_next_visible_line (buffer, 1) == 4);
	tepl_fold_region_set_folded (fold_region2, TRUE);
	g_assert_true (test_next_visible_line (buffer, 1) == 5);

	tepl_fold_region_set_folded (fold_region1, FALSE);
	tepl_fold_region_set_folded (fold_region2, FALSE);

	tepl_fold_region_set_folded (fold_region2, TRUE);
	g_assert_true (test_next_visible_line (buffer, 2) == 5);
	tepl_fold_region_set_folded (fold_region1, TRUE);
	g_assert_true (test_next_visible_line (buffer, 1) == 5);

	tepl_fold_region_set_folded (fold_region1, FALSE);
	g_assert_true (test_next_visible_line (buffer, 2) == 5);
	tepl_fold_region_set_folded (fold_region2, FALSE);
	g_assert_true (test_next_visible_line (buffer, 2) == 3);

	tepl_fold_region_set_folded (fold_region1, TRUE);
	tepl_fold_region_set_folded (fold_region2, TRUE);

	tepl_fold_region_set_folded (fold_region2, FALSE);
	g_assert_true (test_next_visible_line (buffer, 1) == 4);
	tepl_fold_region_set_folded (fold_region1, FALSE);
	g_assert_true (test_next_visible_line (buffer, 2) == 3);

	g_object_unref (fold_region1);
	g_object_unref (fold_region2);
	g_object_unref (buffer);
}

static void
test_call_other_methods_before_set_bounds (void)
{
	GtkTextBuffer *buffer;
	TeplFoldRegion *fold_region;
	GtkTextIter start_iter;
	GtkTextIter end_iter;

	buffer = test_create_and_fill_buffer (6);

	fold_region = g_object_new (TEPL_TYPE_FOLD_REGION,
				    "buffer", buffer,
				    NULL);

	tepl_fold_region_set_folded (fold_region, TRUE);
	tepl_fold_region_set_folded (fold_region, FALSE);
	tepl_fold_region_get_folded (fold_region);
	tepl_fold_region_get_buffer (fold_region);
	tepl_fold_region_get_bounds (fold_region, &start_iter, &end_iter);

	g_object_unref (fold_region);
	g_object_unref (buffer);
}

gint
main (gint    argc,
      gchar **argv)
{
	gtk_test_init (&argc, &argv, NULL);

	g_test_add_func ("/fold-region/fold", test_fold);
	g_test_add_func ("/fold-region/unfold", test_unfold);
	g_test_add_func ("/fold-region/toggle", test_toggle);
	g_test_add_func ("/fold-region/set_bounds", test_set_bounds);
	g_test_add_func ("/fold-region/set_bounds_while_folded", test_set_bounds_while_folded);
	g_test_add_func ("/fold-region/get_bounds", test_get_bounds);
	g_test_add_func ("/fold-region/unref_while_folded", test_unref_while_folded);
	g_test_add_func ("/fold-region/clear_buffer", test_clear_buffer);
	g_test_add_func ("/fold-region/bounds_at_middle_of_line", test_bounds_at_middle_of_line);
	g_test_add_func ("/fold-region/bounds_at_end_of_line", test_bounds_at_end_of_line);
	g_test_add_func ("/fold-region/double_fold", test_double_fold);
	g_test_add_func ("/fold-region/double_unfold", test_double_unfold);
	g_test_add_func ("/fold-region/overlapping_regions", test_overlapping_regions);
	g_test_add_func ("/fold-region/call_other_methods_before_set_bounds", test_call_other_methods_before_set_bounds);

	return g_test_run ();
}
