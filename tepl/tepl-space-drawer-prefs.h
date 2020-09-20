/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifndef TEPL_SPACE_DRAWER_PREFS_H
#define TEPL_SPACE_DRAWER_PREFS_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gtksourceview/gtksource.h>
#include <tepl/tepl-macros.h>

G_BEGIN_DECLS

#define TEPL_TYPE_SPACE_DRAWER_PREFS             (tepl_space_drawer_prefs_get_type ())
#define TEPL_SPACE_DRAWER_PREFS(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEPL_TYPE_SPACE_DRAWER_PREFS, TeplSpaceDrawerPrefs))
#define TEPL_SPACE_DRAWER_PREFS_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TEPL_TYPE_SPACE_DRAWER_PREFS, TeplSpaceDrawerPrefsClass))
#define TEPL_IS_SPACE_DRAWER_PREFS(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEPL_TYPE_SPACE_DRAWER_PREFS))
#define TEPL_IS_SPACE_DRAWER_PREFS_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TEPL_TYPE_SPACE_DRAWER_PREFS))
#define TEPL_SPACE_DRAWER_PREFS_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TEPL_TYPE_SPACE_DRAWER_PREFS, TeplSpaceDrawerPrefsClass))

typedef struct _TeplSpaceDrawerPrefs         TeplSpaceDrawerPrefs;
typedef struct _TeplSpaceDrawerPrefsClass    TeplSpaceDrawerPrefsClass;
typedef struct _TeplSpaceDrawerPrefsPrivate  TeplSpaceDrawerPrefsPrivate;

struct _TeplSpaceDrawerPrefs
{
	GtkGrid parent;

	TeplSpaceDrawerPrefsPrivate *priv;
};

struct _TeplSpaceDrawerPrefsClass
{
	GtkGridClass parent_class;

	gpointer padding[12];
};

_TEPL_EXTERN
GType			tepl_space_drawer_prefs_get_type		(void);

_TEPL_EXTERN
TeplSpaceDrawerPrefs *	tepl_space_drawer_prefs_new			(void);

_TEPL_EXTERN
GtkSourceSpaceDrawer *	tepl_space_drawer_prefs_get_space_drawer	(TeplSpaceDrawerPrefs *prefs);

G_END_DECLS

#endif /* TEPL_SPACE_DRAWER_PREFS_H */
