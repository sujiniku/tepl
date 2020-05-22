/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TEPL_TEST_UTILS_H
#define TEPL_TEST_UTILS_H

#include <gio/gio.h>

G_BEGIN_DECLS

typedef struct _TeplWaitSignalData TeplWaitSignalData;

void	_tepl_test_utils_set_file_content		(GFile       *file,
							 const gchar *content);

gchar *	_tepl_test_utils_get_file_content		(GFile *file);

void	_tepl_test_utils_check_file_content		(GFile       *file,
							 const gchar *expected_file_content);

void	_tepl_test_utils_check_equal_files_content	(GFile *file1,
							 GFile *file2);

TeplWaitSignalData *
	_tepl_test_utils_wait_signal_setup		(GObject     *object,
							 const gchar *detailed_signal_name);

void	_tepl_test_utils_wait_signal			(TeplWaitSignalData *data);

G_END_DECLS

#endif /* TEPL_TEST_UTILS_H */
