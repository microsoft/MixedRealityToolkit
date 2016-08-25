// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#ifndef __FRG_CONSTRAINT_H__
#define __FRG_CONSTRAINT_H__

//------------------------------------------------------------------------------------
// FRGConstraintType_W
//-----------------------------------------------------------------------------
enum FRGConstraintType_W
{
	FRG_CONSTRAINT_NEAR_OF,
	FRG_CONSTRAINT_NEAR_OF_WALL,
	FRG_CONSTRAINT_AWAY_FROM_WALL,
	FRG_CONSTRAINT_NEAR_OF_CENTER,
	FRG_CONSTRAINT_ON_SEGMENT,
	FRG_CONSTRAINT_AWAY_FROM_OTHER_OBJECTS,
	FRG_CONSTRAINT_AWAY_FROM_POINT,
	FRG_CONSTRAINT_FACING,
	FRG_CONSTRAINT_RAYCAST
};

//------------------------------------------------------------------------------------
// Params
//-----------------------------------------------------------------------------
struct FRGConstraintParamsNearOf_W
{
	Vec3f m_vPoint;
	Float m_fDistMin;
	Float m_fDistMax;
};

struct FRGConstraintParamsNearOfWall_W
{
	Bool m_bVirtual;
	Float m_fHeight;
	Float m_fDistMin;
	Float m_fDistMax;
};

struct FRGConstraintParamsNearOfCenter_W
{
	Float m_fDistMin;
	Float m_fDistMax;
};

struct FRGConstraintParamsOnSegment_W
{
	Segment_Z m_segment;
};

struct FRGConstraintParamsAwayFromPoint_W
{
	Vec3fDA m_vPoint;
};

//------------------------------------------------------------------------------------
// FRGConstraint_W
//-----------------------------------------------------------------------------
class FRGConstraint_W
{
public:
	FRGConstraintType_W m_type;

	FRGConstraintParamsNearOf_W m_paramsNearOf;
	FRGConstraintParamsNearOfCenter_W m_paramsNearOfCenter;
	FRGConstraintParamsNearOfWall_W m_paramsNearOfWall;
	FRGConstraintParamsOnSegment_W m_paramsOnSegment;
	FRGConstraintParamsAwayFromPoint_W m_paramsAwayFromPoint;
};

#endif