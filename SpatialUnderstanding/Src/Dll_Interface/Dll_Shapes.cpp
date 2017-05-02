// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include <pch.h>
#include <Dll_Interface\Dll_Shapes.h>
#include <UnderstandingMgr_W.h>
#include <System_Z.h>

int OutputShapes(
	int shapeCount,
	Dll_Interface::ShapeResult* shapeData,
	Vec3fDA& outPos,
	Vec3f halfDims)
{
	// If they didn't provide any data, just report the size
	if ((shapeCount == 0) || (shapeData == NULL))
	{
		shapeCount = outPos.GetSize();
		return shapeCount;
	}

	UnderstandingMgr_W &UnderstandingMgr = UnderstandingMgr_W::GetUnderstandingMgr();

	// Transform out of dll
	Quat quatDLLToWorld = UnderstandingMgr.GetFrameTransfoForOutput();
	Mat4x4	matDLLToWorld;
	quatDLLToWorld.GetMatrix(matDLLToWorld);

	// Copy the data out (and transform it back)
	shapeCount = min(outPos.GetSize(), shapeCount);
	for (int i = 0; i < shapeCount; ++i)
	{
		Util_L::Transform_Vec3f_to_XMFLOAT3(outPos[i], shapeData[i].position, matDLLToWorld);
		Util_L::Convert_Vec3f_to_XMFLOAT3(halfDims, shapeData[i].halfDims);
	}

	return shapeCount;
}

int OutputShapes(
	int shapeCount,
	Dll_Interface::ShapeResult* shapeData,
	Vec3fDA& outPos,
	Vec3fDA& outHalfDims)
{
	// If they didn't provide any data, just report the size
	if ((shapeCount == 0) || (shapeData == NULL))
	{
		shapeCount = outPos.GetSize();
		return shapeCount;
	}

	UnderstandingMgr_W &UnderstandingMgr = UnderstandingMgr_W::GetUnderstandingMgr();

	// Transform out of dll
	Quat quatDLLToWorld = UnderstandingMgr.GetFrameTransfoForOutput();
	Mat4x4	matDLLToWorld;
	quatDLLToWorld.GetMatrix(matDLLToWorld);

	// Copy the data out (and transform it back)
	shapeCount = min(outPos.GetSize(), shapeCount);
	for (int i = 0; i < shapeCount; ++i)
	{
		Util_L::Transform_Vec3f_to_XMFLOAT3(outPos[i], shapeData[i].position, matDLLToWorld);
		Util_L::Convert_Vec3f_to_XMFLOAT3(outHalfDims[i], shapeData[i].halfDims);
	}

	return shapeCount;
}


EXTERN_C __declspec(dllexport) int QueryShape_FindPositionsOnShape(
	_In_ char* shapeName,
	_In_ float minRadius,
	_In_ int shapeCount,
	_Inout_ Dll_Interface::ShapeResult* shapeData)
{
	SetAndRestoreFloatControlDownward floatControlDownward;

	Vec3fDA outPos;
	UnderstandingMgr_W::GetUnderstandingMgr().GetPlayspaceInfos().m_ShapeAnalyzer.GetAllPosOnShape(shapeName, NAME_NULL, minRadius, outPos);

	return OutputShapes(shapeCount, shapeData, outPos, Vec3f(0.0f, 0.0f, 0.0f));
}

EXTERN_C __declspec(dllexport) int QueryShape_FindShapeHalfDims(
	_In_ char* shapeName,
	_In_ int shapeCount,
	_Inout_ Dll_Interface::ShapeResult* shapeData)
{
	SetAndRestoreFloatControlDownward floatControlDownward;

	DynArray_Z<const TopologyAnalyzer_W::Surface*> outSurfaces;
	UnderstandingMgr_W::GetUnderstandingMgr().GetPlayspaceInfos().m_ShapeAnalyzer.GetAllSurfacesOnShape(shapeName, NAME_NULL, outSurfaces);

	Vec3fDA outPos, outHalfDims;
	for (int i = 0; i < outSurfaces.GetSize(); ++i)
	{
		Vec3f minPos = outSurfaces[i]->m_vMinPos;
		Vec3f maxPos = outSurfaces[i]->m_vMaxPos;
		outPos.Add((minPos + maxPos) * 0.5f);
		outHalfDims.Add((maxPos - minPos) * 0.5f);
	}

	return OutputShapes(shapeCount, shapeData, outPos, outHalfDims);
}

EXTERN_C __declspec(dllexport) void RemoveAllShapes()
{
	SetAndRestoreFloatControlDownward floatControlDownward;
	ShapeReco::ShapeAnalyzer_W& shapeAnalyzer = UnderstandingMgr_W::GetUnderstandingMgr().GetPlayspaceInfos().m_ShapeAnalyzer;
	shapeAnalyzer.Reset();
}

ShapeReco::SlotDesc ShapeComponent_to_SlotDesc(int componentIndex, Dll_Interface::ShapeComponent& shapeComponent)
{
	ShapeReco::SlotDesc slotDesc;
	slotDesc.SetName(Name_Z(String_Z<8>(componentIndex)));
	for (int i = 0; i < shapeComponent.ConstraintCount; ++i)
	{
		switch (shapeComponent.Constraints[i].Type)
		{
		case Dll_Interface::SurfaceNotPartOfShape	: slotDesc.AddConstraint(ShapeReco::SlotConstraints::SurfacesAreNotPartOf(shapeComponent.Constraints[i].Param_Str_0)); break;

		case Dll_Interface::SurfaceHeight_Min		: slotDesc.AddConstraint(ShapeReco::SlotConstraints::MinSurfaceHeight(shapeComponent.Constraints[i].Param_Float_0)); break;
		case Dll_Interface::SurfaceHeight_Max		: slotDesc.AddConstraint(ShapeReco::SlotConstraints::MaxSurfaceHeight(shapeComponent.Constraints[i].Param_Float_0)); break;
		case Dll_Interface::SurfaceHeight_Between	: slotDesc.AddConstraint(ShapeReco::SlotConstraints::SurfaceHeightBetween(
															shapeComponent.Constraints[i].Param_Float_0, 
															shapeComponent.Constraints[i].Param_Float_1)); break;
		case Dll_Interface::SurfaceHeight_Is		: slotDesc.AddConstraint(ShapeReco::SlotConstraints::SurfaceHeightIs(shapeComponent.Constraints[i].Param_Float_0)); break;

		case Dll_Interface::SurfaceCount_Min		: slotDesc.AddConstraint(ShapeReco::SlotConstraints::MinNbSurfaces(shapeComponent.Constraints[i].Param_Int_0)); break;
		case Dll_Interface::SurfaceCount_Max		: slotDesc.AddConstraint(ShapeReco::SlotConstraints::MaxNbSurfaces(shapeComponent.Constraints[i].Param_Int_0)); break;
		case Dll_Interface::SurfaceCount_Between	: slotDesc.AddConstraint(ShapeReco::SlotConstraints::NbSurfacesBetween(
															shapeComponent.Constraints[i].Param_Int_0,
															shapeComponent.Constraints[i].Param_Int_1)); break;
		case Dll_Interface::SurfaceCount_Is			: slotDesc.AddConstraint(ShapeReco::SlotConstraints::NbSurfacesIs(shapeComponent.Constraints[i].Param_Int_0)); break;

		case Dll_Interface::SurfaceArea_Min			: slotDesc.AddConstraint(ShapeReco::SlotConstraints::MinArea(shapeComponent.Constraints[i].Param_Float_0)); break;
		case Dll_Interface::SurfaceArea_Max			: slotDesc.AddConstraint(ShapeReco::SlotConstraints::MaxArea(shapeComponent.Constraints[i].Param_Float_0)); break;
		case Dll_Interface::SurfaceArea_Between		: slotDesc.AddConstraint(ShapeReco::SlotConstraints::AreaBetween(
															shapeComponent.Constraints[i].Param_Float_0,
															shapeComponent.Constraints[i].Param_Float_1)); break;
		case Dll_Interface::SurfaceArea_Is			: slotDesc.AddConstraint(ShapeReco::SlotConstraints::AreaIs(shapeComponent.Constraints[i].Param_Float_0)); break;

		case Dll_Interface::IsRectangle				: slotDesc.AddConstraint(ShapeReco::SlotConstraints::IsRectangle(shapeComponent.Constraints[i].Param_Float_0)); break;
		case Dll_Interface::RectangleSize_Min		: slotDesc.AddConstraint(ShapeReco::SlotConstraints::MinRectangleSize(
															shapeComponent.Constraints[i].Param_Float_0,
															shapeComponent.Constraints[i].Param_Float_1)); break;
		case Dll_Interface::RectangleSize_Max		: slotDesc.AddConstraint(ShapeReco::SlotConstraints::MaxRectangleSize(
															shapeComponent.Constraints[i].Param_Float_0,
															shapeComponent.Constraints[i].Param_Float_1)); break;
		case Dll_Interface::RectangleSize_Between	: slotDesc.AddConstraint(ShapeReco::SlotConstraints::RectangleSizeBetween(
															shapeComponent.Constraints[i].Param_Float_0, shapeComponent.Constraints[i].Param_Float_1,
															shapeComponent.Constraints[i].Param_Float_2, shapeComponent.Constraints[i].Param_Float_3)); break;
		case Dll_Interface::RectangleSize_Is		: slotDesc.AddConstraint(ShapeReco::SlotConstraints::RectangleSizeIs(
															shapeComponent.Constraints[i].Param_Float_0,
															shapeComponent.Constraints[i].Param_Float_1)); break;

		case Dll_Interface::RectangleLength_Min		: slotDesc.AddConstraint(ShapeReco::SlotConstraints::MinRectangleLength(shapeComponent.Constraints[i].Param_Float_0)); break;
		case Dll_Interface::RectangleLength_Max		: slotDesc.AddConstraint(ShapeReco::SlotConstraints::MaxRectangleLength(shapeComponent.Constraints[i].Param_Float_0)); break;
		case Dll_Interface::RectangleLength_Between	: slotDesc.AddConstraint(ShapeReco::SlotConstraints::RectangleLengthBetween(
															shapeComponent.Constraints[i].Param_Float_0,
															shapeComponent.Constraints[i].Param_Float_1)); break;
		case Dll_Interface::RectangleLength_Is		: slotDesc.AddConstraint(ShapeReco::SlotConstraints::RectangleLengthIs(shapeComponent.Constraints[i].Param_Float_0)); break;

		case Dll_Interface::RectangleWidth_Min		: slotDesc.AddConstraint(ShapeReco::SlotConstraints::MinRectangleWidth(shapeComponent.Constraints[i].Param_Float_0)); break;
		case Dll_Interface::RectangleWidth_Max		: slotDesc.AddConstraint(ShapeReco::SlotConstraints::MaxRectangleWidth(shapeComponent.Constraints[i].Param_Float_0)); break;
		case Dll_Interface::RectangleWidth_Between	: slotDesc.AddConstraint(ShapeReco::SlotConstraints::RectangleWidthBetween(
															shapeComponent.Constraints[i].Param_Float_0,
															shapeComponent.Constraints[i].Param_Float_1)); break;
		case Dll_Interface::RectangleWidth_Is		: slotDesc.AddConstraint(ShapeReco::SlotConstraints::RectangleWidthIs(shapeComponent.Constraints[i].Param_Float_0)); break;

		case Dll_Interface::IsSquare				: slotDesc.AddConstraint(ShapeReco::SlotConstraints::IsSquare(shapeComponent.Constraints[i].Param_Float_0)); break;
		case Dll_Interface::SquareSize_Min			: slotDesc.AddConstraint(ShapeReco::SlotConstraints::MinSquareSize(shapeComponent.Constraints[i].Param_Float_0)); break;
		case Dll_Interface::SquareSize_Max			: slotDesc.AddConstraint(ShapeReco::SlotConstraints::MaxSquareSize(shapeComponent.Constraints[i].Param_Float_0)); break;
		case Dll_Interface::SquareSize_Between		: slotDesc.AddConstraint(ShapeReco::SlotConstraints::SquareSizeBetween(
															shapeComponent.Constraints[i].Param_Float_0, 
															shapeComponent.Constraints[i].Param_Float_1)); break;
		case Dll_Interface::SquareSize_Is			: slotDesc.AddConstraint(ShapeReco::SlotConstraints::SquareSizeIs(shapeComponent.Constraints[i].Param_Float_0)); break;

		case Dll_Interface::IsCircle				: slotDesc.AddConstraint(ShapeReco::SlotConstraints::IsCircle(shapeComponent.Constraints[i].Param_Float_0)); break;
		case Dll_Interface::CircleRadius_Min		: slotDesc.AddConstraint(ShapeReco::SlotConstraints::MinCircleRadius(shapeComponent.Constraints[i].Param_Float_0)); break;
		case Dll_Interface::CircleRadius_Max		: slotDesc.AddConstraint(ShapeReco::SlotConstraints::MaxCircleRadius(shapeComponent.Constraints[i].Param_Float_0)); break;
		case Dll_Interface::CircleRadius_Between	: slotDesc.AddConstraint(ShapeReco::SlotConstraints::CircleRadiusBetween(
															shapeComponent.Constraints[i].Param_Float_0, 
															shapeComponent.Constraints[i].Param_Float_1)); break;
		case Dll_Interface::CircleRadius_Is			: slotDesc.AddConstraint(ShapeReco::SlotConstraints::CircleRadiusIs(shapeComponent.Constraints[i].Param_Float_0)); break;
		}
	}
	return slotDesc;
}

void AddShapeConstraintsToShapeDescription(
	ShapeReco::ShapeDesc& shapeDesc, 
	int shapeConstraints,
	Dll_Interface::ShapeConstraint* constraints)
{
	for (int i = 0; i < shapeConstraints; ++i)
	{
		switch (constraints[i].Type)
		{
			case Dll_Interface::NoOtherSurface		: shapeDesc.AddConstraint(ShapeReco::ShapeConstraints::NoOtherSurface()); break;

			case Dll_Interface::AwayFromWalls		: shapeDesc.AddConstraint(ShapeReco::ShapeConstraints::AwayFromWalls()); break;

			case Dll_Interface::RectanglesParallel	: shapeDesc.AddConstraint(ShapeReco::ShapeConstraints::RectangleParallel(
															Name_Z(String_Z<8>(constraints[i].Param_Int_0)), 
															Name_Z(String_Z<8>(constraints[i].Param_Int_1)))); break;
			case Dll_Interface::RectanglesPerpendicular	: shapeDesc.AddConstraint(ShapeReco::ShapeConstraints::RectanglePerpendicular(
															Name_Z(String_Z<8>(constraints[i].Param_Int_0)), 
															Name_Z(String_Z<8>(constraints[i].Param_Int_1)))); break;
			case Dll_Interface::RectanglesAligned	: shapeDesc.AddConstraint(ShapeReco::ShapeConstraints::RectangleAligned(
															Name_Z(String_Z<8>(constraints[i].Param_Int_0)), 
															Name_Z(String_Z<8>(constraints[i].Param_Int_1)),
															constraints[i].Param_Float_0)); break;
			case Dll_Interface::RectanglesSameLength : shapeDesc.AddConstraint(ShapeReco::ShapeConstraints::SameRectangleLength(
															Name_Z(String_Z<8>(constraints[i].Param_Int_0)), 
															Name_Z(String_Z<8>(constraints[i].Param_Int_1)),
															constraints[i].Param_Float_0)); break;

			case Dll_Interface::AtFrontOf			: shapeDesc.AddConstraint(ShapeReco::ShapeConstraints::SlotAtFrontOf(
															Name_Z(String_Z<8>(constraints[i].Param_Int_0)), 
															Name_Z(String_Z<8>(constraints[i].Param_Int_1)))); break;
			case Dll_Interface::AtBackOf			: shapeDesc.AddConstraint(ShapeReco::ShapeConstraints::SlotAtBackOf(
															Name_Z(String_Z<8>(constraints[i].Param_Int_0)), 
															Name_Z(String_Z<8>(constraints[i].Param_Int_1)))); break;
			case Dll_Interface::AtLeftOf			: shapeDesc.AddConstraint(ShapeReco::ShapeConstraints::SlotAtLeftOf(
															Name_Z(String_Z<8>(constraints[i].Param_Int_0)), 
															Name_Z(String_Z<8>(constraints[i].Param_Int_1)))); break;
			case Dll_Interface::AtRightOf			: shapeDesc.AddConstraint(ShapeReco::ShapeConstraints::SlotAtRightOf(
															Name_Z(String_Z<8>(constraints[i].Param_Int_0)), 
															Name_Z(String_Z<8>(constraints[i].Param_Int_1)))); break;
		}
	}
}

EXTERN_C __declspec(dllexport) int AddShape(
	_In_ char* shapeName,
	_In_ int componentCount,
	_In_ Dll_Interface::ShapeComponent* components,
	_In_ int shapeConstraints,
	_In_ Dll_Interface::ShapeConstraint* constraints
)
{
	SetAndRestoreFloatControlDownward floatControlDownward;

	// Convert to the ShapeDesc structure
	ShapeReco::ShapeDesc shapeDesc;
	shapeDesc.SetName(shapeName);
	for (int i = 0; i < componentCount; ++i)
	{
		shapeDesc.AddSlot(ShapeComponent_to_SlotDesc(i, components[i]));
	}
	AddShapeConstraintsToShapeDescription(shapeDesc, shapeConstraints, constraints);

	// Add it
	ShapeReco::ShapeAnalyzer_W& shapeAnalyzer = UnderstandingMgr_W::GetUnderstandingMgr().GetPlayspaceInfos().m_ShapeAnalyzer;
	shapeAnalyzer.AddShapeDesc(shapeDesc);

	return 1;
}

EXTERN_C __declspec(dllexport) void ActivateShapeAnalysis()
{
	SetAndRestoreFloatControlDownward floatControlDownward;
	UnderstandingMgr_W::GetUnderstandingMgr().ActivateShapeAnalysis();
}
