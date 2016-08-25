// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#ifndef __FRG_SOLVER_H__
#define __FRG_SOLVER_H__

class FRGConstraint_W;
class FRGRule_W;
class FRGSolvingInfos_W;
class PlaySpaceInfos_W;
namespace ShapeReco {class ShapeAnalyzer_W;}
class TopologyAnalyzer_W;

// To make life easier for the user (the user just have to include <FRGSolver/FRGSolver_W.h>)
#include <FRGSolver/FRGSolvingInfos_W.h>

#include <Topology/TopologyAnalyzer_W.h>

//------------------------------------------------------------------------------------
// FRGSolver_W
//------------------------------------------------------------------------------------
class FRGSolver_W
{
public:
	FRGSolver_W();
	~FRGSolver_W();

	void Init(const PlaySpaceInfos_W* _pPlaySpaceInfos, const TopologyAnalyzer_W* _pTopologyAnalyzer, const ShapeReco::ShapeAnalyzer_W* _pShapeAnalyzer);
	void Reset();

	Bool HavePlaySpaceInfos() const;
	Bool HaveInitializedPlaySpaceInfos() const;

	// Main solving functions
	Bool Solve(FRGSolvingInfos_W& _infos);

	S32 GetNbSolvedObjects() const;
	const FRGSolvingInfos_W& GetSolvingInfos(S32 _idx) const;
	void SetSolvingInfosAtIndex(S32 _idx,const FRGSolvingInfos_W& _infos);

	S32 AddSolvedObject(const FRGSolvingInfos_W& _infos);
	S32 AddSolvedBox(const Vec3f& _vPos, const Vec3f& _vSize, const Quat& _qRot, const String_Z<ARRAY_CHAR_MAX>& _name, S8 _univers = 0, U8 _flags=0x00); // Just a shorcut
	void RemoveSolvedBox(S32 _idx);
	void RemoveAllSolved();

	// Debug
	void DrawDebug(const Color& _color) const;
	void DrawDebug() const;

	void SetDefaultDebugColor(const Color& _color);
	void SetBoxNameFilter(const Char* _sBoxNameFilter);

	static void DrawBox(const Color& _color, const Vec3f& _vPos, const Vec3f& _vSize, const Quat& _qRot, const String_Z<ARRAY_CHAR_MAX>& _sName );

	// Rules
	Bool CheckRules(const FRGSolvingInfos_W& _infos) const;
	Bool CheckRule(const FRGSolvingInfos_W& _infos, const FRGRule_W& _rule) const;

	Bool CheckRuleAwayFrom(const FRGSolvingInfos_W& _infos, const FRGRule_W& _rule) const;
	Bool CheckRuleAwayFromWalls(const FRGSolvingInfos_W& _infos, const FRGRule_W& _rule) const;
	Bool CheckRuleAwayFromOtherObjects(const FRGSolvingInfos_W& _infos, const FRGRule_W& _rule) const;
	Bool CheckRuleOutOfInterval(const FRGSolvingInfos_W& _infos, const FRGRule_W&	_rule) const;

	Bool CheckClearPathToOppositeWall(FRGSolvingInfos_W& _infos);
	Bool CheckClearPathToCeiling(FRGSolvingInfos_W& _infos, Vec3f _vOnFloorPos, Vec3f _vOnCeilingPos);
	
	// Constraints
	Float ComputeScoreWithConstraints(const FRGSolvingInfos_W& _infos) const;
	Float ComputeScoreWithConstraint(const FRGSolvingInfos_W& _infos, const FRGConstraint_W& _constraint) const;

	Float ComputeScoreNearOf(const FRGSolvingInfos_W& _infos, const FRGConstraint_W& _constraint) const;
	Float ComputeScoreNearOfCenter(const FRGSolvingInfos_W& _infos, const FRGConstraint_W& _constraint) const;
	Float ComputeScoreNearOfWall(const FRGSolvingInfos_W& _infos, const FRGConstraint_W& _constraint) const;
	Float ComputeScoreAwayFromWall(const FRGSolvingInfos_W& _infos, const FRGConstraint_W& _constraint) const;
	Float ComputeScoreOnSegment(const FRGSolvingInfos_W& _infos, const FRGConstraint_W& _constraint) const;
	Float ComputeScoreFacing(const FRGSolvingInfos_W& _infos, const FRGConstraint_W& _constraint) const;
	Float ComputeScoreRayCast(const FRGSolvingInfos_W& _infos, const FRGConstraint_W& _constraint) const;
	Float ComputeScoreAwayFromOtherObjects(const FRGSolvingInfos_W& _infos, const FRGConstraint_W& _constraint) const;
	Float ComputeScoreAwayFromPoint(const FRGSolvingInfos_W& _infos, const FRGConstraint_W& _constraint) const;

	// Collisions management
	void PrintDebugCollision(const FRGSolvingInfos_W& _infos, const FRGSolvingInfos_W& _other) const;

	Bool CollidWithOtherObjects(const FRGSolvingInfos_W& _infos, U8 _uFlags = 0) const;
	Bool CollidWithOtherObjects(const Vec3f& _vPos, const Vec3f& _vSize, const Quat& _qRot, S8 _univers = 0,U8 _flags=0x00) const;
	void GetAllObjects(DynArray_Z<const FRGSolvingInfos_W*,128,FALSE,FALSE>& _outObjects, S8 _univers = 0,U8 _flags=0x00) const;

	static Bool BoxCollide(const Vec3f& _vPos1, const Vec3f& _vSize1, const SafeArray_Z<Vec3f, 8, FALSE, FALSE>& _box1, const Vec3f& _vPos2, const Vec3f& _vSize2, const Quat& _qRot2);	
	static Bool BuildBox(const Vec3f& _vPos1, const Vec3f& _vSize1, const Quat& _qRot, SafeArray_Z<Vec3f, 8, FALSE, FALSE>& _outBox);
	static Bool BoxCollide(const SafeArray_Z<Vec3f, 8, FALSE, FALSE>& _box1, const SafeArray_Z<Vec3f, 8, FALSE, FALSE>& _box2);
	static Bool PlaneBetweenShapes(const Vec3f& _vNormal, const Vec3f& _vPoint, const SafeArray_Z<Vec3f, 8, FALSE, FALSE>& _shape1, const SafeArray_Z<Vec3f, 8, FALSE, FALSE>& _shape2);

	// Positions computation
	static Bool CheckWallType(const FRGSolvingInfos_W& _infos, const TopologyAnalyzer_W::Wall& _wall);

	void ComputeBox(FRGSolvingInfos_W& _infos,const Vec3f& _pos, const Quat& _rot, Vec3f& _topleft,Vec3f& _topright,Vec3f& _bottomleft,Vec3f& _bottomright);
	Bool ComputePos(FRGSolvingInfos_W& _infos);

	void ComputeWallLateralBrowsing(FRGSolvingInfos_W& _infos, const TopologyAnalyzer_W::Wall& _wall, Float& _outFirstLateralOffset, Float& _outLastLateralOffset, Float& _outLateralIncrement);
	Bool ComputeWallYBrowsing(FRGSolvingInfos_W& _infos, const TopologyAnalyzer_W::Wall& _wall, Float _fLateralOffset, Float& _outFirstY, Float& _outLastY, Float& _outYIncrement);

	Bool ComputePosOnFloor(FRGSolvingInfos_W& _infos);
	Bool ComputePosOnWall(FRGSolvingInfos_W& _infos);
	Bool ComputePosOnCeiling(FRGSolvingInfos_W& _infos);
	Bool ComputePosOnShape(FRGSolvingInfos_W& _infos);
	Bool ComputePosOnEdge(FRGSolvingInfos_W& _infos);
	Bool ComputePosOnFloorAndCeiling(FRGSolvingInfos_W& _infos);
	Bool ComputePosRandomInTheAir(FRGSolvingInfos_W& _infos);
	Bool ComputePosInTheMidAir(FRGSolvingInfos_W& _infos);
	Bool ComputePosUnderFurnitureEdge(FRGSolvingInfos_W& _infos);

	Bool ComputePosOnSurfaces(FRGSolvingInfos_W& _infos, const DynArray_Z<const TopologyAnalyzer_W::Surface*>& _daSurfaces, Float _fYOffset);

	const DynArray_Z<FRGSolvingInfos_W>& GetSolvedObjects() const { return  m_daSolvedObjects;}
private:
	Color m_defaultDebugColor;
	String_Z<ARRAY_CHAR_MAX> m_sBoxNameFilter;

	Random_Z m_randomGenerator;

	Float m_fSizeVoxel;
	Float m_fYGround;
	Float m_fYCeiling;

	Bool m_bPerfBuild;

	SafeArray_Z<BoolDA, 10> m_saSRGrid; // Several grids for several size of path (= several size of wall)
	SafeArray_Z<S32DA, 10> m_saSRConexity;
	Vec2i m_vSRGridSize;

	const PlaySpaceInfos_W* m_pPlaySpaceInfos;
	const TopologyAnalyzer_W* m_pTopologyAnalyzer;
	const ShapeReco::ShapeAnalyzer_W* m_pShapeAnalyzer;

	DynArray_Z<FRGSolvingInfos_W> m_daSolvedObjects;
};

#endif