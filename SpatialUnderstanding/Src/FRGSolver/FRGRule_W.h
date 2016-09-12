// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#ifndef __FRG_RULE_H__
#define __FRG_RULE_H__

//------------------------------------------------------------------------------------
// FRGRuleType_W
//------------------------------------------------------------------------------------
enum FRGRuleType_W
{
	FRG_RULE_AWAY_FROM,
	FRG_RULE_AWAY_FROM_WALLS,
	FRG_RULE_AWAY_FROM_OTHER_OBJECTS,
	FRG_RULE_OCCLUDED,
	FRG_RULE_NOT_OCCLUDED,
	FRG_RULE_OUT_OF_INTERVAL
};

//------------------------------------------------------------------------------------
// Params
//-----------------------------------------------------------------------------------
struct FRGRuleParamsAwayFrom_W
{
	Float m_fDistMin;
	Vec3f m_vPoint;
};

struct FRGRuleParamsAwayFromWalls_W
{
	Float m_fDistMin;
	Float m_fWallHeightMin;
};

struct FRGRuleParamsAwayFromOtherObjects_W
{
	Float m_fDistMin;
};

struct FRGRuleParamsOutOfInterval_W
{
	Float	m_fInfBound;
	Float	m_fSupBound;
	S32		m_iObjectsToCheck;
};

//------------------------------------------------------------------------------------
// FRGRule_W
//-----------------------------------------------------------------------------------
class FRGRule_W
{
public:
	FRGRuleType_W m_type;

	FRGRuleParamsAwayFrom_W m_paramsAwayFrom;
	FRGRuleParamsAwayFromWalls_W m_paramsAwayFromWalls;
	FRGRuleParamsAwayFromOtherObjects_W m_paramsAwayFromOtherObjects;
	FRGRuleParamsOutOfInterval_W m_paramsOutOfInterval;
	
};

#endif