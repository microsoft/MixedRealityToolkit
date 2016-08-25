// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#ifndef _DLL_SHAPES
#define _DLL_SHAPES

namespace Dll_Interface
{
#pragma pack(push, 1)
	// Shape queries
	struct ShapeResult
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 halfDims;
	};

	// Shape definition
	enum ShapeComponentConstraintType
	{
		SurfaceNotPartOfShape,

		SurfaceHeight_Min,
		SurfaceHeight_Max,
		SurfaceHeight_Between,
		SurfaceHeight_Is,

		SurfaceCount_Min,
		SurfaceCount_Max,
		SurfaceCount_Between,
		SurfaceCount_Is,

		SurfaceArea_Min,
		SurfaceArea_Max,
		SurfaceArea_Between,
		SurfaceArea_Is,

		IsRectangle,
		RectangleSize_Min,
		RectangleSize_Max,
		RectangleSize_Between,
		RectangleSize_Is,

		RectangleLength_Min,
		RectangleLength_Max,
		RectangleLength_Between,
		RectangleLength_Is,

		RectangleWidth_Min,
		RectangleWidth_Max,
		RectangleWidth_Between,
		RectangleWidth_Is,

		IsSquare,
		SquareSize_Min,
		SquareSize_Max,
		SquareSize_Between,
		SquareSize_Is,

		IsCircle,
		CircleRadius_Min,
		CircleRadius_Max,
		CircleRadius_Between,
		CircleRadius_Is,
	};
	struct ShapeComponentConstraint
	{
		ShapeComponentConstraintType Type;
		float Param_Float_0;
		float Param_Float_1;
		float Param_Float_2;
		float Param_Float_3;
		int Param_Int_0;
		int Param_Int_1;
		char* Param_Str_0;
	};
	struct ShapeComponent
	{
		int ConstraintCount;
		ShapeComponentConstraint* Constraints;
	};
	enum ShapeConstraintType
	{
		NoOtherSurface,
		AwayFromWalls,

		RectanglesParallel,
		RectanglesPerpendicular,
		RectanglesAligned,
		RectanglesSameLength,

		AtFrontOf,
		AtBackOf,
		AtLeftOf,
		AtRightOf,
	};
	struct ShapeConstraint
	{
		ShapeConstraintType Type;
		float Param_Float_0;
		int Param_Int_0;
		int Param_Int_1;
	};
#pragma pack(pop)

	// Shape queries
	EXTERN_C __declspec(dllexport) int QueryShape_FindPositionsOnShape(
		_In_ char* shapeName,
		_In_ float minRadius,
		_In_ int shapeCount,
		_Inout_ ShapeResult* shapeData);
	EXTERN_C __declspec(dllexport) int QueryShape_FindShapeHalfDims(
		_In_ char* shapeName,
		_In_ int shapeCount,
		_Inout_ ShapeResult* shapeData);

	// Shape definition
	EXTERN_C __declspec(dllexport) int AddShape(
		_In_ char* shapeName,
		_In_ int componentCount,
		_In_ ShapeComponent* components,
		_In_ int shapeConstraints,
		_In_ ShapeConstraint* constraints);
	EXTERN_C __declspec(dllexport) void RemoveAllShapes();
}

#endif
