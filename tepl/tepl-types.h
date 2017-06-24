/*
 * This file is part of Tepl, a text editor library.
 *
 * Copyright 2016, 2017 - Sébastien Wilmet <swilmet@gnome.org>
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

#ifndef TEPL_TYPES_H
#define TEPL_TYPES_H

#if !defined (TEPL_H_INSIDE) && !defined (TEPL_COMPILATION)
#error "Only <tepl/tepl.h> can be included directly."
#endif

#include <glib.h>

G_BEGIN_DECLS

typedef struct _TeplActionInfo			TeplActionInfo;
typedef struct _TeplActionInfoEntry		TeplActionInfoEntry;
typedef struct _TeplActionInfoStore		TeplActionInfoStore;
typedef struct _TeplActionInfoCentralStore	TeplActionInfoCentralStore;
typedef struct _TeplApplication			TeplApplication;
typedef struct _TeplApplicationWindow		TeplApplicationWindow;
typedef struct _TeplBuffer			TeplBuffer;
typedef struct _TeplEncoding			TeplEncoding;
typedef struct _TeplFile			TeplFile;
typedef struct _TeplFileLoader			TeplFileLoader;
typedef struct _TeplFileMetadata		TeplFileMetadata;
typedef struct _TeplFileSaver			TeplFileSaver;
typedef struct _TeplFoldRegion			TeplFoldRegion;
typedef struct _TeplGutterRendererFolds		TeplGutterRendererFolds;
typedef struct _TeplInfoBar			TeplInfoBar;
typedef struct _TeplMenuShell			TeplMenuShell;
typedef struct _TeplNotebook			TeplNotebook;
typedef struct _TeplTab				TeplTab;
typedef struct _TeplTabList			TeplTabList;
typedef struct _TeplView			TeplView;

G_END_DECLS

#endif /* TEPL_TYPES_H */
