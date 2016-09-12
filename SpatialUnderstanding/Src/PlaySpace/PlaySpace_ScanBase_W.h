// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#ifndef __PLAYSPACE_SCAN_BASE_H__
#define __PLAYSPACE_SCAN_BASE_H__

#include <PlaySpace\PlaySpace_Surfel_W.h>
#include <PlaySpace\PlaySpace_Tools_W.h>


class HMapBaseInfos3D
{
protected:
	Bool			m_IsInit;

public:
	S32				m_NbCellX;
	S32				m_NbCellY;
	S32				m_NbCellZ;

	Vec3f			m_PosMin;
	Vec3f			m_PosMax;
	Float			m_SizeCell;

	HMapBaseInfos3D();
	virtual ~HMapBaseInfos3D();

	FINLINE_Z Bool	IsInit() { return m_IsInit; }
	virtual void	Init(Playspace_Area &_Area) {Init(_Area.Min,_Area.Max,_Area.SizeVoxel,_Area.NbCellX,_Area.NbCellH,_Area.NbCellY);}
	virtual void	Init(Vec3f &_Min, Vec3f &_Max, Float _CellSize, S32 _SizeX, S32 _SizeY, S32 _SizeZ);
	virtual void	Move(Vec3f &_Min, Vec3f &_Max, S32 _SizeX, S32 _SizeY, S32 _SizeZ);
};

#endif //__PLAYSPACE_SCAN_H__
