/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TEPL_LANGUAGE_CHOOSER_H
#define TEPL_LANGUAGE_CHOOSER_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gtksourceview/gtksource.h>
#include <tepl/tepl-macros.h>

G_BEGIN_DECLS

#define TEPL_TYPE_LANGUAGE_CHOOSER               (tepl_language_chooser_get_type ())
#define TEPL_LANGUAGE_CHOOSER(obj)               (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEPL_TYPE_LANGUAGE_CHOOSER, TeplLanguageChooser))
#define TEPL_IS_LANGUAGE_CHOOSER(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEPL_TYPE_LANGUAGE_CHOOSER))
#define TEPL_LANGUAGE_CHOOSER_GET_INTERFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), TEPL_TYPE_LANGUAGE_CHOOSER, TeplLanguageChooserInterface))

typedef struct _TeplLanguageChooser          TeplLanguageChooser;
typedef struct _TeplLanguageChooserInterface TeplLanguageChooserInterface;

/**
 * TeplLanguageChooserInterface:
 * @parent_interface: The parent interface.
 * @select_language: The virtual function pointer for tepl_language_chooser_select_language().
 *   By default, does nothing.
 * @language_activated: The function pointer for the
 *   #TeplLanguageChooser::language-activated signal.
 *
 * The virtual function table for #TeplLanguageChooser.
 *
 * Since: 5.2
 */
struct _TeplLanguageChooserInterface
{
	GTypeInterface parent_interface;

	/* Interface functions */
	void (* select_language)	(TeplLanguageChooser *chooser,
					 GtkSourceLanguage   *language);

	/* Signals */
	void (* language_activated)	(TeplLanguageChooser *chooser,
					 GtkSourceLanguage   *language);
};

_TEPL_EXTERN
GType	tepl_language_chooser_get_type		(void);

_TEPL_EXTERN
void	tepl_language_chooser_select_language	(TeplLanguageChooser *chooser,
						 GtkSourceLanguage   *language);

G_END_DECLS

#endif /* TEPL_LANGUAGE_CHOOSER_H */
