/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TEPL_HIGHLIGHT_MODE_SELECTOR_H
#define TEPL_HIGHLIGHT_MODE_SELECTOR_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gtksourceview/gtksource.h>
#include <tepl/tepl-macros.h>

G_BEGIN_DECLS

#define TEPL_TYPE_HIGHLIGHT_MODE_SELECTOR             (tepl_highlight_mode_selector_get_type ())
#define TEPL_HIGHLIGHT_MODE_SELECTOR(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEPL_TYPE_HIGHLIGHT_MODE_SELECTOR, TeplHighlightModeSelector))
#define TEPL_HIGHLIGHT_MODE_SELECTOR_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TEPL_TYPE_HIGHLIGHT_MODE_SELECTOR, TeplHighlightModeSelectorClass))
#define TEPL_IS_HIGHLIGHT_MODE_SELECTOR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEPL_TYPE_HIGHLIGHT_MODE_SELECTOR))
#define TEPL_IS_HIGHLIGHT_MODE_SELECTOR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TEPL_TYPE_HIGHLIGHT_MODE_SELECTOR))
#define TEPL_HIGHLIGHT_MODE_SELECTOR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TEPL_TYPE_HIGHLIGHT_MODE_SELECTOR, TeplHighlightModeSelectorClass))

typedef struct _TeplHighlightModeSelector         TeplHighlightModeSelector;
typedef struct _TeplHighlightModeSelectorClass    TeplHighlightModeSelectorClass;
typedef struct _TeplHighlightModeSelectorPrivate  TeplHighlightModeSelectorPrivate;

struct _TeplHighlightModeSelector
{
	GtkGrid parent;

	TeplHighlightModeSelectorPrivate *priv;
};

struct _TeplHighlightModeSelectorClass
{
	GtkGridClass parent_class;

	/* Signals */
	void (* language_selected)	(TeplHighlightModeSelector *selector,
					 GtkSourceLanguage         *language);

	gpointer padding[12];
};

_TEPL_EXTERN
GType		tepl_highlight_mode_selector_get_type	(void);

_TEPL_EXTERN
TeplHighlightModeSelector *
		tepl_highlight_mode_selector_new	(void);

G_END_DECLS

#endif /* TEPL_HIGHLIGHT_MODE_SELECTOR_H */
