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

#ifndef TEPL_ABSTRACT_FACTORY_VALA_H
#define TEPL_ABSTRACT_FACTORY_VALA_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include <tepl/tepl-types.h>
#include <tepl/tepl-abstract-factory.h>

G_BEGIN_DECLS

#define TEPL_TYPE_ABSTRACT_FACTORY_VALA             (tepl_abstract_factory_vala_get_type ())
#define TEPL_ABSTRACT_FACTORY_VALA(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEPL_TYPE_ABSTRACT_FACTORY_VALA, TeplAbstractFactoryVala))
#define TEPL_ABSTRACT_FACTORY_VALA_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TEPL_TYPE_ABSTRACT_FACTORY_VALA, TeplAbstractFactoryValaClass))
#define TEPL_IS_ABSTRACT_FACTORY_VALA(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEPL_TYPE_ABSTRACT_FACTORY_VALA))
#define TEPL_IS_ABSTRACT_FACTORY_VALA_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TEPL_TYPE_ABSTRACT_FACTORY_VALA))
#define TEPL_ABSTRACT_FACTORY_VALA_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TEPL_TYPE_ABSTRACT_FACTORY_VALA, TeplAbstractFactoryValaClass))

typedef struct _TeplAbstractFactoryValaClass TeplAbstractFactoryValaClass;

struct _TeplAbstractFactoryVala
{
	TeplAbstractFactory parent;
};

/**
 * TeplAbstractFactoryValaClass:
 * @parent_class: The parent class.
 * @create_main_window_vala: Virtual function pointer for
 *   tepl_abstract_factory_vala_create_main_window_vala(). It is not implemented
 *   by default. The #TeplAbstractFactory ::create_main_window vfunc is
 *   implemented by #TeplAbstractFactoryVala by calling
 *   tepl_abstract_factory_vala_create_main_window_vala() and adapting the ref.
 */
struct _TeplAbstractFactoryValaClass
{
	TeplAbstractFactoryClass parent_class;

	GtkApplicationWindow *	(* create_main_window_vala)	(TeplAbstractFactoryVala *factory_vala,
								 GtkApplication          *app);

	/*< private >*/
	gpointer padding[12];
};

GType	tepl_abstract_factory_vala_get_type			(void);

void	tepl_abstract_factory_vala_set_singleton_vala		(TeplAbstractFactoryVala *factory_vala);

GtkApplicationWindow *
	tepl_abstract_factory_vala_create_main_window_vala	(TeplAbstractFactoryVala *factory_vala,
								 GtkApplication          *app);

G_END_DECLS

#endif /* TEPL_ABSTRACT_FACTORY_VALA_H */
