/* SPDX-FileCopyrightText: 2016 - David Rabel <david.rabel@noresoft.com>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TEPL_GUTTER_RENDERER_FOLDS_SUB_H
#define TEPL_GUTTER_RENDERER_FOLDS_SUB_H

#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>
#include <tepl/tepl.h>

G_BEGIN_DECLS

#define TEPL_TYPE_GUTTER_RENDERER_FOLDS_SUB 		(tepl_gutter_renderer_folds_sub_get_type ())
#define TEPL_GUTTER_RENDERER_FOLDS_SUB(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEPL_TYPE_GUTTER_RENDERER_FOLDS_SUB, TeplGutterRendererFoldsSub))
#define TEPL_GUTTER_RENDERER_FOLDS_SUB_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TEPL_TYPE_GUTTER_RENDERER_FOLDS_SUB, TeplGutterRendererFoldsSubClass))
#define TEPL_IS_GUTTER_RENDERER_FOLDS_SUB(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEPL_TYPE_GUTTER_RENDERER_FOLDS_SUB))
#define TEPL_IS_GUTTER_RENDERER_FOLDS_SUB_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TEPL_TYPE_GUTTER_RENDERER_FOLDS_SUB))
#define TEPL_GUTTER_RENDERER_FOLDS_SUB_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TEPL_TYPE_GUTTER_RENDERER_FOLDS_SUB, TeplGutterRendererFoldsSubClass))

typedef struct _TeplGutterRendererFoldsSubClass		TeplGutterRendererFoldsSubClass;
typedef struct _TeplGutterRendererFoldsSub		TeplGutterRendererFoldsSub;

struct _TeplGutterRendererFoldsSub
{
	TeplGutterRendererFolds parent_instance;
};

struct _TeplGutterRendererFoldsSubClass
{
	TeplGutterRendererFoldsClass parent_class;
};

GType				tepl_gutter_renderer_folds_sub_get_type		(void) G_GNUC_CONST;

GtkSourceGutterRenderer *	tepl_gutter_renderer_folds_sub_new		(void);

G_END_DECLS

#endif /* TEPL_GUTTER_RENDERER_FOLDS_SUB_H */
