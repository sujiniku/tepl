/*
 * This file is part of Gtef, a text editor library.
 *
 * Copyright 2016, 2017 - Sébastien Wilmet <swilmet@gnome.org>
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

#ifndef GTEF_TYPES_H
#define GTEF_TYPES_H

#if !defined (GTEF_H_INSIDE) && !defined (GTEF_COMPILATION)
#error "Only <gtef/gtef.h> can be included directly."
#endif

#include <glib.h>

G_BEGIN_DECLS

typedef struct _GtefActionInfo			GtefActionInfo;
typedef struct _GtefActionInfoEntry		GtefActionInfoEntry;
typedef struct _GtefActionInfoStore		GtefActionInfoStore;
typedef struct _GtefActionInfoCentralStore	GtefActionInfoCentralStore;
typedef struct _GtefApplication			GtefApplication;
typedef struct _GtefBuffer			GtefBuffer;
typedef struct _GtefEncoding			GtefEncoding;
typedef struct _GtefFile			GtefFile;
typedef struct _GtefFileLoader			GtefFileLoader;
typedef struct _GtefFileMetadata		GtefFileMetadata;
typedef struct _GtefFileSaver			GtefFileSaver;
typedef struct _GtefFoldRegion			GtefFoldRegion;
typedef struct _GtefGutterRendererFolds		GtefGutterRendererFolds;
typedef struct _GtefInfoBar			GtefInfoBar;
typedef struct _GtefTab				GtefTab;
typedef struct _GtefView			GtefView;

G_END_DECLS

#endif /* GTEF_TYPES_H */
