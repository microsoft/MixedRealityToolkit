// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#ifndef _DLL_SHAPES
#define _DLL_SHAPES

namespace Dll_Interface
{
#pragma pack(push, 1)
	// Path finding
	struct PathQuery
	{
		DirectX::XMFLOAT3 Start;
		DirectX::XMFLOAT3 End;
		float MaxRadius;
	};
	struct PathResultNode
	{
		DirectX::XMFLOAT3 Position;
	};

	// Object placement
	enum ObjectPlacementType
	{
		Place_OnFloor,
		Place_OnWall,
		Place_OnCeiling,
		Place_OnShape,
		Place_OnEdge,
		Place_OnFloorAndCeiling,
		Place_RandomInAir,
		Place_InMidAir,
		Place_UnderPlatformEdge,
	};
	struct ObjectPlacementDefinition
	{
		ObjectPlacementType Type;
		int PlacementParam_Int_0;
		float PlacementParam_Float_0;
		float PlacementParam_Float_1;
		float PlacementParam_Float_2;
		float PlacementParam_Float_3;
		char* PlacementParam_Str_0;
		int WallFlags;
		DirectX::XMFLOAT3 HalfDims;
	};
	enum ObjectPlacementRuleType
	{
		Rule_AwayFromPosition,
		Rule_AwayFromWalls,
		Rule_AwayFromOtherObjects,
	};
	struct ObjectPlacementRule
	{
		ObjectPlacementRuleType Type;
		int RuleParam_Int_0;
		float RuleParam_Float_0;
		float RuleParam_Float_1;
		DirectX::XMFLOAT3 RuleParam_Vec3_0;
	};
	enum ObjectPlacementConstraintType
	{
		Constraint_NearPoint,
		Constraint_NearWall,
		Constraint_AwayFromWalls,
		Constraint_NearCenter,
		Constraint_AwayFromOtherObjects,
		Constraint_AwayFromPoint
	};
	struct ObjectPlacementConstraint
	{
		ObjectPlacementConstraintType Type;
		int RuleParam_Int_0;
		float RuleParam_Float_0;
		float RuleParam_Float_1;
		float RuleParam_Float_2;
		DirectX::XMFLOAT3 RuleParam_Vec3_0;
	};
	struct ObjectPlacementResult
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT3 HalfDims;
		DirectX::XMFLOAT3 Forward;
		DirectX::XMFLOAT3 Right;
		DirectX::XMFLOAT3 Up;
	};
#pragma pack(pop)

	// Object placement
	EXTERN_C __declspec(dllexport) int Solver_PlaceObject(
		_In_ char* objectName, 
		_In_ ObjectPlacementDefinition* placementDefinition,
		_In_ int placementRuleCount,
		_In_ ObjectPlacementRule* placementRules,
		_In_ int constraintCount,
		_In_ ObjectPlacementConstraint* placementConstraints,
		_Out_ ObjectPlacementResult* placementResult
	);
	EXTERN_C __declspec(dllexport) int Solver_RemoveObject(
		_In_ char* objectName
	);
	EXTERN_C __declspec(dllexport) void Solver_RemoveAllObjects();
}

#endif
