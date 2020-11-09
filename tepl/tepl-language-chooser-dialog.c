/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "config.h"
#include "tepl-language-chooser-dialog.h"
#include <glib/gi18n-lib.h>
#include "tepl-language-chooser.h"
#include "tepl-language-chooser-widget.h"

/**
 * SECTION:language-chooser-dialog
 * @Title: TeplLanguageChooserDialog
 * @Short_description: A dialog for choosing a #GtkSourceLanguage
 *
 * #TeplLanguageChooserDialog is a #GtkDialog to choose a #GtkSourceLanguage.
 * #TeplLanguageChooserDialog implements the #TeplLanguageChooser interface.
 *
 * The #GtkDialog contains two buttons: Cancel and Select. During the
 * #GtkDialog::response signal emission, if the user has clicked on the Select
 * button, then the #TeplLanguageChooser::language-activated signal is emitted.
 *
 * You probably want to connect to the #GtkDialog::response signal with the
 * %G_CONNECT_AFTER flag, to destroy the dialog.
 */

struct _TeplLanguageChooserDialogPrivate
{
	TeplLanguageChooserWidget *chooser_widget;
};

static void tepl_language_chooser_interface_init (gpointer g_iface,
						  gpointer iface_data);

G_DEFINE_TYPE_WITH_CODE (TeplLanguageChooserDialog,
			 tepl_language_chooser_dialog,
			 GTK_TYPE_DIALOG,
			 G_ADD_PRIVATE (TeplLanguageChooserDialog)
			 G_IMPLEMENT_INTERFACE (TEPL_TYPE_LANGUAGE_CHOOSER,
						tepl_language_chooser_interface_init))

static void
tepl_language_chooser_dialog_dispose (GObject *object)
{
	TeplLanguageChooserDialog *chooser_dialog = TEPL_LANGUAGE_CHOOSER_DIALOG (object);

	chooser_dialog->priv->chooser_widget = NULL;

	G_OBJECT_CLASS (tepl_language_chooser_dialog_parent_class)->dispose (object);
}

static void
tepl_language_chooser_dialog_response (GtkDialog *dialog,
				       gint       response_id)
{
	TeplLanguageChooserDialog *chooser_dialog = TEPL_LANGUAGE_CHOOSER_DIALOG (dialog);

	if (response_id == GTK_RESPONSE_OK)
	{
		_tepl_language_chooser_widget_activate_selected_language (chooser_dialog->priv->chooser_widget);
	}

	if (GTK_DIALOG_CLASS (tepl_language_chooser_dialog_parent_class)->response != NULL)
	{
		GTK_DIALOG_CLASS (tepl_language_chooser_dialog_parent_class)->response (dialog, response_id);
	}
}

static void
tepl_language_chooser_dialog_class_init (TeplLanguageChooserDialogClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkDialogClass *dialog_class = GTK_DIALOG_CLASS (klass);

	object_class->dispose = tepl_language_chooser_dialog_dispose;

	dialog_class->response = tepl_language_chooser_dialog_response;
}

static void
tepl_language_chooser_dialog_select_language (TeplLanguageChooser *chooser,
					      GtkSourceLanguage   *language)
{
	TeplLanguageChooserDialog *chooser_dialog = TEPL_LANGUAGE_CHOOSER_DIALOG (chooser);

	tepl_language_chooser_select_language (TEPL_LANGUAGE_CHOOSER (chooser_dialog->priv->chooser_widget),
					       language);
}

static void
tepl_language_chooser_interface_init (gpointer g_iface,
				      gpointer iface_data)
{
	TeplLanguageChooserInterface *interface = g_iface;

	interface->select_language = tepl_language_chooser_dialog_select_language;
}

static void
chooser_widget_language_activated_cb (TeplLanguageChooserWidget *chooser_widget,
				      GtkSourceLanguage         *language,
				      TeplLanguageChooserDialog *chooser_dialog)
{
	if (language != NULL)
	{
		g_object_ref (language);
	}

	g_signal_emit_by_name (chooser_dialog, "language-activated", language);

	if (language != NULL)
	{
		g_object_unref (language);
	}
}

static void
tepl_language_chooser_dialog_init (TeplLanguageChooserDialog *chooser_dialog)
{
	GtkBox *content_area;

	chooser_dialog->priv = tepl_language_chooser_dialog_get_instance_private (chooser_dialog);

	/* chooser_dialog config */
	gtk_window_set_title (GTK_WINDOW (chooser_dialog), _("Highlight Mode"));
	gtk_window_set_modal (GTK_WINDOW (chooser_dialog), TRUE);

	/* Action area */
	gtk_dialog_add_buttons (GTK_DIALOG (chooser_dialog),
				_("_Cancel"), GTK_RESPONSE_CANCEL,
				_("_Select"), GTK_RESPONSE_OK,
				NULL);
	gtk_dialog_set_default_response (GTK_DIALOG (chooser_dialog), GTK_RESPONSE_OK);

	/* Content area: the TeplLanguageChooserWidget */
	chooser_dialog->priv->chooser_widget = tepl_language_chooser_widget_new ();
	gtk_container_set_border_width (GTK_CONTAINER (chooser_dialog->priv->chooser_widget), 11);

	g_signal_connect (chooser_dialog->priv->chooser_widget,
			  "language-activated",
			  G_CALLBACK (chooser_widget_language_activated_cb),
			  chooser_dialog);

	content_area = GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (chooser_dialog)));
	gtk_container_add (GTK_CONTAINER (content_area),
			   GTK_WIDGET (chooser_dialog->priv->chooser_widget));
	gtk_widget_show_all (GTK_WIDGET (content_area));
}

/**
 * tepl_language_chooser_dialog_new:
 * @parent: (nullable): transient parent of the dialog, or %NULL.
 *
 * Returns: a new #TeplLanguageChooserDialog widget.
 * Since: 5.2
 */
TeplLanguageChooserDialog *
tepl_language_chooser_dialog_new (GtkWindow *parent)
{
	g_return_val_if_fail (parent == NULL || GTK_IS_WINDOW (parent), NULL);

	return g_object_new (TEPL_TYPE_LANGUAGE_CHOOSER_DIALOG,
			     "transient-for", parent,
			     "use-header-bar", TRUE,
			     NULL);
}
