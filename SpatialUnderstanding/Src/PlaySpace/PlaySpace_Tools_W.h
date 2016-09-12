// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#ifndef __PLAYSPACE_TOOLS_H__
#define __PLAYSPACE_TOOLS_H__

#include <BnkDynarray_Z.h>
#include <PlaySpace\PlaySpace_Surfel_W.h>

class Playspace_Mesh;

class Vec2Dhi
{
public:
	S32	x,y,h;

	Vec2Dhi() {};
	Vec2Dhi(S32 _x,S32 _y,S32 _h) {x=_x;y=_y,h=_h;}
};

class Playspace_Area
{
public:
	Vec3f	Min;
	Vec3f	Max;
	Vec3f	Center;
	Float	SizeVoxel;

	S32		NbCellX;
	S32		NbCellY;

	S32		NbCellH;
};

///////////////////////////////////////////////////////
// Alignement Tool.
///////////////////////////////////////////////////////

class	PlaySpaceAlign_W
{
protected:
	static const S32	NbAllocNormal = 32;
	static S32			NbCheckNormal;
	S32		CurNormal;
	S32		NbNormal;
	Vec3f	TabNormal[NbAllocNormal];

	void	AddNormal(Vec3f	_Normal);
	Bool	IsOkForAlign(Vec3f	&_Normal);

public:
	PlaySpaceAlign_W();
	void	Flush();
	static void	SetNbCheckNormal(S32 _Nb);

	// Mesh.
	Float	GetAlignAxis_Mesh(Playspace_Mesh &_Mesh, Float _YGround, Float _YCeiling, Vec3f &_ResultNormal, Bool _DrawIt);	// Return surface.
	Bool	SearchStabilizedAlignAxis_Mesh(Playspace_Mesh	&_Mesh, Float _YGround, Float _YCeiling, Vec3f	&_ResultAxis, Bool _ForceIt, Bool _DrawIt);
};

///////////////////////////////////////////////////////
// QUADTREE
///////////////////////////////////////////////////////
class EmptyClass
{
};

template<class InfosObject, class InfosCell = EmptyClass> class PlaySpace_CellBoard
{
public:
	class ObjectChain : public InfosObject
	{
	public:
		ObjectChain	*pNext;
	};
	class Cell : public InfosCell
	{
	public:
		ObjectChain	*pFirst;
		Cell() { pFirst = 0; }
	};
	S32										m_SizeX;
	S32										m_SizeY;
	ObjectChain								*m_pFirstFreeObject;
	BnkDynArray_Z<ObjectChain, 8192, TRUE, TRUE> m_ObjectTab;
	DynArray_Z<Cell, 256, TRUE, TRUE>			m_CellBoard;

	PlaySpace_CellBoard(S32 _SizeX, S32 _SizeY)
	{
		m_SizeX = _SizeX;
		m_SizeY = _SizeY;
		m_CellBoard.SetSize(_SizeX*_SizeY);
		m_pFirstFreeObject = 0;
	}
	~PlaySpace_CellBoard()
	{
	}

	FINLINE_Z void Empty()
	{
		memset(m_CellBoard.GetArrayPtr(), 0, sizeof(Cell)* m_CellBoard.GetSize());
		m_pFirstFreeObject = NULL;
		m_ObjectTab.Empty();
	}
	FINLINE_Z void Flush()
	{
		memset(m_CellBoard.GetArrayPtr(), 0, sizeof(Cell)* m_CellBoard.GetSize());
		m_pFirstFreeObject = NULL;
		m_ObjectTab.Flush();
	}

	FINLINE_Z ObjectChain *New()
	{
		if (m_pFirstFreeObject)
		{
			ObjectChain *pNewObject = m_pFirstFreeObject;
			m_pFirstFreeObject = m_pFirstFreeObject->pNext;
			pNewObject->pNext = NULL;
			return pNewObject;
		}
		ObjectChain *pNewObject = &m_ObjectTab.Add();
		pNewObject->pNext = NULL;
		return pNewObject;
	}

	FINLINE_Z void Delete(ObjectChain *_pObject)
	{
		_pObject->pNext = m_pFirstFreeObject;
		m_pFirstFreeObject = _pObject;
	}
	FINLINE_Z void	Add(S32 x, S32 z, ObjectChain *_pObject)
	{
		Add(GetCell(x, z), _pObject);
	}

	FINLINE_Z void	Add(Cell *_pCell, ObjectChain *_pObject)
	{
		// Insert (sans tri... à voir si tri peut devenir rentable).
		_pObject->pNext = _pCell->pFirst;
		_pCell->pFirst = _pObject;
	}
	FINLINE_Z	void	Remove(Cell *_pCell, ObjectChain *_pObject, Bool _DeleteObject = TRUE)
	{
		if (_pCell->pFirst == _pObject)
			_pCell->pFirst = _pObject->pNext;
		else
		{
			ObjectChain	*pCur = _pCell->pFirst;
			while (pCur->pNext != _pObject)
				pCur = pCur->pNext;
			EXCEPTIONC_Z(pCur, "PlaySpace_CellBoard::Remove Not found...");
			pCur->pNext = _pObject->pNext;
		}

		// Set As Free.
		if (_DeleteObject)
			Delete(_pObject);
	}
	FINLINE_Z Cell	*GetCell(S32 x, S32 z)
	{
		EXCEPTIONC_Z(x >= 0, "PlaySpace_CellBoard::Get Out of playspace");
		EXCEPTIONC_Z(z >= 0, "PlaySpace_CellBoard::Get Out of playspace");
		EXCEPTIONC_Z(x < m_SizeX, "PlaySpace_CellBoard::Get Out of playspace");
		EXCEPTIONC_Z(z < m_SizeY, "PlaySpace_CellBoard::Get Out of playspace");
		return &(m_CellBoard[z * m_SizeX + x]);
	}

	FINLINE_Z	void	FlushCell(Cell *_pCell)
	{
		ObjectChain	*pCur = _pCell->pFirst;
		if (pCur)
		{
			while (pCur->pNext)
				pCur = pCur->pNext;

			pCur->pNext = m_pFirstFreeObject;
			m_pFirstFreeObject = _pCell->pFirst;

			_pCell->pFirst = NULL;
		}
	}
};

#endif //__PLAYSPACE_TOOLS_H__
