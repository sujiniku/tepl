/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TEPL_TEST_UTILS_H
#define TEPL_TEST_UTILS_H

#include <gio/gio.h>

G_BEGIN_DECLS

void	_tepl_test_utils_set_file_content	(GFile       *file,
						 const gchar *content);

G_END_DECLS

#endif /* TEPL_TEST_UTILS_H */
