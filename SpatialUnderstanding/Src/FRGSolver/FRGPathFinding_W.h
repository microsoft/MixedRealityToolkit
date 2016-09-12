// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#ifndef __FRG_PATH_FINDING_H__
#define __FRG_PATH_FINDING_H__

//------------------------------------------------------------------------------------
// FRGFindPath
//-----------------------------------------------------------------------------------
// _daGrid must be a 2D grid where a cell is TRUE if there is a wall
Bool FRGFindPath(const BoolDA& _daGrid, const S32DA& _daConexity, const Vec2i& _vSize, const Vec2i& _vBegin, const Vec2i& _vEnd, DynArray_Z<Vec2i>& _outPath);

//------------------------------------------------------------------------------------
// FRGComputeConexity
//-----------------------------------------------------------------------------------
// _daGrid must be a 2D grid where a cell is TRUE if there is a wall
void FRGComputeConexity(const BoolDA& _daIsWall, const Vec2i& _vSize, S32DA& _outConexity);

#endif