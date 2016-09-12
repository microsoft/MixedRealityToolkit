// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#ifndef __SHAPE_ANALYZER_CONSTRAINTS_H__
#define __SHAPE_ANALYZER_CONSTRAINTS_H__

namespace ShapeReco
{
namespace SlotConstraints
{
//------------------------------------------------------------------------------------
// OrConstraint
//------------------------------------------------------------------------------------
//
class OrConstraint : public SlotConstraintImpl
{
public:
    OrConstraint(const SlotConstraint& _constraint1, const SlotConstraint& _constraint2) :
        m_constraint1(_constraint1),
        m_constraint2(_constraint2)
    {
    }

    virtual SlotConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z OrConstraint(m_constraint1, m_constraint2);
    }

    virtual Bool TryMatchWith(Slot& _slot, Shape& _shape, const ShapeAnalyzer_W& _shapeAnalyzer) const OVERRIDE_Z
    {
        return m_constraint1.TryMatchWith(_slot, _shape, _shapeAnalyzer) || m_constraint2.TryMatchWith(_slot, _shape, _shapeAnalyzer);
    }


private :
    SlotConstraint m_constraint1;
    SlotConstraint m_constraint2;
};

//------------------------------------------------------------------------------------
// AndConstraint
//------------------------------------------------------------------------------------
//
class AndConstraint : public SlotConstraintImpl
{
public:
    AndConstraint(const SlotConstraint& _constraint1, const SlotConstraint& _constraint2) :
        m_constraint1(_constraint1),
        m_constraint2(_constraint2)
    {
    }

    virtual SlotConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z AndConstraint(m_constraint1, m_constraint2);
    }

    virtual Bool TryMatchWith(Slot& _slot, Shape& _shape, const ShapeAnalyzer_W& _shapeAnalyzer) const OVERRIDE_Z
    {
        return m_constraint1.TryMatchWith(_slot, _shape, _shapeAnalyzer) && m_constraint2.TryMatchWith(_slot, _shape, _shapeAnalyzer);
    }


private :
    SlotConstraint m_constraint1;
    SlotConstraint m_constraint2;
};

//------------------------------------------------------------------------------------
// MinSurfaceHeight
//------------------------------------------------------------------------------------
//
class MinSurfaceHeight : public SlotConstraintImpl
{
public:
    MinSurfaceHeight(Float _minHeight) :
        m_minHeight(_minHeight)
    {
    }

    virtual SlotConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z MinSurfaceHeight(m_minHeight);
    }

    virtual Bool TryMatchWith(Slot& _slot, Shape& /*_shape*/, const ShapeAnalyzer_W& /*_shapeAnalyzer*/) const OVERRIDE_Z
    {
        for (S32 i = 0; i < _slot.m_surfaces.GetSize(); ++i)
        {
            if (_slot.m_surfaces[i]->m_fHeightFromGround < m_minHeight)
                return FALSE;
        }

        return TRUE;
    }

private:
    Float m_minHeight;
};

//------------------------------------------------------------------------------------
// MaxSurfaceHeight
//------------------------------------------------------------------------------------
//
class MaxSurfaceHeight : public SlotConstraintImpl
{
public:
    MaxSurfaceHeight(Float _maxHeight) :
        m_maxHeight(_maxHeight)
    {
    }

    virtual SlotConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z MaxSurfaceHeight(m_maxHeight);
    }

    virtual Bool TryMatchWith(Slot& _slot, Shape& /*_shape*/, const ShapeAnalyzer_W& /*_shapeAnalyzer*/) const OVERRIDE_Z
    {
        for (S32 i = 0; i < _slot.m_surfaces.GetSize(); ++i)
        {
            if (_slot.m_surfaces[i]->m_fHeightFromGround > m_maxHeight)
                return FALSE;
        }

        return TRUE;
    }

private:
    Float m_maxHeight;
};

//------------------------------------------------------------------------------------
// SurfaceHeightBetween
//------------------------------------------------------------------------------------
//
class SurfaceHeightBetween : public SlotConstraintImpl
{
public:
    SurfaceHeightBetween(Float _minHeight, Float _maxHeight) :
        m_minHeight(_minHeight),
        m_maxHeight(_maxHeight)
    {
    }

    virtual SlotConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z SurfaceHeightBetween(m_minHeight, m_maxHeight);
    }

    virtual Bool TryMatchWith(Slot& _slot, Shape& /*_shape*/, const ShapeAnalyzer_W& /*_shapeAnalyzer*/) const OVERRIDE_Z
    {
        for (S32 i = 0; i < _slot.m_surfaces.GetSize(); ++i)
        {
            if (_slot.m_surfaces[i]->m_fHeightFromGround < m_minHeight || _slot.m_surfaces[i]->m_fHeightFromGround > m_maxHeight)
                return FALSE;
        }

        return TRUE;
    }

private:
    Float m_minHeight;
    Float m_maxHeight;
};

//------------------------------------------------------------------------------------
// SurfaceHeightIs
//------------------------------------------------------------------------------------
//
class SurfaceHeightIs : public SlotConstraintImpl
{
public:
    SurfaceHeightIs(Float _height) :
        m_height(_height)
    {
    }

    virtual SlotConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z SurfaceHeightIs(m_height);
    }

    virtual Bool TryMatchWith(Slot& _slot, Shape& /*_shape*/, const ShapeAnalyzer_W& /*_shapeAnalyzer*/) const OVERRIDE_Z
    {
        for (S32 i = 0; i < _slot.m_surfaces.GetSize(); ++i)
        {
            if (_slot.m_surfaces[i]->m_fHeightFromGround != m_height)
                return FALSE;
        }

        return TRUE;
    }

private:
    Float m_height;
};

//------------------------------------------------------------------------------------
// MinNbSurfaces
//------------------------------------------------------------------------------------
//
class MinNbSurfaces : public SlotConstraintImpl
{
public:
    MinNbSurfaces(S32 _minNbSurfaces) :
        m_minNbSurfaces(_minNbSurfaces)
    {
    }

    virtual SlotConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z MinNbSurfaces(m_minNbSurfaces);
    }

    virtual Bool TryMatchWith(Slot& _slot, Shape& /*_shape*/, const ShapeAnalyzer_W& /*_shapeAnalyzer*/) const OVERRIDE_Z
    {
        return _slot.m_surfaces.GetSize() >= m_minNbSurfaces;
    }

private:
    S32 m_minNbSurfaces;
};

//------------------------------------------------------------------------------------
// MaxNbSurfaces
//------------------------------------------------------------------------------------
//
class MaxNbSurfaces : public SlotConstraintImpl
{
public:
    MaxNbSurfaces(S32 _maxNbSurfaces) :
        m_maxNbSurfaces(_maxNbSurfaces)
    {
    }

    virtual SlotConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z MaxNbSurfaces(m_maxNbSurfaces);
    }

    virtual Bool TryMatchWith(Slot& _slot, Shape& /*_shape*/, const ShapeAnalyzer_W& /*_shapeAnalyzer*/) const OVERRIDE_Z
    {
        return _slot.m_surfaces.GetSize() <= m_maxNbSurfaces;
    }

private:
    S32 m_maxNbSurfaces;
};

//------------------------------------------------------------------------------------
// NbSurfacesBetween
//------------------------------------------------------------------------------------
//
class NbSurfacesBetween : public SlotConstraintImpl
{
public:
    NbSurfacesBetween(S32 _minNbSurfaces, S32 _maxNbSurfaces) :
        m_minNbSurfaces(_minNbSurfaces),
        m_maxNbSurfaces(_maxNbSurfaces)
    {
    }

    virtual SlotConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z NbSurfacesBetween(m_minNbSurfaces, m_maxNbSurfaces);
    }

    virtual Bool TryMatchWith(Slot& _slot, Shape& /*_shape*/, const ShapeAnalyzer_W& /*_shapeAnalyzer*/) const OVERRIDE_Z
    {
        return (_slot.m_surfaces.GetSize() >= m_minNbSurfaces && _slot.m_surfaces.GetSize() <= m_maxNbSurfaces);
    }

private:
    S32 m_minNbSurfaces;
    S32 m_maxNbSurfaces;
};

//------------------------------------------------------------------------------------
// NbSurfacesIs
//------------------------------------------------------------------------------------
//
class NbSurfacesIs : public SlotConstraintImpl
{
public:
    NbSurfacesIs(S32 _nNbSurfaces) :
        m_nbSurfaces(_nNbSurfaces)
    {
    }

    virtual SlotConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z NbSurfacesIs(m_nbSurfaces);
    }

    virtual Bool TryMatchWith(Slot& _slot, Shape& /*_shape*/, const ShapeAnalyzer_W& /*_shapeAnalyzer*/) const OVERRIDE_Z
    {
        return _slot.m_surfaces.GetSize() == m_nbSurfaces;
    }

private:
    S32 m_nbSurfaces;
};

//------------------------------------------------------------------------------------
// SurfaceIsNotPartOf
//------------------------------------------------------------------------------------
//
class SurfacesAreNotPartOf : public SlotConstraintImpl
{
public:
    SurfacesAreNotPartOf(const Name_Z& _shapeName, const Name_Z& _slotName = NAME_NULL) :
        m_shapeName(_shapeName),
        m_slotName(_slotName)
    {
    }

    virtual SlotConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z SurfacesAreNotPartOf(m_shapeName, m_slotName);
    }

    virtual Bool TryMatchWith(Slot& _slot, Shape& /*_shape*/, const ShapeAnalyzer_W& _shapeAnalyzer) const OVERRIDE_Z
    {
        DynArray_Z<const TopologyAnalyzer_W::Surface*> badSurfaces;
        _shapeAnalyzer.GetAllSurfacesOnShape(m_shapeName, m_slotName, badSurfaces);

        for (S32 i = 0; i < _slot.m_surfaces.GetSize(); ++i)
        {
            if (badSurfaces.Contains(_slot.m_surfaces[i]) >= 0)
                return FALSE;
        }

        return TRUE;
    }

private:
    Name_Z m_shapeName;
    Name_Z m_slotName;
};

//------------------------------------------------------------------------------------
// MinArea
//------------------------------------------------------------------------------------
//
class MinArea : public SlotConstraintImpl
{
public:
    MinArea(Float _minArea) :
        m_minArea(_minArea)
    {
    }

    virtual SlotConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z MinArea(m_minArea);
    }

    virtual Bool TryMatchWith(Slot& _slot, Shape& /*_shape*/, const ShapeAnalyzer_W& _shapeAnalyzer) const OVERRIDE_Z
    {
        Float area = _shapeAnalyzer.ComputeAndGet<SlotCharacteristics::Area>(_slot).m_value;
        return area >= m_minArea;
    }

private:
    Float m_minArea;
};

//------------------------------------------------------------------------------------
// MaxArea
//------------------------------------------------------------------------------------
//
class MaxArea : public SlotConstraintImpl
{
public:
    MaxArea(Float _maxArea) :
        m_maxArea(_maxArea)
    {
    }

    virtual SlotConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z MaxArea(m_maxArea);
    }

    virtual Bool TryMatchWith(Slot& _slot, Shape& /*_shape*/, const ShapeAnalyzer_W& _shapeAnalyzer) const OVERRIDE_Z
    {
        Float area = _shapeAnalyzer.ComputeAndGet<SlotCharacteristics::Area>(_slot).m_value;
        return area <= m_maxArea;
    }

private:
    Float m_maxArea;
};

//------------------------------------------------------------------------------------
// AreaBetween
//------------------------------------------------------------------------------------
//
class AreaBetween : public SlotConstraintImpl
{
public:
    AreaBetween(Float _minArea, Float _maxArea) : 
        m_minArea(_minArea),
        m_maxArea(_maxArea)
    {
    }

    virtual SlotConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z AreaBetween(m_minArea, m_maxArea);
    }

    virtual Bool TryMatchWith(Slot& _slot, Shape& /*_shape*/, const ShapeAnalyzer_W& _shapeAnalyzer) const OVERRIDE_Z
    {
        Float area = _shapeAnalyzer.ComputeAndGet<SlotCharacteristics::Area>(_slot).m_value;
        return (area >= m_minArea && area <= m_maxArea);
    }

private:
    Float m_minArea;
    Float m_maxArea;
};

//------------------------------------------------------------------------------------
// AreaIs
//------------------------------------------------------------------------------------
//
class AreaIs : public SlotConstraintImpl
{
public:
    AreaIs(Float _area) : 
        m_area(_area)
    {
    }

    virtual SlotConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z AreaIs(m_area);
    }

    virtual Bool TryMatchWith(Slot& _slot, Shape& /*_shape*/, const ShapeAnalyzer_W& _shapeAnalyzer) const OVERRIDE_Z
    {
        Float area = _shapeAnalyzer.ComputeAndGet<SlotCharacteristics::Area>(_slot).m_value;
        return area == m_area;
    }

private:
    Float m_area;
};

//------------------------------------------------------------------------------------
// IsRectangle
//------------------------------------------------------------------------------------
//
class IsRectangle : public SlotConstraintImpl
{
public:
    IsRectangle(Float _similarityMin = 0.50f) :
        m_similarityMin(_similarityMin)
    {
    }

    virtual SlotConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z IsRectangle(m_similarityMin);
    }

    virtual Bool TryMatchWith(Slot& _slot, Shape& /*_shape*/, const ShapeAnalyzer_W& _shapeAnalyzer) const OVERRIDE_Z
    {   
        const SlotCharacteristics::Rectangle& rect = _shapeAnalyzer.ComputeAndGet<SlotCharacteristics::Rectangle>(_slot);
        return rect.m_similarity >= m_similarityMin;
    }

private:
    Float m_similarityMin;
};

//------------------------------------------------------------------------------------
// MinRectangleSize
//------------------------------------------------------------------------------------
//
class MinRectangleSize : public SlotConstraintImpl
{
public:
    MinRectangleSize(Float _minLength, Float _minWidth) :
        m_minLength(_minLength),
        m_minWidth(_minWidth)
    {
    }

    virtual SlotConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z MinRectangleSize(m_minLength, m_minWidth);
    }

    virtual Bool TryMatchWith(Slot& _slot, Shape& /*_shape*/, const ShapeAnalyzer_W& _shapeAnalyzer) const OVERRIDE_Z
    {
        const SlotCharacteristics::Rectangle& rect = _shapeAnalyzer.ComputeAndGet<SlotCharacteristics::Rectangle>(_slot);
        return (rect.m_length >= m_minLength && rect.m_width >= m_minWidth);
    }

private :
    Float m_minLength;
    Float m_minWidth;
};

//------------------------------------------------------------------------------------
// MaxRectangleSize
//------------------------------------------------------------------------------------
//
class MaxRectangleSize : public SlotConstraintImpl
{
public:
    MaxRectangleSize(Float _maxLength, Float _maxWidth) :
        m_maxLength(_maxLength),
        m_maxWidth(_maxWidth)
    {
    }

    virtual SlotConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z MaxRectangleSize(m_maxLength, m_maxWidth);
    }

    virtual Bool TryMatchWith(Slot& _slot, Shape& /*_shape*/, const ShapeAnalyzer_W& _shapeAnalyzer) const OVERRIDE_Z
    {
        const SlotCharacteristics::Rectangle& rect = _shapeAnalyzer.ComputeAndGet<SlotCharacteristics::Rectangle>(_slot);
        return (rect.m_length <= m_maxLength && rect.m_width <= m_maxWidth);
    }

private :
    Float m_maxLength;
    Float m_maxWidth;
};

//------------------------------------------------------------------------------------
// RectangleSizeBetween
//------------------------------------------------------------------------------------
//
class RectangleSizeBetween : public SlotConstraintImpl
{
public:
    RectangleSizeBetween(Float _minLength, Float _minWidth, Float _maxLength, Float _maxWidth) :
        m_minLength(_minLength),
        m_minWidth(_minWidth),
        m_maxLength(_maxLength),
        m_maxWidth(_maxWidth)
    {
    }

    virtual SlotConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z RectangleSizeBetween(m_minLength, m_minWidth, m_maxLength, m_maxWidth);
    }

    virtual Bool TryMatchWith(Slot& _slot, Shape& /*_shape*/, const ShapeAnalyzer_W& _shapeAnalyzer) const OVERRIDE_Z
    {
        const SlotCharacteristics::Rectangle& rect = _shapeAnalyzer.ComputeAndGet<SlotCharacteristics::Rectangle>(_slot);
        return (rect.m_length >= m_minLength && rect.m_width >= m_minWidth && rect.m_length <= m_maxLength && rect.m_width <= m_maxWidth);
    }

private :
    Float m_minLength;
    Float m_minWidth;
    Float m_maxLength;
    Float m_maxWidth;
};

//------------------------------------------------------------------------------------
// RectangleSizeIs
//------------------------------------------------------------------------------------
//
class RectangleSizeIs : public SlotConstraintImpl
{
public:
    RectangleSizeIs(Float _length, Float _width) :
        m_length(_length),
        m_width(_width)
    {
    }

    virtual SlotConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z RectangleSizeIs(m_length, m_width);
    }

    virtual Bool TryMatchWith(Slot& _slot, Shape& /*_shape*/, const ShapeAnalyzer_W& _shapeAnalyzer) const OVERRIDE_Z
    {
        const SlotCharacteristics::Rectangle& rect = _shapeAnalyzer.ComputeAndGet<SlotCharacteristics::Rectangle>(_slot);
        return (rect.m_length == m_length && rect.m_width == m_width);
    }

private :
    Float m_length;
    Float m_width;
};

//------------------------------------------------------------------------------------
// MinRectangleLength
//------------------------------------------------------------------------------------
//
class MinRectangleLength : public SlotConstraintImpl
{
public:
    MinRectangleLength(Float _minLength) :
        m_minLength(_minLength)
    {
    }

    virtual SlotConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z MinRectangleLength(m_minLength);
    }

    virtual Bool TryMatchWith(Slot& _slot, Shape& /*_shape*/, const ShapeAnalyzer_W& _shapeAnalyzer) const OVERRIDE_Z
    {
        const SlotCharacteristics::Rectangle& rect = _shapeAnalyzer.ComputeAndGet<SlotCharacteristics::Rectangle>(_slot);
        return rect.m_length >= m_minLength;
    }

private :
    Float m_minLength;
};

//------------------------------------------------------------------------------------
// MaxRectangleLength
//------------------------------------------------------------------------------------
//
class MaxRectangleLength : public SlotConstraintImpl
{
public:
    MaxRectangleLength(Float _maxLength) : m_maxLength(_maxLength) {}

    virtual SlotConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z MaxRectangleLength(m_maxLength);
    }

    virtual Bool TryMatchWith(Slot& _slot, Shape& /*_shape*/, const ShapeAnalyzer_W& _shapeAnalyzer) const OVERRIDE_Z
    {
        const SlotCharacteristics::Rectangle& rect = _shapeAnalyzer.ComputeAndGet<SlotCharacteristics::Rectangle>(_slot);
        return rect.m_length <= m_maxLength;
    }

private :
    Float m_maxLength;
};

//------------------------------------------------------------------------------------
// RectangleLengthBetween
//------------------------------------------------------------------------------------
//
class RectangleLengthBetween : public SlotConstraintImpl
{
public:
    RectangleLengthBetween(Float _minLength, Float _maxLength) :
        m_minLength(_minLength),
        m_maxLength(_maxLength)
    {
    }

    virtual SlotConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z RectangleLengthBetween(m_minLength, m_maxLength);
    }

    virtual Bool TryMatchWith(Slot& _slot, Shape& /*_shape*/, const ShapeAnalyzer_W& _shapeAnalyzer) const OVERRIDE_Z
    {
        const SlotCharacteristics::Rectangle& rect = _shapeAnalyzer.ComputeAndGet<SlotCharacteristics::Rectangle>(_slot);
        return (rect.m_length >= m_minLength && rect.m_length <= m_maxLength);
    }

private :
    Float m_minLength;
    Float m_maxLength;
};

//------------------------------------------------------------------------------------
// RectangleLengthIs
//------------------------------------------------------------------------------------
//
class RectangleLengthIs : public SlotConstraintImpl
{
public:
    RectangleLengthIs(Float _length) :
        m_length(_length)
    {
    }

    virtual SlotConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z RectangleLengthIs(m_length);
    }

    virtual Bool TryMatchWith(Slot& _slot, Shape& /*_shape*/, const ShapeAnalyzer_W& _shapeAnalyzer) const OVERRIDE_Z
    {
        const SlotCharacteristics::Rectangle& rect = _shapeAnalyzer.ComputeAndGet<SlotCharacteristics::Rectangle>(_slot);
        return rect.m_length == m_length;
    }

private :
    Float m_length;
};

//------------------------------------------------------------------------------------
// MinRectangleWidth
//------------------------------------------------------------------------------------
//
class MinRectangleWidth : public SlotConstraintImpl
{
public:
    MinRectangleWidth(Float _minWidth) :
        m_minWidth(_minWidth)
    {
    }

    virtual SlotConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z MinRectangleWidth(m_minWidth);
    }

    virtual Bool TryMatchWith(Slot& _slot, Shape& /*_shape*/, const ShapeAnalyzer_W& _shapeAnalyzer) const OVERRIDE_Z
    {
        const SlotCharacteristics::Rectangle& rect = _shapeAnalyzer.ComputeAndGet<SlotCharacteristics::Rectangle>(_slot);
        return rect.m_width >= m_minWidth;
    }

private :
    Float m_minWidth;
};

//------------------------------------------------------------------------------------
// MaxRectangleWidth
//------------------------------------------------------------------------------------
//
class MaxRectangleWidth : public SlotConstraintImpl
{
public:
    MaxRectangleWidth(Float _maxWidth) :
        m_maxWidth(_maxWidth)
    {
    }

    virtual SlotConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z MaxRectangleWidth(m_maxWidth);
    }

    virtual Bool TryMatchWith(Slot& _slot, Shape& /*_shape*/, const ShapeAnalyzer_W& _shapeAnalyzer) const OVERRIDE_Z
    {
        const SlotCharacteristics::Rectangle& rect = _shapeAnalyzer.ComputeAndGet<SlotCharacteristics::Rectangle>(_slot);
        return rect.m_width <= m_maxWidth;
    }

private :
    Float m_maxWidth;
};

//------------------------------------------------------------------------------------
// RectangleWidthBetween
//------------------------------------------------------------------------------------
//
class RectangleWidthBetween : public SlotConstraintImpl
{
public:
    RectangleWidthBetween(Float _minWidth, Float _maxWidth) :
        m_minWidth(_minWidth),
        m_maxWidth(_maxWidth)
    {
    }

    virtual SlotConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z RectangleWidthBetween(m_minWidth, m_maxWidth);
    }

    virtual Bool TryMatchWith(Slot& _slot, Shape& /*_shape*/, const ShapeAnalyzer_W& _shapeAnalyzer) const OVERRIDE_Z
    {
        const SlotCharacteristics::Rectangle& rect = _shapeAnalyzer.ComputeAndGet<SlotCharacteristics::Rectangle>(_slot);
        return (rect.m_width >= m_minWidth && rect.m_width <= m_maxWidth);
    }

private :
    Float m_minWidth;
    Float m_maxWidth;
};

//------------------------------------------------------------------------------------
// RectangleWidthIs
//------------------------------------------------------------------------------------
//
class RectangleWidthIs : public SlotConstraintImpl
{
public:
    RectangleWidthIs(Float _width) :
        m_width(_width)
    {
    }

    virtual SlotConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z RectangleWidthIs(m_width);
    }

    virtual Bool TryMatchWith(Slot& _slot, Shape& /*_shape*/, const ShapeAnalyzer_W& _shapeAnalyzer) const OVERRIDE_Z
    {
        const SlotCharacteristics::Rectangle& rect = _shapeAnalyzer.ComputeAndGet<SlotCharacteristics::Rectangle>(_slot);
        return rect.m_width == m_width;
    }

private :
    Float m_width;
};

//------------------------------------------------------------------------------------
// IsSquare
//------------------------------------------------------------------------------------
//
class IsSquare : public SlotConstraintImpl
{
public:
    IsSquare(Float _similarityMin = 0.50f) :
        m_similarityMin(_similarityMin)
    {
    }

    virtual SlotConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z IsSquare(m_similarityMin);
    }

    virtual Bool TryMatchWith(Slot& _slot, Shape& /*_shape*/, const ShapeAnalyzer_W& _shapeAnalyzer) const OVERRIDE_Z
    {   
        const SlotCharacteristics::Square& square = _shapeAnalyzer.ComputeAndGet<SlotCharacteristics::Square>(_slot);
        return square.m_similarity >= m_similarityMin;
    }

private:
    Float m_similarityMin;
};

//------------------------------------------------------------------------------------
// MinSquareSize
//------------------------------------------------------------------------------------
//
class MinSquareSize : public SlotConstraintImpl
{
public:
    MinSquareSize(Float _minSize) :
        m_minSize(_minSize)
    {
    }

    virtual SlotConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z MinSquareSize(m_minSize);
    }

    virtual Bool TryMatchWith(Slot& _slot, Shape& /*_shape*/, const ShapeAnalyzer_W& _shapeAnalyzer) const OVERRIDE_Z
    {
        const SlotCharacteristics::Square& square = _shapeAnalyzer.ComputeAndGet<SlotCharacteristics::Square>(_slot);
        return square.m_size >= m_minSize;
    }

private :
    Float m_minSize;
};

//------------------------------------------------------------------------------------
// MaxSquareSize
//------------------------------------------------------------------------------------
//
class MaxSquareSize : public SlotConstraintImpl
{
public:
    MaxSquareSize(Float _maxSize) :
        m_maxSize(_maxSize)
    {
    }

    virtual SlotConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z MaxSquareSize(m_maxSize);
    }

    virtual Bool TryMatchWith(Slot& _slot, Shape& /*_shape*/, const ShapeAnalyzer_W& _shapeAnalyzer) const OVERRIDE_Z
    {
        const SlotCharacteristics::Square& square = _shapeAnalyzer.ComputeAndGet<SlotCharacteristics::Square>(_slot);
        return square.m_size <= m_maxSize;
    }

private :
    Float m_maxSize;
};

//------------------------------------------------------------------------------------
// SquareSizeBetween
//------------------------------------------------------------------------------------
//
class SquareSizeBetween : public SlotConstraintImpl
{
public:
    SquareSizeBetween(Float _minSize, Float _maxSize) :
        m_minSize(_minSize),
        m_maxSize(_maxSize)
    {
    }

    virtual SlotConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z SquareSizeBetween(m_minSize, m_maxSize);
    }

    virtual Bool TryMatchWith(Slot& _slot, Shape& /*_shape*/, const ShapeAnalyzer_W& _shapeAnalyzer) const OVERRIDE_Z
    {
        const SlotCharacteristics::Square& square = _shapeAnalyzer.ComputeAndGet<SlotCharacteristics::Square>(_slot);
        return (square.m_size >= m_minSize && square.m_size <= m_maxSize);
    }

private :
    Float m_minSize;
    Float m_maxSize;
};

//------------------------------------------------------------------------------------
// SquareSizeIs
//------------------------------------------------------------------------------------
//
class SquareSizeIs : public SlotConstraintImpl
{
public:
    SquareSizeIs(Float _size) :
        m_size(_size)
    {
    }

    virtual SlotConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z SquareSizeIs(m_size);
    }

    virtual Bool TryMatchWith(Slot& _slot, Shape& /*_shape*/, const ShapeAnalyzer_W& _shapeAnalyzer) const OVERRIDE_Z
    {
        const SlotCharacteristics::Square& square = _shapeAnalyzer.ComputeAndGet<SlotCharacteristics::Square>(_slot);
        return square.m_size == m_size;
    }

private :
    Float m_size;
};

//------------------------------------------------------------------------------------
// IsCircle
//------------------------------------------------------------------------------------
//
class IsCircle : public SlotConstraintImpl
{
public:
    IsCircle(Float _similarityMin = 0.50f) :
        m_similarityMin(_similarityMin)
    {
    }

    virtual SlotConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z IsCircle(m_similarityMin);
    }

    virtual Bool TryMatchWith(Slot& _slot, Shape& /*_shape*/, const ShapeAnalyzer_W& _shapeAnalyzer) const OVERRIDE_Z
    {   
        const SlotCharacteristics::Circle& circle = _shapeAnalyzer.ComputeAndGet<SlotCharacteristics::Circle>(_slot);
        return circle.m_similarity >= m_similarityMin;
    }

private:
    Float m_similarityMin;
};

//------------------------------------------------------------------------------------
// MinCircleRadius
//------------------------------------------------------------------------------------
//
class MinCircleRadius : public SlotConstraintImpl
{
public:
    MinCircleRadius(Float _minRadius) :
        m_minRadius(_minRadius)
    {
    }

    virtual SlotConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z MinCircleRadius(m_minRadius);
    }

    virtual Bool TryMatchWith(Slot& _slot, Shape& /*_shape*/, const ShapeAnalyzer_W& _shapeAnalyzer) const OVERRIDE_Z
    {
       const SlotCharacteristics::Circle& circle = _shapeAnalyzer.ComputeAndGet<SlotCharacteristics::Circle>(_slot);
        return circle.m_radius >= m_minRadius;
    }

private :
    Float m_minRadius;
};

//------------------------------------------------------------------------------------
// MaxCircleRadius
//------------------------------------------------------------------------------------
//
class MaxCircleRadius : public SlotConstraintImpl
{
public:
    MaxCircleRadius(Float _maxRadius) :
        m_maxRadius(_maxRadius)
    {
    }

    virtual SlotConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z MaxCircleRadius(m_maxRadius);
    }

    virtual Bool TryMatchWith(Slot& _slot, Shape& /*_shape*/, const ShapeAnalyzer_W& _shapeAnalyzer) const OVERRIDE_Z
    {
        const SlotCharacteristics::Circle& circle = _shapeAnalyzer.ComputeAndGet<SlotCharacteristics::Circle>(_slot);
        return circle.m_radius <= m_maxRadius;
    }

private :
    Float m_maxRadius;
};

//------------------------------------------------------------------------------------
// CircleRadiusBetween
//------------------------------------------------------------------------------------
//
class CircleRadiusBetween : public SlotConstraintImpl
{
public:
    CircleRadiusBetween(Float _minRadius, Float _maxRadius) :
        m_minRadius(_minRadius),
        m_maxRadius(_maxRadius)
    {
    }

    virtual SlotConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z CircleRadiusBetween(m_minRadius, m_maxRadius);
    }

    virtual Bool TryMatchWith(Slot& _slot, Shape& /*_shape*/, const ShapeAnalyzer_W& _shapeAnalyzer) const OVERRIDE_Z
    {
        const SlotCharacteristics::Circle& circle = _shapeAnalyzer.ComputeAndGet<SlotCharacteristics::Circle>(_slot);
        return (circle.m_radius >= m_minRadius && circle.m_radius <= m_maxRadius);
    }

private :
    Float m_minRadius;
    Float m_maxRadius;
};

//------------------------------------------------------------------------------------
// CircleRadiusIs
//------------------------------------------------------------------------------------
//
class CircleRadiusIs : public SlotConstraintImpl
{
public:
    CircleRadiusIs(Float _radius) :
        m_radius(_radius)
    {
    }

    virtual SlotConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z CircleRadiusIs(m_radius);
    }

    virtual Bool TryMatchWith(Slot& _slot, Shape& /*_shape*/, const ShapeAnalyzer_W& _shapeAnalyzer) const OVERRIDE_Z
    {
        const SlotCharacteristics::Circle& circle = _shapeAnalyzer.ComputeAndGet<SlotCharacteristics::Circle>(_slot);
        return circle.m_radius == m_radius;
    }

private :
    Float m_radius;
};
} // namespace SlotConstraints

namespace ShapeConstraints
{
//------------------------------------------------------------------------------------
// OrConstraint
//------------------------------------------------------------------------------------
//
class OrConstraint : public ShapeConstraintImpl
{
public:
    OrConstraint(const ShapeConstraint& _constraint1, const ShapeConstraint& _constraint2) :
        m_constraint1(_constraint1),
        m_constraint2(_constraint2)
    {
    }

    virtual ShapeConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z OrConstraint(m_constraint1, m_constraint2);
    }

    virtual Bool TryMatchWith(Shape& _shape, const ShapeAnalyzer_W& _shapeAnalyzer) const OVERRIDE_Z
    {
        return m_constraint1.TryMatchWith(_shape, _shapeAnalyzer) || m_constraint2.TryMatchWith(_shape, _shapeAnalyzer);
    }


private :
    ShapeConstraint m_constraint1;
    ShapeConstraint m_constraint2;
};

//------------------------------------------------------------------------------------
// AndConstraint
//------------------------------------------------------------------------------------
//
class AndConstraint : public ShapeConstraintImpl
{
public:
    AndConstraint(const ShapeConstraint& _constraint1, const ShapeConstraint& _constraint2) :
        m_constraint1(_constraint1),
        m_constraint2(_constraint2)
    {
    }

    virtual ShapeConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z AndConstraint(m_constraint1, m_constraint2);
    }

    virtual Bool TryMatchWith(Shape& _shape, const ShapeAnalyzer_W& _shapeAnalyzer) const OVERRIDE_Z
    {
        return m_constraint1.TryMatchWith(_shape, _shapeAnalyzer) && m_constraint2.TryMatchWith(_shape, _shapeAnalyzer);
    }


private :
    ShapeConstraint m_constraint1;
    ShapeConstraint m_constraint2;
};

//------------------------------------------------------------------------------------
// NoOtherSurface
//------------------------------------------------------------------------------------
//
class NoOtherSurface : public ShapeConstraintImpl
{
public :
    NoOtherSurface()
    {
    }

    virtual ShapeConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z NoOtherSurface();
    }

    virtual Bool TryMatchWith(Shape& _shape, const ShapeAnalyzer_W& _shapeAnalyzer) const OVERRIDE_Z
    {
        S32 totalNbSurfaces = _shapeAnalyzer.ComputeAndGet<ShapeCharacteristics::TotalNbSurfaces>(_shape).m_value;
        return totalNbSurfaces == _shape.m_originalGroup->GetSize();
    }
};

//------------------------------------------------------------------------------------
// AwayFromWalls
//------------------------------------------------------------------------------------
//
class AwayFromWalls : public ShapeConstraintImpl
{
public :
    AwayFromWalls()
    {
    }

    virtual ShapeConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z AwayFromWalls();
    }

    virtual Bool TryMatchWith(Shape& _shape, const ShapeAnalyzer_W& _shapeAnalyzer) const OVERRIDE_Z
    {
        return _shapeAnalyzer.GetTopology().GroupsIsAwayFromWalls(*_shape.m_originalGroup); // TODO not perfect
    }
};

//------------------------------------------------------------------------------------
// RectangleParallel
//------------------------------------------------------------------------------------
//
class RectangleParallel : public ShapeConstraintImpl
{
public :
    RectangleParallel(const Name_Z& _name1, const Name_Z& _name2) :
        m_name1(_name1),
        m_name2(_name2)
    {
    }

    virtual ShapeConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z RectangleParallel(m_name1, m_name2);
    }

    virtual Bool TryMatchWith(Shape& _shape, const ShapeAnalyzer_W& _shapeAnalyzer) const OVERRIDE_Z
    {
        Slot* slot1 = NULL;
        Slot* slot2 = NULL;

        for (S32 i = 0; i < _shape.m_slots.GetSize(); ++i)
        {
            if (_shape.m_slots[i].m_name == m_name1)
                slot1 = &_shape.m_slots[i];
            else if (_shape.m_slots[i].m_name == m_name2)
                slot2 = &_shape.m_slots[i];
        }
        
        SHAPE_ANALYZER_ASSERT(slot1 != NULL && slot2 != NULL);

        const Vec3f& dir1 = _shapeAnalyzer.ComputeAndGet<SlotCharacteristics::Rectangle>(*slot1).m_lengthDir;
        const Vec3f& dir2 = _shapeAnalyzer.ComputeAndGet<SlotCharacteristics::Rectangle>(*slot2).m_lengthDir;

        return Vec3_GetNorm2(dir1 ^ dir2) <= 0.1f;
    }

private :
    Name_Z m_name1;
    Name_Z m_name2;
};

//------------------------------------------------------------------------------------
// RectanglePerpendicular
//------------------------------------------------------------------------------------
//
class RectanglePerpendicular : public ShapeConstraintImpl
{
public :
    RectanglePerpendicular(const Name_Z& _name1, const Name_Z& _name2) :
        m_name1(_name1),
        m_name2(_name2)
    {
    }

    virtual ShapeConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z RectanglePerpendicular(m_name1, m_name2);
    }

    virtual Bool TryMatchWith(Shape& _shape, const ShapeAnalyzer_W& _shapeAnalyzer) const OVERRIDE_Z
    {
        Slot* slot1 = NULL;
        Slot* slot2 = NULL;

        for (S32 i = 0; i < _shape.m_slots.GetSize(); ++i)
        {
            if (_shape.m_slots[i].m_name == m_name1)
                slot1 = &_shape.m_slots[i];
            else if (_shape.m_slots[i].m_name == m_name2)
                slot2 = &_shape.m_slots[i];
        }

        SHAPE_ANALYZER_ASSERT(slot1 != NULL && slot2 != NULL);

        const Vec3f& dir1 = _shapeAnalyzer.ComputeAndGet<SlotCharacteristics::Rectangle>(*slot1).m_lengthDir;
        const Vec3f& dir2 = _shapeAnalyzer.ComputeAndGet<SlotCharacteristics::Rectangle>(*slot2).m_lengthDir;

        return dir1 * dir2 <= 0.1f;
    }

private :
    Name_Z m_name1;
    Name_Z m_name2;
};

//------------------------------------------------------------------------------------
// RectangleAligned
//------------------------------------------------------------------------------------
//
class RectangleAligned : public ShapeConstraintImpl
{
public :
    RectangleAligned(const Name_Z& _name1, const Name_Z& _name2, Float _diffMax = 0.1f) :
        m_name1(_name1),
        m_name2(_name2),
        m_diffMax(_diffMax)
    {
    }

    virtual ShapeConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z RectangleAligned(m_name1, m_name2, m_diffMax);
    }

    virtual Bool TryMatchWith(Shape& _shape, const ShapeAnalyzer_W& _shapeAnalyzer) const OVERRIDE_Z
    {
        Slot* slot1 = NULL;
        Slot* slot2 = NULL;

        for (S32 i = 0; i < _shape.m_slots.GetSize(); ++i)
        {
            if (_shape.m_slots[i].m_name == m_name1)
                slot1 = &_shape.m_slots[i];
            else if (_shape.m_slots[i].m_name == m_name2)
                slot2 = &_shape.m_slots[i];
        }

        SHAPE_ANALYZER_ASSERT(slot1 != NULL && slot2 != NULL);

        const Vec3f& dir1 = _shapeAnalyzer.ComputeAndGet<SlotCharacteristics::Rectangle>(*slot1).m_lengthDir;
        const Vec3f& dir2 = _shapeAnalyzer.ComputeAndGet<SlotCharacteristics::Rectangle>(*slot2).m_lengthDir;
        const Vec3f& center1 = _shapeAnalyzer.ComputeAndGet<SlotCharacteristics::Rectangle>(*slot1).m_center;
        const Vec3f& center2 = _shapeAnalyzer.ComputeAndGet<SlotCharacteristics::Rectangle>(*slot2).m_center;

		Float AbsDotDir = Abs(dir1*dir2);
		if (AbsDotDir >= 0.95f)
        {
            // Rectangle parallel
            Float dot1 = dir1 * center1;
            Float dot2 = dir1 * center2;

            return Abs(dot1 - dot2) < m_diffMax;
        }
		else if (AbsDotDir <= 0.3f)
        {
            // Rectangle perpendicular
            Vec3f widthDir = VEC3F_DOWN ^ dir1;

            Float dot1 = widthDir * center1;
            Float dot2 = widthDir * center2;

            return Abs(dot1 - dot2) < m_diffMax;
        }
        else
            return FALSE;
    }

private :
    Name_Z m_name1;
    Name_Z m_name2;
    Float m_diffMax;
};

//------------------------------------------------------------------------------------
// SameRectangleLength
//------------------------------------------------------------------------------------
//
class SameRectangleLength : public ShapeConstraintImpl
{
public :
    SameRectangleLength(const Name_Z& _name1, const Name_Z& _name2, Float _similarityMin = 0.80f) :
        m_name1(_name1),
        m_name2(_name2),
        m_similarityMin(_similarityMin)
    {
    }

    virtual ShapeConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z SameRectangleLength(m_name1, m_name2, m_similarityMin);
    }

    virtual Bool TryMatchWith(Shape& _shape, const ShapeAnalyzer_W& _shapeAnalyzer) const OVERRIDE_Z
    {
        Slot* slot1 = NULL;
        Slot* slot2 = NULL;

        for (S32 i = 0; i < _shape.m_slots.GetSize(); ++i)
        {
            if (_shape.m_slots[i].m_name == m_name1)
                slot1 = &_shape.m_slots[i];
            else if (_shape.m_slots[i].m_name == m_name2)
                slot2 = &_shape.m_slots[i];
        }

        SHAPE_ANALYZER_ASSERT(slot1 != NULL && slot2 != NULL);

        Float length1 = _shapeAnalyzer.ComputeAndGet<SlotCharacteristics::Rectangle>(*slot1).m_length;
        Float length2 = _shapeAnalyzer.ComputeAndGet<SlotCharacteristics::Rectangle>(*slot2).m_length;

        return Min(length1, length2) / Max(length1, length2) > m_similarityMin;
    }

private :
    Name_Z m_name1;
    Name_Z m_name2;
    Float m_similarityMin;
};

//------------------------------------------------------------------------------------
// SlotAtFrontOf
//------------------------------------------------------------------------------------
//
class SlotAtFrontOf : public ShapeConstraintImpl
{
public :
    SlotAtFrontOf(const Name_Z& _name1, const Name_Z& _name2) :
        m_name1(_name1),
        m_name2(_name2)
    {
    }

    virtual ShapeConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z SlotAtFrontOf(m_name1, m_name2);
    }

    virtual Bool TryMatchWith(Shape& _shape, const ShapeAnalyzer_W& _shapeAnalyzer) const OVERRIDE_Z
    {
        Slot* slot1 = NULL;
        Slot* slot2 = NULL;

        for (S32 i = 0; i < _shape.m_slots.GetSize(); ++i)
        {
            if (_shape.m_slots[i].m_name == m_name1)
                slot1 = &_shape.m_slots[i];
            else if (_shape.m_slots[i].m_name == m_name2)
                slot2 = &_shape.m_slots[i];
        }

        SHAPE_ANALYZER_ASSERT(slot1 != NULL && slot2 != NULL);

        const Vec3f& avgPos1 = _shapeAnalyzer.ComputeAndGet<SlotCharacteristics::AvgPos>(*slot1).m_pos;
        const Vec3f& avgPos2 = _shapeAnalyzer.ComputeAndGet<SlotCharacteristics::AvgPos>(*slot2).m_pos;

        Vec3f two2One = (avgPos1 - avgPos2);
        two2One.y = 0.f;

        if (_shape.Have<ShapeCharacteristics::Orientation>())
        {
            // The shape is already oriented so we have to check tha its orientation match

            return _shape.Get<ShapeCharacteristics::Orientation>().m_front * two2One > 0;
        }
        else
        {
             // The shape is not oriented yet so this constraint orients its

            ShapeCharacteristics::Orientation& orientation = _shape.Create<ShapeCharacteristics::Orientation>();
            orientation.m_front = two2One.Normalize();
            return TRUE;
        }
    }

private :
    Name_Z m_name1;
    Name_Z m_name2;
};

//------------------------------------------------------------------------------------
// SlotAtBackOf
//------------------------------------------------------------------------------------
//
class SlotAtBackOf : public SlotAtFrontOf
{
public :
    SlotAtBackOf(const Name_Z& _name1, const Name_Z& _name2) : SlotAtFrontOf(_name2, _name1)
    {
    }
};

//------------------------------------------------------------------------------------
// SlotAtLeftOf
//------------------------------------------------------------------------------------
//
class SlotAtLeftOf : public ShapeConstraintImpl
{
public :
    SlotAtLeftOf(const Name_Z& _name1, const Name_Z& _name2) :
        m_name1(_name1),
        m_name2(_name2)
    {
    }

    virtual ShapeConstraintImpl* clone() const OVERRIDE_Z
    {
        return New_Z SlotAtLeftOf(m_name1, m_name2);
    }

    virtual Bool TryMatchWith(Shape& _shape, const ShapeAnalyzer_W& _shapeAnalyzer) const OVERRIDE_Z
    {
        Slot* slot1 = NULL;
        Slot* slot2 = NULL;

        for (S32 i = 0; i < _shape.m_slots.GetSize(); ++i)
        {
            if (_shape.m_slots[i].m_name == m_name1)
                slot1 = &_shape.m_slots[i];
            else if (_shape.m_slots[i].m_name == m_name2)
                slot2 = &_shape.m_slots[i];
        }

        SHAPE_ANALYZER_ASSERT(slot1 != NULL && slot2 != NULL);

        const Vec3f& avgPos1 = _shapeAnalyzer.ComputeAndGet<SlotCharacteristics::AvgPos>(*slot1).m_pos;
        const Vec3f& avgPos2 = _shapeAnalyzer.ComputeAndGet<SlotCharacteristics::AvgPos>(*slot2).m_pos;

        Vec3f two2One = (avgPos1 - avgPos2);
        two2One.y = 0.f;
      
        if (_shape.Have<ShapeCharacteristics::Orientation>())
        {
            // The shape is already oriented so we have to check tha its orientation match

            Vec3f left = _shape.Get<ShapeCharacteristics::Orientation>().m_front ^ VEC3F_DOWN;
            return left * two2One > 0;
        }
        else
        {
             // The shape is not oriented yet so this constraint orients its

            ShapeCharacteristics::Orientation& orientation = _shape.Create<ShapeCharacteristics::Orientation>();
            orientation.m_front = (two2One.Normalize() ^ VEC3F_UP);
            return TRUE;
        }
    }

private :
    Name_Z m_name1;
    Name_Z m_name2;
};

//------------------------------------------------------------------------------------
// SlotAtRightOf
//------------------------------------------------------------------------------------
//
class SlotAtRightOf : public SlotAtLeftOf
{
public :
    SlotAtRightOf(const Name_Z& _name1, const Name_Z& _name2) : SlotAtLeftOf(_name2, _name1)
    {
    }
};
} // namespace ShapeConstraints
} // namespace ShapeReco

//------------------------------------------------------------------------------------
// Boolean operators
//------------------------------------------------------------------------------------

// For slots
inline ShapeReco::SlotConstraint operator||(const ShapeReco::SlotConstraint& _constraint1, const ShapeReco::SlotConstraint& _constraint2)
{
    return ShapeReco::SlotConstraints::OrConstraint(_constraint1, _constraint2);
}

inline ShapeReco::SlotConstraint operator&&(const ShapeReco::SlotConstraint& _constraint1, const ShapeReco::SlotConstraint& _constraint2)
{
    return ShapeReco::SlotConstraints::AndConstraint(_constraint1, _constraint2);
}

// For shapes
inline ShapeReco::ShapeConstraint operator||(const ShapeReco::ShapeConstraint& _constraint1, const ShapeReco::ShapeConstraint& _constraint2)
{
    return ShapeReco::ShapeConstraints::OrConstraint(_constraint1, _constraint2);
}

inline ShapeReco::ShapeConstraint operator&&(const ShapeReco::ShapeConstraint& _constraint1, const ShapeReco::ShapeConstraint& _constraint2)
{
    return ShapeReco::ShapeConstraints::AndConstraint(_constraint1, _constraint2);
}

#endif