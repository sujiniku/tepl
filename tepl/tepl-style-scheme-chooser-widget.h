/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2020 - Sébastien Wilmet <swilmet@gnome.org>
 *
 * Tepl is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * Tepl is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TEPL_STYLE_SCHEME_CHOOSER_WIDGET_H
#define TEPL_STYLE_SCHEME_CHOOSER_WIDGET_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include <tepl/tepl-macros.h>

G_BEGIN_DECLS

#define TEPL_TYPE_STYLE_SCHEME_CHOOSER_WIDGET             (tepl_style_scheme_chooser_widget_get_type ())
#define TEPL_STYLE_SCHEME_CHOOSER_WIDGET(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEPL_TYPE_STYLE_SCHEME_CHOOSER_WIDGET, TeplStyleSchemeChooserWidget))
#define TEPL_STYLE_SCHEME_CHOOSER_WIDGET_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TEPL_TYPE_STYLE_SCHEME_CHOOSER_WIDGET, TeplStyleSchemeChooserWidgetClass))
#define TEPL_IS_STYLE_SCHEME_CHOOSER_WIDGET(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEPL_TYPE_STYLE_SCHEME_CHOOSER_WIDGET))
#define TEPL_IS_STYLE_SCHEME_CHOOSER_WIDGET_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TEPL_TYPE_STYLE_SCHEME_CHOOSER_WIDGET))
#define TEPL_STYLE_SCHEME_CHOOSER_WIDGET_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TEPL_TYPE_STYLE_SCHEME_CHOOSER_WIDGET, TeplStyleSchemeChooserWidgetClass))

typedef struct _TeplStyleSchemeChooserWidget         TeplStyleSchemeChooserWidget;
typedef struct _TeplStyleSchemeChooserWidgetClass    TeplStyleSchemeChooserWidgetClass;
typedef struct _TeplStyleSchemeChooserWidgetPrivate  TeplStyleSchemeChooserWidgetPrivate;

struct _TeplStyleSchemeChooserWidget
{
	GtkBin parent;

	TeplStyleSchemeChooserWidgetPrivate *priv;
};

struct _TeplStyleSchemeChooserWidgetClass
{
	GtkBinClass parent_class;

	gpointer padding[12];
};

_TEPL_EXTERN
GType		tepl_style_scheme_chooser_widget_get_type		(void);

_TEPL_EXTERN
TeplStyleSchemeChooserWidget *
		tepl_style_scheme_chooser_widget_new			(void);

_TEPL_EXTERN
gchar *		tepl_style_scheme_chooser_widget_get_style_scheme_id	(TeplStyleSchemeChooserWidget *chooser);

_TEPL_EXTERN
void		tepl_style_scheme_chooser_widget_set_style_scheme_id	(TeplStyleSchemeChooserWidget *chooser,
									 const gchar                  *style_scheme_id);

G_END_DECLS

#endif /* TEPL_STYLE_SCHEME_CHOOSER_WIDGET_H */
