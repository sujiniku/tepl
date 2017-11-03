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

#include "tepl-abstract-factory-vala.h"

/**
 * SECTION:abstract-factory-vala
 * @Short_description: #TeplAbstractFactory subclass to work-around Vala bugs
 * @Title: TeplAbstractFactoryVala
 *
 * #TeplAbstractFactoryVala is a subclass of #TeplAbstractFactory to work-around
 * Vala bugs.
 *
 * There are two bugs:
 * - See tepl_abstract_factory_vala_set_singleton_vala().
 * - For the functions or vfuncs in #TeplAbstractFactory which have a (transfer
 *   floating) return value: apparently when a Vala function returns a newly
 *   created #GInitiallyUnowned object, the return value is (transfer full).
 *   Which is not correct, it should be (transfer floating) instead (i.e. equal
 *   to (transfer none)). So to be able to implement easily those vfuncs in
 *   Vala, new Vala-specific vfuncs have been added which have a (transfer full)
 *   return value. The original vfuncs are implemented by
 *   #TeplAbstractFactoryVala by calling the Vala ones and transforming the
 *   strong ref into a floating ref.
 *
 * It would have been possible to modify the #TeplAbstractFactory API so that
 * it's usable in Vala as well, but in that case the API would be less
 * convenient and less natural in C. So #TeplAbstractFactoryVala has been
 * implemented as something separate, to not clutter #TeplAbstractFactory.
 */

G_DEFINE_TYPE (TeplAbstractFactoryVala,
	       tepl_abstract_factory_vala,
	       TEPL_TYPE_ABSTRACT_FACTORY)

static GtkApplicationWindow *
tepl_abstract_factory_vala_create_main_window (TeplAbstractFactory *factory,
					       GtkApplication      *app)
{
	TeplAbstractFactoryVala *factory_vala = TEPL_ABSTRACT_FACTORY_VALA (factory);
	GtkApplicationWindow *main_window;

	main_window = tepl_abstract_factory_vala_create_main_window_vala (factory_vala, app);
	if (main_window == NULL)
	{
		return NULL;
	}

	/* Transform strong ref to floating ref. */
	g_return_val_if_fail (!g_object_is_floating (main_window), main_window);
	g_object_force_floating (G_OBJECT (main_window));

	return main_window;
}

static void
tepl_abstract_factory_vala_class_init (TeplAbstractFactoryValaClass *klass)
{
	TeplAbstractFactoryClass *parent_class = TEPL_ABSTRACT_FACTORY_CLASS (klass);

	parent_class->create_main_window = tepl_abstract_factory_vala_create_main_window;
}

static void
tepl_abstract_factory_vala_init (TeplAbstractFactoryVala *factory_vala)
{
}

/**
 * tepl_abstract_factory_vala_set_singleton_vala:
 * @factory_vala: (transfer none): a #TeplAbstractFactoryVala.
 *
 * Like tepl_abstract_factory_set_singleton(), but with (transfer none) for the
 * @factory_vala parameter.
 *
 * Apparently Vala doesn't support (transfer full) for the self parameter,
 * resulting to a double unref if tepl_abstract_factory_set_singleton() is
 * called in Vala...
 *
 * Since: 4.0
 */
void
tepl_abstract_factory_vala_set_singleton_vala (TeplAbstractFactoryVala *factory_vala)
{
	g_return_if_fail (TEPL_IS_ABSTRACT_FACTORY_VALA (factory_vala));

	tepl_abstract_factory_set_singleton (g_object_ref (factory_vala));
}

/**
 * tepl_abstract_factory_vala_create_main_window_vala:
 * @factory_vala: the #TeplAbstractFactoryVala.
 * @app: a #GtkApplication.
 *
 * Like tepl_abstract_factory_create_main_window(), but with a (transfer full)
 * return value.
 *
 * Returns: (transfer full) (nullable): a new main application window, or %NULL
 * if the vfunc is not implemented.
 * Since: 4.0
 */
GtkApplicationWindow *
tepl_abstract_factory_vala_create_main_window_vala (TeplAbstractFactoryVala *factory_vala,
						    GtkApplication          *app)
{
	g_return_val_if_fail (TEPL_IS_ABSTRACT_FACTORY_VALA (factory_vala), NULL);
	g_return_val_if_fail (GTK_IS_APPLICATION (app), NULL);

	if (TEPL_ABSTRACT_FACTORY_VALA_GET_CLASS (factory_vala)->create_main_window_vala != NULL)
	{
		return TEPL_ABSTRACT_FACTORY_VALA_GET_CLASS (factory_vala)->create_main_window_vala (factory_vala, app);
	}

	g_warning ("The TeplAbstractFactoryVala::create_main_window_vala vfunc is not implemented.");
	return NULL;
}
