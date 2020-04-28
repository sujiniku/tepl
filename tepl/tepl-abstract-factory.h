/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2017 - Sébastien Wilmet <swilmet@gnome.org>
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

#ifndef TEPL_ABSTRACT_FACTORY_H
#define TEPL_ABSTRACT_FACTORY_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include <tepl/tepl-file.h>
#include <tepl/tepl-tab.h>

G_BEGIN_DECLS

#define TEPL_TYPE_ABSTRACT_FACTORY             (tepl_abstract_factory_get_type ())
#define TEPL_ABSTRACT_FACTORY(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEPL_TYPE_ABSTRACT_FACTORY, TeplAbstractFactory))
#define TEPL_ABSTRACT_FACTORY_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TEPL_TYPE_ABSTRACT_FACTORY, TeplAbstractFactoryClass))
#define TEPL_IS_ABSTRACT_FACTORY(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEPL_TYPE_ABSTRACT_FACTORY))
#define TEPL_IS_ABSTRACT_FACTORY_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TEPL_TYPE_ABSTRACT_FACTORY))
#define TEPL_ABSTRACT_FACTORY_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TEPL_TYPE_ABSTRACT_FACTORY, TeplAbstractFactoryClass))

typedef struct _TeplAbstractFactory      TeplAbstractFactory;
typedef struct _TeplAbstractFactoryClass TeplAbstractFactoryClass;

struct _TeplAbstractFactory
{
	GObject parent;
};

/**
 * TeplAbstractFactoryClass:
 * @parent_class: The parent class.
 * @create_main_window: Virtual function pointer for
 *   tepl_abstract_factory_create_main_window(). It is not implemented by
 *   default.
 * @create_tab: Virtual function pointer for tepl_abstract_factory_create_tab().
 *   By default the #TeplTab is created with tepl_tab_new().
 * @create_tab_label: Virtual function pointer for
 *   tepl_abstract_factory_create_tab_label(). By default the tab label is
 *   created with tepl_tab_label_new().
 * @create_file: Virtual function pointer for
 *   tepl_abstract_factory_create_file(). By default the #TeplFile is created
 *   with tepl_file_new().
 * @create_metadata_manager_file: Virtual function pointer for
 *   tepl_abstract_factory_create_metadata_manager_file(). It is not implemented
 *   by default.
 */
struct _TeplAbstractFactoryClass
{
	GObjectClass parent_class;

	GtkApplicationWindow *	(* create_main_window)			(TeplAbstractFactory *factory,
									 GtkApplication      *app);

	TeplTab *		(* create_tab)				(TeplAbstractFactory *factory);

	GtkWidget *		(* create_tab_label)			(TeplAbstractFactory *factory,
									 TeplTab             *tab);

	TeplFile *		(* create_file)				(TeplAbstractFactory *factory);

	GFile *			(* create_metadata_manager_file)	(TeplAbstractFactory *factory);

	/*< private >*/
	gpointer padding[12];
};

_TEPL_EXTERN
GType			tepl_abstract_factory_get_type			(void);

_TEPL_EXTERN
void			tepl_abstract_factory_set_singleton		(TeplAbstractFactory *factory);

_TEPL_EXTERN
TeplAbstractFactory *	tepl_abstract_factory_get_singleton		(void);

_TEPL_EXTERN
GtkApplicationWindow *	tepl_abstract_factory_create_main_window	(TeplAbstractFactory *factory,
									 GtkApplication      *app);

_TEPL_EXTERN
TeplTab *		tepl_abstract_factory_create_tab		(TeplAbstractFactory *factory);

_TEPL_EXTERN
GtkWidget *		tepl_abstract_factory_create_tab_label		(TeplAbstractFactory *factory,
									 TeplTab             *tab);

_TEPL_EXTERN
TeplFile *		tepl_abstract_factory_create_file		(TeplAbstractFactory *factory);

_TEPL_EXTERN
GFile *			tepl_abstract_factory_create_metadata_manager_file
									(TeplAbstractFactory *factory);

G_GNUC_INTERNAL
void			_tepl_abstract_factory_unref_singleton		(void);

G_END_DECLS

#endif /* TEPL_ABSTRACT_FACTORY_H */
