/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "tepl-language-chooser.h"

/**
 * SECTION:language-chooser
 * @Title: TeplLanguageChooser
 * @Short_description: Interface implemented by widgets for choosing a #GtkSourceLanguage
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

	/* FIXME: do like in GtkSourceCompletionProposal, have an initialized
	 * gboolean variable? To ensure to init the signal only once.
	 */
	/**
	 * TeplLanguageChooser::language-activated:
	 * @chooser: the #TeplLanguageChooser emitting the signal.
	 * @language: the #GtkSourceLanguage object that has been selected.
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
 * @language: (nullable): a #GtkSourceLanguage, or %NULL.
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
