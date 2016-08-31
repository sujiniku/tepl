/*
 * This file is part of Gtef, a text editor library.
 *
 * Copyright 2016 - David Rabel <david.rabel@noresoft.com>
 *
 * Gtef is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * Gtef is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GTEF_GUTTER_RENDERER_FOLDS_SUB_H
#define GTEF_GUTTER_RENDERER_FOLDS_SUB_H

#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>
#include <gtef/gtef.h>

G_BEGIN_DECLS

#define GTEF_TYPE_GUTTER_RENDERER_FOLDS_SUB 		(gtef_gutter_renderer_folds_sub_get_type ())
#define GTEF_GUTTER_RENDERER_FOLDS_SUB(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTEF_TYPE_GUTTER_RENDERER_FOLDS_SUB, GtefGutterRendererFoldsSub))
#define GTEF_GUTTER_RENDERER_FOLDS_SUB_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GTEF_TYPE_GUTTER_RENDERER_FOLDS_SUB, GtefGutterRendererFoldsSubClass))
#define GTEF_IS_GUTTER_RENDERER_FOLDS_SUB(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTEF_TYPE_GUTTER_RENDERER_FOLDS_SUB))
#define GTEF_IS_GUTTER_RENDERER_FOLDS_SUB_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GTEF_TYPE_GUTTER_RENDERER_FOLDS_SUB))
#define GTEF_GUTTER_RENDERER_FOLDS_SUB_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GTEF_TYPE_GUTTER_RENDERER_FOLDS_SUB, GtefGutterRendererFoldsSubClass))

typedef struct _GtefGutterRendererFoldsSubClass		GtefGutterRendererFoldsSubClass;
typedef struct _GtefGutterRendererFoldsSub		GtefGutterRendererFoldsSub;

struct _GtefGutterRendererFoldsSub
{
	GtefGutterRendererFolds parent_instance;
};

struct _GtefGutterRendererFoldsSubClass
{
	GtefGutterRendererFoldsClass parent_class;
};

GType				gtef_gutter_renderer_folds_sub_get_type		(void) G_GNUC_CONST;

GtkSourceGutterRenderer *	gtef_gutter_renderer_folds_sub_new		(void);

G_END_DECLS

#endif /* GTEF_GUTTER_RENDERER_FOLDS_SUB_H */
