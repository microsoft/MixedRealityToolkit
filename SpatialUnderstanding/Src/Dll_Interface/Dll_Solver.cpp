// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include <pch.h>
#include <Dll_Interface\Dll_Solver.h>
#include <UnderstandingMgr_W.h>
#include <System_Z.h>

EXTERN_C __declspec(dllexport) int Solver_Init()
{
	SetAndRestoreFloatControlDownward floatControlDownward;

	FRGSolver_W& solver = UnderstandingMgr_W::GetUnderstandingMgr().GetSolver();

	// Init the solver
	solver.Init(UnderstandingMgr_W::GetUnderstandingMgr().GetPlayspaceInfosPtr(),
		&UnderstandingMgr_W::GetUnderstandingMgr().GetPlayspaceInfos().m_TopologyAnalyzer,
		&UnderstandingMgr_W::GetUnderstandingMgr().GetPlayspaceInfos().m_ShapeAnalyzer);

	return 1;
}

void PlacementDefinition_to_FRGSolvingInfos_W(
	char* objectName,
	Dll_Interface::ObjectPlacementDefinition& placementDefinition,
	FRGSolvingInfos_W& solvingInfos)
{
	// Type
	switch (placementDefinition.Type)
	{
		case Dll_Interface::Place_OnFloor: 
			solvingInfos.SetPositionOnFloor();
			break;
		case Dll_Interface::Place_OnWall: 
			solvingInfos.SetPositionOnWall(placementDefinition.PlacementParam_Float_0, placementDefinition.PlacementParam_Float_1, placementDefinition.WallFlags);
			break;
		case Dll_Interface::Place_OnCeiling: 
			solvingInfos.SetPositionOnCeiling();
			break;
		case Dll_Interface::Place_OnShape: 
			solvingInfos.SetPositionOnShape(Name_Z(placementDefinition.PlacementParam_Str_0), Name_Z(String_Z<8>(placementDefinition.PlacementParam_Int_0)));
			break;
		case Dll_Interface::Place_OnEdge:
			solvingInfos.SetPositionOnEdge(Vec3f(placementDefinition.PlacementParam_Float_0, placementDefinition.PlacementParam_Float_1, placementDefinition.PlacementParam_Float_2) * 2.0f);
			break;
		case Dll_Interface::Place_OnFloorAndCeiling: 
			solvingInfos.SetPositionOnFloorAndCeiling(Vec3f(placementDefinition.PlacementParam_Float_0, placementDefinition.PlacementParam_Float_1, placementDefinition.PlacementParam_Float_2) * 2.0f);
			break;
		case Dll_Interface::Place_RandomInAir: 
			solvingInfos.SetPositionRandomInTheAir();
			break;
		case Dll_Interface::Place_InMidAir: 
			solvingInfos.SetPositionInTheMidAir();
			break;
		case Dll_Interface::Place_UnderPlatformEdge:
			solvingInfos.SetPositionUnderFurnitureEdge();
			break;
	}
	
	// Parameters
	solvingInfos.m_univers = INALLUNIVERS;
	solvingInfos.m_vSize = Util_L::Convert_XMFLOAT3_to_Vec3f(placementDefinition.HalfDims) * 2.0f;
	solvingInfos.SetObjectName(objectName);

	solvingInfos.m_vCenter = VEC3F_NULL;
	solvingInfos.m_vEmptySize = solvingInfos.m_vSize;
	solvingInfos.m_vEmptyCenter = VEC3F_NULL;
	solvingInfos.m_vClearanceSize = solvingInfos.m_vSize;
	solvingInfos.m_vClearanceCenter = VEC3F_NULL;
	solvingInfos.m_bHasFullBox = FALSE;
	solvingInfos.ComputeClearanceBoxAndEmptyBoxAreDifferent();
}

static void Solver_AddRule(FRGSolvingInfos_W& solvingInfos, Dll_Interface::ObjectPlacementRule& placementRule)
{
	switch (placementRule.Type)
	{
	case Dll_Interface::Rule_AwayFromPosition:
		solvingInfos.AddRuleAwayFrom(
			placementRule.RuleParam_Float_0,
			Util_L::Transform_XMFLOAT3_to_Vec3f(placementRule.RuleParam_Vec3_0, UnderstandingMgr_W::GetUnderstandingMgr().GetFrameTransfoForInputMat()));
		break;
	case Dll_Interface::Rule_AwayFromWalls:
		solvingInfos.AddRuleAwayFromWalls(placementRule.RuleParam_Float_0, placementRule.RuleParam_Float_1);
		break;
	case Dll_Interface::Rule_AwayFromOtherObjects:
		solvingInfos.AddRuleAwayFromOtherObjects(placementRule.RuleParam_Float_0);
		break;
	}
}

static void Solver_AddConstraint(FRGSolvingInfos_W& solvingInfos, Dll_Interface::ObjectPlacementConstraint& placementConstraint)
{
	switch (placementConstraint.Type)
	{
	case Dll_Interface::Constraint_NearPoint:
		solvingInfos.AddConstraintNearOf(
			Util_L::Transform_XMFLOAT3_to_Vec3f(placementConstraint.RuleParam_Vec3_0, UnderstandingMgr_W::GetUnderstandingMgr().GetFrameTransfoForInputMat()), 
			placementConstraint.RuleParam_Float_0, 
			placementConstraint.RuleParam_Float_1);
		break;
	case Dll_Interface::Constraint_NearWall:
		solvingInfos.AddConstraintNearOfWall(
			placementConstraint.RuleParam_Int_0 > 0, 
			placementConstraint.RuleParam_Float_2, 
			placementConstraint.RuleParam_Float_0, 
			placementConstraint.RuleParam_Float_1);
		break;
	case Dll_Interface::Constraint_AwayFromWalls:
		solvingInfos.AddConstraintWayFromWall();
		break;
	case Dll_Interface::Constraint_NearCenter:
		solvingInfos.AddConstraintNearfOfCenter(
			placementConstraint.RuleParam_Float_0, 
			placementConstraint.RuleParam_Float_1);
		break;
	case Dll_Interface::Constraint_AwayFromOtherObjects:
		solvingInfos.AddConstraintAwayFromOtherObjects();
		break;
	case Dll_Interface::Constraint_AwayFromPoint:
		solvingInfos.AddConstraintAwayFromPoint(
			Util_L::Transform_XMFLOAT3_to_Vec3f(placementConstraint.RuleParam_Vec3_0, UnderstandingMgr_W::GetUnderstandingMgr().GetFrameTransfoForInputMat()));
		break;
	}
}

EXTERN_C __declspec(dllexport) int Solver_PlaceObject(
	_In_ char* objectName,
	_In_ Dll_Interface::ObjectPlacementDefinition* placementDefinition,
	_In_ int placementRuleCount,
	_In_ Dll_Interface::ObjectPlacementRule* placementRules,
	_In_ int constraintCount,
	_In_ Dll_Interface::ObjectPlacementConstraint* placementConstraints,
	_Out_ Dll_Interface::ObjectPlacementResult* placementResult)
{
	SetAndRestoreFloatControlDownward floatControlDownward;

	FRGSolver_W& solver = UnderstandingMgr_W::GetUnderstandingMgr().GetSolver();

	// Convert to a solvingInfo structure
	FRGSolvingInfos_W solvingInfos;
	PlacementDefinition_to_FRGSolvingInfos_W(objectName, *placementDefinition, solvingInfos);

	// Add rules
	for (int i = 0; i < placementRuleCount; ++i)
	{
		Solver_AddRule(solvingInfos, placementRules[i]);
	}

	// Add constraints
	for (int i = 0; i < constraintCount; ++i)
	{
		Solver_AddConstraint(solvingInfos, placementConstraints[i]);
	}

	// And run the solver
	if (solver.Solve(solvingInfos))
	{
		// Setup for transforming back to external space (and from the solved local orientation)
		Mat4x4 matFromDll = UnderstandingMgr_W::GetUnderstandingMgr().GetFrameTransfoForOutputMat();
		Mat4x4 matFromSolve; solvingInfos.m_qRot.GetMatrix(matFromSolve);
		matFromSolve = matFromDll * matFromSolve;

		// Read out the data
		Util_L::Transform_Vec3f_to_XMFLOAT3(solvingInfos.m_vPos, placementResult->Position, matFromDll);
		Util_L::Transform_Vec3f_to_XMFLOAT3(Vec3f(1.0f, 0.0f, 0.0f), placementResult->Right, matFromSolve);
		Util_L::Transform_Vec3f_to_XMFLOAT3(Vec3f(0.0f, 1.0f, 0.0f), placementResult->Up, matFromSolve);
		Util_L::Transform_Vec3f_to_XMFLOAT3(Vec3f(0.0f, 0.0f, 1.0f), placementResult->Forward, matFromSolve);
		Util_L::Convert_Vec3f_to_XMFLOAT3(solvingInfos.m_vSize * 0.5f, placementResult->HalfDims);
		
		return 1;
	}

	return 0;
}

EXTERN_C __declspec(dllexport) int Solver_RemoveObject(
	_In_ char* placementName)
{
	SetAndRestoreFloatControlDownward floatControlDownward;

	FRGSolver_W& solver = UnderstandingMgr_W::GetUnderstandingMgr().GetSolver();
	const DynArray_Z<FRGSolvingInfos_W>& solvedObjects = solver.GetSolvedObjects();
	Name_Z placementNameZ = Name_Z(placementName);
	for (int i = 0; i < solvedObjects.GetSize(); ++i)
	{
		if (solvedObjects[i].GetObjectName() == placementNameZ)
		{
			solver.RemoveSolvedBox(i);
			return 1;
		}
	}

	return 0;
}

EXTERN_C __declspec(dllexport) void Solver_RemoveAllObjects()
{
	SetAndRestoreFloatControlDownward floatControlDownward;
	FRGSolver_W& solver = UnderstandingMgr_W::GetUnderstandingMgr().GetSolver();
	solver.RemoveAllSolved();
}
