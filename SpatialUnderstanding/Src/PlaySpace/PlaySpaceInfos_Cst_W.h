// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#ifndef __PLAYSPACEINFOS_CST_H__
#define __PLAYSPACEINFOS_CST_H__

#define		SHIFT_CACHE_SCANNING_CELL	7

#define		SIZE_CACHE_SCANNING_CELL	(1<<SHIFT_CACHE_SCANNING_CELL)		// exposant de 2
#define		MSK_CACHE_SCANNING_CELL		(SIZE_CACHE_SCANNING_CELL-1)

#define		POS_CACHE_SCANNING_CELL(ix,iy) ((ix & MSK_CACHE_SCANNING_CELL) + ((iy & MSK_CACHE_SCANNING_CELL) << SHIFT_CACHE_SCANNING_CELL))

#define		DEFAULT_SIZE_VOXEL	0.08f
#define		MARGIN_WALL_DETECT	(8.f * DEFAULT_SIZE_VOXEL)
#define		WALL_MAX_HEIGHT		1.4f			// Hauteur par rapport aux yeux !

#define		MARGIN_GROUND_CEILING	0.2f

#define		CONEXITY_MAX_SURF	8192
#define		CONEXITY_MAX_ZONE	4096

#endif //__PLAYSPACEINFOS_CST_H__
