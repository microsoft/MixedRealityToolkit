// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include <pch.h>
#include <FRGSolver/FRGPathFinding_W.h>

namespace
{
	template <class T>
	class Queue
	{
	public:
		Queue(S32 _initialSize = 128) :
			m_size(_initialSize),
			m_circularArray(reinterpret_cast<T*>(Alloc_Z(m_size * sizeof(T)))),
			m_front(0),
			m_back(0)
		{
#ifdef _GAMEDEBUG
			EXCEPTIONC_Z(m_circularArray != NULL, "FRGSolver/FRGUtils_W.h alloc error");
#endif
		}

		~Queue()
		{
			Free_Z(m_circularArray);
		}

		Bool IsEmpty() const
		{
			return (m_front == m_back);
		}

		Bool Contains(const T& elem) const
		{
			for (S32 i = m_front; i != m_back; i = (i + 1) % m_size)
			{
				if (m_circularArray[i] == elem)
					return TRUE;
			}

			return FALSE;
		}

		void Push(const T& elem)
		{
			if (IsFull())
				Realoc();

			m_circularArray[m_back] = elem;
			m_back = (m_back + 1) % m_size;
		}

		void Pop(T& elem)
		{
#ifdef _GAMEDEBUG
			EXCEPTIONC_Z(!IsEmpty(), "FRGSolver/FRGUtils_W.h Queue is empty");
#endif

			elem = m_circularArray[m_front];
			m_front = (m_front + 1) % m_size;
		}

	private:
		Bool IsFull() const
		{
			return (m_back - m_front == -1 || m_back - m_front == m_size - 1);
		}

		void Realoc()
		{
			T* tmp = reinterpret_cast<T*>(Alloc_Z(m_size * 2 * sizeof(T)));
#ifdef _GAMEDEBUG
			EXCEPTIONC_Z(tmp != NULL, "FRGSolver/FRGUtils_W.h alloc error");
#endif

			S32 j = 0;
			for (S32 i = m_front; i != m_back; i = (i + 1) % m_size, ++j)
				tmp[j] = m_circularArray[i];
			tmp[j] = m_circularArray[m_back];

			m_front = 0;
			m_back = j;

			Free_Z(m_circularArray);
			m_circularArray = tmp;

			m_size *= 2;
		}

		S32 m_size;
		T* m_circularArray;

		S32 m_front;
		S32 m_back;
	};
}

Bool FRGFindNearestFree(const BoolDA& _daGrid, const Vec2i& _vSize, const Vec2i& _vPos, Float _fMaxDist, Vec2i& _outPos)
{
#ifdef _GAMEDEBUG
    EXCEPTIONC_Z(_vSize.x * _vSize.y == _daGrid.GetSize(), "FRGFindNearestFree: bad size");
	EXCEPTIONC_Z(_vPos.x >= 0 && _vPos.x < _vSize.x && _vPos.y >= 0 && _vPos.y < _vSize.y, "FRGFindNearestFree: bad pos");
#endif

	const Float fMaxDistSquare = Square(_fMaxDist);

	Char* visited = reinterpret_cast<Char*>(AllocF_Z(_vSize.x * _vSize.y * sizeof(*visited)));
	memset(visited, 0, _vSize.x * _vSize.y * sizeof(*visited));

	Queue<Vec2i> q;
	q.Push(_vPos);

	while (!q.IsEmpty())
	{
		Vec2i vElem;
		q.Pop(vElem);

		S32 iElem = vElem.y * _vSize.x + vElem.x;
		visited[iElem] = 1;

		if (!_daGrid[iElem])
		{
			_outPos = vElem;
			Free_Z(visited);
			return TRUE;
		}

		static const Vec2i neighborOffsets[] = {Vec2i(-1, -1), Vec2i(0, -1), Vec2i(1, -1),
												Vec2i(-1,  0),               Vec2i(1,  0),
												Vec2i(-1,  1), Vec2i(0,  1), Vec2i(1,  1)};
		static const S32 nbNeighbors = sizeof(neighborOffsets) / sizeof(neighborOffsets[0]);

		for (S32 i = 0; i < nbNeighbors; ++i)
        {
			Vec2i vNeighbor = vElem + neighborOffsets[i];

			// Neighbor is in the map ?
			if (vNeighbor.x < 0 || vNeighbor.x >= _vSize.x || vNeighbor.y < 0 || vNeighbor.y >= _vSize.y)
				continue;

			if (Square(_vPos.x - vNeighbor.x) + Square(_vPos.y - vNeighbor.y) > fMaxDistSquare)
				continue;

			S32 iNeighbor = vNeighbor.y * _vSize.x + vNeighbor.x;

			// Already visited .
			if (visited[iNeighbor] == 1)
				continue;

			if (!q.Contains(vNeighbor))
				q.Push(vNeighbor);
		}
	}

	Free_Z(visited);
	return FALSE;
}

void FRGSetNeighbors(BoolDA& _daGrid, const Vec2i& _vSize, S32 _idxPos, S32 _radius)
{
#ifdef _GAMEDEBUG
    EXCEPTIONC_Z(_vSize.x * _vSize.y == _daGrid.GetSize(), "FRGSetNeighbors: bad size");
	EXCEPTIONC_Z(_idxPos >= 0 && _idxPos < _daGrid.GetSize(), "FRGSetNeighbors: bad pos");
#endif

	Vec2i vPos;
	vPos.x = _idxPos % _vSize.x;
	vPos.y = _idxPos / _vSize.x;

	for (S32 x = vPos.x - _radius; x <= vPos.x + _radius; ++x)
	{
		for (S32 y = vPos.y - _radius; y <= vPos.y + _radius; ++y)
		{
			if (x < 0 || x >= _vSize.x || y < 0 || y >= _vSize.y)
				continue;

			_daGrid[y * _vSize.x + x] = TRUE;
		}
	}
}