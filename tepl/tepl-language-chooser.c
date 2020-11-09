/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "tepl-language-chooser.h"

/**
 * SECTION:language-chooser
 * @Title: TeplLanguageChooser
 * @Short_description: Interface implemented by widgets for choosing a #GtkSourceLanguage
 *
 * A #TeplLanguageChooser widget shows a list of available #GtkSourceLanguage's,
 * as returned by gtk_source_language_manager_get_default(). The list contains a
 * "Plain Text" item for the %NULL language.
 *
 * The typical workflow when using a #TeplLanguageChooser widget in an
 * application is:
 * 1. Create a #TeplLanguageChooser widget.
 * 2. Call tepl_language_chooser_select_language() with the value of the
 *    #GtkSourceBuffer:language property.
 * 3. Wait for the #TeplLanguageChooser::language-activated signal to be emitted
 *    and/or destroy the #TeplLanguageChooser widget.
 */

/* API design:
 * Consistency with GtkSourceStyleSchemeChooser:
 * - The name, but applied for GtkSourceLanguage.
 * - The fact to have an interface with different implementations.
 *
 * Previous names, in gedit: GeditHighlightModeSelector for the widget,
 * GeditHighlightModeDialog for the dialog. Without an interface, the Selector
 * object (used by composition) was exposed in the API of the Dialog, to avoid
 * repeating the API.
 */

/* TODO: implement a TeplLanguageChooserButton class.
 * For TeplLanguageChooserButton, it would probably be more convenient to have
 * two properties instead of the ::language-activated signal: the :language and
 * :language-id properties (the :language-id property would be useful to bind it
 * to a GSettings key).
 * A TeplLanguageChooserButton would be useful to choose the default language
 * for new files, by adding the button in a preferences dialog.
 */

enum
{
	SIGNAL_LANGUAGE_ACTIVATED,
	N_SIGNALS
};

static guint signals[N_SIGNALS];

G_DEFINE_INTERFACE (TeplLanguageChooser, tepl_language_chooser, G_TYPE_OBJECT)

static void
tepl_language_chooser_select_language_default (TeplLanguageChooser *chooser,
					       GtkSourceLanguage   *language)
{
}

static void
tepl_language_chooser_default_init (TeplLanguageChooserInterface *interface)
{
	interface->select_language = tepl_language_chooser_select_language_default;

	/**
	 * TeplLanguageChooser::language-activated:
	 * @chooser: the #TeplLanguageChooser emitting the signal.
	 * @language: (nullable): the #GtkSourceLanguage object that has been
	 *   selected, or %NULL if "Plain Text" has been selected.
	 *
	 * This signal is emitted when the user has chosen a language.
	 *
	 * Since: 5.2
	 */
	/* Note about the signal name, it's to be consistent with
	 * ::row-activated.
	 */
	signals[SIGNAL_LANGUAGE_ACTIVATED] =
		g_signal_new ("language-activated",
			      G_TYPE_FROM_INTERFACE (interface),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (TeplLanguageChooserInterface, language_activated),
			      NULL, NULL, NULL,
			      G_TYPE_NONE, 1, GTK_SOURCE_TYPE_LANGUAGE);
}

/**
 * tepl_language_chooser_select_language:
 * @chooser: a #TeplLanguageChooser.
 * @language: (nullable): a #GtkSourceLanguage, or %NULL for "Plain Text".
 *
 * Selects @language in the list.
 *
 * Since: 5.2
 */
void
tepl_language_chooser_select_language (TeplLanguageChooser *chooser,
				       GtkSourceLanguage   *language)
{
	g_return_if_fail (TEPL_IS_LANGUAGE_CHOOSER (chooser));
	g_return_if_fail (language == NULL || GTK_SOURCE_IS_LANGUAGE (language));

	TEPL_LANGUAGE_CHOOSER_GET_INTERFACE (chooser)->select_language (chooser, language);
}
