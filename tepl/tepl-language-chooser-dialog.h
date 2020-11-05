/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TEPL_LANGUAGE_CHOOSER_DIALOG_H
#define TEPL_LANGUAGE_CHOOSER_DIALOG_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include <tepl/tepl-macros.h>

G_BEGIN_DECLS

#define TEPL_TYPE_LANGUAGE_CHOOSER_DIALOG             (tepl_language_chooser_dialog_get_type ())
#define TEPL_LANGUAGE_CHOOSER_DIALOG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEPL_TYPE_LANGUAGE_CHOOSER_DIALOG, TeplLanguageChooserDialog))
#define TEPL_LANGUAGE_CHOOSER_DIALOG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TEPL_TYPE_LANGUAGE_CHOOSER_DIALOG, TeplLanguageChooserDialogClass))
#define TEPL_IS_LANGUAGE_CHOOSER_DIALOG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEPL_TYPE_LANGUAGE_CHOOSER_DIALOG))
#define TEPL_IS_LANGUAGE_CHOOSER_DIALOG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TEPL_TYPE_LANGUAGE_CHOOSER_DIALOG))
#define TEPL_LANGUAGE_CHOOSER_DIALOG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TEPL_TYPE_LANGUAGE_CHOOSER_DIALOG, TeplLanguageChooserDialogClass))

typedef struct _TeplLanguageChooserDialog         TeplLanguageChooserDialog;
typedef struct _TeplLanguageChooserDialogClass    TeplLanguageChooserDialogClass;
typedef struct _TeplLanguageChooserDialogPrivate  TeplLanguageChooserDialogPrivate;

struct _TeplLanguageChooserDialog
{
	GtkDialog parent;

	TeplLanguageChooserDialogPrivate *priv;
};

struct _TeplLanguageChooserDialogClass
{
	GtkDialogClass parent_class;

	gpointer padding[12];
};

_TEPL_EXTERN
GType				tepl_language_chooser_dialog_get_type	(void);

_TEPL_EXTERN
TeplLanguageChooserDialog *	tepl_language_chooser_dialog_new	(GtkWindow *parent);

G_END_DECLS

#endif /* TEPL_LANGUAGE_CHOOSER_DIALOG_H */
