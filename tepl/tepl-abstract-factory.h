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
#include <tepl/tepl-types.h>

G_BEGIN_DECLS

#define TEPL_TYPE_ABSTRACT_FACTORY             (tepl_abstract_factory_get_type ())
#define TEPL_ABSTRACT_FACTORY(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEPL_TYPE_ABSTRACT_FACTORY, TeplAbstractFactory))
#define TEPL_ABSTRACT_FACTORY_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TEPL_TYPE_ABSTRACT_FACTORY, TeplAbstractFactoryClass))
#define TEPL_IS_ABSTRACT_FACTORY(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEPL_TYPE_ABSTRACT_FACTORY))
#define TEPL_IS_ABSTRACT_FACTORY_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TEPL_TYPE_ABSTRACT_FACTORY))
#define TEPL_ABSTRACT_FACTORY_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TEPL_TYPE_ABSTRACT_FACTORY, TeplAbstractFactoryClass))

typedef struct _TeplAbstractFactoryClass TeplAbstractFactoryClass;

struct _TeplAbstractFactory
{
	GObject parent;
};

struct _TeplAbstractFactoryClass
{
	GObjectClass parent_class;

	/*< private >*/
	gpointer padding[12];
};

GType			tepl_abstract_factory_get_type			(void);

void			tepl_abstract_factory_set_singleton		(TeplAbstractFactory *factory);

TeplAbstractFactory *	tepl_abstract_factory_get_singleton		(void);

G_GNUC_INTERNAL
void			_tepl_abstract_factory_unref_singleton		(void);

G_END_DECLS

#endif /* TEPL_ABSTRACT_FACTORY_H */
