// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#ifndef __FRG_SOLVING_INFOS_H__
#define __FRG_SOLVING_INFOS_H__

#include <Position3D_L.h>
#include <FRGSolver/FRGConstraint_W.h>
#include <FRGSolver/FRGPosition_W.h>
#include <FRGSolver/FRGRule_W.h>
#include <FRGSolver/FRGUtils_W.h>


#define INALLUNIVERS 127

#define	SOLVINFOFLAG_WANTLIGHT		1

//------------------------------------------------------------------------------------
// FRGSolvingInfos_W
//------------------------------------------------------------------------------------
class FRGSolvingInfos_W
{
private:
	String_Z<ARRAY_CHAR_MAX>	m_sObjectName;		  
	Name_Z						m_nObjectName;		  
public:
	FRGSolvingInfos_W();

	// Size
	Vec3f m_vSize;
	Vec3f m_vCenter; // Delta from node pos to box center 
	Vec3f m_vEmptySize;				// What I use to see others
	Vec3f m_vEmptyCenter;
	Vec3f m_vEmptyWorldCenter;
	Bool m_bEmptyVsClearanceDifferent;
	Vec3f m_vClearanceSize;			// How others see me
	Vec3f m_vClearanceCenter;
	Vec3f m_vClearanceWorldCenter;

	Bool m_bHasFullBox;
	Vec3f m_vFullSize;			// How others see me
	Vec3f m_vFullCenter;
	Float m_fFactor;

	DynArray_Z<Box_Z> m_daAOBoxes; // In world coordinates

	Bool m_bAllowPartiallyInWall;

	Bool	m_bOnlyComputePossiblePos;
	Vec3f	m_vOppositePos;

	Bool m_bCheckOppositePos;
	Bool m_bUseSmallerBox;

	Name_ZDA	m_daIgnoredSolvedBox;

	S32		m_iChosenPlacement;

	Bool	m_bDebugInfo;

	void SetObjectName(const String_Z<ARRAY_CHAR_MAX>& _name ) 
	{
		m_sObjectName = _name;
		m_nObjectName = m_sObjectName;
	}

	const Name_Z& GetObjectName() const 
	{
		return m_nObjectName;
	}

	const String_Z<ARRAY_CHAR_MAX>& GetObjectNameStr() const 
	{
		return m_sObjectName;
	}

	void ComputeWorldPos()
	{
		m_vClearanceWorldCenter = m_vPos + m_qRot * m_vClearanceCenter; 
		m_vEmptyWorldCenter = m_vPos + m_qRot * m_vEmptyCenter; 
	}

	INLINE_Z void ComputeClearanceBoxAndEmptyBoxAreDifferent() 
	{
		m_bEmptyVsClearanceDifferent = (Abs(m_vClearanceCenter.x - m_vEmptyCenter.x) > Float_Eps ||
				Abs(m_vClearanceCenter.y - m_vEmptyCenter.y) > Float_Eps ||
				Abs(m_vClearanceCenter.z - m_vEmptyCenter.z) > Float_Eps ||
				Abs(m_vClearanceSize.x - m_vEmptySize.x) > Float_Eps ||
				Abs(m_vClearanceSize.y - m_vEmptySize.y) > Float_Eps ||
				Abs(m_vClearanceSize.z - m_vEmptySize.z) > Float_Eps);
	}

	INLINE_Z Bool ClearanceBoxAndEmptyBoxAreDifferent() const
	{
		return m_bEmptyVsClearanceDifferent;
	}

	// Position
	FRGPosition_W m_position;

	void SetPositionOnFloor();
	void SetPositionOnWall(Float _fHeightMin, Float _fHeightMax, U32 _uWallTypes, Float _fLeftMargin = 0.f, Float _fRightMargin = 0.f);
	void SetPositionOnCeiling();
	void SetPositionOnShape(const Name_Z& _nShapeName, const Name_Z& _nSlotName);
	void SetPositionOnEdge(const Vec3f& _vSizeBottomObject);
	void SetPositionOnFloorAndCeiling(const Vec3f& _vSizeBottomObject);
	void SetPositionRandomInTheAir();
	void SetPositionInTheMidAir();
	void SetPositionUnderFurnitureEdge();

	// Rules
	DynArray_Z<FRGRule_W> m_daRules;

	void AddRuleAwayFrom(Float _fDistMin, const Vec3f& _vPoint);
	void AddRuleAwayFromWalls(Float _fDistMin, Float _fWallHeightMin);
	void AddRuleAwayFromOtherObjects(Float _fDistMin);
	void AddRuleOccluded();
	void AddRuleNotOccluded();
	void AddRuleOutOfInterval(Float _fInfBound, Float _fSupBound, S32 _Idxs);


	// Constraints
	DynArray_Z<FRGConstraint_W> m_daConstraints;

	void AddConstraintNearOf(const Vec3f& _vPoint, Float _fDistMin = 0.f, Float _fDistMax = 0.f);
	void AddConstraintNearOfWall(Bool _virtual = FALSE, Float _height = 0.f, Float _fDistMin = 0.f, Float _fDistMax = 0.f);
	void AddConstraintWayFromWall();
	void AddConstraintNearfOfCenter(Float _fDistMin = 0.f, Float _fDistMax = 0.f);
	void AddConstraintOnSegment(const Segment_Z& _segment);
	void AddConstraintFacing(const Segment_Z& _segment);
	void AddConstraintRayCast(const Segment_Z& _segment);
	void AddConstraintAwayFromOtherObjects();
	void AddConstraintAwayFromPoint(const Vec3f& _vPoint);


	// Next variables are affected by the solver
	S32 m_idx;

	Vec3f m_vPos;
	Quat m_qRot;
	S8 m_univers;
 	U8 m_SolvingFlags;

	FRGAVL<Float, Position3D_L> m_avlPossiblePos;
};

#endif