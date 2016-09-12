// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#ifndef __FRG_POSITION_H__
#define __FRG_POSITION_H__

//------------------------------------------------------------------------------------
// FRGPositionType_W
//------------------------------------------------------------------------------------
enum FRGPositionType_W
{
	FRG_POSITION_ON_FLOOR,
	FRG_POSITION_ON_WALL,
	FRG_POSITION_ON_CEILING,
	FRG_POSITION_ON_SHAPE,
	FRG_POSITION_ON_EDGE,
	FRG_POSITION_ON_FLOOR_AND_CEILING,
	FRG_POSITION_RANDOM_IN_THE_AIR,
	FRG_POSITION_IN_THE_MID_AIR,
	FRG_POSITION_UNDER_FURNITURE_EDGE
};

//------------------------------------------------------------------------------------
// Params
//-----------------------------------------------------------------------------------
enum FRGWallType_W
{
	FRG_NORMAL_WALL				= 1 << 0,
	FRG_EXTERNAL_WALL			= 1 << 1,
	FRG_VIRTUAL_WALL			= 1 << 2,
	FRG_EXTERNAL_VIRTUAL_WALL	= 1 << 3
};

enum FRGWallPosition_W
{
	FRG_ON_WALL, FRG_ON_WALL_NEAR_FLOOR, FRG_ON_WALL_NEAR_CEILING, FRG_ON_WALL_NEAR_SURFACE
};

struct FRGPositionParamsOnWall_W
{
	// Height is the height of the middle of the object relative to the floor
	Float m_fHeightMin;
	Float m_fHeightMax;

	Bool m_bOnlyFullWall;
	Bool m_bNotOnFullWall;

	U32 m_uWallTypes;
	FRGWallPosition_W m_positionOnWall;

	Float m_fLeftMargin;
	Float m_fRightMargin;

	FRGPositionParamsOnWall_W()
	{
		m_bNotOnFullWall = FALSE;
	}
};

struct FRGPositionParamsOnShape_W
{
	Name_Z m_nShapeName;
	Name_Z m_nSlotName;
};

struct FRGPositionParamsOnEdge_W
{
	Vec3f m_vSizeBottomObject;

	// Next variables are affected by the solver
	S32 m_idxTopObject;
	S32 m_idxBottomObject;
};

//------------------------------------------------------------------------------------
// FRGPosition_W
//-----------------------------------------------------------------------------------
class FRGPosition_W
{
public:
	FRGPositionType_W m_type;

	FRGPositionParamsOnWall_W m_paramsOnWall;
	FRGPositionParamsOnShape_W m_paramsOnShape;
	FRGPositionParamsOnEdge_W m_paramsOnEdge;

	Bool m_bHaveFixedRot;
	Vec3f m_vPointToLook;

	Bool m_bHaveTiledPos;
	Vec3f m_vTiledPos;

	Bool m_bHas90RandomRotation;
};

#endif