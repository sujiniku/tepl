/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TEPL_LANGUAGE_CHOOSER_WIDGET_H
#define TEPL_LANGUAGE_CHOOSER_WIDGET_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gtksourceview/gtksource.h>
#include <tepl/tepl-macros.h>

G_BEGIN_DECLS

#define TEPL_TYPE_LANGUAGE_CHOOSER_WIDGET             (tepl_language_chooser_widget_get_type ())
#define TEPL_LANGUAGE_CHOOSER_WIDGET(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEPL_TYPE_LANGUAGE_CHOOSER_WIDGET, TeplLanguageChooserWidget))
#define TEPL_LANGUAGE_CHOOSER_WIDGET_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TEPL_TYPE_LANGUAGE_CHOOSER_WIDGET, TeplLanguageChooserWidgetClass))
#define TEPL_IS_LANGUAGE_CHOOSER_WIDGET(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEPL_TYPE_LANGUAGE_CHOOSER_WIDGET))
#define TEPL_IS_LANGUAGE_CHOOSER_WIDGET_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TEPL_TYPE_LANGUAGE_CHOOSER_WIDGET))
#define TEPL_LANGUAGE_CHOOSER_WIDGET_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TEPL_TYPE_LANGUAGE_CHOOSER_WIDGET, TeplLanguageChooserWidgetClass))

typedef struct _TeplLanguageChooserWidget         TeplLanguageChooserWidget;
typedef struct _TeplLanguageChooserWidgetClass    TeplLanguageChooserWidgetClass;
typedef struct _TeplLanguageChooserWidgetPrivate  TeplLanguageChooserWidgetPrivate;

struct _TeplLanguageChooserWidget
{
	GtkGrid parent;

	TeplLanguageChooserWidgetPrivate *priv;
};

struct _TeplLanguageChooserWidgetClass
{
	GtkGridClass parent_class;

	gpointer padding[12];
};

_TEPL_EXTERN
GType		tepl_language_chooser_widget_get_type	(void);

_TEPL_EXTERN
TeplLanguageChooserWidget *
		tepl_language_chooser_widget_new	(void);

G_END_DECLS

#endif /* TEPL_LANGUAGE_CHOOSER_WIDGET_H */
