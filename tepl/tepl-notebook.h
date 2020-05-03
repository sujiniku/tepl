/* Copyright 2017 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TEPL_NOTEBOOK_H
#define TEPL_NOTEBOOK_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include <tepl/tepl-macros.h>

G_BEGIN_DECLS

#define TEPL_TYPE_NOTEBOOK             (tepl_notebook_get_type ())
#define TEPL_NOTEBOOK(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEPL_TYPE_NOTEBOOK, TeplNotebook))
#define TEPL_NOTEBOOK_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TEPL_TYPE_NOTEBOOK, TeplNotebookClass))
#define TEPL_IS_NOTEBOOK(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEPL_TYPE_NOTEBOOK))
#define TEPL_IS_NOTEBOOK_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TEPL_TYPE_NOTEBOOK))
#define TEPL_NOTEBOOK_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TEPL_TYPE_NOTEBOOK, TeplNotebookClass))

typedef struct _TeplNotebook         TeplNotebook;
typedef struct _TeplNotebookClass    TeplNotebookClass;
typedef struct _TeplNotebookPrivate  TeplNotebookPrivate;

struct _TeplNotebook
{
	GtkNotebook parent;

	TeplNotebookPrivate *priv;
};

struct _TeplNotebookClass
{
	GtkNotebookClass parent_class;

	gpointer padding[12];
};

_TEPL_EXTERN
GType		tepl_notebook_get_type	(void);

_TEPL_EXTERN
GtkWidget *	tepl_notebook_new	(void);

G_END_DECLS

#endif /* TEPL_NOTEBOOK_H */
