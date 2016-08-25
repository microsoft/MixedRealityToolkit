// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include <pch.h>
#include <PlaySpace\PlaySpace_ScanBase_W.h>

HMapBaseInfos3D::HMapBaseInfos3D()
{
	m_IsInit = FALSE;
	m_NbCellX = 0;
	m_NbCellY = 0;
	m_NbCellZ = 0;

	m_PosMin = VEC3F_NULL;
	m_PosMax = VEC3F_NULL;
	m_SizeCell = 0.f;
}

/**************************************************************************/

HMapBaseInfos3D::~HMapBaseInfos3D()
{
}

/**************************************************************************/

void	HMapBaseInfos3D::Init(Vec3f &_Min, Vec3f &_Max, Float _CellSize, S32 _SizeX, S32 _SizeY, S32 _SizeZ)
{
	m_IsInit = TRUE;
	m_NbCellX = _SizeX;
	m_NbCellY = _SizeY;
	m_NbCellZ = _SizeZ;

	m_PosMin = _Min;
	m_PosMax = _Max;
	m_SizeCell = _CellSize;
}

/**************************************************************************/

void	HMapBaseInfos3D::Move(Vec3f &_Min, Vec3f &_Max, S32 _SizeX, S32 _SizeY, S32 _SizeZ)
{
	m_NbCellX = _SizeX;
	m_NbCellY = _SizeY;
	m_NbCellZ = _SizeZ;

	m_PosMin = _Min;
	m_PosMax = _Max;
}

/**************************************************************************/
