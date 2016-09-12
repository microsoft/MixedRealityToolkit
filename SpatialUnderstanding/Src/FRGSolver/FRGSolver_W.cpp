// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include <pch.h>
#include <FRGSolver/FRGPathFinding_W.h>
#include <FRGSolver/FRGSolver_W.h>
#include <FRGSolver/FRGSolvingInfos_W.h>
#include <FRGSolver/FRGUtils_W.h>
#include <PlaySpace/PlaySpaceInfos_W.h>
#include <Topology/ShapeAnalyzer_W.h>
#include <Topology/TopologyAnalyzer_W.h>

FRGSolver_W::FRGSolver_W()
{
	Reset();
	m_defaultDebugColor = COLOR_BLUE;
}

FRGSolver_W::~FRGSolver_W()
{
}

void FRGSolver_W::Init(const PlaySpaceInfos_W* _pPlaySpaceInfos, const TopologyAnalyzer_W* _pTopologyAnalyzer, const ShapeReco::ShapeAnalyzer_W* _pShapeAnalyzer)
{
	m_bPerfBuild = false;

	if(!m_bPerfBuild)
	{
		m_randomGenerator.InitRandom(GetDAbsoluteTime() * 1000.f);
	}

	m_pPlaySpaceInfos = _pPlaySpaceInfos;
	m_pTopologyAnalyzer = _pTopologyAnalyzer;
	m_pShapeAnalyzer = _pShapeAnalyzer;

	m_fSizeVoxel = m_pPlaySpaceInfos->m_SizeVoxel;
	m_fYGround = m_pPlaySpaceInfos->m_YGround;
	m_fYCeiling = m_pPlaySpaceInfos->m_YCeiling;
}

void FRGSolver_W::Reset()
{
	m_fSizeVoxel = -1.f;
	m_pPlaySpaceInfos = NULL;
	m_pTopologyAnalyzer = NULL;
	m_pShapeAnalyzer = NULL;

	m_daSolvedObjects.Flush();
}

Bool FRGSolver_W::HavePlaySpaceInfos() const
{
	return (m_pPlaySpaceInfos != NULL);
}

Bool FRGSolver_W::HaveInitializedPlaySpaceInfos() const
{
	return (m_pPlaySpaceInfos != NULL && m_pPlaySpaceInfos->IsInitialized());
}

Bool FRGSolver_W::Solve(FRGSolvingInfos_W& _infos)
{
	if (ComputePos(_infos))
	{
		if (!_infos.m_bOnlyComputePossiblePos &&
			_infos.m_position.m_type != FRG_POSITION_ON_EDGE &&
			_infos.m_position.m_type != FRG_POSITION_ON_FLOOR_AND_CEILING)
		{
			_infos.m_idx = m_daSolvedObjects.GetSize();
			if (_infos.m_bCheckOppositePos)
			{
				_infos.m_vClearanceSize.x /= _infos.m_fFactor;
				_infos.m_vClearanceSize.z /= _infos.m_fFactor;
				_infos.m_vSize.x /= _infos.m_fFactor;
				_infos.m_vSize.z /= _infos.m_fFactor;
			}
			else
			{
				_infos.m_vClearanceSize /= _infos.m_fFactor;
				_infos.m_vSize /= _infos.m_fFactor;
			}

			AddSolvedObject(_infos);
		}

		return TRUE;
	}

	return FALSE;
}

S32 FRGSolver_W::GetNbSolvedObjects() const
{
	return m_daSolvedObjects.GetSize();
}

const FRGSolvingInfos_W& FRGSolver_W::GetSolvingInfos(S32 _idx) const
{
#ifdef _GAMEDEBUG
	EXCEPTIONC_Z(_idx >= 0 && _idx < m_daSolvedObjects.GetSize(), "FRGSolver_W::GetSolvingInfos bad idx");
#endif

	return m_daSolvedObjects[_idx];
}

void FRGSolver_W::SetSolvingInfosAtIndex(S32 _idx,const FRGSolvingInfos_W& _infos)
{
#ifdef _GAMEDEBUG
	EXCEPTIONC_Z(_idx >= 0 && _idx < m_daSolvedObjects.GetSize(), "FRGSolver_W::SetSolvingInfosAtIndex bad idx");
#endif

	m_daSolvedObjects[_idx] = _infos;
	m_daSolvedObjects[_idx].ComputeWorldPos();
}

S32 FRGSolver_W::AddSolvedObject(const FRGSolvingInfos_W& _infos)
{
	S32 idx = -1;

	for(S32 i = 0; i < m_daSolvedObjects.GetSize(); ++i)
	{
		if (_infos.m_univers == m_daSolvedObjects[i].m_univers && m_daSolvedObjects[i].GetObjectName() == _infos.GetObjectName())
		{
			idx = i;
			break;
		}
	}

	if (idx == -1)
		idx = m_daSolvedObjects.Add();

	m_daSolvedObjects[idx] = _infos;
	m_daSolvedObjects[idx].m_idx = idx;
	m_daSolvedObjects[idx].ComputeWorldPos();

	return idx;
}

void FRGSolver_W::RemoveAllSolved()
{
	m_daSolvedObjects.Flush();
}

void FRGSolver_W::RemoveSolvedBox(S32 _idx)
{
	m_daSolvedObjects.Remove(_idx);
}

S32 FRGSolver_W::AddSolvedBox(const Vec3f& _vPos, const Vec3f& _vSize, const Quat& _qRot, const String_Z<ARRAY_CHAR_MAX>& _name, S8 _univers, U8 _flags)
{
	FRGSolvingInfos_W infos;
	infos.m_vPos = _vPos;
	infos.m_vSize = _vSize;
	infos.m_qRot = _qRot;
	infos.m_univers = _univers;
	infos.m_SolvingFlags = _flags;
	infos.m_vEmptySize = _vSize;
	infos.m_vEmptyCenter = VEC3F_NULL;
	infos.m_vClearanceSize = _vSize;
	infos.m_vClearanceCenter = VEC3F_NULL;
	infos.m_bHasFullBox = FALSE;
	infos.m_bAllowPartiallyInWall = FALSE;
	infos.SetObjectName(_name);
	return AddSolvedObject(infos);
}

void FRGSolver_W::DrawDebug(const Color& _color) const
{
	String_Z<ARRAY_CHAR_MAX> name;

	for (S32 i = 0; i < m_daSolvedObjects.GetSize(); ++i)
	{
		if (!m_sBoxNameFilter.IsEmpty() && m_daSolvedObjects[i].GetObjectNameStr().StrStr(m_sBoxNameFilter) == NULL)
			continue;

		Color color = _color;
		if( m_daSolvedObjects[i].m_bUseSmallerBox )
			color = COLOR_RED;

		name.Sprintf("%s_SR_BOX", m_daSolvedObjects[i].GetObjectNameStr().Get());
		DrawBox(color, m_daSolvedObjects[i].m_vPos, m_daSolvedObjects[i].m_vSize, m_daSolvedObjects[i].m_qRot,name);

		name.Sprintf("%s_CLEARANCE_BOX", m_daSolvedObjects[i].GetObjectNameStr().Get());
		DrawBox(color, m_daSolvedObjects[i].m_vPos + m_daSolvedObjects[i].m_qRot * m_daSolvedObjects[i].m_vClearanceCenter, m_daSolvedObjects[i].m_vClearanceSize, m_daSolvedObjects[i].m_qRot, name);
	
		if (m_daSolvedObjects[i].ClearanceBoxAndEmptyBoxAreDifferent())
		{
			name.Sprintf("%s_EMPTY_BOX", m_daSolvedObjects[i].GetObjectNameStr().Get());
			DrawBox(_color, m_daSolvedObjects[i].m_vPos + m_daSolvedObjects[i].m_qRot * m_daSolvedObjects[i].m_vEmptyCenter, m_daSolvedObjects[i].m_vEmptySize, m_daSolvedObjects[i].m_qRot, name);
		}
	}
}

void FRGSolver_W::DrawDebug() const
{
	DrawDebug(m_defaultDebugColor);
}

void FRGSolver_W::SetDefaultDebugColor(const Color& _color)
{
	m_defaultDebugColor = _color;
}

void FRGSolver_W::SetBoxNameFilter(const Char* _sBoxNameFilter)
{
	m_sBoxNameFilter.StrCpy(_sBoxNameFilter);
}

void FRGSolver_W::DrawBox(const Color& _color, const Vec3f& _vPos, const Vec3f& _vSize, const Quat& _qRot, const String_Z<ARRAY_CHAR_MAX>& _sName )
{
	Vec3f halfSize = _vSize * 0.5f;

	DRAW_DEBUG_LINE3D_FAST(_vPos + _qRot * Vec3f(-halfSize.x, -halfSize.y, -halfSize.z), _vPos + _qRot * Vec3f(+halfSize.x, -halfSize.y, -halfSize.z), _color);
	DRAW_DEBUG_LINE3D_FAST(_vPos + _qRot * Vec3f(-halfSize.x, -halfSize.y, +halfSize.z), _vPos + _qRot * Vec3f(+halfSize.x, -halfSize.y, +halfSize.z), _color);
	DRAW_DEBUG_LINE3D_FAST(_vPos + _qRot * Vec3f(-halfSize.x, +halfSize.y, -halfSize.z), _vPos + _qRot * Vec3f(+halfSize.x, +halfSize.y, -halfSize.z), _color);
	DRAW_DEBUG_LINE3D_FAST(_vPos + _qRot * Vec3f(-halfSize.x, +halfSize.y, +halfSize.z), _vPos + _qRot * Vec3f(+halfSize.x, +halfSize.y, +halfSize.z), _color);
	DRAW_DEBUG_LINE3D_FAST(_vPos + _qRot * Vec3f(-halfSize.x, -halfSize.y, -halfSize.z), _vPos + _qRot * Vec3f(-halfSize.x, +halfSize.y, -halfSize.z), _color);
	DRAW_DEBUG_LINE3D_FAST(_vPos + _qRot * Vec3f(+halfSize.x, -halfSize.y, -halfSize.z), _vPos + _qRot * Vec3f(+halfSize.x, +halfSize.y, -halfSize.z), _color);
	DRAW_DEBUG_LINE3D_FAST(_vPos + _qRot * Vec3f(-halfSize.x, -halfSize.y, +halfSize.z), _vPos + _qRot * Vec3f(-halfSize.x, +halfSize.y, +halfSize.z), _color);
	DRAW_DEBUG_LINE3D_FAST(_vPos + _qRot * Vec3f(+halfSize.x, -halfSize.y, +halfSize.z), _vPos + _qRot * Vec3f(+halfSize.x, +halfSize.y, +halfSize.z), _color);
	DRAW_DEBUG_LINE3D_FAST(_vPos + _qRot * Vec3f(-halfSize.x, -halfSize.y, -halfSize.z), _vPos + _qRot * Vec3f(-halfSize.x, -halfSize.y, +halfSize.z), _color);
	DRAW_DEBUG_LINE3D_FAST(_vPos + _qRot * Vec3f(+halfSize.x, -halfSize.y, -halfSize.z), _vPos + _qRot * Vec3f(+halfSize.x, -halfSize.y, +halfSize.z), _color);
	DRAW_DEBUG_LINE3D_FAST(_vPos + _qRot * Vec3f(-halfSize.x, +halfSize.y, -halfSize.z), _vPos + _qRot * Vec3f(-halfSize.x, +halfSize.y, +halfSize.z), _color);
	DRAW_DEBUG_LINE3D_FAST(_vPos + _qRot * Vec3f(+halfSize.x, +halfSize.y, -halfSize.z), _vPos + _qRot * Vec3f(+halfSize.x, +halfSize.y, +halfSize.z), _color);

	if( _sName.GetSize() )
	{
		Vec3f dir = Vec3f(halfSize.x, halfSize.y, halfSize.z);
		dir.SetNorm(.25f);
		Vec3f pos = _vPos + _qRot * Vec3f(halfSize.x, halfSize.y, halfSize.z);
		DRAW_DEBUG_LINE3D_FAST(pos, pos + dir, _color);
		DRAW_DEBUG_STRING3D_FAST(pos + dir, _color,"%s",_sName.Get());

	}
}

Bool FRGSolver_W::CheckRules(const FRGSolvingInfos_W& _infos) const
{
	for (S32 i = 0; i < _infos.m_daRules.GetSize(); ++i)
	{
		if (!CheckRule(_infos, _infos.m_daRules[i]))
			return FALSE;
	}

	return TRUE;
}

Bool FRGSolver_W::CheckRule(const FRGSolvingInfos_W& _infos, const FRGRule_W& _rule) const
{
	switch (_rule.m_type)
	{
	case FRG_RULE_AWAY_FROM:
		return CheckRuleAwayFrom(_infos, _rule);
	case FRG_RULE_AWAY_FROM_WALLS:
		return CheckRuleAwayFromWalls(_infos, _rule);
	case FRG_RULE_AWAY_FROM_OTHER_OBJECTS:
		return CheckRuleAwayFromOtherObjects(_infos, _rule);
	case FRG_RULE_OUT_OF_INTERVAL:
		return CheckRuleOutOfInterval(_infos, _rule);
	}

#ifdef _GAMEDEBUG
	CANCEL_EXCEPTIONC_Z(FALSE, "FRGSolver_W::CheckRule unkonkwn rule type");
#endif

	return FALSE;
}

Bool FRGSolver_W::CheckRuleAwayFrom(const FRGSolvingInfos_W& _infos, const FRGRule_W& _rule) const
{
	Float minDistSquare = Square(_rule.m_paramsAwayFrom.m_fDistMin);

	return ((_infos.m_vPos - _rule.m_paramsAwayFrom.m_vPoint).HGetNorm2() >= minDistSquare);
}

Bool FRGSolver_W::CheckRuleAwayFromWalls(const FRGSolvingInfos_W& _infos, const FRGRule_W& _rule) const
{
#ifdef _GAMEDEBUG
	EXCEPTIONC_Z(m_pTopologyAnalyzer != NULL, "FRGSolver_W::CheckRuleAwayFromWalls no TopologyAnalyzer defined");
#endif

	return !m_pTopologyAnalyzer->HasWallNear(_infos.m_vPos, _rule.m_paramsAwayFromWalls.m_fDistMin, _rule.m_paramsAwayFromWalls.m_fDistMin);
}

Bool FRGSolver_W::CheckRuleAwayFromOtherObjects(const FRGSolvingInfos_W& _infos, const FRGRule_W& _rule) const
{
	Float minDistSquare = Square(_rule.m_paramsAwayFromOtherObjects.m_fDistMin);

    for (S32 i = 0; i < m_daSolvedObjects.GetSize(); ++i)
	{
		if ((_infos.m_vPos - m_daSolvedObjects[i].m_vPos).HGetNorm2() < minDistSquare)
			return FALSE;
	}

	return TRUE;
}

Bool FRGSolver_W::CheckRuleOutOfInterval(const FRGSolvingInfos_W& _infos, const FRGRule_W& _rule) const
{
	Float fInfBound2 = Square(_rule.m_paramsOutOfInterval.m_fInfBound);
	Float fSupBound2 = Square(_rule.m_paramsOutOfInterval.m_fSupBound);

	Vec3f vPos = m_daSolvedObjects[_rule.m_paramsOutOfInterval.m_iObjectsToCheck].m_vPos;
	Float fDist2 = (_infos.m_vPos - vPos).GetNorm2();
	if(fDist2 > fInfBound2 &&  fDist2 < fSupBound2)
		return FALSE;

	return TRUE;
}

//Check if there is something between the current pos and the opposite wall
Bool FRGSolver_W::CheckClearPathToOppositeWall(FRGSolvingInfos_W& _infos)
{
#ifdef _GAMEDEBUG
	EXCEPTIONC_Z(m_pTopologyAnalyzer != NULL, "FRGSolver_W::CheckClearPathToOppositeWall no TopologyAnalyzer defined");
#endif

	TopologyRayCastResult_W result;
	if (!m_pTopologyAnalyzer->TopologyRayCast(_infos.m_vPos, _infos.m_qRot * VEC3F_FRONT, result))
		return FALSE;

	if (result.m_type != TopologyRayCastResult_W::WALL)
		return FALSE;

	Vec3f vOppositeCenter = result.m_vPos;
	S32 iOppositeWallIdx = result.m_iWallIdx;

	EXCEPTION_Z(iOppositeWallIdx >= 0 && iOppositeWallIdx < m_pTopologyAnalyzer->m_daWalls.GetSize());
	const TopologyAnalyzer_W::Wall& wall = m_pTopologyAnalyzer->m_daWalls[iOppositeWallIdx];

	if (!CheckWallType(_infos, wall))
		return FALSE;

	Bool isFullWall = m_pTopologyAnalyzer->IsFullWall(wall);
	if (isFullWall)
	{
		if (_infos.m_position.m_paramsOnWall.m_bNotOnFullWall)
			return FALSE;
	}
	else
	{
		if (_infos.m_position.m_paramsOnWall.m_bOnlyFullWall)
			return FALSE;
	}

	// Wall must be parallel from the first wall
	Float fDot = (_infos.m_qRot * VEC3F_LEFT) * wall.m_vNormal;
	if (fDot > 0.1f || fDot < -0.1f)
		return FALSE;

	Float fDeltaX = _infos.m_vSize.x * 0.5f;
	Float fDeltaY = _infos.m_vSize.y * 0.5f;

	// Ray cast from the 4 angles to be sure nothing is on the way
	for (S32 i = -1; i <= 1; i += 2)										
	{
		for (S32 j = -1; j <= 1; j += 2)
		{
			Vec3f vOrg = _infos.m_vPos;
			vOrg += _infos.m_qRot * VEC3F_LEFT * i * fDeltaX;
			vOrg += _infos.m_qRot * VEC3F_UP * j * fDeltaY;
			
			TopologyRayCastResult_W result2;
			if (!m_pTopologyAnalyzer->TopologyRayCast(vOrg, _infos.m_qRot * VEC3F_FRONT, result2))
				return FALSE;

			if (result2.m_type != TopologyRayCastResult_W::WALL)
				return FALSE;

			fDot = wall.m_vNormal * (vOppositeCenter - result.m_vPos);
			if (fDot > 0.01f || fDot < -0.01f)
				return FALSE;
		}
	}

	_infos.m_vOppositePos = vOppositeCenter;
	return TRUE;
}

//Check if there is something between the floor pos and the ceiling pos
Bool FRGSolver_W::CheckClearPathToCeiling(FRGSolvingInfos_W& _infos, Vec3f _vOnFloorPos, Vec3f _vOnCeilingPos)
{
#ifdef _GAMEDEBUG
	EXCEPTIONC_Z(m_pTopologyAnalyzer != NULL, "FRGSolver_W::CheckClearPathToOppositeWall no TopologyAnalyzer defined");
#endif

	TopologyRayCastResult_W result;
	if (!m_pTopologyAnalyzer->TopologyRayCast(_infos.m_vPos, VEC3F_UP, result))
		return FALSE;

	if (result.m_type != TopologyRayCastResult_W::SURFACE)
		return FALSE;

	if (!m_pTopologyAnalyzer->m_daSurfaces[result.m_iSufaceIdx].m_bIsCeiling)
		return FALSE;

	Float fDeltaX = _infos.m_vSize.x * 0.5f;
	Float fDeltaZ = _infos.m_vSize.z * 0.5f;
	
	// Ray cast from the 4 angles to be sure nothing is on the way
	for (S32 i = -1; i <= 1; i += 2)										
	{
		for (S32 j = -1; j <= 1; j += 2)
		{
			Vec3f vOrg = _vOnFloorPos;
			vOrg += _infos.m_qRot * VEC3F_LEFT * i * fDeltaX;
			vOrg += _infos.m_qRot * VEC3F_FRONT * j * fDeltaZ;

			TopologyRayCastResult_W result2;
			if (!m_pTopologyAnalyzer->TopologyRayCast(vOrg, VEC3F_UP, result2))
				return FALSE;

			if (result2.m_type != TopologyRayCastResult_W::SURFACE)
				return FALSE;

			if (!m_pTopologyAnalyzer->m_daSurfaces[result2.m_iSufaceIdx].m_bIsCeiling)
				return FALSE;

			Float fDot = VEC3F_UP * (result2.m_vPos - result.m_vPos);
			if (fDot > 0.01f || fDot < -0.01f)
				return FALSE;
		}
	}

	_infos.m_vOppositePos = result.m_vPos;
	return TRUE;
}

Float FRGSolver_W::ComputeScoreWithConstraints(const FRGSolvingInfos_W& _infos) const
{
	if (_infos.m_daConstraints.GetSize() == 0)
		return 1.f; // No constraints = maximum score

	Float ratio = 1.f / static_cast<Float>(_infos.m_daConstraints.GetSize());
	Float score = 0.f;

	for (S32 i = 0; i < _infos.m_daConstraints.GetSize(); ++i)
		score += ComputeScoreWithConstraint(_infos, _infos.m_daConstraints[i]) * ratio;

	return score;
}

Float FRGSolver_W::ComputeScoreWithConstraint(const FRGSolvingInfos_W& _infos, const FRGConstraint_W& _constraint) const
{
	switch (_constraint.m_type)
	{
	case FRG_CONSTRAINT_NEAR_OF:
		return ComputeScoreNearOf(_infos, _constraint);
	case FRG_CONSTRAINT_NEAR_OF_CENTER:
		return ComputeScoreNearOfCenter(_infos, _constraint);
	case FRG_CONSTRAINT_NEAR_OF_WALL:
		return ComputeScoreNearOfWall(_infos, _constraint);
	case FRG_CONSTRAINT_AWAY_FROM_WALL:
		return ComputeScoreAwayFromWall(_infos, _constraint);
	case FRG_CONSTRAINT_ON_SEGMENT:
		return ComputeScoreOnSegment(_infos, _constraint);
	case FRG_CONSTRAINT_FACING:
		return ComputeScoreFacing(_infos, _constraint);
	case FRG_CONSTRAINT_RAYCAST:
		return ComputeScoreRayCast(_infos, _constraint);
	case FRG_CONSTRAINT_AWAY_FROM_OTHER_OBJECTS:
		return ComputeScoreAwayFromOtherObjects(_infos, _constraint);
	case FRG_CONSTRAINT_AWAY_FROM_POINT:
		return ComputeScoreAwayFromPoint(_infos, _constraint);
	}

#ifdef _GAMEDEBUG
	CANCEL_EXCEPTIONC_Z(FALSE, "FRGSolver_W::ComputeScoreWithConstraint unkonkwn constraint type");
#endif

	return 0.f; // Unknown constraint = minimum score
}

Float FRGSolver_W::ComputeScoreNearOf(const FRGSolvingInfos_W& _infos, const FRGConstraint_W& _constraint) const
{
#ifdef _GAMEDEBUG
	CANCEL_EXCEPTIONC_Z(_constraint.m_paramsNearOf.m_fDistMin >= 0.f && _constraint.m_paramsNearOf.m_fDistMax >= 0.f && _constraint.m_paramsNearOf.m_fDistMin <= _constraint.m_paramsNearOf.m_fDistMax, "FRGSolver_W::ComputeScoreNearOf bad distMin/distMax");
#endif

	Float dist = Vec3f(_constraint.m_paramsNearOf.m_vPoint - _infos.m_vPos).GetNorm();

	// This function return 1 if dist is between min and max
	// and return a value who go progressively to 0 elsewere

	if (dist > _constraint.m_paramsNearOf.m_fDistMax)
		return 1.f / (dist - _constraint.m_paramsNearOf.m_fDistMax + 1.f); // f(x) = 1 / (x - max + 1)
	else if (dist < _constraint.m_paramsNearOf.m_fDistMin)
		return -1.f / (dist - _constraint.m_paramsNearOf.m_fDistMin - 1.f); // f(x) = -1 / (x - min - 1)
	else
		return 1.f;
}

Float FRGSolver_W::ComputeScoreNearOfCenter(const FRGSolvingInfos_W& _infos, const FRGConstraint_W& _constraint) const
{
#ifdef _GAMEDEBUG
	EXCEPTIONC_Z(m_pTopologyAnalyzer != NULL, "FRGSolver_W::ComputeScoreNearOfCenter no TopologyAnalyzer defined");
	CANCEL_EXCEPTIONC_Z(_constraint.m_paramsNearOfCenter.m_fDistMin >= 0.f && _constraint.m_paramsNearOfCenter.m_fDistMax >= 0.f && _constraint.m_paramsNearOfCenter.m_fDistMin <= _constraint.m_paramsNearOfCenter.m_fDistMax, "FRGSolver_W::ComputeScoreNearOfCenter bad distMin/distMax");
#endif

	Float dist = Vec3f(m_pTopologyAnalyzer->m_vRoomCenter - _infos.m_vPos).HGetNorm();

	// This function return 1 if dist is between min and max
	// and return a value who go progressively to 0 elsewere

	if (dist > _constraint.m_paramsNearOfCenter.m_fDistMax)
		return 1.f / (dist - _constraint.m_paramsNearOfCenter.m_fDistMax + 1.f); // f(x) = 1 / (x - max + 1)
	else if (dist < _constraint.m_paramsNearOfCenter.m_fDistMin)
		return -1.f / (dist - _constraint.m_paramsNearOfCenter.m_fDistMin - 1.f); // f(x) = -1 / (x - min - 1)
	else
		return 1.f;
}

Float FRGSolver_W::ComputeScoreNearOfWall(const FRGSolvingInfos_W& _infos, const FRGConstraint_W& _constraint) const
{
#ifdef _GAMEDEBUG
	CANCEL_EXCEPTIONC_Z(_constraint.m_paramsNearOfWall.m_fDistMin >= 0.f && _constraint.m_paramsNearOfWall.m_fDistMax >= 0.f && _constraint.m_paramsNearOfWall.m_fDistMin <= _constraint.m_paramsNearOfWall.m_fDistMax, "FRGSolver_W::ComputeScoreNearOfWall bad distMin/distMax");
#endif
	Float dist = m_pTopologyAnalyzer->GetDistWallNear(_infos.m_vPos, _constraint.m_paramsNearOfWall.m_fHeight, _constraint.m_paramsNearOfWall.m_bVirtual);

	// This function return 1 if dist is between min and max
	// and return a value who go progressively to 0 elsewere

	if (dist > _constraint.m_paramsNearOf.m_fDistMax)
		return 1.f / (dist - _constraint.m_paramsNearOfWall.m_fDistMax + 1.f); // f(x) = 1 / (x - max + 1)
	else if (dist < _constraint.m_paramsNearOf.m_fDistMin)
		return -1.f / (dist - _constraint.m_paramsNearOfWall.m_fDistMin - 1.f); // f(x) = -1 / (x - min - 1)
	else
		return 1.f;
}

Float FRGSolver_W::ComputeScoreAwayFromWall(const FRGSolvingInfos_W& _infos, const FRGConstraint_W& _constraint) const
{
#ifdef _GAMEDEBUG
	EXCEPTIONC_Z(m_pTopologyAnalyzer != NULL, "FRGSolver_W::ComputeScoreAwayFromWall no TopologyAnalyzer defined");
#endif

	Float dist = m_pTopologyAnalyzer->GetDistWallNear(_infos.m_vPos, 1.00f,	FALSE); // At least 1m height

	return (1.f - 1.f / (dist + 1.f)); // f(x) = 1 - 1/(x + 1)
}

Float FRGSolver_W::ComputeScoreFacing(const FRGSolvingInfos_W& _infos, const FRGConstraint_W& _constraint) const
{
	Segment_Z	daseg=_constraint.m_paramsOnSegment.m_segment;
	Vec3f	OtherPos=daseg.Org;
	daseg.Org=_infos.m_vPos;
	daseg.Dir=_infos.m_qRot*daseg.Dir;

	Vec3f	dir1 = daseg.Dir;
	Vec3f	dir2 = OtherPos - daseg.Org;
	dir1.CHNormalize();
	dir2.CHNormalize();
	Float	fdot = dir1 * dir2;

	Float score;
	
	if (fdot >0.f)
	{
		fdot=fdot/Max(0.1f,(1.f-daseg.Len));
		fdot=ClampVal(fdot);
		score= fdot * 0.9f + 0.1f;	// f(x) = 1 / (x + 1)
	}
	else
		score = fdot * 0.1f + 0.1f;	// f(x) = 1 / (x + 1)

	// Pos behind are bad
//	if (daseg.Dir * (OtherPos - daseg.Org) <= 0.f)
//       return score = (score >= 0.5f ? score - 0.5f : 0.f);

    return score;
}

Float FRGSolver_W::ComputeScoreRayCast(const FRGSolvingInfos_W& _infos, const FRGConstraint_W& _constraint) const
{

	////// !!!!!!!! _constraint.m_paramsOnSegment.m_segment => segment d'origine locale à l'objet vers destination world
	const Segment_Z& Param = _constraint.m_paramsOnSegment.m_segment;
	Vec3f vSegOrg = _infos.m_vPos + (_infos.m_qRot * Param.Org );
	Vec3f vSegDest = Param.Org+Param.Len*Param.Dir;
	Vec3f vSegDir = vSegDest - vSegOrg;
	vSegDir.CNormalize();
	Float fLength2 = (vSegDest - vSegOrg).GetNorm2();
	TopologyRayCastResult_W result;
	if(m_pTopologyAnalyzer->TopologyRayCast(vSegOrg, vSegDir, result) && (result.m_vPos - vSegOrg).GetNorm2() < fLength2)
	{
		return 0.f;
	}
	else
	{
		return 1.f;
	}
}

Float FRGSolver_W::ComputeScoreOnSegment(const FRGSolvingInfos_W& _infos, const FRGConstraint_W& _constraint) const
{
	Vec3f vPos = _infos.m_vPos - (_infos.m_qRot * _infos.m_vCenter );
	Float score = 1.f / (_constraint.m_paramsOnSegment.m_segment.DistToPoint(vPos) + 1.f);	// f(x) = 1 / (x + 1)

	// Pos behind are bad	// move segment further to avoid behind points...
//	if (_constraint.m_paramsOnSegment.m_segment.Dir * (_infos.m_vPos - _constraint.m_paramsOnSegment.m_segment.Org) <= 0.f)
//        return score = (score >= 0.5f ? score - 0.5f : 0.f);

    return score;
}

Float FRGSolver_W::ComputeScoreAwayFromOtherObjects(const FRGSolvingInfos_W& _infos, const FRGConstraint_W& /*_constraint*/) const
{
	Float minDistSquare = Float_Max;

    for (S32 i = 0; i < m_daSolvedObjects.GetSize(); ++i)
    {
        Float distSquare = Vec3f(m_daSolvedObjects[i].m_vPos - _infos.m_vPos).GetNorm2();
        if (distSquare < minDistSquare)
            minDistSquare = distSquare;
    }

    return 1.f + 1.f / (-1.f - minDistSquare); // f(x) = 1 + 1 / (-1 - x)
}

Float FRGSolver_W::ComputeScoreAwayFromPoint(const FRGSolvingInfos_W& _infos, const FRGConstraint_W& _constraint) const
{
	Float	lowestscore=1.f;
	for (int i=0; i<_constraint.m_paramsAwayFromPoint.m_vPoint.GetSize(); i++)
	{
		Float distSquare = (_constraint.m_paramsAwayFromPoint.m_vPoint[i] - _infos.m_vPos).GetNorm2();;
		Float	thisscore=1.f + 1.f / (-1.f - distSquare); // f(x) = 1 + 1 / (-1 - x);
		if (thisscore<lowestscore)
			lowestscore=thisscore;
	}

    return lowestscore;
}

void FRGSolver_W::PrintDebugCollision(const FRGSolvingInfos_W& _infos, const FRGSolvingInfos_W& _other) const
{
	static Name_ZDA DegubAlreadyPrinted;
	S32 IdToPrint = Name_Z("Mirror").ID;

	if (_infos.GetObjectName().ID == IdToPrint)
	{
		if (DegubAlreadyPrinted.Contains(_other.GetObjectName()) == -1)
		{
			DegubAlreadyPrinted.Add(_other.GetObjectName());
			MESSAGE_Z("%s COLLIDE WITH %s", _infos.GetObjectNameStr().Get(), _other.GetObjectNameStr().Get());
		}
	}
}

Bool FRGSolver_W::CollidWithOtherObjects(const FRGSolvingInfos_W& _infos, U8 _uFlags) const
{	
	// 0.9999f pour éviter que deux objets se touchent alors qu'ils se touchent pas.
	Vec3f vClearanceSize = _infos.m_vClearanceSize * 0.9999f;
	Vec3f vEmptySize = _infos.m_vEmptySize * 0.9999f;
	Vec3f vClearanceWorldCenter = _infos.m_vPos + _infos.m_qRot * _infos.m_vClearanceCenter;
	Vec3f vEmptyWorldCenter = _infos.m_vPos + _infos.m_qRot * _infos.m_vEmptyCenter;

		// Generic algorithm
    SafeArray_Z<Vec3f, 8, FALSE, FALSE> clearanceBox,emptyBox;
	BuildBox(vClearanceWorldCenter, vClearanceSize, _infos.m_qRot, clearanceBox);
	if( _infos.ClearanceBoxAndEmptyBoxAreDifferent() )
		BuildBox(vEmptyWorldCenter, vEmptySize, _infos.m_qRot, emptyBox);

	for (S32 i = 0; i < m_daSolvedObjects.GetSize(); ++i)
	{
		const FRGSolvingInfos_W& other = m_daSolvedObjects[i];
		
		if (_uFlags != 0 && (other.m_SolvingFlags & _uFlags) == 0)
			continue;

		// Check ignored sovled box
		if (_infos.m_daIgnoredSolvedBox.Contains(other.GetObjectName()) != -1)
			continue;
		
		// Check univers
		if (other.m_univers != INALLUNIVERS && _infos.m_univers != INALLUNIVERS && (other.m_univers < 0 || other.m_univers != _infos.m_univers)) //TODO GUERAIN
			continue;
		
		// Check clearances
		if (BoxCollide(vClearanceWorldCenter, vClearanceSize, clearanceBox, other.m_vClearanceWorldCenter, other.m_vClearanceSize, other.m_qRot))
		{
#ifdef _GAMEDEBUG
			PrintDebugCollision(_infos, other);
#endif
			return TRUE;
		}

		// Check empty boxes
		if (_infos.ClearanceBoxAndEmptyBoxAreDifferent())
		{
			if (BoxCollide(vEmptyWorldCenter, vEmptySize, emptyBox, other.m_vClearanceWorldCenter, other.m_vClearanceSize, other.m_qRot))
			{
#ifdef _GAMEDEBUG
				PrintDebugCollision(_infos, other);
#endif
				return TRUE;
			}
		}

		if (other.ClearanceBoxAndEmptyBoxAreDifferent())
		{
			if (BoxCollide(vClearanceWorldCenter, vClearanceSize, clearanceBox, other.m_vEmptyWorldCenter, other.m_vEmptySize, other.m_qRot))
			{
#ifdef _GAMEDEBUG
			PrintDebugCollision(_infos, other);
#endif
				return TRUE;
			}
		}

		// Empty vs empty.... (use for act2 window/library)
		if (_infos.ClearanceBoxAndEmptyBoxAreDifferent() && other.ClearanceBoxAndEmptyBoxAreDifferent())
		{
			if (BoxCollide(vEmptyWorldCenter, vEmptySize, emptyBox, other.m_vEmptyWorldCenter, other.m_vEmptySize, other.m_qRot))
			{
#ifdef _GAMEDEBUG
				PrintDebugCollision(_infos, other);
#endif
				return TRUE;
			}
		}
	}

	return FALSE;
}

Bool FRGSolver_W::CollidWithOtherObjects(const Vec3f& _vPos, const Vec3f& _vSize, const Quat& _qRot, S8 _univers, U8 _flags) const
{
	FRGSolvingInfos_W infos;
	infos.m_vPos = _vPos;
	infos.m_qRot = _qRot;

	infos.m_vSize = _vSize;
	infos.m_vCenter = VEC3F_NULL;
    infos.m_vEmptySize = _vSize;
    infos.m_vEmptyCenter = VEC3F_NULL;
    infos.m_vClearanceSize = _vSize;
    infos.m_vClearanceCenter = VEC3F_NULL;
 	infos.m_bHasFullBox = FALSE;

	infos.m_univers = _univers;

	return CollidWithOtherObjects(infos, _flags);
}

void FRGSolver_W::GetAllObjects(DynArray_Z<const FRGSolvingInfos_W*, 128, FALSE, FALSE>& _outObjects, S8 _univers /*= 0*/,U8 _flags/*=0x00*/) const
{
	for (S32 i = 0; i < m_daSolvedObjects.GetSize(); ++i)
	{
		const FRGSolvingInfos_W& curObj = m_daSolvedObjects[i];

		if (_flags != 0 && (curObj.m_SolvingFlags & _flags) == 0)
			continue;

		// Check univers
		if (curObj.m_univers != INALLUNIVERS && _univers != INALLUNIVERS && (curObj.m_univers < 0 || curObj.m_univers != _univers)) 
			continue;

		_outObjects.Add(&curObj);
	}
}

Bool FRGSolver_W::BuildBox(const Vec3f& _vPos1, const Vec3f& _vSize1, const Quat& _qRot1, SafeArray_Z<Vec3f, 8, FALSE, FALSE>& _outBox)
{
	Vec3f halfSize1 = _vSize1 * 0.5f;
	    
	// Generic algorithm
    _outBox[0] = _vPos1 + _qRot1 * Vec3f(+halfSize1.x, +halfSize1.y, -halfSize1.z);
    _outBox[1] = _vPos1 + _qRot1 * Vec3f(-halfSize1.x, +halfSize1.y, -halfSize1.z);
    _outBox[2] = _vPos1 + _qRot1 * Vec3f(-halfSize1.x, +halfSize1.y, +halfSize1.z);
    _outBox[3] = _vPos1 + _qRot1 * Vec3f(+halfSize1.x, +halfSize1.y, +halfSize1.z);
    _outBox[4] = _vPos1 + _qRot1 * Vec3f(+halfSize1.x, -halfSize1.y, -halfSize1.z);
    _outBox[5] = _vPos1 + _qRot1 * Vec3f(-halfSize1.x, -halfSize1.y, -halfSize1.z);
    _outBox[6] = _vPos1 + _qRot1 * Vec3f(-halfSize1.x, -halfSize1.y, +halfSize1.z);
    _outBox[7] = _vPos1 + _qRot1 * Vec3f(+halfSize1.x, -halfSize1.y, +halfSize1.z);

	return TRUE;

}

Bool FRGSolver_W::BoxCollide(const Vec3f& _vPos1, const Vec3f& _vSize1, const SafeArray_Z<Vec3f, 8, FALSE, FALSE>& _box1, const Vec3f& _vPos2, const Vec3f& _vSize2, const Quat& _qRot2)
{
	// First check sphere
    Float radius1 = Sqrt(Square(_vSize1.x* 0.5f) + Square(_vSize1.y* 0.5f) + Square(_vSize1.z* 0.5f));
    Float radius2 = Sqrt(Square(_vSize2.x* 0.5f) + Square(_vSize2.y* 0.5f) + Square(_vSize2.z* 0.5f));
	if ((_vPos2 - _vPos1).GetNorm2() > Square(radius1 + radius2))
        return FALSE;

	// Generic algorithm
    SafeArray_Z<Vec3f, 8, FALSE, FALSE> box2;
	BuildBox(_vPos2, _vSize2, _qRot2, box2);
    
	return BoxCollide(_box1, box2);
}

//-----------------------------------------------------------------------------------------------
// BoxCollide
//-----------------------------------------------------------------------------------------------
// Test if two boxes collide
// Each array should contains the 8 vertices of each box
// This algorithme is the "theoreme de l'axe séparateur" 3D adapted
Bool FRGSolver_W::BoxCollide(const SafeArray_Z<Vec3f, 8, FALSE, FALSE>& _box1, const SafeArray_Z<Vec3f, 8, FALSE, FALSE>& _box2)
{
	if (PlaneBetweenShapes(_box1[0] - _box1[1], _box1[0], _box1, _box2)) return FALSE;   
    if (PlaneBetweenShapes(_box1[1] - _box1[0], _box1[1], _box1, _box2)) return FALSE; 
    if (PlaneBetweenShapes(_box1[0] - _box1[3], _box1[0], _box1, _box2)) return FALSE;   
    if (PlaneBetweenShapes(_box1[3] - _box1[0], _box1[3], _box1, _box2)) return FALSE;
    if (PlaneBetweenShapes(_box1[0] - _box1[4], _box1[0], _box1, _box2)) return FALSE;   
    if (PlaneBetweenShapes(_box1[4] - _box1[0], _box1[4], _box1, _box2)) return FALSE;
    
    if (PlaneBetweenShapes(_box2[0] - _box2[1], _box2[0], _box2, _box1)) return FALSE;   
    if (PlaneBetweenShapes(_box2[1] - _box2[0], _box2[1], _box2, _box1)) return FALSE; 
    if (PlaneBetweenShapes(_box2[0] - _box2[3], _box2[0], _box2, _box1)) return FALSE;   
    if (PlaneBetweenShapes(_box2[3] - _box2[0], _box2[3], _box2, _box1)) return FALSE;
    if (PlaneBetweenShapes(_box2[0] - _box2[4], _box2[0], _box2, _box1)) return FALSE;   
    if (PlaneBetweenShapes(_box2[4] - _box2[0], _box2[4], _box2, _box1)) return FALSE;
    
    return TRUE;
}

//-----------------------------------------------------------------------------------------------
// PlaneBetweenShapes
//-----------------------------------------------------------------------------------------------
// Test if the plane defined by (_vNormal, _vPoint) separates the two shapes
// 1) Each point of a shape must be at the same side of the plane
// 2) The shapes must be on either side of the plane
Bool FRGSolver_W::PlaneBetweenShapes(const Vec3f& _vNormal, const Vec3f& _vPoint, const SafeArray_Z<Vec3f, 8, FALSE, FALSE>& _shape1, const SafeArray_Z<Vec3f, 8, FALSE, FALSE>& _shape2)
{
	for (S32 i=0; i < _shape1.GetSize(); ++i)
    {
        if (Vec3_Dot(_vNormal, _shape2[i] - _vPoint) < 0)
            return FALSE;
    }

	return TRUE;
}

Bool FRGSolver_W::CheckWallType(const FRGSolvingInfos_W& _infos, const TopologyAnalyzer_W::Wall& _wall)
{
	if (_wall.m_bIsVirtual && _wall.m_bIsExternal)
		return ((_infos.m_position.m_paramsOnWall.m_uWallTypes & FRG_EXTERNAL_VIRTUAL_WALL) != 0);
	else if (_wall.m_bIsVirtual)
		return ((_infos.m_position.m_paramsOnWall.m_uWallTypes & FRG_VIRTUAL_WALL) != 0);
	else if (_wall.m_bIsExternal)
		return ((_infos.m_position.m_paramsOnWall.m_uWallTypes & FRG_EXTERNAL_WALL) != 0);
	else
		return ((_infos.m_position.m_paramsOnWall.m_uWallTypes & FRG_NORMAL_WALL) != 0);
}

void FRGSolver_W::ComputeBox(FRGSolvingInfos_W& _infos,const Vec3f& _pos, const Quat& _rot, Vec3f& _topleft,Vec3f& _topright,Vec3f& _bottomleft,Vec3f& _bottomright)
{
	Float sizeX = _infos.m_vSize.x;
	Float sizeZ = _infos.m_vSize.z;

	if(m_bPerfBuild )
	{
		sizeX = 0.f;
		sizeZ = 0.f;
	}
	_topleft = _pos + _rot * Vec3f(-sizeX / 2.f, 0.f, -sizeZ / 2.f);
	_topright = _pos + _rot * Vec3f(-sizeX / 2.f, 0.f, sizeZ / 2.f);
	_bottomleft = _pos + _rot * Vec3f(sizeX / 2.f, 0.f, -sizeZ / 2.f);
	_bottomright = _pos + _rot * Vec3f(sizeX / 2.f, 0.f, sizeZ / 2.f);
}

Bool FRGSolver_W::ComputePos(FRGSolvingInfos_W& _infos)
{
	switch (_infos.m_position.m_type)
	{
	case FRG_POSITION_ON_FLOOR:
		return ComputePosOnFloor(_infos);
	case FRG_POSITION_ON_WALL:
		return ComputePosOnWall(_infos);
	case FRG_POSITION_ON_CEILING:
		return ComputePosOnCeiling(_infos);
	case FRG_POSITION_ON_SHAPE:
		return ComputePosOnShape(_infos);
	case FRG_POSITION_ON_EDGE:
		return ComputePosOnEdge(_infos);
	case FRG_POSITION_ON_FLOOR_AND_CEILING:
		return ComputePosOnFloorAndCeiling(_infos);
	case FRG_POSITION_RANDOM_IN_THE_AIR:
		return ComputePosRandomInTheAir(_infos);
	case FRG_POSITION_IN_THE_MID_AIR:
		return ComputePosInTheMidAir(_infos);
	case FRG_POSITION_UNDER_FURNITURE_EDGE:
		return ComputePosUnderFurnitureEdge(_infos);
	}

#ifdef _GAMEDEBUG
	CANCEL_EXCEPTIONC_Z(FALSE, "FRGSolver_W::ComputePos unkonkwn position type");
#endif

	return FALSE;
}

void FRGSolver_W::ComputeWallLateralBrowsing(FRGSolvingInfos_W& _infos, const TopologyAnalyzer_W::Wall& _wall, Float& _outFirstLateralOffset, Float& _outLastLateralOffset, Float& _outLateralIncrement)
{
	Float fColumnSize;
	if (_infos.m_position.m_bHaveTiledPos)
	{
		if (_wall.IsXWall())
			fColumnSize = _infos.m_position.m_vTiledPos.x;
		else
			fColumnSize = _infos.m_position.m_vTiledPos.z;
	}
	else
		fColumnSize = _wall.m_fWidth / _wall.m_daColumns.GetSize();

	Float fLeftMargin = _infos.m_vSize.x * 0.5f + _infos.m_position.m_paramsOnWall.m_fLeftMargin;
	Float fRightMargin = _infos.m_vSize.x * 0.5f + _infos.m_position.m_paramsOnWall.m_fRightMargin;

	S32 iFirstColumn = CEILFF(Max(0.f, fLeftMargin - fColumnSize * 0.5f) / fColumnSize);
	S32 iLastColumn = FLOORF((_wall.m_fWidth - Max(0.f, fRightMargin - fColumnSize * 0.5f)) / fColumnSize) - 1;

	_outFirstLateralOffset = iFirstColumn * fColumnSize + fColumnSize * 0.5f;
	_outLastLateralOffset = iLastColumn * fColumnSize + fColumnSize * 0.5f;
	_outLateralIncrement = fColumnSize;
}

Bool FRGSolver_W::ComputeWallYBrowsing(FRGSolvingInfos_W& _infos, const TopologyAnalyzer_W::Wall& _wall, Float _fLateralOffset, Float& _outFirstY, Float& _outLastY, Float& _outYIncrement)
{
	_outYIncrement = m_fSizeVoxel;

	if (_infos.m_position.m_paramsOnWall.m_positionOnWall == FRG_ON_WALL_NEAR_FLOOR)
	{
		CANCEL_EXCEPTIONC_Z(!_infos.m_position.m_bHaveTiledPos || _infos.m_position.m_vTiledPos.y < Float_Eps, "Solving OnWallNearFloor does not support Y tile");
		
		if (Abs(_wall.m_vBaseStart.y - m_fYGround) <= m_fSizeVoxel)
			_outFirstY = _wall.m_vBaseStart.y; // Try align with the wall
		else
			_outFirstY = m_fYGround;
		_outFirstY += (_infos.m_vSize / _infos.m_fFactor).y * 0.5f;
		
		_outLastY = _outFirstY;
		_outLastY += 0.1f; // ground tolerance
		
		return TRUE;
	}
	else if (_infos.m_position.m_paramsOnWall.m_positionOnWall == FRG_ON_WALL_NEAR_CEILING)
	{
		CANCEL_EXCEPTIONC_Z(!_infos.m_position.m_bHaveTiledPos || _infos.m_position.m_vTiledPos.y < Float_Eps, "Solving OnWallNearCeiling does not support Y tile");
	
		_outFirstY = m_fYCeiling - _infos.m_vSize.y * 0.5f;
		_outLastY = _outFirstY;
		_outFirstY -= 0.1f; // ceilling tolerance
		return TRUE;
	}
	else if (_infos.m_position.m_paramsOnWall.m_positionOnWall == FRG_ON_WALL_NEAR_SURFACE)
	{
		CANCEL_EXCEPTIONC_Z(!_infos.m_position.m_bHaveTiledPos || _infos.m_position.m_vTiledPos.y < Float_Eps, "Solving OnWallNearSurface does not support Y tile");
	
		Float fColumnSize = _wall.m_fWidth / _wall.m_daColumns.GetSize();
		S32 iWallColumnIdx = _fLateralOffset / fColumnSize;
		EXCEPTION_Z(iWallColumnIdx >= 0 && iWallColumnIdx < _wall.m_daColumns.GetSize());

		if( !_wall.m_daColumns[iWallColumnIdx].m_bIsValid )
			return FALSE;

		_outFirstY = _wall.m_daColumns[iWallColumnIdx].m_fMin + (_infos.m_vSize / _infos.m_fFactor).y * 0.5f;
		_outLastY = _outFirstY;

		if (_outFirstY < m_fYGround + _infos.m_position.m_paramsOnWall.m_fHeightMin || _outFirstY > m_fYGround + _infos.m_position.m_paramsOnWall.m_fHeightMax)
			_outFirstY += 1.f; // _outFirstY will be bigger than _outLastY so we are sure we doesn't use this position

		return TRUE;
	}

	_outFirstY = Max(m_fYGround + _infos.m_position.m_paramsOnWall.m_fHeightMin, _wall.m_vBaseStart.y + _infos.m_vSize.y * 0.5f);
	_outLastY = Min(m_fYGround + _infos.m_position.m_paramsOnWall.m_fHeightMax, _wall.m_vBaseStart.y + _wall.m_fHeight - _infos.m_vSize.y * 0.5f);

	if (_infos.m_position.m_bHaveTiledPos && _infos.m_position.m_vTiledPos.y >= Float_Eps)
	{
		Float fNewFirstY = m_pTopologyAnalyzer->m_YGround + FLOORF((_outFirstY - m_pTopologyAnalyzer->m_YGround) / _infos.m_position.m_vTiledPos.y) * _infos.m_position.m_vTiledPos.y;
		if (fNewFirstY < _outFirstY - Float_Eps)
			_outFirstY = fNewFirstY + _infos.m_position.m_vTiledPos.y;
		else
			_outFirstY = fNewFirstY;

		_outYIncrement = _infos.m_position.m_vTiledPos.y;
	}

	return TRUE;
}

Bool FRGSolver_W::ComputePosOnFloor(FRGSolvingInfos_W& _infos)
{
#ifdef _GAMEDEBUG
    EXCEPTIONC_Z(m_pTopologyAnalyzer != NULL, "FRGSolver_W::ComputePosOnFloor no TopologyAnalyzer defined");
#endif

	DynArray_Z<const TopologyAnalyzer_W::Surface*> daSurfaces;
	for(S32 i = 0; i < m_pTopologyAnalyzer->m_daSurfaces.GetSize(); ++i)
	{
		if(m_pTopologyAnalyzer->m_daSurfaces[i].m_bIsGround)
			daSurfaces.Add(&m_pTopologyAnalyzer->m_daSurfaces[i]);
	}

	return ComputePosOnSurfaces(_infos, daSurfaces, _infos.m_vSize.y * 0.5f);
}

//#define DEBUG_NAME Name_Z("LibraryObject")
#ifdef DEBUG_NAME
#define DEBUG_WALL(_pos_,_color_) DRAW_DEBUG_SPHERE3D_IF(_infos.GetObjectName() == DEBUG_NAME && _infos.m_iChosenPlacement == 0, _pos_,_color_,0.04f,.displayDuration(1e7f))
#else
#define DEBUG_WALL(_pos_,_color_)
#endif

Bool FRGSolver_W::ComputePosOnWall(FRGSolvingInfos_W& _infos)
{
	if (m_pTopologyAnalyzer == NULL)
	{
		return false;
	}

	Float fClearanceSizeZ = _infos.m_vClearanceSize.z;
	Float fClearanceCenterZ = _infos.m_vClearanceCenter.z;

    Vec3fDA bests;
    QuatDA bestsRot;
	Vec3fDA	bestsClearanceSize;
	Vec3fDA	bestsClearanceCenter;
    Float bestScore = 0;
    for (S32 w = 0; w < m_pTopologyAnalyzer->m_daWalls.GetSize(); w++)
	{
		const TopologyAnalyzer_W::Wall& wall = m_pTopologyAnalyzer->m_daWalls[w];

		if (!CheckWallType(_infos, wall))
		{
			DEBUG_WALL(wall.m_vCentroid,COLOR_YELLOW);
			continue;
		}

		Bool isFullWall = m_pTopologyAnalyzer->IsFullWall(wall);
		if( isFullWall )
		{
			if(_infos.m_position.m_paramsOnWall.m_bNotOnFullWall)
			{
				DEBUG_WALL(wall.m_vCentroid,COLOR_ORANGE);
				continue;
			}
		}
		else
		{
			if (_infos.m_position.m_paramsOnWall.m_bOnlyFullWall)
			{
				DEBUG_WALL(wall.m_vCentroid,COLOR_ORANGE);
				continue;
			}
		}
		

		Float fFirstLateralOffset, fLastLateralOffset, fLateralIncrement;
		ComputeWallLateralBrowsing(_infos, wall, fFirstLateralOffset, fLastLateralOffset, fLateralIncrement);

		for (Float fLateralOffset = fFirstLateralOffset; fLateralOffset <= fLastLateralOffset + Float_Eps; fLateralOffset += fLateralIncrement)
		{
			Float fFirstY, fLastY, fYIncrement;
			if( !ComputeWallYBrowsing(_infos, wall, fLateralOffset, fFirstY, fLastY, fYIncrement) )
				continue;

			for (Float y = fFirstY; y <= fLastY + Float_Eps; y += fYIncrement)
			{
				Vec3f pos = wall.m_vBaseStart + wall.m_vTangent * fLateralOffset;
				pos += wall.m_vUp * (y - wall.m_vBaseStart.y);

				if (!m_pTopologyAnalyzer->SpaceOnWallIsFreeFloat(fLateralOffset - _infos.m_vSize.x * 0.5f, fLateralOffset + _infos.m_vSize.x * 0.5f, pos.y - _infos.m_vSize.y / 2.f, pos.y + _infos.m_vSize.y / 2.f, FLOORINT(_infos.m_vSize.z / m_fSizeVoxel)+1, _infos.m_bAllowPartiallyInWall, wall))
				{
					DEBUG_WALL(pos,COLOR_BLUE);
					continue;
				}

				// near ceilling & floor case
				if( _infos.m_position.m_paramsOnWall.m_positionOnWall == FRG_ON_WALL_NEAR_FLOOR 
					|| _infos.m_position.m_paramsOnWall.m_positionOnWall == FRG_ON_WALL_NEAR_SURFACE
					|| _infos.m_position.m_paramsOnWall.m_positionOnWall == FRG_ON_WALL_NEAR_CEILING)
				{
					if( _infos.m_position.m_paramsOnWall.m_positionOnWall == FRG_ON_WALL_NEAR_CEILING )
					{
						pos.y = wall.m_vBaseStart.y + wall.m_fHeight - (_infos.m_vSize.y * .5f);
					}
					else
					{
						Vec3f vUnscaledSize = _infos.m_vSize / _infos.m_fFactor;

						Vec3f boxPos = pos - VEC3F_UP * 0.16f + wall.m_vNormal * vUnscaledSize.z * .5f;
						Vec3f boxSize = _infos.m_vSize;
						boxSize.y =  vUnscaledSize.y;
						boxSize.y += 0.32f;
						const TopologyAnalyzer_W::Surface* pSurface = m_pTopologyAnalyzer->GetHigherSurfaceInBox(boxPos,boxSize);
						if( !pSurface )
						{
							DEBUG_WALL(pos,COLOR_PURPLE);
							continue;
						}

						pos.y = pSurface->m_fWorldHeight + (vUnscaledSize.y * .5f);

						Vec3f centerPos = pos + wall.m_vNormal * vUnscaledSize.z;
						Vec3f topLeft = centerPos + wall.m_vNormal * _infos.m_vSize.z * .5f + wall.m_vTangent * _infos.m_vSize.x * .5f;
						Vec3f topRight = topLeft - wall.m_vTangent * _infos.m_vSize.x;
						Vec3f bottomLeft = centerPos - wall.m_vNormal * _infos.m_vSize.z * .5f + wall.m_vTangent * _infos.m_vSize.x * .5f;
						Vec3f bottomRight = bottomLeft - wall.m_vTangent * _infos.m_vSize.x;

						if (!m_pTopologyAnalyzer->RectangleIsOk(topLeft, topRight, bottomLeft, bottomRight, *pSurface, m_fSizeVoxel, TOPOLOGY_REQUEST_VALID))
						{
							DEBUG_WALL(pos, COLOR_DARKRED);
							DEBUG_WALL(topLeft+VEC3F_UP*.02f, COLOR_DARKRED);
							DEBUG_WALL(topRight + VEC3F_UP*.02f, COLOR_DARKRED);
							DEBUG_WALL(bottomLeft + VEC3F_UP*.02f, COLOR_DARKRED);
							DEBUG_WALL(bottomRight + VEC3F_UP*.02f, COLOR_DARKRED);/*
							if(_infos.GetObjectName() == DEBUG_NAME && _infos.m_iChosenPlacement == 0)
								DRAW_DEBUG_BOX3D(Quat(wall.m_vNormal, VEC3F_FRONT), boxPos, _infos.m_vSize, 0.001f, COLOR_PURPLE, .displayDuration(Float_Max));*/
							continue;
						}
					}

					static S32 order[3] = {-3,2,1};
					Vec3f vHNormal = wall.m_vNormal;
					vHNormal.CHNormalize();

					_infos.m_vPos = pos + vHNormal * _infos.m_vSize.z / 2;
					_infos.m_qRot = Util_L::GetOrientationQuat(_infos.m_vPos,_infos.m_vPos+vHNormal,VEC3F_UP,order,NULL);
				}
				else
				{
					_infos.m_vPos = pos + wall.m_vNormal * _infos.m_vSize.z / 2;

					static S32 order[3] = {-3,2,1};
					Quat orient = Util_L::GetOrientationQuat(_infos.m_vPos,_infos.m_vPos+wall.m_vNormal,wall.m_vUp,order,NULL);
					_infos.m_qRot = orient;
				}
				
				//For the drone pipes
				if(_infos.m_bCheckOppositePos)
				{
					if(!CheckClearPathToOppositeWall(_infos))
						continue;
					else
					{
						//Update box size
						Vec3f vBeginToEnd = _infos.m_vOppositePos - _infos.m_vPos;

						Float fOffset = vBeginToEnd.GetNorm();
						_infos.m_vClearanceSize.z = fOffset + fClearanceSizeZ;
						_infos.m_vClearanceCenter.z += fOffset*.5f - fClearanceSizeZ * .5f; 
					}
				}

				if (CollidWithOtherObjects(_infos))
				{
					DEBUG_WALL(pos,COLOR_RED);
					_infos.m_vClearanceSize.z = fClearanceSizeZ;
					_infos.m_vClearanceCenter.z = fClearanceCenterZ;
					continue;
				}

				if (!CheckRules(_infos))
				{
					DEBUG_WALL(pos,COLOR_LIGHTRED);
					_infos.m_vClearanceSize.z = fClearanceSizeZ;
					_infos.m_vClearanceCenter.z = fClearanceCenterZ;
					continue;
				}

				// Test de visibilité à 1m80 du sol. Permet d'éviter d'avoir des trucs derriere des murs depuis le centre du playspace.
				//   Et s'il y a un mur dans le centre du playspace on fait quoi? => Pour le moment ça ne teste pas à moins d'1m du centre
				if (!m_pTopologyAnalyzer->CellIsVisibleFromPlayspaceCenter(m_pPlaySpaceInfos,_infos.m_vPos+wall.m_vNormal*0.2f))
				{
					DEBUG_WALL(pos,COLOR_PINK);
					_infos.m_vClearanceSize.z = fClearanceSizeZ;
					_infos.m_vClearanceCenter.z = fClearanceCenterZ;
					continue;
				}

				Float score = ComputeScoreWithConstraints(_infos);
				if (_infos.m_bOnlyComputePossiblePos)
                {
					DEBUG_WALL(pos,COLOR_CYAN);
                    _infos.m_avlPossiblePos.Add(score, Position3D_L(_infos.m_vPos, _infos.m_qRot));
					_infos.m_vClearanceSize.z = fClearanceSizeZ;
					_infos.m_vClearanceCenter.z = fClearanceCenterZ;
					continue;
                }

				DEBUG_WALL(pos,COLOR_GREEN);

                if (score > bestScore)
                {
                    bests.Flush();
                    bestsRot.Flush();
					bestsClearanceSize.Flush();
					bestsClearanceCenter.Flush();
                }
                if (bests.GetSize() == 0 || Abs(score - bestScore) < Float_Eps)
		        {
			        bests.Add(_infos.m_vPos);
                    bestsRot.Add(_infos.m_qRot);
					bestsClearanceSize.Add(_infos.m_vClearanceSize);
					bestsClearanceCenter.Add(_infos.m_vClearanceCenter);
			        bestScore = score;
		        }

				_infos.m_vClearanceSize.z = fClearanceSizeZ;
				_infos.m_vClearanceCenter.z = fClearanceCenterZ;
			}
		}
	}

	if (_infos.m_bOnlyComputePossiblePos)
        return (_infos.m_avlPossiblePos.GetNbElems() != 0);

    if (bests.GetSize() == 0)
		return FALSE;

    S32 finalBest = m_randomGenerator.Alea(0, bests.GetSize() - 1);

	_infos.m_vPos = bests[finalBest];
    _infos.m_qRot = bestsRot[finalBest];
	if(_infos.m_bCheckOppositePos)
	{
		_infos.m_vClearanceSize = bestsClearanceSize[finalBest];
		_infos.m_vClearanceCenter = bestsClearanceCenter[finalBest];
	}

	return TRUE;
}

Bool FRGSolver_W::ComputePosOnCeiling(FRGSolvingInfos_W& _infos)
{
#ifdef _GAMEDEBUG
    EXCEPTIONC_Z(m_pTopologyAnalyzer != NULL, "FRGSolver_W::ComputePosOnCeiling no TopologyAnalyzer defined");
#endif

	DynArray_Z<const TopologyAnalyzer_W::Surface*> daSurfaces;
	for(S32 i = 0; i < m_pTopologyAnalyzer->m_daSurfaces.GetSize(); ++i)
	{
		if(m_pTopologyAnalyzer->m_daSurfaces[i].m_bIsCeiling)
			daSurfaces.Add(&m_pTopologyAnalyzer->m_daSurfaces[i]);
	}

	return ComputePosOnSurfaces(_infos, daSurfaces, _infos.m_vSize.y * -0.5f);
}

Bool FRGSolver_W::ComputePosOnShape(FRGSolvingInfos_W& _infos)
{
#ifdef _GAMEDEBUG
	EXCEPTIONC_Z(m_pShapeAnalyzer != NULL, "FRGSolver_W::ComputePosOnShape no ShapeAnalyzer defined");
#endif

	DynArray_Z<const TopologyAnalyzer_W::Surface*> daSurfaces;
	m_pShapeAnalyzer->GetAllSurfacesOnShape(_infos.m_position.m_paramsOnShape.m_nShapeName, _infos.m_position.m_paramsOnShape.m_nSlotName, daSurfaces);

	return ComputePosOnSurfaces(_infos, daSurfaces, _infos.m_vSize.y * 0.5f);
}

Bool FRGSolver_W::ComputePosOnEdge(FRGSolvingInfos_W& _infos)
{
#ifdef _GAMEDEBUG
    EXCEPTIONC_Z(m_pTopologyAnalyzer != NULL, "FRGSolver_W::ComputePosOnEdge no TopologyAnalyzer defined");
	EXCEPTIONC_Z(!_infos.m_bOnlyComputePossiblePos, "FRGSolver_W::ComputePosOnEdge doesn't support only compute possible pos");
	// TMP
	EXCEPTIONC_Z(m_pShapeAnalyzer != NULL, "FRGSolver_W::ComputePosOnEdge no ShapeAnalyzer defined");
    // END TMP
#endif

	// Potential bottom object
	FRGSolvingInfos_W bottomInfos = _infos;
	bottomInfos.m_vSize = _infos.m_position.m_paramsOnEdge.m_vSizeBottomObject;

    U32 dist = FLOORINT(_infos.m_vSize.x / 2.f / m_fSizeVoxel) + 1;

    Vec3fDA normals;
    normals.Add(Vec3f(-1, 0, 0));
    normals.Add(Vec3f(0, 0, -1));
    normals.Add(Vec3f(0, 0, 1));
    normals.Add(Vec3f(1, 0, 0));

    // TMP
    DynArray_Z<const TopologyAnalyzer_W::Surface*> badSurfaces;
	m_pShapeAnalyzer->GetAllSurfacesOnShape("couch", "back", badSurfaces);
    // END TMP

    Vec3fDA bests;
    Vec3fDA bests2;
    QuatDA bestsRot;
    Float bestScore = 0;
	for (S32 s = 0; s < m_pTopologyAnalyzer->m_daSurfaces.GetSize(); ++s)
	{
		const TopologyAnalyzer_W::Surface& surface = m_pTopologyAnalyzer->m_daSurfaces[s];
        if (surface.m_bIsGround)
            continue;

        // TMP
        if (badSurfaces.Contains(&surface) >= 0)
            continue;
        // END TMP

        for (S32 c = 0; c < surface.m_daCells.GetSize(); ++c)
		{
            if (surface.m_daCells[c].m_iDistFromFloor != dist)
                continue;

            if (surface.m_daCells[c].m_iDistFromBorder != dist)
                continue;

            Vec3f onSurfacePos = m_pTopologyAnalyzer->GetCellPosition(surface, c);

            for (S32 n = 0; n < normals.GetSize(); ++n)
            {
                const Vec3f& normal = normals[n];
                Vec3f tangent = normal ^ VEC3F_DOWN;
                tangent.Normalize(); // We need the tangent normalized

                // Check normal
				const TopologyAnalyzer_W::CellInfo* pCell = m_pTopologyAnalyzer->GetCellInSurface(onSurfacePos + normal * m_fSizeVoxel * (dist - 1), surface, m_fSizeVoxel);
				if (!pCell || !pCell->IsValid() || pCell->m_iDistFromFloor != 1)
                    continue;

                // Check width
                Bool withOk = TRUE;
                for (S32 i = 0; i < (_infos.m_vSize.z / 2.f) / m_fSizeVoxel; ++i)
                {
					const TopologyAnalyzer_W::CellInfo* pOtherCellPos = m_pTopologyAnalyzer->GetCellInSurface(onSurfacePos + tangent * m_fSizeVoxel * i, surface, m_fSizeVoxel);
					const TopologyAnalyzer_W::CellInfo* pOtherCellNeg = m_pTopologyAnalyzer->GetCellInSurface(onSurfacePos - tangent * m_fSizeVoxel * i, surface, m_fSizeVoxel);
					if (!pOtherCellPos || !pOtherCellNeg || !pOtherCellPos->IsValid() || !pOtherCellNeg->IsValid() )
                    {
                        withOk = FALSE;
                        break;
                    }
                }

                if (!withOk)
                    continue;

                // Now we need normal normalized
                Vec3f normalizedNormal = normal;
                normalizedNormal.Normalize();

                // Maybe we have found our couple of pos
				_infos.m_vPos = onSurfacePos + VEC3F_UP * _infos.m_vSize.y / 2.f + normalizedNormal * _infos.m_vSize.x * 0.33f;
				_infos.m_qRot = Quat(VEC3F_FRONT, tangent);

				Vec3f onFloorBottomPos = onSurfacePos - VEC3F_UP * surface.m_fHeightFromGround + normalizedNormal * CEILFF((_infos.m_vSize.x / 2.f + bottomInfos.m_vSize.x / 2.f) / m_fSizeVoxel) * m_fSizeVoxel;
				bottomInfos.m_vPos = onFloorBottomPos + VEC3F_UP * bottomInfos.m_vSize.y / 2.f;
				bottomInfos.m_qRot = Quat(VEC3F_FRONT, tangent);

                Vec3f topLeft = onFloorBottomPos + bottomInfos.m_qRot * Vec3f(-bottomInfos.m_vSize.x / 2.f, 0.f, -bottomInfos.m_vSize.z / 2.f);
                Vec3f topRight = onFloorBottomPos + bottomInfos.m_qRot * Vec3f(-bottomInfos.m_vSize.x / 2.f, 0.f, bottomInfos.m_vSize.z / 2.f);
                Vec3f bottomLeft = onFloorBottomPos + bottomInfos.m_qRot * Vec3f(bottomInfos.m_vSize.x / 2.f, 0.f, -bottomInfos.m_vSize.z / 2.f);
                Vec3f bottomRight = onFloorBottomPos + bottomInfos.m_qRot * Vec3f(bottomInfos.m_vSize.x / 2.f, 0.f, bottomInfos.m_vSize.z / 2.f);

				Bool isOnGround = FALSE;
				for (S32 i = 0; i < m_pTopologyAnalyzer->m_daSurfaces.GetSize(); ++i)
				{
					if (m_pTopologyAnalyzer->m_daSurfaces[i].m_bIsGround &&
						m_pTopologyAnalyzer->RectangleIsOk(topLeft, topRight, bottomLeft, bottomRight, m_pTopologyAnalyzer->m_daSurfaces[i], m_fSizeVoxel,TOPOLOGY_REQUEST_VALID))
					{
						isOnGround = TRUE;
						break;
					}
				}

				if (!isOnGround)
					continue;

				if (CollidWithOtherObjects(_infos))
                    continue;
                if (CollidWithOtherObjects(bottomInfos))
                    continue;

				if (!CheckRules(_infos))
					continue;

				// Test de visibilité à 1m80 du sol. Permet d'éviter d'avoir des trucs derriere des murs depuis le centre du playspace.
				//   Et s'il y a un mur dans le centre du playspace on fait quoi? => Pour le moment ça ne teste pas à moins d'1m du centre
				if (!m_pTopologyAnalyzer->CellIsVisibleFromPlayspaceCenter(m_pPlaySpaceInfos,_infos.m_vPos))
					continue;

				Float score = ComputeScoreWithConstraints(_infos);
                if (score > bestScore)
                {
                    bests.Flush();
                    bests2.Flush();
                    bestsRot.Flush();
                }
                if (bests.GetSize() == 0 || Abs(score - bestScore) < Float_Eps)
		        {
		            bests.Add(_infos.m_vPos);
                    bests2.Add(bottomInfos.m_vPos);
                    bestsRot.Add(_infos.m_qRot);
		            bestScore = score;
		        }
            }
		}
	}

    if (bests.GetSize() == 0)
		return FALSE;

    S32 finalBest = m_randomGenerator.Alea(0, bests.GetSize() - 1);

	_infos.m_vPos = bests[finalBest];
	_infos.m_qRot = bestsRot[finalBest];
    bottomInfos.m_vPos = bests2[finalBest];
	bottomInfos.m_qRot = bestsRot[finalBest];

	S32 topIdx = m_daSolvedObjects.Add(_infos);
	S32 bottomIdx = m_daSolvedObjects.Add(bottomInfos);

	m_daSolvedObjects[topIdx].m_idx = topIdx;
	m_daSolvedObjects[topIdx].m_position.m_paramsOnEdge.m_idxTopObject = topIdx;
	m_daSolvedObjects[topIdx].m_position.m_paramsOnEdge.m_idxBottomObject = bottomIdx;
	m_daSolvedObjects[topIdx].ComputeWorldPos();

	m_daSolvedObjects[bottomIdx].m_idx = bottomIdx;
	m_daSolvedObjects[bottomIdx].m_position.m_paramsOnEdge.m_idxTopObject = topIdx;
	m_daSolvedObjects[bottomIdx].m_position.m_paramsOnEdge.m_idxBottomObject = bottomIdx;
	m_daSolvedObjects[bottomIdx].ComputeWorldPos();

	// The _infos give by the user is updated as the top
	_infos.m_idx = topIdx;
	_infos.m_position.m_paramsOnEdge.m_idxTopObject = topIdx;
	_infos.m_position.m_paramsOnEdge.m_idxBottomObject = bottomIdx;

	return TRUE;
}

//#define DEBUG_FLOORCEILING_NAME Name_Z("Ceiling_Hatch")
#ifdef DEBUG_FLOORCEILING_NAME
#define DEBUG_FLOORCEILING(_pos_,_color_) DRAW_DEBUG_SPHERE3D_IF(_infos.GetObjectName() == DEBUG_FLOORCEILING_NAME, _pos_,_color_,0.04f,.displayDuration(1e7f))
#else
#define DEBUG_FLOORCEILING(_pos_,_color_)
#endif

Bool FRGSolver_W::ComputePosOnFloorAndCeiling(FRGSolvingInfos_W& _infos)
{
#ifdef _GAMEDEBUG
    EXCEPTIONC_Z(m_pTopologyAnalyzer != NULL, "FRGSolver_W::ComputePosOnFloorAndCeiling no TopologyAnalyzer defined");
	EXCEPTIONC_Z(!_infos.m_bOnlyComputePossiblePos, "FRGSolver_W::ComputePosOnFloorAndCeiling doesn't support only compute possible pos");
#endif

	// Potential bottom object
	FRGSolvingInfos_W bottomInfos = _infos;
	bottomInfos.m_vSize = _infos.m_position.m_paramsOnEdge.m_vSizeBottomObject;

	Float botomMinWidth = Min(bottomInfos.m_vSize.x, bottomInfos.m_vSize.z);
    U32 bottomWidthMinSize = FLOOR(botomMinWidth * 0.5f / m_fSizeVoxel) + 1;

    Vec3fDA bests;
    Vec3fDA bests2;
    QuatDA bestsRot;
	Vec3fDA	bestsClearanceSize;
	Vec3fDA	bestsClearanceCenter;
    Float bestScore = 0;
	for (S32 s = 0; s < m_pTopologyAnalyzer->m_daSurfaces.GetSize(); ++s)
	{
		const TopologyAnalyzer_W::Surface& surface = m_pTopologyAnalyzer->m_daSurfaces[s];
        if (!surface.m_bIsGround)
            continue;

		Float fClearanceSizeY = bottomInfos.m_vClearanceSize.y;
		Float fClearanceCenterY = bottomInfos.m_vClearanceCenter.y;
        for (S32 c = 0; c < surface.m_daCells.GetSize(); ++c)
		{
			if (surface.m_daCells[c].m_iDistFromBorder < bottomWidthMinSize)
			{
				DEBUG_FLOORCEILING(m_pTopologyAnalyzer->GetCellPosition(surface, c), COLOR_BLUE);
				continue;
			}

			Vec3f vOnFloorPos = m_pTopologyAnalyzer->GetCellPosition(surface, c);
            Quat rot(VEC3F_FRONT, VEC3F_FRONT); // Only one rot for now
		
			// Compute bottom box
			Vec3f topLeft, topRight, bottomLeft, bottomRight;
			ComputeBox(bottomInfos, vOnFloorPos, rot, topLeft, topRight, bottomLeft, bottomRight);

			if (!m_pTopologyAnalyzer->RectangleIsOk(topLeft, topRight, bottomLeft, bottomRight, surface, m_fSizeVoxel, TOPOLOGY_REQUEST_VALID))
			{
				DEBUG_FLOORCEILING(vOnFloorPos, COLOR_YELLOW);
				continue;
			}

			// Find good ceiling
			const TopologyAnalyzer_W::Surface* ceiling = NULL;
			for (S32 s2 = 0; s2 < m_pTopologyAnalyzer->m_daSurfaces.GetSize(); ++s2)
			{
				const TopologyAnalyzer_W::Surface& surface2 = m_pTopologyAnalyzer->m_daSurfaces[s2];
				if (!surface2.m_bIsCeiling)
					continue;

				if (surface2.m_vMinPos.x > vOnFloorPos.x || surface2.m_vMinPos.z > vOnFloorPos.z ||
					surface2.m_vMaxPos.x < vOnFloorPos.x || surface2.m_vMaxPos.z < vOnFloorPos.z)
					continue;

				Vec3f centerBox = vOnFloorPos;
				centerBox.y = surface2.m_vMinPos.y;

				// Compute up BOX
				Vec3f topLeft2, topRight2, bottomLeft2, bottomRight2;
				ComputeBox(_infos, centerBox, rot, topLeft2, topRight2, bottomLeft2, bottomRight2);

				// Check as partially !!

				// CHECK ALL INSIDE (NOT ONE OUTSIDE)
				if (m_pTopologyAnalyzer->RectangleIsOk(topLeft2, topRight2, bottomLeft2, bottomRight2, surface2, m_fSizeVoxel, TOPOLOGY_REQUEST_ONLYONE | TOPOLOGY_REQUEST_OUTSIDE))
					continue;

				// CHECK ONE VALID
				if (!m_pTopologyAnalyzer->RectangleIsOk(topLeft2, topRight2, bottomLeft2, bottomRight2, surface2, m_fSizeVoxel, TOPOLOGY_REQUEST_ONLYONE | TOPOLOGY_REQUEST_VALID))
					continue;

				ceiling = &surface2;
				break;
			}
			if (ceiling == NULL)
			{
				DEBUG_FLOORCEILING(vOnFloorPos, COLOR_ORANGE);
				continue;
			}

			Vec3f vOnCeilingPos = vOnFloorPos;
			vOnCeilingPos.y = ceiling->m_vMinPos.y;

			_infos.m_vPos = vOnCeilingPos - VEC3F_UP * _infos.m_vSize.y / 2;
			_infos.m_qRot = rot;

			//For the drone pipes
			if(_infos.m_bCheckOppositePos)
			{
				if (!CheckClearPathToCeiling(_infos, vOnFloorPos, vOnCeilingPos))
				{
					DEBUG_FLOORCEILING(vOnFloorPos, COLOR_PURPLE);
					continue;
				}
				else
				{
					//Update box size
					Vec3f vBeginToEnd = vOnFloorPos - vOnCeilingPos;

					Float fOffset = vBeginToEnd.GetNorm();
					bottomInfos.m_vClearanceSize.y = fOffset + fClearanceSizeY;
					bottomInfos.m_vClearanceCenter.y += fOffset*.5f - fClearanceSizeY * .5f; 
				}
			}

			if (CollidWithOtherObjects(_infos))
			{
				DEBUG_FLOORCEILING(vOnFloorPos, COLOR_LIGHTRED);
				bottomInfos.m_vClearanceSize.y = fClearanceSizeY;
				bottomInfos.m_vClearanceCenter.y = fClearanceCenterY;
				continue;
			}

			bottomInfos.m_vPos = vOnFloorPos + VEC3F_UP * bottomInfos.m_vSize.y / 2;
			bottomInfos.m_qRot = rot;

			if (CollidWithOtherObjects(bottomInfos))
			{
				DEBUG_FLOORCEILING(vOnFloorPos, COLOR_RED);
				bottomInfos.m_vClearanceSize.y = fClearanceSizeY;
				bottomInfos.m_vClearanceCenter.y = fClearanceCenterY;
				continue;
			}

			if (!CheckRules(_infos))
			{
				DEBUG_FLOORCEILING(vOnFloorPos, COLOR_BLUE);
				bottomInfos.m_vClearanceSize.y = fClearanceSizeY;
				bottomInfos.m_vClearanceCenter.y = fClearanceCenterY;
				continue;
			}

			// Test de visibilité à 1m80 du sol. Permet d'éviter d'avoir des trucs derriere des murs depuis le centre du playspace.
			//   Et s'il y a un mur dans le centre du playspace on fait quoi? => Pour le moment ça ne teste pas à moins d'1m du centre
			if (!m_pTopologyAnalyzer->CellIsVisibleFromPlayspaceCenter(m_pPlaySpaceInfos,_infos.m_vPos))
			{
				DEBUG_FLOORCEILING(vOnFloorPos, COLOR_CYAN);
				bottomInfos.m_vClearanceSize.y = fClearanceSizeY;
				bottomInfos.m_vClearanceCenter.y = fClearanceCenterY;
				continue;
			}

			DEBUG_FLOORCEILING(vOnFloorPos, COLOR_GREEN);
			
			Float score = ComputeScoreWithConstraints(_infos);
			//DRAW_DEBUG_STRING3D_DURATION_IF(_infos.GetObjectName() == DEBUG_FLOORCEILING_NAME, vOnFloorPos + Vec3f(0.f, 0.05f, 0.f), 1e7f, "%.3f", score);
			if (score > bestScore)
			{
				bests.Flush();
				bests2.Flush();
				bestsRot.Flush();
				bestsClearanceSize.Flush();
				bestsClearanceCenter.Flush();
			}
			if (bests.GetSize() == 0 || Abs(score - bestScore) < Float_Eps)
			{
				bests.Add(_infos.m_vPos);
				bests2.Add(bottomInfos.m_vPos);
				bestsRot.Add(_infos.m_qRot);
				bestsClearanceSize.Add(bottomInfos.m_vClearanceSize);
				bestsClearanceCenter.Add(bottomInfos.m_vClearanceCenter);
				bestScore = score;
			}

			bottomInfos.m_vClearanceSize.y = fClearanceSizeY;
			bottomInfos.m_vClearanceCenter.y = fClearanceCenterY;
		}
	}

    if (bests.GetSize() == 0)
		return FALSE;

    S32 finalBest = m_randomGenerator.Alea(0, bests.GetSize() - 1);

	_infos.m_vPos = bests[finalBest];
	_infos.m_qRot = bestsRot[finalBest];
    bottomInfos.m_vPos = bests2[finalBest];
	bottomInfos.m_qRot = bestsRot[finalBest];
	if(_infos.m_bCheckOppositePos)
	{
		bottomInfos.m_vClearanceSize = bestsClearanceSize[finalBest];
		bottomInfos.m_vClearanceCenter = bestsClearanceCenter[finalBest];
	}

	S32 topIdx = m_daSolvedObjects.Add(_infos);
	S32 bottomIdx = m_daSolvedObjects.Add(bottomInfos);

	m_daSolvedObjects[topIdx].m_idx = topIdx;
	m_daSolvedObjects[topIdx].m_position.m_paramsOnEdge.m_idxTopObject = topIdx;
	m_daSolvedObjects[topIdx].m_position.m_paramsOnEdge.m_idxBottomObject = bottomIdx;
	m_daSolvedObjects[topIdx].ComputeWorldPos();

	m_daSolvedObjects[bottomIdx].m_idx = bottomIdx;
	m_daSolvedObjects[bottomIdx].m_position.m_paramsOnEdge.m_idxTopObject = topIdx;
	m_daSolvedObjects[bottomIdx].m_position.m_paramsOnEdge.m_idxBottomObject = bottomIdx;
	m_daSolvedObjects[bottomIdx].ComputeWorldPos();

	// The _infos give by the user is updated as the top
	_infos.m_idx = topIdx;
	_infos.m_position.m_paramsOnEdge.m_idxTopObject = topIdx;
	_infos.m_position.m_paramsOnEdge.m_idxBottomObject = bottomIdx;

	return TRUE;
}

Bool FRGSolver_W::ComputePosRandomInTheAir(FRGSolvingInfos_W& _infos)
{
#ifdef _GAMEDEBUG
    EXCEPTIONC_Z(m_pTopologyAnalyzer != NULL, "FRGSolver_W::ComputePosRandomInTheAir no TopologyAnalyzer defined");
#endif

	// Very rough position finding of any position in the air
	
	SafeArray_Z<Quat,8> saRot;
	S32 r=0;
	for(S32 x=-1; x<=1; x+=2)
	{
		for(S32 y=-1; y<=1; y+=2)
		{
			for(S32 z=-1; z<=1; z+=2)
			{
				Vec3f vFront(x, y, z);
				vFront.CNormalize();
				Quat Rot(VEC3F_FRONT, vFront);
				Rot.Normalize();
				saRot[r++] = Rot;
			}
		}
	}
	
	S32 startSurface = m_randomGenerator.Alea(0, m_pTopologyAnalyzer->m_daSurfaces.GetSize() - 1);
    for(S32 s = 0; s < m_pTopologyAnalyzer->m_daSurfaces.GetSize(); ++s)
    {
		S32 SurfaceIndice = (startSurface + s) % m_pTopologyAnalyzer->m_daSurfaces.GetSize();
		const TopologyAnalyzer_W::Surface& surface = m_pTopologyAnalyzer->m_daSurfaces[SurfaceIndice];

		Float fMinSpace = surface.m_fMinSpace;
		SafeArray_Z<Float, 2> saHeight;
		for(S32 i=0; i<saHeight.GetSize(); i++)
		{
			saHeight[i] = fMinSpace*(Float)(i+1)/((Float)saHeight.GetSize()+1);
		}
		S32 startHeight = m_randomGenerator.Alea(0, saHeight.GetSize() - 1);
		for (S32 i = 0; i < saHeight.GetSize(); ++i)
		{
			S32 HeightIndice = (startHeight + i) % saHeight.GetSize();
			Float fHeight = saHeight[HeightIndice];

			Vec3f vPos = 0.5f*(surface.m_vMaxPos + surface.m_vMinPos);
			vPos.y += fHeight;
			vPos.y = Min(0.8f, vPos.y); // Blindage pour pas arriver au dessus du plafond! (check surface.m_fMinSpace computation)

			S32 startRot = m_randomGenerator.Alea(0, saRot.GetSize() - 1);
			for (S32 j = 0; j < saRot.GetSize(); ++j)
			{	
				S32 RotIndice = (startRot + j) % saRot.GetSize();
				Quat& qRot = saRot[RotIndice];

				// Compute BOX
				Vec3f topLeft,topRight,bottomLeft,bottomRight;
				ComputeBox(_infos,vPos,qRot,topLeft,topRight,bottomLeft,bottomRight);
				
				_infos.m_vPos = vPos + (qRot*VEC3F_UP) * _infos.m_vSize.y / 2;
				_infos.m_qRot = qRot;

				if (CollidWithOtherObjects(_infos))
					continue;

				if (!CheckRules(_infos))
					continue;

				// Test de visibilité à 1m80 du sol. Permet d'éviter d'avoir des trucs derriere des murs depuis le centre du playspace.
				//   Et s'il y a un mur dans le centre du playspace on fait quoi? => Pour le moment ça ne teste pas à moins d'1m du centre
				if (!m_pTopologyAnalyzer->CellIsVisibleFromPlayspaceCenter(m_pPlaySpaceInfos,_infos.m_vPos))
					continue;

				return TRUE;
			}
		}
	}
		
    return FALSE;
}


//#define DEBUG_MIDAIR_NAME Name_Z("WATER_Main_Dummy")
#ifdef DEBUG_MIDAIR_NAME
#define DEBUG_MIDAIR(_pos_,_color_) DRAW_DEBUG_SPHERE3D_IF(_infos.GetObjectName() == DEBUG_MIDAIR_NAME, _pos_,_color_,0.04f,.displayDuration(1e7f))
#else
#define DEBUG_MIDAIR(_pos_,_color_)
#endif

Bool FRGSolver_W::ComputePosInTheMidAir(FRGSolvingInfos_W& _infos)
{
#ifdef _GAMEDEBUG
    EXCEPTIONC_Z(m_pTopologyAnalyzer != NULL, "FRGSolver_W::ComputePosInTheMidAir no TopologyAnalyzer defined");
#endif

	Float minWidth = Max(_infos.m_vSize.x, _infos.m_vSize.z);
    U32 widthMinSize = FLOOR(minWidth * 0.5f / m_fSizeVoxel) + 1;

	//const Vec3f& vPlaySpaceCenter = m_pTopologyAnalyzer->m_vPlaySpaceCenter;

	static const Float AngleStep = DegToRad(30.f); /* Abitrary.... to have nice look... */
	QuatDA rots;
	for (S32 i = 0; i < DegToRad(360.f) / AngleStep; ++i)
		rots.Add(Quat(i * AngleStep, VEC3F_UP));
	
	Position3D_L bestPosition = POSITION3D_NULL;
	Bool bFound = FALSE;
	Float fBestScore = 0.f;
	for(S32 s = 0; s < m_pTopologyAnalyzer->m_daSurfaces.GetSize(); ++s)
	{
		const TopologyAnalyzer_W::Surface& surface = m_pTopologyAnalyzer->m_daSurfaces[s];
		if(!surface.m_bIsGround)
		{
			continue;
		}
		
		for(S32 c = 0; c < surface.m_daCells.GetSize(); ++c)
        {
			if (surface.m_daCells[c].m_iDistFromVoid <= widthMinSize)
				continue;

			Vec3f vOnFloorPos = m_pTopologyAnalyzer->GetCellPosition(surface, c);

			for (S32 r = 0; r < rots.GetSize(); r++)
			{
				Quat rot = rots[r];

				Float minY = surface.m_fWorldHeight - 0.1f;
				Float maxY = minY + _infos.m_vClearanceSize.y;
				Float y = m_pTopologyAnalyzer->GetMaxYInRectangle(m_pPlaySpaceInfos, Vec2f(vOnFloorPos.x, vOnFloorPos.z), Vec2f(_infos.m_vClearanceSize.x*.8f, _infos.m_vClearanceSize.z*.8f), rot, minY, maxY);
				if (y > surface.m_fWorldHeight + .5f || y < surface.m_fWorldHeight - Float_Eps)
				{
					DEBUG_MIDAIR(vOnFloorPos, COLOR_BLUE);
					continue;
				}

				_infos.m_vPos = vOnFloorPos;
				_infos.m_vPos.y += 1.2f;
				_infos.m_qRot = rot;

				if (CollidWithOtherObjects(_infos))
				{
					DEBUG_MIDAIR(vOnFloorPos, COLOR_RED);
					continue;
				}
				if (!CheckRules(_infos))
				{
					DEBUG_MIDAIR(vOnFloorPos, COLOR_YELLOW);
					continue;
				}

				Float score = ComputeScoreWithConstraints(_infos);
				DEBUG_MIDAIR(vOnFloorPos, COLOR_GREEN);

				if (score > fBestScore || !bFound)
				{
					fBestScore = score;
					bestPosition.m_vPos = _infos.m_vPos;
					bestPosition.m_qQuat = _infos.m_qRot;
					bFound = TRUE;
				}
			}
		}
	}

	if( !bFound )
		return FALSE;
	
	_infos.m_vPos = bestPosition.GetPos();
	_infos.m_qRot = bestPosition.GetRot();

	return TRUE;
}

Bool FRGSolver_W::ComputePosUnderFurnitureEdge(FRGSolvingInfos_W& _infos)
{
#ifdef _GAMEDEBUG
	EXCEPTIONC_Z(m_pShapeAnalyzer != NULL, "FRGSolver_W::ComputePosUnderFurnitureEdge no m_pShapeAnalyzer defined");
#endif

	DynArray_Z<const ShapeReco::Shape*> daShapes;
	m_pShapeAnalyzer->GetShapes("table", daShapes);

    Vec3fDA bests;
    QuatDA bestsRot;
    Float bestScore = 0.f;
    for (S32 s = 0; s < daShapes.GetSize(); ++s)
	{
		const ShapeReco::Shape* pShape = daShapes[s];

		for (S32 j = 0; j < pShape->m_slots.GetSize(); ++j)
		{
			const ShapeReco::Slot* pSlot = &pShape->m_slots[j];

			const ShapeReco::SlotCharacteristics::Rectangle& rectangle = pSlot->Get<ShapeReco::SlotCharacteristics::Rectangle>();
			Vec3f widthDir = rectangle.m_lengthDir ^ VEC3F_DOWN;

			Vec3fDA daDirs;
			Vec3fDA daFirstPoints;
			FloatDA daMaxOffsets;

			daDirs.Add(rectangle.m_lengthDir);
			daFirstPoints.Add(rectangle.m_center - rectangle.m_lengthDir * rectangle.m_length * 0.5f - widthDir * rectangle.m_width * 0.5f);
			daMaxOffsets.Add(rectangle.m_length);

			daDirs.Add(rectangle.m_lengthDir);
			daFirstPoints.Add(rectangle.m_center - rectangle.m_lengthDir * rectangle.m_length * 0.5f + widthDir * rectangle.m_width * 0.5f);
			daMaxOffsets.Add(rectangle.m_length);

			daDirs.Add(widthDir);
			daFirstPoints.Add(rectangle.m_center - rectangle.m_lengthDir * rectangle.m_length * 0.5f - widthDir * rectangle.m_width * 0.5f);
			daMaxOffsets.Add(rectangle.m_width);

			daDirs.Add(widthDir);
			daFirstPoints.Add(rectangle.m_center + rectangle.m_lengthDir * rectangle.m_length * 0.5f - widthDir * rectangle.m_width * 0.5f);
			daMaxOffsets.Add(rectangle.m_width);

			for (S32 i = 0; i < daDirs.GetSize(); ++i)
			{
				for (Float offset = _infos.m_vSize.z * 0.5f; offset <= daMaxOffsets[i] - _infos.m_vSize.z * 0.5f + Float_Eps; offset += m_fSizeVoxel)
				{
					_infos.m_vPos = daFirstPoints[i] + daDirs[i] * offset;
					_infos.m_vPos.y = m_fYGround + _infos.m_vSize.y * 0.5f;

					_infos.m_qRot = Quat(VEC3F_FRONT, daDirs[i]);

					if (CollidWithOtherObjects(_infos))
						continue;

					if (!CheckRules(_infos))
						continue;

					Float score = ComputeScoreWithConstraints(_infos);
					if (_infos.m_bOnlyComputePossiblePos)
					{
						_infos.m_avlPossiblePos.Add(score, Position3D_L(_infos.m_vPos, _infos.m_qRot));
						continue;
					}

					if (score > bestScore)
					{
						bests.Flush();
						bestsRot.Flush();
					}
					if (bests.GetSize() == 0 || Abs(score - bestScore) < Float_Eps)
					{
						bests.Add(_infos.m_vPos);
						bestsRot.Add(_infos.m_qRot);
						bestScore = score;
					}
				}
			}
		}
	}

	if (_infos.m_bOnlyComputePossiblePos)
        return (_infos.m_avlPossiblePos.GetNbElems() != 0);

    if (bests.GetSize() == 0)
		return FALSE;

    S32 finalBest = m_randomGenerator.Alea(0, bests.GetSize() - 1);

	_infos.m_vPos = bests[finalBest];
    _infos.m_qRot = bestsRot[finalBest];

	return TRUE;
}

//#define DEBUG_SURFACE_NAME Name_Z("THEMASK_Jacob_Dummy")
#ifdef DEBUG_SURFACE_NAME
#define DEBUG_SURFACE(_pos_,_color_) DRAW_DEBUG_SPHERE3D_IF(_infos.GetObjectName() == DEBUG_SURFACE_NAME && _infos.m_iChosenPlacement == 0, _pos_,_color_,0.04f,.displayDuration(1e7f))
#else
#define DEBUG_SURFACE(_pos_,_color_)
#endif

Bool FRGSolver_W::ComputePosOnSurfaces(FRGSolvingInfos_W& _infos, const DynArray_Z<const TopologyAnalyzer_W::Surface*>& _daSurfaces, Float _fYOffset)
{
	Float minWidth = Min(_infos.m_vSize.x, _infos.m_vSize.z);

    U32 widthMinSize = FLOOR(minWidth * 0.5f / m_fSizeVoxel) + 1;

    static const Float AngleStep = DegToRad(15.f);
    QuatDA rots;
	if (!_infos.m_position.m_bHaveFixedRot)
	{
		for (S32 i = 0; i < DegToRad(360.f) / AngleStep; ++i)
			rots.Add(Quat(i * AngleStep, VEC3F_UP));
	}

    Vec3fDA bests;
    QuatDA bestsRot;
    Float bestScore = 0.f;
	Float fMinScore = 1E6f;
	S32 iDebugS = -1;
	S32 iDebugC = -1; 
	S32 iDebugX = -1;
	S32 iDebugZ = -1;
	Float fDebugMinScore = -1.f;
    for(S32 s = 0; s < _daSurfaces.GetSize(); ++s)
    {
        const TopologyAnalyzer_W::Surface& surface = *_daSurfaces[s];

        for(S32 c = 0; c < surface.m_daCells.GetSize(); ++c)
        {
			if (!_infos.m_bAllowPartiallyInWall)
			{
				if (surface.m_daCells[c].m_iDistFromBorder < widthMinSize)
					continue;
			}

			Vec3f vOnFloorPos = m_pTopologyAnalyzer->GetCellPosition(surface, c);

			Float	xplusmax=Float_Eps;
			Float	zplusmax=Float_Eps;
			Float	xplusmin=0.f;
			Float	zplusmin=0.f;
			Float	xplusstep=1.f;
			Float	zplusstep=1.f;
			if (_infos.m_position.m_bHaveTiledPos)
			{
				Vec2i	itestpos;
				m_pPlaySpaceInfos->ComputeCellIndice(vOnFloorPos, itestpos.x, itestpos.y);
				Vec2i	itilepos;
				itilepos.x=Max((S32)1,(S32)(0.0001f+(_infos.m_position.m_vTiledPos.x/m_fSizeVoxel)));
				itilepos.y=Max((S32)1,(S32)(0.0001f+(_infos.m_position.m_vTiledPos.z/m_fSizeVoxel)));
				if (itestpos.x%itilepos.x)
					continue;
				if (itestpos.y%itilepos.y)
					continue;
			}

			Vec3f vOnFloorPosRef=vOnFloorPos;
			for (Float xplus=xplusmin; xplus<xplusmax; xplus+=xplusstep)
			{
				for (Float zplus=zplusmin; zplus<zplusmax; zplus+=zplusstep)
				{
					vOnFloorPos=vOnFloorPosRef+Vec3f(xplus,0.f,zplus);

					if (_infos.m_position.m_bHaveFixedRot)
					{
						Vec3f toLook = _infos.m_position.m_vPointToLook;
						toLook.y = vOnFloorPos.y;

						rots.Flush();
						rots.Add(Quat(VEC3F_FRONT, (toLook - vOnFloorPos).Normalize()));
					}

					fMinScore = 1E6f;
					S32 startRot = m_randomGenerator.Alea(0, rots.GetSize() - 1);
					for (S32 i = 0; i < rots.GetSize(); ++i)
					{
						S32 indice = (startRot + i) % rots.GetSize();
						Quat	&ThisRot=rots[indice];

						// Compute BOX
						Vec3f topLeft,topRight,bottomLeft,bottomRight;
						ComputeBox(_infos,vOnFloorPos,ThisRot,topLeft,topRight,bottomLeft,bottomRight);
						
						if( _infos.m_bAllowPartiallyInWall )
						{
							// CHECK ALL INSIDE (NOT ONE OUTSIDE)
							if( m_pTopologyAnalyzer->RectangleIsOk(topLeft,topRight,bottomLeft,bottomRight,surface,m_fSizeVoxel,TOPOLOGY_REQUEST_ONLYONE|TOPOLOGY_REQUEST_OUTSIDE) )
							{
								DEBUG_SURFACE(vOnFloorPos,COLOR_GREY);
								continue;
							}

							// CHECK ONE VALID
							if( !m_pTopologyAnalyzer->RectangleIsOk(topLeft,topRight,bottomLeft,bottomRight,surface,m_fSizeVoxel,TOPOLOGY_REQUEST_ONLYONE|TOPOLOGY_REQUEST_VALID) )
							{
								DEBUG_SURFACE(vOnFloorPos,COLOR_PURPLE);
								continue;
							}

							// CHECK NOT ONE VOID
							if( _infos.m_position.m_type != FRG_POSITION_ON_CEILING )
							{
								if( m_pTopologyAnalyzer->RectangleIsOk(topLeft,topRight,bottomLeft,bottomRight,surface,m_fSizeVoxel,TOPOLOGY_REQUEST_ONLYONE|TOPOLOGY_REQUEST_VOID) )
								{
									DEBUG_SURFACE(vOnFloorPos,COLOR_CYAN);
									continue;
								}
							}
						}
						else
						{
							// CHECK ALL VALID
							if( !m_pTopologyAnalyzer->RectangleIsOk(topLeft,topRight,bottomLeft,bottomRight,surface,m_fSizeVoxel,TOPOLOGY_REQUEST_VALID) )
							{
								DEBUG_SURFACE(vOnFloorPos,COLOR_YELLOW);
								continue;
							}

							// CHECK ENOUGH SPACE
							S32 limit = FLOORINT(_infos.m_vClearanceSize.y / m_fSizeVoxel);
							if( !m_pTopologyAnalyzer->RectangleIsOk(topLeft,topRight,bottomLeft,bottomRight,surface,m_fSizeVoxel,TOPOLOGY_REQUEST_SPACE,limit) )
							{
								DEBUG_SURFACE(vOnFloorPos,COLOR_ORANGE);
								continue;
							}
						}

						// check full box
						if( _infos.m_bHasFullBox )
						{
							// TODO
						}

						_infos.m_vPos = vOnFloorPos + VEC3F_UP * _fYOffset;
						_infos.m_qRot = ThisRot;

						if (!CheckRules(_infos))
						{
							DEBUG_SURFACE(vOnFloorPos,COLOR_BLUE);
							continue;
						}
						
						// Test de visibilité à 1m80 du sol. Permet d'éviter d'avoir des trucs derriere des murs depuis le centre du playspace.
						//   Et s'il y a un mur dans le centre du playspace on fait quoi? => Pour le moment ça ne teste pas à moins d'1m du centre
						if (!m_pTopologyAnalyzer->CellIsVisibleFromPlayspaceCenter(m_pPlaySpaceInfos,_infos.m_vPos))
						{
							DEBUG_SURFACE(vOnFloorPos,COLOR_PINK);
							continue;
						}

						if (CollidWithOtherObjects(_infos))
						{
							DEBUG_SURFACE(vOnFloorPos,COLOR_RED);
							continue;
						}

						DEBUG_SURFACE(vOnFloorPos,COLOR_GREEN);

						Float score = ComputeScoreWithConstraints(_infos);	

						if (_infos.m_bOnlyComputePossiblePos)
						{
							_infos.m_avlPossiblePos.Add(score, Position3D_L(_infos.m_vPos, _infos.m_qRot));
							continue;
						}

						if (score > bestScore)
						{
							bests.Flush();
							bestsRot.Flush();
						}
						if(score<fMinScore)
						{
							fMinScore = score;
						}
						if (bests.GetSize() == 0 || Abs(score - bestScore) < Float_Eps)
						{
							bests.Add(_infos.m_vPos);
							bestsRot.Add(_infos.m_qRot);
							bestScore = score;
							if(_infos.m_bDebugInfo)
							{
								iDebugS = s;
								iDebugC = c;
								iDebugX = xplus;
								iDebugZ = zplus;
								fDebugMinScore = fMinScore;
							}
						}
					}
				}
            }
        }
    }

	if (_infos.m_bOnlyComputePossiblePos)
		return (_infos.m_avlPossiblePos.GetNbElems() != 0);

    if (bests.GetSize() == 0)
        return FALSE;

    S32 finalBest = m_randomGenerator.Alea(0, bests.GetSize() - 1);

	_infos.m_vPos = bests[finalBest];
    _infos.m_qRot = bestsRot[finalBest];

	if (_infos.m_position.m_bHas90RandomRotation)
		_infos.m_qRot *= Quat(Alea(0, 3) * DegToRad(90.f), VEC3F_UP);

    return TRUE;
}
