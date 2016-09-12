// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#ifndef __SHAPE_ANALYZER_H__
#define __SHAPE_ANALYZER_H__

#include <Topology\TopologyAnalyzer_W.h>

#include <Topology\ShapeAnalyzerTools_W.h>
#include <Topology\ShapeAnalyzerCharacteristics_W.h>
#include <Topology\ShapeAnalyzerTypes_W.h>

namespace ShapeReco
{
//------------------------------------------------------------------------------------
// ShapeAnalyzer_W
//------------------------------------------------------------------------------------
// This class use a TopologyAnalyzer_W to identify some shape
class ShapeAnalyzer_W
{
public:
    // Public interface
    ShapeAnalyzer_W(const TopologyAnalyzer_W& _topology);
    void	Reset();

    const TopologyAnalyzer_W& GetTopology() const;

    void AddShapeDesc(const ShapeDesc& _shapeDesc);
    void GetShapes(const Name_Z& _shapeName, DynArray_Z<const Shape*>& _outShapes) const;

    void GetAllSurfacesOnShape(const Name_Z& _shapeName, const Name_Z& _slotName, DynArray_Z<const TopologyAnalyzer_W::Surface*>& _outSurfaces) const;
    void GetAllPosOnShape(const Name_Z& _shapeName, const Name_Z& _slotName, Float _fMinRadius, Vec3fDA& _outPos) const;
    Bool GetShapeFromZoneId(S32 _ZoneId,Name_Z& _shapeName, Name_Z& _slotName) const;

	Bool	IsAnalysed() const {return m_IsAnalysed;}
	void	Analyse();
    void	ResetAnalyse();

	void DrawDebug() const;

    // Helpers for constraints and characteristics
    void GetAverageDir(const Vec3fDA& _poss, Vec3f& _outDir) const;

    void GetRectFromLengthDir(const Vec3fDA& _poss, Vec3f& _lengthDir, Vec3f& _outCenter, Float& _outLength, Float& _outWidth) const;
    void GetRect(const Vec3fDA& _poss, Vec3f& _outCenter, Vec3f& _outLengthDir, Float& _outLength, Float& _outWidth) const;

    void GetCircle(const Vec3fDA& _poss, Vec3f& _outCenter, Float& _outRadius) const;
    
    // Characteristics computation
    template <class T, class O> const T& ComputeAndGet(O& _obj) const
    {
        if (!_obj.Have<T>())
            Compute(_obj, _obj.Create<T>());
        
        return _obj.Get<T>();
    }

    void Compute(const Slot& _slot, SlotCharacteristics::Area& _area) const;
    void Compute(const Slot& _slot, SlotCharacteristics::AllPos& _allPos) const;
    void Compute(const Slot& _slot, SlotCharacteristics::AvgPos& _cavgPos) const;
    void Compute(const Slot& _slot, SlotCharacteristics::Rectangle& _rectangle) const;
    void Compute(const Slot& _slot, SlotCharacteristics::Square& _square) const;
    void Compute(const Slot& _slot, SlotCharacteristics::Circle& _circle) const;

    void Compute(const Shape& _shape, ShapeCharacteristics::TotalNbSurfaces& _totalNbSurfaces) const;
    void Compute(const Shape& _shape, ShapeCharacteristics::AvgPos& _avgPos) const;

    // For PlayspaceInfo_W
	static void CreateDefaultShapeDesc(ShapeAnalyzer_W &_ShapeAnalyser);	// Callback for PlayspaceInfo_W.
	static Bool	ShapeDesc_IsItCouch(const Name_Z& _shapeName, const Name_Z& _slotName);

	void GetSittablePosOnCouch(Float _minHeight, Float _maxHeight, Float _depth, Float _widthMin, Vec3fDA& _outPos, Vec3fDA& _outNormal) const;

	// For Conquer
	struct PosOnSurfaceParams_W
	{
		Float m_fRadius;

		Bool m_bIsSolved;
		Vec3f m_vPos;
	};

	void GetPosOnSurface(DynArray_Z<PosOnSurfaceParams_W>& _daParams, Float _fHeightMin, Float _fHeightMax, const Vec3f& _vFirstPointToBeNear, Float fMinDistance) const;
	Bool GetPosOnSurface(Float _fRadius, Float _fHeightMin, Float _fHeightMax, const Vec3f& _vPointToBeNear, Vec3f& _outPos) const;

	Bool SurfaceIsInShape(const Name_Z& _shapeName, const TopologyAnalyzer_W::Surface& _surface) const;

private:
    // Helper for analyse
    Bool TryMatchSlotConstraints(const SlotDesc& _slotDesc, Slot& _slot, Shape& _shape) const;
    Bool TryMatchShapeConstraints(const ShapeDesc& _shapeDesc, Shape& _shape) const;
    Bool TryToMacthGroupWithDesc(const ShapeDesc& _shapeDesc, const DynArray_Z<const TopologyAnalyzer_W::Surface*>& _group, Shape& _potentialShape, S32 _nbSurfacesToPlace);
    Bool TryAndAddIfOk(const ShapeDesc& _shapeDesc, const DynArray_Z<const TopologyAnalyzer_W::Surface*>& _originalGroup, DynArray_Z<const TopologyAnalyzer_W::Surface*>& _group);

    // Attributes
    const TopologyAnalyzer_W& m_topology;

    DynArray_Z<ShapeDesc> m_shapeDescs;
    DynArray_Z<Shape> m_shapes;

	Bool		m_IsAnalysed;
};
} // namespace ShapeReco

#include <Topology\ShapeAnalyzerConstraints_W.h>

#endif