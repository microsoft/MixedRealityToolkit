
#include <pch.h>
#include <FRGSolver/FRGSolvingInfos_W.h>

FRGSolvingInfos_W::FRGSolvingInfos_W()
{
	m_bOnlyComputePossiblePos = FALSE;
	m_bCheckOppositePos = FALSE;
	m_vOppositePos = VEC3F_NULL;
	m_bAllowPartiallyInWall = FALSE;
	m_idx = -1;
	m_iChosenPlacement = -1;

	m_bHasFullBox = FALSE;
	
	m_position.m_bHaveFixedRot = FALSE;
	m_position.m_bHaveTiledPos = FALSE;
	m_position.m_bHas90RandomRotation = FALSE;

	m_bEmptyVsClearanceDifferent = FALSE;
	m_bUseSmallerBox = FALSE;
	m_fFactor = 1.f;
}

//------------------------------------------------------------------------------------
// Position
//-----------------------------------------------------------------------------------
void FRGSolvingInfos_W::SetPositionOnFloor()
{
	m_position.m_type = FRG_POSITION_ON_FLOOR;
}

void FRGSolvingInfos_W::SetPositionOnWall(Float _fHeightMin, Float _fHeightMax, U32 _uWallTypes, Float _fLeftMargin, Float _fRightMargin)
{
	m_position.m_type = FRG_POSITION_ON_WALL;
	m_position.m_paramsOnWall.m_fHeightMin = _fHeightMin;
	m_position.m_paramsOnWall.m_fHeightMax = _fHeightMax;
	m_position.m_paramsOnWall.m_uWallTypes = _uWallTypes;
	m_position.m_paramsOnWall.m_fLeftMargin = _fLeftMargin;
	m_position.m_paramsOnWall.m_fRightMargin = _fRightMargin;
}

void FRGSolvingInfos_W::SetPositionRandomInTheAir()
{
	m_position.m_type = FRG_POSITION_RANDOM_IN_THE_AIR;
}

void FRGSolvingInfos_W::SetPositionInTheMidAir()
{
	m_position.m_type = FRG_POSITION_IN_THE_MID_AIR;
}

void FRGSolvingInfos_W::SetPositionUnderFurnitureEdge()
{
	m_position.m_type = FRG_POSITION_UNDER_FURNITURE_EDGE;
}

void FRGSolvingInfos_W::SetPositionOnCeiling()
{
	m_position.m_type = FRG_POSITION_ON_CEILING;
}

void FRGSolvingInfos_W::SetPositionOnShape(const Name_Z& _nShapeName, const Name_Z& _nSlotName)
{
	m_position.m_type = FRG_POSITION_ON_SHAPE;
	m_position.m_paramsOnShape.m_nShapeName = _nShapeName;
	m_position.m_paramsOnShape.m_nSlotName = _nSlotName;
}

void FRGSolvingInfos_W::SetPositionOnEdge(const Vec3f& _vSizeBottomObject)
{
	m_position.m_type = FRG_POSITION_ON_EDGE;
	m_position.m_paramsOnEdge.m_vSizeBottomObject = _vSizeBottomObject;
}

void FRGSolvingInfos_W::SetPositionOnFloorAndCeiling(const Vec3f& _vSizeBottomObject)
{
	m_position.m_type = FRG_POSITION_ON_FLOOR_AND_CEILING;
	m_position.m_paramsOnEdge.m_vSizeBottomObject = _vSizeBottomObject;
}

//------------------------------------------------------------------------------------
// Rules
//-----------------------------------------------------------------------------------
void FRGSolvingInfos_W::AddRuleAwayFrom(Float _fDistMin, const Vec3f& _vPoint)
{
	FRGRule_W& rule = m_daRules[m_daRules.Add()];
	rule.m_type = FRG_RULE_AWAY_FROM;
	rule.m_paramsAwayFrom.m_fDistMin = _fDistMin;
	rule.m_paramsAwayFrom.m_vPoint = _vPoint;
}

void FRGSolvingInfos_W::AddRuleAwayFromWalls(Float _fDistMin, Float _fWallHeightMin)
{
	FRGRule_W& rule = m_daRules[m_daRules.Add()];
	rule.m_type = FRG_RULE_AWAY_FROM_WALLS;
	rule.m_paramsAwayFromWalls.m_fDistMin = _fDistMin;
	rule.m_paramsAwayFromWalls.m_fWallHeightMin = _fWallHeightMin;
}

void FRGSolvingInfos_W::AddRuleAwayFromOtherObjects(Float _fDistMin)
{
	FRGRule_W& rule = m_daRules[m_daRules.Add()];
	rule.m_type = FRG_RULE_AWAY_FROM_OTHER_OBJECTS;
	rule.m_paramsAwayFromOtherObjects.m_fDistMin = _fDistMin;
}

void FRGSolvingInfos_W::AddRuleOccluded()
{
	FRGRule_W& rule = m_daRules[m_daRules.Add()];
	rule.m_type = FRG_RULE_OCCLUDED;
}

void FRGSolvingInfos_W::AddRuleNotOccluded()
{
	FRGRule_W& rule = m_daRules[m_daRules.Add()];
	rule.m_type = FRG_RULE_NOT_OCCLUDED;
}

void FRGSolvingInfos_W::AddRuleOutOfInterval(Float _fInfBound, Float _fSupBound, S32 _Idxs)
{
	FRGRule_W& rule = m_daRules[m_daRules.Add()];
	rule.m_type = FRG_RULE_OUT_OF_INTERVAL;
	rule.m_paramsOutOfInterval.m_fInfBound = _fInfBound;
	rule.m_paramsOutOfInterval.m_fSupBound = _fSupBound;
	rule.m_paramsOutOfInterval.m_iObjectsToCheck = _Idxs;
}

//------------------------------------------------------------------------------------
// Constraints
//-----------------------------------------------------------------------------------
void FRGSolvingInfos_W::AddConstraintNearOf(const Vec3f& _vPoint, Float _fDistMin, Float _fDistMax)
{
	FRGConstraint_W& constraint = m_daConstraints[m_daConstraints.Add()];
	constraint.m_type = FRG_CONSTRAINT_NEAR_OF;
	constraint.m_paramsNearOf.m_vPoint = _vPoint;
	constraint.m_paramsNearOf.m_fDistMin = _fDistMin;
	constraint.m_paramsNearOf.m_fDistMax = _fDistMax;
}

void FRGSolvingInfos_W::AddConstraintNearOfWall(Bool _virtual, Float _height, Float _fDistMin, Float _fDistMax)
{
	FRGConstraint_W& constraint = m_daConstraints[m_daConstraints.Add()];
	constraint.m_type = FRG_CONSTRAINT_NEAR_OF_WALL;
	constraint.m_paramsNearOfWall.m_bVirtual = _virtual;
	constraint.m_paramsNearOfWall.m_fHeight = _height;
	constraint.m_paramsNearOfWall.m_fDistMin = _fDistMin;
	constraint.m_paramsNearOfWall.m_fDistMax = _fDistMax;
}

void FRGSolvingInfos_W::AddConstraintWayFromWall()
{
	FRGConstraint_W& constraint = m_daConstraints[m_daConstraints.Add()];
	constraint.m_type = FRG_CONSTRAINT_AWAY_FROM_WALL;;
}

void FRGSolvingInfos_W::AddConstraintNearfOfCenter(Float _fDistMin, Float _fDistMax)
{
	FRGConstraint_W& constraint = m_daConstraints[m_daConstraints.Add()];
	constraint.m_type = FRG_CONSTRAINT_NEAR_OF_CENTER;
	constraint.m_paramsNearOfCenter.m_fDistMin = _fDistMin;
	constraint.m_paramsNearOfCenter.m_fDistMax = _fDistMax;
#ifdef _GAMEDEBUG
	CANCEL_EXCEPTIONC_Z(constraint.m_paramsNearOfCenter.m_fDistMin >= 0.f && constraint.m_paramsNearOfCenter.m_fDistMax >= 0.f && constraint.m_paramsNearOfCenter.m_fDistMin <= constraint.m_paramsNearOfCenter.m_fDistMax, "FRGSolver_W::ComputeScoreNearOfCenter bad distMin/distMax");
#endif
}

void FRGSolvingInfos_W::AddConstraintOnSegment(const Segment_Z& _segment)
{
	FRGConstraint_W& constraint = m_daConstraints[m_daConstraints.Add()];
	constraint.m_type = FRG_CONSTRAINT_ON_SEGMENT;
	constraint.m_paramsOnSegment.m_segment = _segment;
}

void FRGSolvingInfos_W::AddConstraintFacing(const Segment_Z& _segment)
{
	FRGConstraint_W& constraint = m_daConstraints[m_daConstraints.Add()];
	constraint.m_type = FRG_CONSTRAINT_FACING;
	constraint.m_paramsOnSegment.m_segment = _segment;
}

void FRGSolvingInfos_W::AddConstraintRayCast(const Segment_Z& _segment)
{
	FRGConstraint_W& constraint = m_daConstraints[m_daConstraints.Add()];
	constraint.m_type = FRG_CONSTRAINT_RAYCAST;
	constraint.m_paramsOnSegment.m_segment = _segment;
}

void FRGSolvingInfos_W::AddConstraintAwayFromOtherObjects()
{
	FRGConstraint_W& constraint = m_daConstraints[m_daConstraints.Add()];
	constraint.m_type = FRG_CONSTRAINT_AWAY_FROM_OTHER_OBJECTS;
}

void FRGSolvingInfos_W::AddConstraintAwayFromPoint(const Vec3f& _vPoint)
{
	FRGConstraint_W& constraint = m_daConstraints[m_daConstraints.Add()];
	constraint.m_type = FRG_CONSTRAINT_AWAY_FROM_POINT;
	constraint.m_paramsAwayFromPoint.m_vPoint.Add(_vPoint);
}