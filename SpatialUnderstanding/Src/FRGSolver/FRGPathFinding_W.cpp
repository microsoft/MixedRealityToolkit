// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include <pch.h>
#include <FRGSolver/FRGPathFinding_W.h>

namespace
{
	struct Infos
	{
		Infos()
		{
			m_parent = -1;
			m_bIAmInOpenList = FALSE;
			m_bIAmInCloseList = FALSE;
		}

		S32 m_parent;
		Float m_fDistBeginToMe;
		Float m_fDistMeToEnd;

		Bool m_bIAmInOpenList;
		Bool m_bIAmInCloseList;
	};

	INLINE_Z Float Dist(const Vec2i& _vA, const Vec2i& _vB)
	{
		return Sqrt((Float)(Square(_vB.x - _vA.x) + Square(_vB.y - _vA.y)));
	}

	INLINE_Z S32 Linearize(const Vec2i& _vSize, const Vec2i& _vPoint)
	{
		return _vPoint.y * _vSize.x + _vPoint.x;
	}

	INLINE_Z Vec2i Unlinearize(const Vec2i& _vSize, S32 _indice)
	{
		Vec2i v;
		v.x = _indice % _vSize.x;
		v.y = _indice / _vSize.x;
		return v;
	}

	INLINE_Z S32 ComputeBestInOpenList(const BoolDA& _daGrid, const Vec2i& _vSize, const Vec2i& _vEnd, DynArray_Z<Infos>& _daInfos, S32DA& _daOpenList)
	{
		S32 best = 0;

		for (S32 i = 1; i < _daOpenList.GetSize(); ++i)
		{
			if (_daInfos[_daOpenList[i]].m_fDistBeginToMe + _daInfos[_daOpenList[i]].m_fDistMeToEnd < _daInfos[_daOpenList[best]].m_fDistBeginToMe + _daInfos[_daOpenList[best]].m_fDistMeToEnd)
				best = i;
		}

		return best;
	}

	INLINE_Z void ComputePath(const Vec2i& _vSize, DynArray_Z<Infos>& _daInfos, S32 _end, DynArray_Z<Vec2i>& _outPath)
	{
		S32 size = 0;
		for (S32 i = _end; i != -1; i = _daInfos[i].m_parent)
			++size;

		_outPath.SetSize(size);

		S32 j = size - 1;
		for (S32 i = _end; i != -1; i = _daInfos[i].m_parent)
		{
			_outPath[j] = Unlinearize(_vSize, i);
			--j;
		}
	}
}

Bool FRGFindPath(const BoolDA& _daGrid, const S32DA& _daConexity, const Vec2i& _vSize, const Vec2i& _vBegin, const Vec2i& _vEnd, DynArray_Z<Vec2i>& _outPath)
{
#ifdef _GAMEDEBUG
    EXCEPTIONC_Z(_vSize.x * _vSize.y == _daGrid.GetSize(), "FRGPathFinding: bad size");
	EXCEPTIONC_Z(_vBegin.x >= 0 && _vBegin.x < _vSize.x && _vBegin.y >= 0 && _vBegin.y < _vSize.y, "FRGPathFinding: bad begin point");
	EXCEPTIONC_Z(_vEnd.x >= 0 && _vEnd.x < _vSize.x && _vEnd.y >= 0 && _vEnd.y < _vSize.y, "FRGPathFinding: bad end point");
#endif

	S32 endIndice = Linearize(_vSize, _vEnd);
	S32 beginIndice = Linearize(_vSize, _vBegin);
	if (_daConexity[endIndice] != _daConexity[beginIndice])
		return FALSE;

    DynArray_Z<Infos> daInfos;
    daInfos.SetSize(_daGrid.GetSize());

    S32DA daOpenList;
    daOpenList.Add(beginIndice);
	daInfos[beginIndice].m_fDistBeginToMe = 0.f;
	daInfos[beginIndice].m_fDistMeToEnd = Dist(_vBegin, _vEnd);
	daInfos[beginIndice].m_bIAmInOpenList = TRUE;
    
    while (daOpenList.GetSize() != 0)
    {
        S32 currentInOpenList = ComputeBestInOpenList(_daGrid, _vSize,  _vEnd, daInfos, daOpenList);

        S32 current = daOpenList[currentInOpenList];
		Vec2i vCurrent = Unlinearize(_vSize, current);

		daOpenList.Remove(currentInOpenList);
		daInfos[current].m_bIAmInOpenList = FALSE;
		daInfos[current].m_bIAmInCloseList = TRUE;

		static const Vec2i neighborOffsets[] = {Vec2i(-1, -1), Vec2i(0, -1), Vec2i(1, -1),
									            Vec2i(-1,  0),               Vec2i(1,  0),
												Vec2i(-1,  1), Vec2i(0,  1), Vec2i(1,  1)};
		static const S32 nbNeighbors = sizeof(neighborOffsets) / sizeof(neighborOffsets[0]);

		for (S32 i = 0; i < nbNeighbors; ++i)
        {
			Vec2i vNeighbor = vCurrent + neighborOffsets[i];

			// Neighbor is in the map ?
			if (vNeighbor.x < 0 || vNeighbor.x >= _vSize.x || vNeighbor.y < 0 || vNeighbor.y >= _vSize.y)
				continue;

			S32 neighbor = Linearize(_vSize, vNeighbor);

			// Neighbor is in a wall ? or neighbor is alredy in close list ?
			if (_daGrid[neighbor] || daInfos[neighbor].m_bIAmInCloseList)
				continue;

			if (daInfos[neighbor].m_bIAmInOpenList)
			{
				Float newDistBeginToMe = daInfos[current].m_fDistBeginToMe + Dist(vCurrent, vNeighbor);
				if (newDistBeginToMe < daInfos[neighbor].m_fDistBeginToMe)
				{
					daInfos[neighbor].m_parent = current;
					daInfos[neighbor].m_fDistBeginToMe = newDistBeginToMe;
				}
			}
			else
			{
				daOpenList.Add(neighbor);
				daInfos[neighbor].m_bIAmInOpenList = TRUE;
				daInfos[neighbor].m_parent = current;
				daInfos[neighbor].m_fDistBeginToMe = daInfos[current].m_fDistBeginToMe + Dist(vCurrent, vNeighbor);
				daInfos[neighbor].m_fDistMeToEnd = Dist(vNeighbor, _vEnd);

				if (neighbor == endIndice)
				{
					ComputePath(_vSize, daInfos, neighbor, _outPath);
					return TRUE;
				}
			}
        }
    }

    return FALSE;
}

void FRGComputeConexity(const BoolDA& _daIsWall, const Vec2i& _vSize, S32DA& _outConexity)
{
#ifdef _GAMEDEBUG
    EXCEPTIONC_Z(_daIsWall.GetSize() == _outConexity.GetSize(), "FRGComputeConexity: bad size");
	EXCEPTIONC_Z(_vSize.x * _vSize.y == _daIsWall.GetSize(), "FRGComputeConexity: bad size");
#endif

	for (S32 i = 0; i < _daIsWall.GetSize(); ++i)
		_outConexity[i] = -1;

	S32 iLastFreeZoneIndice = 0;
	for (S32 i = 0; i < _daIsWall.GetSize(); ++i)
	{
		if (_outConexity[i] != -1 || _daIsWall[i])
			continue;

		S32DA daToTest;
		daToTest.Add(i);
		S32 iCurrentZoneIndice = iLastFreeZoneIndice++;
		while (daToTest.GetSize() > 0)
		{
			S32 idx = daToTest[daToTest.GetSize() - 1]; // No matter the order
			daToTest.Remove(daToTest.GetSize() - 1);

			_outConexity[idx] = iCurrentZoneIndice;

			static const Vec2i neighborOffsets[] = {Vec2i(-1, -1), Vec2i(0, -1), Vec2i(1, -1),
										            Vec2i(-1,  0),               Vec2i(1,  0),
													Vec2i(-1,  1), Vec2i(0,  1), Vec2i(1,  1)};
			static const S32 nbNeighbors = sizeof(neighborOffsets) / sizeof(neighborOffsets[0]);
	
			for (S32 j = 0; j < nbNeighbors; ++j)
			{
				Vec2i vNeighbor = Unlinearize(_vSize, idx) + neighborOffsets[j];
				if (vNeighbor.x < 0 || vNeighbor.y < 0 || vNeighbor.x >= _vSize.x || vNeighbor.y >= _vSize.y)
					continue;

				S32 iNeighborIdx = Linearize(_vSize, vNeighbor);
				if (_outConexity[iNeighborIdx] != -1 || _daIsWall[iNeighborIdx] || daToTest.Contains(iNeighborIdx) != -1)
					continue;

				daToTest.Add(iNeighborIdx);
			}
		}
	}
}