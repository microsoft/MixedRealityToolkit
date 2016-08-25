// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include <pch.h>
#include <Topology\ShapeAnalyzer_W.h>

//#define DRAW_TIMES

// DEBUG
namespace
{
    S32 NbUsefullSaved = 0;
    S32 MaxNbUse = 0;
}

namespace ShapeReco
{
//-----------------------------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------------------------
//
ShapeAnalyzer_W::ShapeAnalyzer_W(const TopologyAnalyzer_W& _topology) :
    m_topology(_topology)
{
	m_IsAnalysed = FALSE;
}

//-----------------------------------------------------------------------------------------------
// Reset
//-----------------------------------------------------------------------------------------------
// Reset the analyse and all the descriptors previously added
// Some descriptor must be add before the next analyse
void	ShapeAnalyzer_W::Reset()
{
	ResetAnalyse();
	m_shapeDescs.Flush();
}

//-----------------------------------------------------------------------------------------------
// GetTopology
//-----------------------------------------------------------------------------------------------
//
const TopologyAnalyzer_W& ShapeAnalyzer_W::GetTopology() const
{
    return m_topology;
}

//-----------------------------------------------------------------------------------------------
// AddShapeDesc
//-----------------------------------------------------------------------------------------------
//
void ShapeAnalyzer_W::AddShapeDesc(const ShapeDesc& _shapeDesc)
{
    m_shapeDescs.Add(_shapeDesc);
}

//-----------------------------------------------------------------------------------------------
// GetShapes
//-----------------------------------------------------------------------------------------------
// Get the shapes recognized with the descriptor added with _className
void ShapeAnalyzer_W::GetShapes(const Name_Z& _shapeName, DynArray_Z<const Shape*>& _outShapes) const
{
    for (S32 s = 0; s < m_shapes.GetSize(); ++s)
    {
        if (m_shapes[s].m_name == _shapeName)
            _outShapes.Add(&m_shapes[s]);
    }
}

//-----------------------------------------------------------------------------------------------
// GetAllSurfacesOnShape
//-----------------------------------------------------------------------------------------------
// Return all surfaces on an identified shape
void ShapeAnalyzer_W::GetAllSurfacesOnShape(const Name_Z& _shapeName, const Name_Z& _slotName, DynArray_Z<const TopologyAnalyzer_W::Surface*>& _outSurfaces) const
{
    for (S32 sh = 0; sh < m_shapes.GetSize(); ++sh)
    {
        const Shape& shape = m_shapes[sh];
		if (shape.m_name != _shapeName)
			continue;

        for (S32 g = 0; g < shape.m_slots.GetSize(); ++g)
        {
            const Slot& slot = shape.m_slots[g];
            if (_slotName != NAME_NULL && slot.m_name != _slotName)
                continue;

            for (S32 sl = 0; sl < slot.m_surfaces.GetSize(); ++sl)
                _outSurfaces.Add(slot.m_surfaces[sl]);
        }
    }
}

//-----------------------------------------------------------------------------------------------
// GetAllPosOnShape
//-----------------------------------------------------------------------------------------------
// Return all position on an identified shape
void ShapeAnalyzer_W::GetAllPosOnShape(const Name_Z& _shapeName, const Name_Z& _slotName, Float _fMinRadius, Vec3fDA& _outPos) const
{
    for (S32 sh = 0; sh < m_shapes.GetSize(); ++sh)
    {
		if (m_shapes[sh].m_name != _shapeName)
			continue;

		const Shape& shape = m_shapes[sh];
        for (S32 g = 0; g < shape.m_slots.GetSize(); ++g)
        {
            const Slot& slot = shape.m_slots[g];

            if (_slotName != NAME_NULL && slot.m_name != _slotName)
                continue;

            for (S32 sl = 0; sl < slot.m_surfaces.GetSize(); ++sl)
                m_topology.GetAllPosOnSurface(*slot.m_surfaces[sl], _fMinRadius * 2.f, _outPos);
        }
    }
}

//-----------------------------------------------------------------------------------------------
// GetShapeFromZoneId
//-----------------------------------------------------------------------------------------------
//
Bool ShapeAnalyzer_W::GetShapeFromZoneId(S32 _ZoneId,Name_Z& _shapeName, Name_Z& _slotName) const
{
    for (S32 sh = 0; sh < m_shapes.GetSize(); ++sh)
    {
        const Shape& shape = m_shapes[sh];
        for (S32 g = 0; g < shape.m_slots.GetSize(); ++g)
        {
            const Slot& slot = shape.m_slots[g];

	        for (S32 sl = 0; sl < slot.m_surfaces.GetSize(); ++sl)
			{
				if (slot.m_surfaces[sl]->m_iZoneId == _ZoneId)
				{
					_shapeName = shape.m_name;
					_slotName = slot.m_name;
					return TRUE;
				}
			}
        }
    }
	return FALSE;
}

//-----------------------------------------------------------------------------------------------
// Analyse
//-----------------------------------------------------------------------------------------------
// Analyse all surfaces of the scene to find some groups of surfaces that match with some descriptors
void ShapeAnalyzer_W::Analyse()
{
	ResetAnalyse();
    NbUsefullSaved = 0;
    MaxNbUse = 0;

#ifdef DRAW_TIMES
    Double start = GetDAbsoluteTime();
#endif

    const DynArray_Z<DynArray_Z<const TopologyAnalyzer_W::Surface*>>& daSurfaceGroups = m_topology.m_daSurfaceGroups;

    for (S32 sd = 0; sd < m_shapeDescs.GetSize(); ++sd)
    {
        for (S32 g = 0; g < daSurfaceGroups.GetSize(); ++g)
        {
            DynArray_Z<const TopologyAnalyzer_W::Surface*> group;
            group.SetSize(daSurfaceGroups[g].GetSize());
            for (S32 i = 0; i < daSurfaceGroups[g].GetSize(); ++i)
                group[i] = daSurfaceGroups[g][i];

            while (group.GetSize() > 0 && TryAndAddIfOk(m_shapeDescs[sd], daSurfaceGroups[g], group))
                ;
        }
    }

#ifdef DRAW_TIMES
	Double end = GetDAbsoluteTime();
	MESSAGE_Z("ShapeAnalyzer::Analyse time : %lf seconds", end - start);

    for (S32 sd = 0; sd < m_shapeDescs.GetSize(); ++sd)
    {
        for (S32 s = 0; s < m_shapeDescs[sd].m_slotDescs.GetSize(); ++s)
        {
            if (m_shapeDescs[sd].m_slotDescs[s].m_name != NAME_NULL)
                MESSAGE_Z("Nb combinaison saved for %s::%s : %d (tree height : %d)", m_shapeDescs[sd].m_name.GetString(), m_shapeDescs[sd].m_slotDescs[s].m_name.GetString(), m_shapeDescs[sd].m_slotDescs[s].m_savedInfos.GetNbElems(), m_shapeDescs[sd].m_slotDescs[s].m_savedInfos.GetHeight());
            else     
                MESSAGE_Z("Nb combinaison saved for %s::%d : %d (tree height : %d)", m_shapeDescs[sd].m_name.GetString(), s, m_shapeDescs[sd].m_slotDescs[s].m_savedInfos.GetNbElems(), m_shapeDescs[sd].m_slotDescs[s].m_savedInfos.GetHeight());
        }
    }

    MESSAGE_Z("Nb usefull saved : %d      max(%d)", NbUsefullSaved, MaxNbUse);
#endif

	m_IsAnalysed = TRUE;
}

//-----------------------------------------------------------------------------------------------
// ResetAnalyse
//-----------------------------------------------------------------------------------------------
// Reset recognized shape
// The descriptors are preserved for future analyse
void ShapeAnalyzer_W::ResetAnalyse()
{
	m_IsAnalysed = FALSE;
    m_shapes.Flush();

    for (S32 i = 0; i < m_shapeDescs.GetSize(); ++i)
    {
        for (S32 j = 0; j < m_shapeDescs[i].m_slotDescs.GetSize(); ++j)
            m_shapeDescs[i].m_slotDescs[j].m_savedInfos.Clear();
    }
}

//-----------------------------------------------------------------------------------------------
// DrawDebug
//-----------------------------------------------------------------------------------------------
// Draw debug informations in the scene according to the options setted by the user
void ShapeAnalyzer_W::DrawDebug() const
{
    Float fSizeVoxel = m_topology.m_fSizeVoxel;
    for (S32 i = 0; i < m_shapes.GetSize(); ++i) // For all shapes
    {
        const Shape& shape = m_shapes[i];
          
        for (S32 g = 0; g < shape.m_slots.GetSize(); ++g) // For all slot in shape
        {
            const Slot& slot = shape.m_slots[g];
            if (slot.m_debugColor == COLOR_NULL) continue;

            for (S32 s = 0; s < slot.m_surfaces.GetSize(); ++s) // For all surface in slot
            {
                const TopologyAnalyzer_W::Surface& surface = *slot.m_surfaces[s];

                for (S32 i = 0; i < surface.m_daCells.GetSize(); ++i) // For each cell in surface
                {
                    if (surface.m_daCells[i].m_iDistFromVoid != 0 
						&& surface.m_daCells[i].m_iDistFromWall != 0
						&& surface.m_daCells[i].IsValid())
                    {
                        Vec3f pos = m_topology.GetCellPosition(surface, i);

                        //DRAW_DEBUG_SPHERE3D_FAST(pos, 0.03f, group.m_debugColor);
                        DRAW_DEBUG_FACE(pos + VEC3F_FRONT * fSizeVoxel / 2.f + VEC3F_LEFT * fSizeVoxel / 2.f,
                                        pos - VEC3F_FRONT * fSizeVoxel / 2.f + VEC3F_LEFT * fSizeVoxel / 2.f,
                                        pos - VEC3F_FRONT * fSizeVoxel / 2.f - VEC3F_LEFT * fSizeVoxel / 2.f, slot.m_debugColor);
                        DRAW_DEBUG_FACE(pos + VEC3F_FRONT * fSizeVoxel / 2.f + VEC3F_LEFT * fSizeVoxel / 2.f,
                                        pos - VEC3F_FRONT * fSizeVoxel / 2.f - VEC3F_LEFT * fSizeVoxel / 2.f,
                                        pos + VEC3F_FRONT * fSizeVoxel / 2.f - VEC3F_LEFT * fSizeVoxel / 2.f, slot.m_debugColor);
                    }
                } // For each cell in surface
            } // For all surface in slot

            if (slot.Have<SlotCharacteristics::Rectangle>() || slot.Have<SlotCharacteristics::Square>())
            {
                Vec3f center, lengthDir, widthDir;
                Float length, width, similarity;

                if (slot.Have<SlotCharacteristics::Rectangle>())
                {
                    center = slot.Get<SlotCharacteristics::Rectangle>().m_center;
                    lengthDir = slot.Get<SlotCharacteristics::Rectangle>().m_lengthDir;
                    length = slot.Get<SlotCharacteristics::Rectangle>().m_length;
                    width = slot.Get<SlotCharacteristics::Rectangle>().m_width;
                    similarity = slot.Get<SlotCharacteristics::Rectangle>().m_similarity;
                }
                else
                {
                    center = slot.Get<SlotCharacteristics::Square>().m_center;
                    lengthDir = slot.Get<SlotCharacteristics::Square>().m_dir;
                    length = slot.Get<SlotCharacteristics::Square>().m_size;
                    width = length;
                    similarity = slot.Get<SlotCharacteristics::Square>().m_similarity;
                }

                widthDir = lengthDir ^ VEC3F_DOWN;

                Vec3f a = center + lengthDir * length / 2.f + widthDir * width / 2.f;
                Vec3f b = center + lengthDir * length / 2.f - widthDir * width / 2.f;
                Vec3f c = center - lengthDir * length / 2.f - widthDir * width / 2.f;
                Vec3f d = center - lengthDir * length / 2.f + widthDir * width / 2.f;

                DRAW_DEBUG_LINE3D_FAST(a + VEC3F_UP * 0.10f, b + VEC3F_UP * 0.10f, slot.m_debugColor);
                DRAW_DEBUG_LINE3D_FAST(b + VEC3F_UP * 0.10f, c + VEC3F_UP * 0.10f, slot.m_debugColor);
                DRAW_DEBUG_LINE3D_FAST(c + VEC3F_UP * 0.10f, d + VEC3F_UP * 0.10f, slot.m_debugColor);
                DRAW_DEBUG_LINE3D_FAST(d + VEC3F_UP * 0.10f, a + VEC3F_UP * 0.10f, slot.m_debugColor);

                DRAW_DEBUG_STRING3D_SIZE_COLOR(center + VEC3F_UP * 0.10f, 0.020f, slot.m_debugColor, "%f", similarity);

                DRAW_DEBUG_STRING3D_SIZE_COLOR(center + VEC3F_UP * 0.10f + widthDir * width / 2.f, 0.013f, slot.m_debugColor, "%f", length);
                //if (slot.Have<SlotCharacteristics::Rectangle>())
                //    DRAW_DEBUG_STRING3D_SIZE_COLOR(center + VEC3F_UP * 0.10f + lengthDir * length / 2.f, 0.013f, slot.m_debugColor, "%f", width);
            }
            else if (slot.Have<SlotCharacteristics::Circle>())
            {
                Vec3f center = slot.Get<SlotCharacteristics::Circle>().m_center;
                Float radius = slot.Get<SlotCharacteristics::Circle>().m_radius;

                DRAW_DEBUG_ARC3D(center + VEC3F_UP * 0.10f, VEC3F_UP, 360.f, slot.m_debugColor, radius, .thickness(0.01f));
                DRAW_DEBUG_STRING3D_SIZE_COLOR(center + VEC3F_UP * 0.10f, 0.020f, slot.m_debugColor, "%f", slot.Get<SlotCharacteristics::Circle>().m_similarity);
                DRAW_DEBUG_LINE3D_FAST(center + VEC3F_UP * 0.10f, center + VEC3F_UP * 0.10f + VEC3F_FRONT * radius, slot.m_debugColor);
                DRAW_DEBUG_STRING3D_SIZE_COLOR(center + VEC3F_UP * 0.10f + VEC3F_FRONT * radius / 2.f, 0.013f, slot.m_debugColor, "%f", radius);
            }
        } // For all slot in shape

        if (shape.Have<ShapeCharacteristics::Orientation>())
        {
            const Vec3f& pos = ComputeAndGet<ShapeCharacteristics::AvgPos>(const_cast<Shape&>(shape)).m_pos; // Cast ok just for debug ?
            const Vec3f& front = shape.Get<ShapeCharacteristics::Orientation>().m_front;

            DRAW_DEBUG_ARROW(Segment_Z(pos + VEC3F_UP * 0.60f - front * 0.30f, pos + VEC3F_UP * 0.60f + front * 0.30f), COLOR_WHITE, 0.05f);
        }
    } // For all shapes

	Vec3fDA pos;
	Vec3fDA normals;

	static Float MinHeight = 0.3f;
	static Float Maxheight = 0.65f;
	static Float Depth = 0.70f;
	static Float Width = 0.30f;
	GetSittablePosOnCouch(MinHeight, Maxheight, Depth, Width, pos, normals);

	for (S32 s = 0; s < pos.GetSize(); ++s)
	{
		DRAW_DEBUG_SPHERE3D_FAST(pos[s], 0.03f, COLOR_YELLOW);
		DRAW_DEBUG_ARROW(Segment_Z(pos[s], pos[s] + normals[s] * 0.15f), COLOR_YELLOW, 0.01f);
	}
}

//-----------------------------------------------------------------------------------------------
// GetAverageDir
//-----------------------------------------------------------------------------------------------
// Return the average direction of a set of point
void ShapeAnalyzer_W::GetAverageDir(const Vec3fDA& _poss, Vec3f& _outDir) const
{
    SHAPE_ANALYZER_ASSERT(_poss.GetSize() >= 2);

	static const S32 NbCouplesMax = 256;

	// Get NbCouplesMax couple of points with the max distance
	SafeArray_Z<S32, NbCouplesMax> saFirstPos;
	SafeArray_Z<S32, NbCouplesMax> saSecondPos;
	SafeArray_Z<Float, NbCouplesMax> saMaxDists;

    S32 nbCouples = 0;

    // Get and sort couple of max dist saMaxDists[0] is the max of the maxs
    // Sorting is needed to be sure we keep the K max dist couples
	for (S32 i = 0; i < _poss.GetSize(); ++i)
	{
		for (S32 j = i + 1; j < _poss.GetSize(); ++j)
		{
            Float dist = Vec3_GetNorm2(_poss[j] - _poss[i]);

            S32 k;
			for (k = 0; k < nbCouples && k < NbCouplesMax; ++k)
			{
				if (dist > saMaxDists[k])
				{
                    nbCouples = min(nbCouples + 1, NbCouplesMax);
					for (S32 l = nbCouples - 1; l > k; --l)
					{
						saFirstPos[l] = saFirstPos[l - 1];
						saSecondPos[l] = saSecondPos[l - 1];
						saMaxDists[l] = saMaxDists[l - 1];
					}

					saFirstPos[k] = i;
					saSecondPos[k] = j;
					saMaxDists[k] = dist;

					break;
				}
			}


            if (k == nbCouples && k < saMaxDists.GetSize())
            {
                saFirstPos[nbCouples] = i;
				saSecondPos[nbCouples] = j;
				saMaxDists[nbCouples++] = dist;
            }
		}
	}

    // Sort all points
    S32DA daAllPos; daAllPos.SetSize(nbCouples* 2);

	for (S32 i = 0; i < nbCouples; ++i)
	{
		daAllPos[i] = saFirstPos[i];
		daAllPos[nbCouples + i] = saSecondPos[i];
	}

	Vec3f infPoint = _poss[saFirstPos[0]] + (_poss[saSecondPos[0]] - _poss[saFirstPos[0]]) * 20000;

	// Selection sort => min in 0
	for (S32 i = 0; i < nbCouples * 2; ++i)
	{
		S32 min = i;
		Float minDist = Vec3_GetNorm2(infPoint - _poss[daAllPos[min]]);

		for (S32 j = i + 1; j < nbCouples * 2; ++j)
		{
			Float dist = Vec3_GetNorm2(infPoint - _poss[daAllPos[j]]);
			if (dist < minDist)
			{
				min = j;
				minDist = dist;
			}
		}

        Swap(daAllPos[i], daAllPos[min]);
	}

    // Get average of first half average of second half
    Vec3f firstPos = Vec3f(0.f, 0.f, 0.f);
	Vec3f secondPos = Vec3f(0.f, 0.f, 0.f);

	for (S32 i = 0; i < nbCouples; i++)
	{
		firstPos += _poss[daAllPos[i]];
		secondPos += _poss[daAllPos[nbCouples + i]];
	}

    firstPos /= nbCouples;
    secondPos /= nbCouples;

    _outDir = (secondPos - firstPos).Normalize();
}

//-----------------------------------------------------------------------------------------------
// GetRectFromLengthDir
//-----------------------------------------------------------------------------------------------
//
void ShapeAnalyzer_W::GetRectFromLengthDir(const Vec3fDA& _poss, Vec3f& _lengthDir, Vec3f& _outCenter, Float& _outLength, Float& _outWidth) const
{
    SHAPE_ANALYZER_ASSERT(_poss.GetSize() > 0);

	_lengthDir.CHNormalize();

    Vec3f widthDir = _lengthDir ^ VEC3F_DOWN;
	widthDir.CHNormalize();

    Float yAvg = 0.f;
    Float minL = Vec3_Dot(_lengthDir, _poss[0]);
    Float maxL = minL;
    Float minW = Vec3_Dot(widthDir, _poss[0]);
    Float maxW = minW;

    for (S32 c = 1; c < _poss.GetSize(); ++c)
    {
        yAvg += _poss[c].y;

        Float dotL = Vec3_Dot(_lengthDir, _poss[c]);
        if (dotL < minL)
            minL = dotL;
        else if (dotL > maxL)
            maxL = dotL;

        Float dotW = Vec3_Dot(widthDir, _poss[c]);
        if (dotW < minW)
            minW = dotW;
        else if (dotW > maxW)
            maxW = dotW;
    }

    _outLength = maxL - minL;
    _outWidth = maxW - minW;
    _outCenter = _lengthDir * (minL + _outLength / 2.f) + widthDir* (minW + _outWidth / 2.f);
    _outCenter.y = yAvg / _poss.GetSize();
}

//-----------------------------------------------------------------------------------------------
// GetRect
//-----------------------------------------------------------------------------------------------
// Try to match a set of point with a rectangle
// Return the characteristics of the potential rectangle
void ShapeAnalyzer_W::GetRect(const Vec3fDA& _poss, Vec3f& _outCenter, Vec3f& _outLengthDir, Float& _outLength, Float& _outWidth) const
{
    SHAPE_ANALYZER_ASSERT(_poss.GetSize() >= 2);

    // Get the two most away points (a and c)
    S32 a, c;
    Float maxDist = 0.f;

    for (S32 i = 0; i < _poss.GetSize(); ++i)
	{
		for (S32 j = i + 1; j < _poss.GetSize(); ++j)
		{
            Float dist = Vec3_GetNorm(_poss[j] - _poss[i]);
            if (dist > maxDist)
            {
                maxDist = dist;
                a = i;
                c = j;
            }
        }
    }

    // Get the point the most away of the line defined by (a, c)
    Vec3f diag = (_poss[c] - _poss[a]).Normalize();
    Vec3f diagPerpendicular = diag ^ VEC3F_DOWN;

    Float dotMax = diagPerpendicular * (_poss[0] - _poss[a]);
    S32 iDotMax = 0;;

    for (S32 i = 1; i < _poss.GetSize(); ++i)
    {
        Float dot = diagPerpendicular * (_poss[i] - _poss[a]);
        if (Abs(dot) > Abs(dotMax))
        {
            dotMax = dot;
            iDotMax = i;
        }
    }

    // Now we have 3 possibilities of rect
    Vec3fDA lengthDirs, centers;
    FloatDA lengths, widths;

    // First the lengthDir is construct from the two most away points (a and c) (bad approximation but work some times)
    lengthDirs.Add(diag);
    GetRectFromLengthDir(_poss, lengthDirs[lengthDirs.GetSize() - 1], centers[centers.Add()], lengths[lengths.Add()], widths[widths.Add()]);
    // Second the lengthDir is construct from a point among the two most away points (a or c) and the point the most away of the line defined by (a, c) (good approximation)
    if (Vec3_GetNorm2(_poss[iDotMax] - _poss[a]) > Vec3_GetNorm2(_poss[iDotMax] - _poss[c]))
        lengthDirs.Add((_poss[iDotMax] - _poss[a]).Normalize());
    else
        lengthDirs.Add((_poss[iDotMax] - _poss[c]).Normalize());
    GetRectFromLengthDir(_poss, lengthDirs[lengthDirs.GetSize() - 1], centers[centers.Add()], lengths[lengths.Add()], widths[widths.Add()]);

    // Third the lengthDir is construct from the average direction of the shape (very good approximation)
    //GetAverageDir(_poss, lengthDirs[lengthDirs.Add()]);
    //GetRectFromLengthDir(_poss, lengthDirs[lengthDirs.GetSize() - 1], centers[centers.Add()], lengths[lengths.Add()], widths[widths.Add()]);

    // Get the rect with the minimum area (for all of this rect all the points are in the rect, so the best rect is the rect with the minimum area)
    S32 min = 0;
    for (S32 i = 1; i < lengthDirs.GetSize(); ++i)
    {
        if (lengths[i] * widths[i] < lengths[min] * widths[min])
            min = i;
    }

    _outCenter = centers[min];
    _outLengthDir = lengthDirs[min];
    _outLength = lengths[min];
    _outWidth = widths[min];
}

//-----------------------------------------------------------------------------------------------
// GetCircle
//-----------------------------------------------------------------------------------------------
// Try to match a set of point with a circle
// Return the characteristics of the potential circle
void ShapeAnalyzer_W::GetCircle(const Vec3fDA& _poss, Vec3f& _outCenter, Float& _outRadius) const
{
    SHAPE_ANALYZER_ASSERT(_poss.GetSize() >= 1);

    _outCenter = _poss[0];
    for (S32 i = 1; i < _poss.GetSize(); ++i)
        _outCenter += _poss[i];
    _outCenter /= _poss.GetSize();

    S32 max = 0;
    Float maxDist = Vec3_GetNorm2(_poss[0] - _outCenter);

    for (S32 i = 1; i < _poss.GetSize(); ++i)
    {
        Float dist = Vec3_GetNorm2(_poss[i] - _outCenter);
        if (dist > maxDist)
        {
            maxDist = dist;
            max = i;
        }
    }

    _outRadius = Vec3_GetNorm(_poss[max] - _outCenter);
}

//-----------------------------------------------------------------------------------------------
// Compute(Slot& _slot, SlotCharacteristics::Area& _area)
//-----------------------------------------------------------------------------------------------
// Compute the Area characteristics of a slot
void ShapeAnalyzer_W::Compute(const Slot& _slot, SlotCharacteristics::Area& _area) const
{
    Float sizeVoxel = m_topology.m_fSizeVoxel;
    const Vec3fDA& poss = ComputeAndGet<SlotCharacteristics::AllPos>(_slot).m_poss;

    _area.m_value = poss.GetSize() * Square(sizeVoxel);
}

//-----------------------------------------------------------------------------------------------
// Compute(Slot& _slot, SlotCharacteristics::AllPos& _allPos)
//-----------------------------------------------------------------------------------------------
// Compute the AllPos characteristics of a slot
void ShapeAnalyzer_W::Compute(const Slot& _slot, SlotCharacteristics::AllPos& _allPos) const
{
    for (S32 i = 0; i < _slot.m_surfaces.GetSize(); ++i)
        m_topology.GetAllPosOnSurface(*_slot.m_surfaces[i], 0.f, _allPos.m_poss);
}

//-----------------------------------------------------------------------------------------------
// Compute(Slot& _slot, SlotCharacteristics::AvgPos& _avgPos)
//-----------------------------------------------------------------------------------------------
// Compute the AvgPos characteristics of a slot
void ShapeAnalyzer_W::Compute(const Slot& _slot, SlotCharacteristics::AvgPos& _avgPos) const
{
    const Vec3fDA& poss = ComputeAndGet<SlotCharacteristics::AllPos>(_slot).m_poss;

    SHAPE_ANALYZER_ASSERT(poss.GetSize() > 0);

    _avgPos.m_pos = Vec3f(0.f, 0.f, 0.f);
   for (S32 i = 0; i < poss.GetSize(); ++i)
            _avgPos.m_pos += poss[i];

   _avgPos.m_pos /= poss.GetSize();
}

//-----------------------------------------------------------------------------------------------
// Compute(Slot& _slot, SlotCharacteristics::Rectangle& rectangle)
//-----------------------------------------------------------------------------------------------
// Compute the Rectangle characteristics of a slot
void ShapeAnalyzer_W::Compute(const Slot& _slot, SlotCharacteristics::Rectangle& _rectangle) const
{
    Float sizeVoxel = m_topology.m_fSizeVoxel;
    const Vec3fDA& poss = ComputeAndGet<SlotCharacteristics::AllPos>(_slot).m_poss;

    if (poss.GetSize() == 0)
    {
        _rectangle.m_center = VEC3F_NULL;
        _rectangle.m_lengthDir = VEC3F_NULL;
        _rectangle.m_length = 0.f;
        _rectangle.m_width = 0.f;
        _rectangle.m_similarity = 0.f;
    }
    else if (poss.GetSize() == 1)
    {
        _rectangle.m_center = poss[0];
        _rectangle.m_lengthDir = Vec3f(1.f, 0.f, 0.f);
        _rectangle.m_length = sizeVoxel;
        _rectangle.m_width = sizeVoxel;
        _rectangle.m_similarity = 1.f;
    }
    else
    {
        GetRect(poss, _rectangle.m_center, _rectangle.m_lengthDir, _rectangle.m_length, _rectangle.m_width);

        S32 nbPointShouldBeInRect = (FLOOR(_rectangle.m_length / sizeVoxel) + 1) * (FLOOR(_rectangle.m_width / sizeVoxel) + 1);
        _rectangle.m_similarity = static_cast<Float>(poss.GetSize()) / static_cast<Float>(nbPointShouldBeInRect);
    }
}

//-----------------------------------------------------------------------------------------------
// Compute(Slot& _slot, SlotCharacteristics::Square& _square)
//-----------------------------------------------------------------------------------------------
// Compute the Square characteristics of a slot
void ShapeAnalyzer_W::Compute(const Slot& _slot, SlotCharacteristics::Square& _square) const
{
    Float sizeVoxel = m_topology.m_fSizeVoxel;
    const Vec3fDA& poss = ComputeAndGet<SlotCharacteristics::AllPos>(_slot).m_poss;

    if (poss.GetSize() == 0)
    {
        _square.m_center = VEC3F_NULL;
        _square.m_dir = VEC3F_NULL;
        _square.m_size = 0.f;
        _square.m_similarity = 0.f;
    }
    else if (poss.GetSize() == 1)
    {
        _square.m_center = poss[0];
        _square.m_dir = Vec3f(1.f, 0.f, 0.f);
        _square.m_size = sizeVoxel;
        _square.m_similarity = 1.f;
    }
    else
    {
        Float width;
        GetRect(poss, _square.m_center, _square.m_dir, _square.m_size, width);

        S32 nbPointShouldBeInSquare = (FLOOR(_square.m_size / sizeVoxel) + 1) * (FLOOR(_square.m_size / sizeVoxel) + 1);
        _square.m_similarity = static_cast<Float>(poss.GetSize()) / static_cast<Float>(nbPointShouldBeInSquare);
    }
}

//-----------------------------------------------------------------------------------------------
// Compute(Slot& _slot, SlotCharacteristics::Circle& _circle)
//-----------------------------------------------------------------------------------------------
// Compute the Circle characteristics of a slot
void ShapeAnalyzer_W::Compute(const Slot& _slot, SlotCharacteristics::Circle& _circle) const
{
    Float sizeVoxel = m_topology.m_fSizeVoxel;
    const Vec3fDA& poss = ComputeAndGet<SlotCharacteristics::AllPos>(_slot).m_poss;

    if (poss.GetSize() == 0)
    {
        _circle.m_center = VEC3F_NULL;
        _circle.m_radius = 0.f;
        _circle.m_similarity = 0.f;
    }
    else
    {
        GetCircle(poss, _circle.m_center, _circle.m_radius);

        S32 nbPointShouldBeInCircle = Pi * (FLOOR(_circle.m_radius / sizeVoxel) + 1) * (FLOOR(_circle.m_radius/ sizeVoxel) + 1);
        _circle.m_similarity = static_cast<Float>(poss.GetSize()) / static_cast<Float>(nbPointShouldBeInCircle);
    }
}

//-----------------------------------------------------------------------------------------------
// Compute(Shape& _shape, ShapeCharacteristics::TotalNbSurfaces& _totalNbSurfaces)
//-----------------------------------------------------------------------------------------------
// Compute the TotalNbSurfaces characteristics of a shape
void ShapeAnalyzer_W::Compute(const Shape& _shape, ShapeCharacteristics::TotalNbSurfaces& _totalNbSurfaces) const
{
    _totalNbSurfaces.m_value = 0;
    for (S32 i = 0; i < _shape.m_slots.GetSize(); ++i)
        _totalNbSurfaces.m_value += _shape.m_slots[i].m_surfaces.GetSize();
}

//-----------------------------------------------------------------------------------------------
// Compute(Shape& _shape, ShapeCharacteristics::AvgPos& _avgPos)
//-----------------------------------------------------------------------------------------------
// Compute the AvgPos characteristics of a shape
void ShapeAnalyzer_W::Compute(const Shape& _shape, ShapeCharacteristics::AvgPos& _avgPos) const
{
    _avgPos.m_pos = Vec3f(0.f, 0.f, 0.f);
    S32 totalNbPoss = 0;

    for (S32 i = 0; i < _shape.m_slots.GetSize(); ++i)
    {
        const Vec3fDA& poss = ComputeAndGet<SlotCharacteristics::AllPos>(_shape.m_slots[i]).m_poss;
        totalNbPoss += poss.GetSize();

        for (S32 p = 0; p < poss.GetSize(); ++p)
            _avgPos.m_pos += poss[p];
    }

    SHAPE_ANALYZER_ASSERT(totalNbPoss >0);

    _avgPos.m_pos /= totalNbPoss;
}

//-----------------------------------------------------------------------------------------------
// TryMatchSlotConstraints
//-----------------------------------------------------------------------------------------------
// Try to match a slot with the contraints of a slot descriptor
Bool ShapeAnalyzer_W::TryMatchSlotConstraints(const SlotDesc& _slotDesc, Slot& _slot, Shape& _shape) const
{
    // Maybe we have already try this configuration for this slot ?
    SlotDesc::Infos* infos = _slotDesc.m_savedInfos.Find(_slot.m_surfaces);
    if (infos != NULL)
    {
        ++NbUsefullSaved;
        _slot.m_characteristics = infos->m_characteristics;
        ++infos->m_nbUse;

        if (infos->m_nbUse > MaxNbUse)
            MaxNbUse = infos->m_nbUse;

        return infos->m_ok;
    }

    // We have not already try this configuration for this slot so we have to try it
    SlotDesc::Infos& newInfos = _slotDesc.m_savedInfos.Add(_slot.m_surfaces);
    newInfos.m_nbUse = 0;

    for (S32 i = 0; i < _slotDesc.m_constraints.GetSize(); ++i)
    {
        if (!_slotDesc.m_constraints[i].TryMatchWith(_slot, _shape, *this))
        {
            // Add the configuration to the already tested configuration
            newInfos.m_ok = FALSE;
            return FALSE;
        }
    }

    // Add the configuration to the already tested configuration
    newInfos.m_ok = TRUE;
    newInfos.m_characteristics = _slot.m_characteristics;
    return TRUE;
}

//-----------------------------------------------------------------------------------------------
// TryMatchShapeConstraints
//-----------------------------------------------------------------------------------------------
// Try to match a shape with the contraints of a shape descriptor
Bool ShapeAnalyzer_W::TryMatchShapeConstraints(const ShapeDesc& _shapeDesc, Shape& _shape) const
{
    for (S32 i = 0; i < _shapeDesc.m_constraints.GetSize(); ++i)
    {
        if (!_shapeDesc.m_constraints[i].TryMatchWith(_shape, *this))
            return FALSE;
    }

    return TRUE;
}

// HELPERS for TryToMacthGroupWithDesc
namespace
{
FINLINE_Z void _RemoveLastInSlot(Shape& _potentialShape, BoolDA& _slotsToCheck, S32 _slotNum)
{
    _potentialShape.m_slots[_slotNum].m_surfaces.Remove(_potentialShape.m_slots[_slotNum].m_surfaces.GetSize() - 1);
    _slotsToCheck[_slotNum] = TRUE;
}

FINLINE_Z void _AddInSlot(const DynArray_Z<const TopologyAnalyzer_W::Surface*>& _group, Shape& _potentialShape, BoolDA& _slotsToCheck, S32 _slotNum, S32 _surfaceNum)
{
    _potentialShape.m_slots[_slotNum].m_surfaces.Add(_group[_surfaceNum]);
    _slotsToCheck[_slotNum] = TRUE;
}

#define RemoveLastInSlot(_slotNum) \
    _RemoveLastInSlot(_potentialShape, _slotsToCheck, _slotNum)

#define AddInSlot(_slotNum, _surfaceNum) \
    _AddInSlot(_group, _potentialShape, _slotsToCheck, _slotNum, _surfaceNum)

#define NB_SURFACES_MAX_BY_SLOT 2

FINLINE_Z Bool FindNextTryImpl(const DynArray_Z<const TopologyAnalyzer_W::Surface*>& _group, Shape& _potentialShape, const S32& _nbSurfaces, const S32& _nbSlots, BoolDA& _slotsToCheck, S32DA& _surfaceInSlot, S32& _lastSurfacePlaced)
{
    SHAPE_ANALYZER_ASSERT(_lastSurfacePlaced >= 0);

    if (_surfaceInSlot[_lastSurfacePlaced] < _nbSlots - 1)
    {
        RemoveLastInSlot(_surfaceInSlot[_lastSurfacePlaced]);
        S32 newSlot = ++_surfaceInSlot[_lastSurfacePlaced];
        AddInSlot(newSlot, _lastSurfacePlaced);
        return TRUE;
    }
    else if (_lastSurfacePlaced != _nbSurfaces - 1)
    {
        RemoveLastInSlot(_surfaceInSlot[_lastSurfacePlaced]);
        _surfaceInSlot[_lastSurfacePlaced] = -1;

        S32 newSurface = ++_lastSurfacePlaced;
        _surfaceInSlot[newSurface] = 0;
        AddInSlot(0, newSurface);
        return TRUE;
    }
    else
    {
        S32 nbToPlace = 0;

        while (_surfaceInSlot[_lastSurfacePlaced] == _nbSlots - 1)
        {
            RemoveLastInSlot(_surfaceInSlot[_lastSurfacePlaced]);
            _surfaceInSlot[_lastSurfacePlaced] = -1;

            if (_lastSurfacePlaced == 0)
                return FALSE;

            --_lastSurfacePlaced;
            ++nbToPlace;
        }

        while (_surfaceInSlot[_lastSurfacePlaced] == -1)
        {
            if (_lastSurfacePlaced == 0)
                return FALSE;

            --_lastSurfacePlaced;
        }

        if (_surfaceInSlot[_lastSurfacePlaced] != _nbSlots - 1)
        {
            RemoveLastInSlot(_surfaceInSlot[_lastSurfacePlaced]);
            S32 newSlot = ++_surfaceInSlot[_lastSurfacePlaced];
            AddInSlot(newSlot, _lastSurfacePlaced);
        }
        else
        {
            RemoveLastInSlot(_surfaceInSlot[_lastSurfacePlaced]);
            _surfaceInSlot[_lastSurfacePlaced] = -1;

            ++nbToPlace;
        }

        for (S32 i = 0; i < nbToPlace; ++i)
        {
            ++_lastSurfacePlaced;
            _surfaceInSlot[_lastSurfacePlaced] = 0;
            AddInSlot(0, _lastSurfacePlaced);
        }

        return TRUE;
    }
}

FINLINE_Z Bool FindNextTry(const DynArray_Z<const TopologyAnalyzer_W::Surface*>& _group, Shape& _potentialShape, const S32& _nbSurfaces, const S32& _nbSlots, BoolDA& _slotsToCheck, S32DA& _surfaceInSlot, S32& _lastSurfacePlaced)
{
    while (TRUE)
    {
        if (!FindNextTryImpl(_group, _potentialShape, _nbSurfaces, _nbSlots, _slotsToCheck, _surfaceInSlot, _lastSurfacePlaced))
            return FALSE;

        Bool ok = TRUE;
        for (S32 i = 0; i < _potentialShape.m_slots.GetSize(); ++i)
        {
            if (_potentialShape.m_slots[i].m_surfaces.GetSize() > NB_SURFACES_MAX_BY_SLOT)
            {
                ok = FALSE;
                break;
            }
        }

        if (ok)
            return TRUE;
    }
}

FINLINE_Z Bool FindNextTryWhoChangeSlot(const DynArray_Z<const TopologyAnalyzer_W::Surface*>& _group, Shape& _potentialShape, const S32& _nbSurfaces, const S32& _nbSlots, BoolDA& _slotsToCheck, S32DA& _surfaceInSlot, S32& _lastSurfacePlaced, const S32& _slotToChange)
{
    SHAPE_ANALYZER_ASSERT(_slotToChange >= 0 && _slotToChange < _nbSlots);

    //if (_surfaceInSlot[_lastSurfacePlaced] == _slotToChange)
    //    return FindNextTry(_group, _potentialShape, _nbSurfaces, _nbSlots, _slotsToCheck, _surfaceInSlot, _lastSurfacePlaced);
    //else if (_surfaceInSlot[_lastSurfacePlaced] < _slotToChange)
    //{
    //    RemoveLastInSlot(_surfaceInSlot[_lastSurfacePlaced]);
    //    _surfaceInSlot[_lastSurfacePlaced] = _slotToChange;
    //    AddInSlot(_slotToChange, _lastSurfacePlaced);
    //    return TRUE;
    //}
    //else
    {
        _slotsToCheck[_slotToChange] = FALSE;
        while (!_slotsToCheck[_slotToChange])
        {
            if (!FindNextTry(_group, _potentialShape, _nbSurfaces, _nbSlots, _slotsToCheck, _surfaceInSlot, _lastSurfacePlaced))
                return FALSE;
        }

        return TRUE;
    }
}
}
// END HELPERS

//-----------------------------------------------------------------------------------------------
// TryToMacthGroupWithDesc
//-----------------------------------------------------------------------------------------------
// Try to match a group of surface with a shape descriptor
Bool ShapeAnalyzer_W::TryToMacthGroupWithDesc(const ShapeDesc& _shapeDesc, const DynArray_Z<const TopologyAnalyzer_W::Surface*>& _group, Shape& _potentialShape, S32 _nbSurfacesToPlace)
{
    SHAPE_ANALYZER_ASSERT(_shapeDesc.m_slotDescs.GetSize() == _potentialShape.m_slots.GetSize());
    SHAPE_ANALYZER_ASSERT(_shapeDesc.m_slotDescs.GetSize() > 0);

    // Init data structures
    S32 nbSurfaces = _group.GetSize();
    S32 nbSlots = _potentialShape.m_slots.GetSize();

    BoolDA slotsToCheck; slotsToCheck.SetSize(nbSlots);
    for (S32 i = 0; i < nbSlots; ++i)
        slotsToCheck[i] = TRUE;

    S32DA surfaceInSlot; surfaceInSlot.SetSize(nbSurfaces);
    S32 slotNum = 0;
    for (S32 i = 0; i < nbSurfaces; ++i)
    {
        if (i < _nbSurfacesToPlace)
        {
            surfaceInSlot[i] = slotNum;
            _potentialShape.m_slots[slotNum].m_surfaces.Add(_group[i]);

            if (_potentialShape.m_slots[slotNum].m_surfaces.GetSize() == NB_SURFACES_MAX_BY_SLOT)
                ++slotNum;
        }
        else
            surfaceInSlot[i] = -1;
    }

    S32 lastSurfacePlaced = _nbSurfacesToPlace - 1;

    // Algo
    while (TRUE)
    {
        Bool allSlotOk = TRUE;
        S32 slotToChange;

        // Try to match with slot constraints
        for (S32 i = 0; i < nbSlots; ++i)
        {
            if (slotsToCheck[i])
            {
                _potentialShape.m_slots[i].ResetCharacteristics();
                if (!TryMatchSlotConstraints(_shapeDesc.m_slotDescs[i], _potentialShape.m_slots[i], _potentialShape))
                {
                    allSlotOk = FALSE;
                    slotToChange = i;
                    break;
                }

                slotsToCheck[i] = FALSE;
            }
        }

        // Maybe some slot do not match with its constraints
        if (!allSlotOk)
        {
            // If _nbSurfacesToPlace == 0 we have no other posibilities to try
            if (_nbSurfacesToPlace == 0)
                return FALSE;

            if (FindNextTryWhoChangeSlot(_group, _potentialShape, nbSurfaces, nbSlots, slotsToCheck, surfaceInSlot, lastSurfacePlaced, slotToChange))
                continue;

            return FALSE;
        }

        // Try to match with shape constraints
        _potentialShape.ResetCharacteristics();
        if (!TryMatchShapeConstraints(_shapeDesc, _potentialShape))
        {
            // If _nbSurfacesToPlace == 0 we have no other posibilities to try
            if (_nbSurfacesToPlace == 0)
                return FALSE;

            if (FindNextTry(_group, _potentialShape, nbSurfaces, nbSlots, slotsToCheck, surfaceInSlot, lastSurfacePlaced))
                continue;

            return FALSE;
        }

        return TRUE;
    }
}

//-----------------------------------------------------------------------------------------------
// TryAndAddIfOk
//-----------------------------------------------------------------------------------------------
// Try to match a group of surface with a shape descriptor and add the shape if we have a matched configuration
Bool ShapeAnalyzer_W::TryAndAddIfOk(const ShapeDesc& _shapeDesc, const DynArray_Z<const TopologyAnalyzer_W::Surface*>& _originalGroup, DynArray_Z<const TopologyAnalyzer_W::Surface*>& _group)
{
    Shape& shape = m_shapes[m_shapes.Add()]; // Potential shape to add
    shape.m_name = _shapeDesc.m_name;
    shape.m_originalGroup = &_originalGroup;
    shape.m_slots.SetSize(_shapeDesc.m_slotDescs.GetSize());

    for (S32 i = 0; i < shape.m_slots.GetSize(); ++i)
    {
        shape.m_slots[i].m_name = _shapeDesc.m_slotDescs[i].m_name;
        shape.m_slots[i].m_debugColor = _shapeDesc.m_slotDescs[i].m_debugColor;
    }

    for (S32 nbSurfaceToPlace = Min(_group.GetSize(), NB_SURFACES_MAX_BY_SLOT * shape.m_slots.GetSize()); nbSurfaceToPlace >= 0; --nbSurfaceToPlace)
    //for (S32 nbSurfaceToPlace = _group.GetSize(); nbSurfaceToPlace >= 0; --nbSurfaceToPlace)
    {
        if (TryToMacthGroupWithDesc(_shapeDesc, _group, shape, nbSurfaceToPlace))
        {
            // A shape is found, so we remove the surfaces whose compose the shape from the group (for next try)
			BOOL removedGroup = false;
            for (S32 sl = 0; sl < shape.m_slots.GetSize(); ++sl)
            {
                for (S32 s = 0; s < shape.m_slots[sl].m_surfaces.GetSize(); ++s)
                {
                    for (S32 i = 0; i < _group.GetSize(); ++i)
                    {
                        if (_group[i] == shape.m_slots[sl].m_surfaces[s])
                        {
                            _group.Remove(i);
							removedGroup = true;
                            break;
                        }
                    }
                }
            }

            return removedGroup;
        }

#ifdef _DEBUG
        for (S32 i = 0; i < shape.m_slots.GetSize(); ++i)
            SHAPE_ANALYZER_ASSERT(shape.m_slots[i].m_surfaces.GetSize() == 0);
#endif
    }

    // No Shape found so we remove potential shape
    m_shapes.Remove(m_shapes.GetSize() - 1);

    return FALSE;
}

Bool	ShapeAnalyzer_W::ShapeDesc_IsItCouch(const Name_Z& _shapeName, const Name_Z& _slotName)
{
	if ((_shapeName == "couch") && (_slotName == "sit"))
		return TRUE;
	return FALSE;
}

void ShapeAnalyzer_W::GetSittablePosOnCouch(Float _minHeight, Float _maxHeight, Float _depth, Float _widthMin, Vec3fDA& _outPos, Vec3fDA& _outNormal) const
{
	static Vec3f AlignedNormals[] = {Vec3f(1.f, 0.f, 0.f), Vec3f(-1.f, 0.f, 0.f), Vec3f(0.f, 0.f, 1.f), Vec3f(0.f, 0.f, -1.f), Vec3f(-0.7071f, 0, -0.7071f), Vec3f(0.7071f, 0, -0.7071f), Vec3f(-0.7071f, 0, 0.7071f), Vec3f(0.7071f, 0, 0.7071f)};
	static S32 NbAlignedNormals = sizeof(AlignedNormals) / sizeof(AlignedNormals[0]);

	DynArray_Z<const Shape*> daCouchs;
	GetShapes("couch", daCouchs);

	Vec3fDA daPossibleNormals;
	daPossibleNormals.SetSize(1);

	for (S32 i = 0; i < daCouchs.GetSize(); ++i)
	{
		const Shape& couchShape = *daCouchs[i];
		const Vec3f& couchNormal = couchShape.Get<ShapeCharacteristics::Orientation>().m_front;

		// Find the best aligned normal (nearest of the normal of the couch)
		S32 iBestAlignedNormal = 0;
		Float fBestAlignedNormalScore = couchNormal * AlignedNormals[0];
		for (S32 j = 1; j < NbAlignedNormals; ++j)
		{
			Float fCurrentScore = couchNormal * AlignedNormals[j];
			if (fCurrentScore > fBestAlignedNormalScore)
			{
				iBestAlignedNormal = j;
				fBestAlignedNormalScore = fCurrentScore;
			}
		}

		daPossibleNormals[0] = AlignedNormals[iBestAlignedNormal];

		// Get positions
		for (S32 j = 0; j < couchShape.m_slots.GetSize(); ++j)
		{
			const Slot& sitSlot = couchShape.m_slots[j];
			if (sitSlot.m_name != Name_Z("sit"))
				continue;

			for (S32 k = 0; k < sitSlot.m_surfaces.GetSize(); ++k)
				m_topology.GetAllLargePosSittableOnSurface(*sitSlot.m_surfaces[k], daPossibleNormals, _minHeight, _maxHeight, _depth, _widthMin, _outPos, _outNormal);
		}
	}
}

void ShapeAnalyzer_W::GetPosOnSurface(DynArray_Z<PosOnSurfaceParams_W>& _daParams, Float _fHeightMin, Float _fHeightMax, const Vec3f& _vFirstPointToBeNear, Float _fMinDistance) const
{
	Vec3f pointToBeNear = _vFirstPointToBeNear;
	
	for (S32 i = 0; i < _daParams.GetSize(); ++i)
	{
		U32 minSize = FLOOR(_daParams[i].m_fRadius / m_topology.m_fSizeVoxel) + 1;
		Vec3fDA daPossiblePos;
		S32DA daPossibleSurfaceID;

		// Try find pos on surfaces and not on couch
		for (S32 s = 0; s < m_topology.m_daSurfaces.GetSize(); ++s)
		{
			const TopologyAnalyzer_W::Surface& surface = m_topology.m_daSurfaces[s];

			if (surface.m_bIsGround || surface.m_bIsCeiling || surface.m_fHeightFromGround < _fHeightMin || surface.m_fHeightFromGround > _fHeightMax)
				continue;

			// Check surface is not a couch surface
			if (SurfaceIsInShape("couch", surface))
				continue;

			for (S32 c = 0; c < surface.m_daCells.GetSize(); ++c)
			{
				if (surface.m_daCells[c].m_iDistFromBorder >= minSize)
				{
					Vec3f pos = m_topology.GetCellPosition(surface, c);

					Bool collide = FALSE;
					for (S32 j = 0; j < i; ++j)
					{
						if (Square(_daParams[j].m_fRadius + _daParams[i].m_fRadius + _fMinDistance) > (pos - _daParams[j].m_vPos).GetNorm2())
						{
							collide = TRUE;
							break;
						}
					}

					if (!collide)
					{
						daPossiblePos.Add(pos);
						daPossibleSurfaceID.Add(s);
					}
				}
			}
		}

		// Try find pos on couch
		if (daPossiblePos.GetSize() == 0)
		{
			for (S32 s = 0; s < m_topology.m_daSurfaces.GetSize(); ++s)
			{
				const TopologyAnalyzer_W::Surface& surface = m_topology.m_daSurfaces[s];

				if (surface.m_bIsGround || surface.m_bIsCeiling || surface.m_fHeightFromGround < _fHeightMin || surface.m_fHeightFromGround > _fHeightMax)
					continue;

				for (S32 c = 0; c < surface.m_daCells.GetSize(); ++c)
				{
					if (surface.m_daCells[c].m_iDistFromBorder >= minSize)
					{
						Vec3f pos = m_topology.GetCellPosition(surface, c);

						Bool collide = FALSE;
						for (S32 j = 0; j < i; ++j)
						{
							if (Square(_daParams[j].m_fRadius + _daParams[i].m_fRadius + _fMinDistance) > (pos - _daParams[j].m_vPos).GetNorm2())
							{
								collide = TRUE;
								break;
							}
						}

						if (!collide)
						{
							daPossiblePos.Add(pos);
							daPossibleSurfaceID.Add(s);
						}
					}
				}
			}
		}

		// Try find pos on floor
		if (daPossiblePos.GetSize() == 0)
		{
			for (S32 s = 0; s < m_topology.m_daSurfaces.GetSize(); ++s)
			{
				const TopologyAnalyzer_W::Surface& surface = m_topology.m_daSurfaces[s];

				if (!surface.m_bIsGround)
					continue;

				for (S32 c = 0; c < surface.m_daCells.GetSize(); ++c)
				{
					if (surface.m_daCells[c].m_iDistFromBorder < minSize)
						continue;

					Vec3f pos = m_topology.GetCellPosition(surface, c);

					// For pos on floor we have to check collisions
					Bool collide = FALSE;
					for (S32 j = 0; j < i; ++j)
					{
						if (Square(_daParams[j].m_fRadius + _daParams[i].m_fRadius + _fMinDistance) > (pos - _daParams[j].m_vPos).GetNorm2())
						{
							collide = TRUE;
							break;
						}
					}

					if (!collide)
					{
						daPossiblePos.Add(pos);
						daPossibleSurfaceID.Add(s);
					}
				}
			}
		}

		if (daPossiblePos.GetSize() == 0)
		{
			_daParams[i].m_bIsSolved = FALSE;
			continue;
		}

		Float bestScore = -Float_Max;
		S32 best;
		for (S32 j = 0; j < daPossiblePos.GetSize(); ++j)
		{
			Float score = -(daPossiblePos[j] - pointToBeNear).GetNorm2();
			if (score > bestScore)
			{
				bestScore = score;
				best = j;
			}
		}

		_daParams[i].m_bIsSolved = TRUE;
		_daParams[i].m_vPos = daPossiblePos[best];
	}
}

Bool ShapeAnalyzer_W::GetPosOnSurface(Float _fRadius, Float _fHeightMin, Float _fHeightMax, const Vec3f& _vPointToBeNear, Vec3f& _outPos) const
{
	U32 minSize = FLOOR(_fRadius / m_topology.m_fSizeVoxel) + 1;
	Vec3fDA daPossiblePos;

	// Try find pos on surfaces and not on couch
	for (S32 s = 0; s < m_topology.m_daSurfaces.GetSize(); ++s)
	{
		const TopologyAnalyzer_W::Surface& surface = m_topology.m_daSurfaces[s];

		if (surface.m_bIsGround || surface.m_bIsCeiling || surface.m_fHeightFromGround < _fHeightMin || surface.m_fHeightFromGround > _fHeightMax)
			continue;

		// Check surface is not a couch surface
		if (SurfaceIsInShape("couch", surface))
			continue;

		for (S32 c = 0; c < surface.m_daCells.GetSize(); ++c)
		{
			if (surface.m_daCells[c].m_iDistFromBorder >= minSize)
				daPossiblePos.Add(m_topology.GetCellPosition(surface, c));
		}
	}

	// Try find pos on couch
	if (daPossiblePos.GetSize() == 0)
	{
		for (S32 s = 0; s < m_topology.m_daSurfaces.GetSize(); ++s)
		{
			const TopologyAnalyzer_W::Surface& surface = m_topology.m_daSurfaces[s];

			if (surface.m_bIsGround || surface.m_bIsCeiling || surface.m_fHeightFromGround < _fHeightMin || surface.m_fHeightFromGround > _fHeightMax)
				continue;

			for (S32 c = 0; c < surface.m_daCells.GetSize(); ++c)
			{
				if (surface.m_daCells[c].m_iDistFromBorder >= minSize)
					daPossiblePos.Add(m_topology.GetCellPosition(surface, c));
			}
		}
	}

	// Try find pos on floor
	if (daPossiblePos.GetSize() == 0)
	{
		for (S32 s = 0; s < m_topology.m_daSurfaces.GetSize(); ++s)
		{
			const TopologyAnalyzer_W::Surface& surface = m_topology.m_daSurfaces[s];

			if (!surface.m_bIsGround)
				continue;

			for (S32 c = 0; c < surface.m_daCells.GetSize(); ++c)
			{
				if (surface.m_daCells[c].m_iDistFromBorder >= minSize)
					daPossiblePos.Add(m_topology.GetCellPosition(surface, c));
			}
		}
	}

	if (daPossiblePos.GetSize() == 0)
		return FALSE;

	Float bestScore = -Float_Max;
	for (S32 i = 0; i < daPossiblePos.GetSize(); ++i)
	{
		Float score = -(daPossiblePos[i] - _vPointToBeNear).GetNorm2();
		if (score > bestScore)
		{
			bestScore = score;
			_outPos = daPossiblePos[i];
		}
	}

	return TRUE;
}

Bool ShapeAnalyzer_W::SurfaceIsInShape(const Name_Z& _shapeName, const TopologyAnalyzer_W::Surface& _surface) const
{
	DynArray_Z<const ShapeReco::Shape*> shapes;
    GetShapes(_shapeName, shapes);

	for (S32 i = 0; i < shapes.GetSize(); ++i)
	{
		for (S32 j = 0; j < shapes[i]->m_slots.GetSize(); ++j)
		{
			for (S32 k = 0; k < shapes[i]->m_slots[j].m_surfaces.GetSize(); ++k)
			{
				if (shapes[i]->m_slots[j].m_surfaces[k] == &_surface)
					return TRUE;
			}
		}
	}

	return FALSE;
}

//-----------------------------------------------------------------------------------------------
// CreateDefaultShapeDesc
//-----------------------------------------------------------------------------------------------
//
void ShapeAnalyzer_W::CreateDefaultShapeDesc(ShapeAnalyzer_W &_ShapeAnalyser)
{
    ShapeReco::SlotDesc sit;
	sit.SetName("sit");
    sit.m_debugColor = COLOR_CYAN;
    sit.AddConstraint(ShapeReco::SlotConstraints::SurfaceHeightBetween(0.35f, 0.60f));
    sit.AddConstraint(ShapeReco::SlotConstraints::MinNbSurfaces(1));
    sit.AddConstraint(ShapeReco::SlotConstraints::MinArea(0.30f));
    sit.AddConstraint(ShapeReco::SlotConstraints::IsRectangle());
    sit.AddConstraint(ShapeReco::SlotConstraints::RectangleLengthBetween(0.40f, 3.00f));
    sit.AddConstraint(ShapeReco::SlotConstraints::MinRectangleWidth(0.30f));

    ShapeReco::SlotDesc back;
	back.SetName("back");
    back.m_debugColor = COLOR_RED;
    back.AddConstraint(ShapeReco::SlotConstraints::SurfaceHeightBetween(0.61f, 1.00f));
    back.AddConstraint(ShapeReco::SlotConstraints::MinNbSurfaces(1));
    back.AddConstraint(ShapeReco::SlotConstraints::IsRectangle(0.3f));
    back.AddConstraint(ShapeReco::SlotConstraints::RectangleLengthBetween(0.40f, 3.00f));
    back.AddConstraint(ShapeReco::SlotConstraints::MinRectangleWidth(0.05f));

    ShapeReco::ShapeDesc couch;
	couch.SetName("couch");
    couch.AddSlot(sit);
    couch.AddSlot(back);
    couch.AddConstraint(ShapeReco::ShapeConstraints::SameRectangleLength("sit", "back", 0.60f));
	couch.AddConstraint(ShapeReco::ShapeConstraints::RectangleParallel("sit", "back"));
	couch.AddConstraint(ShapeReco::ShapeConstraints::RectangleAligned("sit", "back",0.3f));
	couch.AddConstraint(ShapeReco::ShapeConstraints::SlotAtBackOf("back", "sit"));

   _ShapeAnalyser.AddShapeDesc(couch);

/*    ShapeReco::ShapeDesc backcouch;
	backcouch.SetName("BackCouch");
    backcouch.AddSlot(back);
   _ShapeAnalyser.AddShapeDesc(backcouch);

    ShapeReco::ShapeDesc sitcouch;
	sitcouch.SetName("SitCouch");
    sitcouch.AddSlot(sit);
   _ShapeAnalyser.AddShapeDesc(sitcouch);*/


    ShapeReco::SlotDesc table;
    table.m_debugColor = COLOR_GREEN;
    table.AddConstraint(ShapeReco::SlotConstraints::SurfaceHeightBetween(0.20f, 0.50f));
    table.AddConstraint(ShapeReco::SlotConstraints::MinNbSurfaces(1));
    table.AddConstraint(ShapeReco::SlotConstraints::MinArea(0.25f));
    table.AddConstraint(ShapeReco::SlotConstraints::IsRectangle(0.5f));

    ShapeReco::ShapeDesc tableDesc;
	tableDesc.SetName("table");
    tableDesc.AddSlot(table);
    tableDesc.AddConstraint(ShapeReco::ShapeConstraints::NoOtherSurface());
    tableDesc.AddConstraint(ShapeReco::ShapeConstraints::AwayFromWalls());

    _ShapeAnalyser.AddShapeDesc(tableDesc);


    ShapeReco::SlotDesc allExceptCouchBackSlot;
    //allExceptCouchBackSlot.m_debugColor = COLOR_PURPLE;
    allExceptCouchBackSlot.AddConstraint(ShapeReco::SlotConstraints::MinArea(0.40f));
    allExceptCouchBackSlot.AddConstraint(ShapeReco::SlotConstraints::MaxSurfaceHeight(1.70f));
    allExceptCouchBackSlot.AddConstraint(ShapeReco::SlotConstraints::SurfacesAreNotPartOf("couch", "back"));

    ShapeReco::ShapeDesc allExceptCouchBack;
    allExceptCouchBack.SetName("allExceptCouchBack");
    allExceptCouchBack.AddSlot(allExceptCouchBackSlot);

    _ShapeAnalyser.AddShapeDesc(allExceptCouchBack);


	ShapeReco::SlotDesc allSlot;
    //allExceptCouchBackSlot.m_debugColor = COLOR_PURPLE;
    allSlot.AddConstraint(ShapeReco::SlotConstraints::MinArea(0.40f));
    allSlot.AddConstraint(ShapeReco::SlotConstraints::MaxSurfaceHeight(1.70f));

    ShapeReco::ShapeDesc all;
    all.SetName("all");
    all.AddSlot(allSlot);

    _ShapeAnalyser.AddShapeDesc(all);
}


} // namespace ShapeReco
