// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#ifndef _GENERICSORT_Z_H
#define _GENERICSORT_Z_H

// TEMPLATE DE TRIE.
// Fonction      GenericQSortValAsc     => Tri de valeur ou class Ascending order
//               GenericQSortValDesc    => Tri de valeur ou class Descending order
//
//               GenericQSortPtrAsc     => Tri de pointeur sur valeur ou class Ascending order
//               GenericQSortPtrDesc    => Tri de pointeur sur valeur ou class Ascending order
// 
// Les valeurs ou classes tri�es doivent avoir au moins les op�rateur < et >.
// ex :
//		class  Test
//		{
//		public:
//			int		val;
//
//			FINLNE_Z Bool	operator<(Test &b) {return val < b.val;}
//			FINLNE_Z Bool	operator>(Test &b) {return val > b.val;}
//		};


#define GenericQSort_M_NBELEM		10
#define GenericQSort_NSTACK			1024

template<class TReal> void GenericQSortValAsc(S32 Nb,TReal *Tab)
{
class T
{
public:
	U8	Datas[sizeof(TReal)];
};
	if (Nb<2)
		return;

	T				*BoundStack[GenericQSort_NSTACK];
	int				NbStack = 0;

	BoundStack[NbStack+0]=(T*)Tab;
	BoundStack[NbStack+1]=(T*)(Tab+Nb-1);
	NbStack+=2;

	while (NbStack)
	{
		NbStack-=2;
		T *pLeft=BoundStack[NbStack];
		T *pRight=BoundStack[NbStack+1];

		if (pRight-pLeft < GenericQSort_M_NBELEM)
		{ // si pas bcp d'element : tri de base
			for (T *pCur = pLeft+1 ; pCur <= pRight ; pCur++)
			{
				register T	Cur=*pCur;
				register T	*pCursor;
				for (pCursor=pCur-1 ; pCursor >= (T*)Tab ; pCursor--)
				{
					if (!(*(TReal*)pCursor > *(TReal*)&Cur)) break;
					pCursor[1] = pCursor[0];
				}
				pCursor[1]=Cur;
			}
		}
		else
		{
			T *pStart = pLeft;
			T *pEnd = pRight-1;

			// Get pivot and set to the end.
			int	IdPivot = static_cast<int>(pRight-pLeft)>>1;
			T	CurRight=pStart[IdPivot];
			pStart[IdPivot] = *pRight;
			*pRight = CurRight;

			// Loop to determine partition
			for( ;; )
			{
				// Part (partition) towards the right while it's < rightmost
				while( *(TReal*)pStart < *(TReal*)&CurRight)
					pStart++;

				// Move j from right-1 to Part
				while( pEnd>pStart && (*(TReal*)pEnd>*(TReal*)&CurRight))	// Equal value is skip to avoid O2 bug.
					pEnd--;

				// If Part and j crossed paths, or j is out of bounds, we got the Part
				if( pStart >= pEnd)
					break;

				// Otherwise, swap these lines (here's where data is moved far)
				{
					register T val=*pStart;
					*pStart = *pEnd;
					*pEnd = val;
				}

				// Move Pointer : Faster and necessary for Equal values.
				pStart++;
				pEnd--;
			}
			if( pStart != pRight)
			{
				register T val=*pStart;
				*pStart = *pRight;
				*pRight = val;
			}

			// Fix QSort : Have to start by the shortest list.
			size_t Delta1 = (pStart-1) - pLeft;
			size_t Delta2 = pRight - (pStart+1);

			if (Delta1 > Delta2)
			{
				if (Delta1 > 0)
				{
					BoundStack[NbStack++]=pLeft;
					BoundStack[NbStack++]=pStart-1;
				}
				if (Delta2 > 0)
				{
					BoundStack[NbStack++]=pStart+1;
					BoundStack[NbStack++]=pRight;
				}
			}
			else
			{
				if (Delta2 > 0)
				{
					BoundStack[NbStack++]=pStart+1;
					BoundStack[NbStack++]=pRight;
				}
				if (Delta1 > 0)
				{
					BoundStack[NbStack++]=pLeft;
					BoundStack[NbStack++]=pStart-1;
				}
			}
			EXCEPTIONC_Z(NbStack < GenericQSort_NSTACK,"Have to do something for avoid Stack pb");
		}
	}
}

template<class TReal> void GenericQSortValDesc(S32 Nb,TReal *Tab)
{
class T
{
	U8	Datas[sizeof(TReal)];

	inline	Bool	operator ==(const T &_Name) {EXCEPTIONC_Z(0,"Nooo");return FALSE;}
	inline	Bool	operator<(T &b) {EXCEPTIONC_Z(0,"Nooo");return FALSE;}
	inline	Bool	operator>(T &b) {EXCEPTIONC_Z(0,"Nooo");return FALSE;}
};
	if (Nb<2)
		return;

	T				*BoundStack[GenericQSort_NSTACK];
	int				NbStack = 0;

	BoundStack[NbStack+0]=(T*)Tab;
	BoundStack[NbStack+1]=(T*)(Tab+Nb-1);
	NbStack+=2;

	while (NbStack)
	{
		NbStack-=2;
		T *pLeft=BoundStack[NbStack];
		T *pRight=BoundStack[NbStack+1];

		if (pRight-pLeft < GenericQSort_M_NBELEM)
		{ // si pas bcp d'element : tri de base
			for (T *pCur = pLeft+1 ; pCur <= pRight ; pCur++)
			{
				register T	Cur=*pCur;
				register T	*pCursor;
				for (pCursor=pCur-1 ; pCursor >= (T*)Tab ; pCursor--)
				{
					if (!(*(TReal*)pCursor < *(TReal*)&Cur)) break;
					pCursor[1] = pCursor[0];
				}
				pCursor[1]=Cur;
			}
		}
		else
		{
			T *pStart = pLeft;
			T *pEnd = pRight-1;

			// Get pivot and set to the end.
			int	IdPivot = static_cast<int>(pRight-pLeft)>>1;
			T	CurRight=pStart[IdPivot];
			pStart[IdPivot] = *pRight;
			*pRight = CurRight;

			// Loop to determine partition
			for( ;; )
			{
				// Part (partition) towards the right while it's < rightmost
				while( *(TReal*)pStart > *(TReal*)&CurRight)
					pStart++;

				// Move j from right-1 to Part
				while( pEnd>pStart && (*(TReal*)pEnd<*(TReal*)&CurRight))	// Equal value is skip to avoid O2 bug.
					pEnd--;

				// If Part and j crossed paths, or j is out of bounds, we got the Part
				if( pStart >= pEnd)
					break;

				// Otherwise, swap these lines (here's where data is moved far)
				{
					register T val=*pStart;
					*pStart = *pEnd;
					*pEnd = val;
				}

				// Move Pointer : Faster and necessary for Equal values.
				pStart++;
				pEnd--;
			}
			if( pStart != pRight)
			{
				register T val=*pStart;
				*pStart = *pRight;
				*pRight = val;
			}

			// Fix QSort : Have to start by the shortest list.
			size_t Delta1 = (pStart-1) - pLeft;
			size_t Delta2 = pRight - (pStart+1);

			if (Delta1 > Delta2)
			{
				if (Delta1 > 0)
				{
					BoundStack[NbStack++]=pLeft;
					BoundStack[NbStack++]=pStart-1;
				}
				if (Delta2 > 0)
				{
					BoundStack[NbStack++]=pStart+1;
					BoundStack[NbStack++]=pRight;
				}
			}
			else
			{
				if (Delta2 > 0)
				{
					BoundStack[NbStack++]=pStart+1;
					BoundStack[NbStack++]=pRight;
				}
				if (Delta1 > 0)
				{
					BoundStack[NbStack++]=pLeft;
					BoundStack[NbStack++]=pStart-1;
				}
			}
			EXCEPTIONC_Z(NbStack < GenericQSort_NSTACK,"Have to do something for avoid Stack pb");
		}
	}
}

template<class T> void GenericQSortPtrAsc(S32 Nb,T **Tab)
{
	if (Nb<2)
		return;

	T		**BoundStack[GenericQSort_NSTACK];
	int		NbStack = 0;

	BoundStack[NbStack+0]=Tab;
	BoundStack[NbStack+1]=Tab+Nb-1;
	NbStack+=2;

	while (NbStack)
	{
		NbStack-=2;
		T **pLeft=BoundStack[NbStack];
		T **pRight=BoundStack[NbStack+1];

		if (pRight-pLeft < GenericQSort_M_NBELEM)
		{ // si pas bcp d'element : tri de base
			for (T **pCur = pLeft+1 ; pCur <= pRight ; pCur++)
			{
				T	*Cur=*pCur;
				register T	**pCursor;
				for (pCursor=pCur-1 ; pCursor >= Tab ; pCursor--)
				{
					if (!(**pCursor > *Cur)) break;
					pCursor[1] = pCursor[0];
				}
				pCursor[1]=Cur;
			}
		}
		else
		{
			T **pStart = pLeft;
			T **pEnd = pRight-1;

			// Get pivot and set to the end.
			int	IdPivot = (pRight-pLeft)>>1;
			T	*CurRight=pStart[IdPivot];
			pStart[IdPivot] = *pRight;
			*pRight = CurRight;

			// Loop to determine partition
			for( ;; )
			{
				// Part (partition) towards the right while it's < rightmost
				while( **pStart < *CurRight)
					pStart++;

				// Move j from right-1 to Part
				while( pEnd>pStart && (**pEnd>*CurRight))	// Equal value is skip to avoid O2 bug.
					pEnd--;

				// If Part and j crossed paths, or j is out of bounds, we got the Part
				if( pStart >= pEnd)
					break;

				// Otherwise, swap these lines (here's where data is moved far)
				{
					T *val=*pStart;
					*pStart = *pEnd;
					*pEnd = val;
				}

				// Move Pointer : Faster and necessary for Equal values.
				pStart++;
				pEnd--;
			}
			if( pStart != pRight)
			{
				T *val=*pStart;
				*pStart = *pRight;
				*pRight = val;
			}

			// Fix QSort : Have to start by the shortest list.
			size_t Delta1 = (pStart-1) - pLeft;
			size_t Delta2 = pRight - (pStart+1);

			if (Delta1 > Delta2)
			{
				if (Delta1 > 0)
				{
					BoundStack[NbStack++]=pLeft;
					BoundStack[NbStack++]=pStart-1;
				}
				if (Delta2 > 0)
				{
					BoundStack[NbStack++]=pStart+1;
					BoundStack[NbStack++]=pRight;
				}
			}
			else
			{
				if (Delta2 > 0)
				{
					BoundStack[NbStack++]=pStart+1;
					BoundStack[NbStack++]=pRight;
				}
				if (Delta1 > 0)
				{
					BoundStack[NbStack++]=pLeft;
					BoundStack[NbStack++]=pStart-1;
				}
			}
			EXCEPTIONC_Z(NbStack < GenericQSort_NSTACK,"Have to do something for avoid Stack pb");
		}
	}
}

template<class T> void GenericQSortPtrDesc(S32 Nb,T **Tab)
{
	if (Nb<2)
		return;

	T		**BoundStack[GenericQSort_NSTACK];
	int		NbStack = 0;

	BoundStack[NbStack+0]=Tab;
	BoundStack[NbStack+1]=Tab+Nb-1;
	NbStack+=2;

	while (NbStack)
	{
		NbStack-=2;
		T **pLeft=BoundStack[NbStack];
		T **pRight=BoundStack[NbStack+1];

		if (pRight-pLeft < GenericQSort_M_NBELEM)
		{ // si pas bcp d'element : tri de base
			for (T **pCur = pLeft+1 ; pCur <= pRight ; pCur++)
			{
				T	*Cur=*pCur;
				register T	**pCursor;
				for (pCursor=pCur-1 ; pCursor >= Tab ; pCursor--)
				{
					if (!(**pCursor < *Cur)) break;
					pCursor[1] = pCursor[0];
				}
				pCursor[1]=Cur;
			}
		}
		else
		{
			T **pStart = pLeft;
			T **pEnd = pRight-1;

			// Get pivot and set to the end.
			int	IdPivot = (pRight-pLeft)>>1;
			T	*CurRight=pStart[IdPivot];
			pStart[IdPivot] = *pRight;
			*pRight = CurRight;

			// Loop to determine partition
			for( ;; )
			{
				// Part (partition) towards the right while it's < rightmost
				while( **pStart > *CurRight)
					pStart++;

				// Move j from right-1 to Part
				while( pEnd>pStart && (**pEnd<*CurRight))	// Equal value is skip to avoid O2 bug.
					pEnd--;

				// If Part and j crossed paths, or j is out of bounds, we got the Part
				if( pStart >= pEnd)
					break;

				// Otherwise, swap these lines (here's where data is moved far)
				{
					T *val=*pStart;
					*pStart = *pEnd;
					*pEnd = val;
				}

				// Move Pointer : Faster and necessary for Equal values.
				pStart++;
				pEnd--;
			}
			if( pStart != pRight)
			{
				T *val=*pStart;
				*pStart = *pRight;
				*pRight = val;
			}

			// Fix QSort : Have to start by the shortest list.
			size_t Delta1 = (pStart-1) - pLeft;
			size_t Delta2 = pRight - (pStart+1);

			if (Delta1 > Delta2)
			{
				if (Delta1 > 0)
				{
					BoundStack[NbStack++]=pLeft;
					BoundStack[NbStack++]=pStart-1;
				}
				if (Delta2 > 0)
				{
					BoundStack[NbStack++]=pStart+1;
					BoundStack[NbStack++]=pRight;
				}
			}
			else
			{
				if (Delta2 > 0)
				{
					BoundStack[NbStack++]=pStart+1;
					BoundStack[NbStack++]=pRight;
				}
				if (Delta1 > 0)
				{
					BoundStack[NbStack++]=pLeft;
					BoundStack[NbStack++]=pStart-1;
				}
			}
			EXCEPTIONC_Z(NbStack < GenericQSort_NSTACK,"Have to do something for avoid Stack pb");
		}
	}
}

inline	U32	FilterListInPlace(U32 *list,U32 nb)
{
	GenericQSortValAsc(nb,list);
	U32	nbout=0;
	U32	oldelement=0xffffffff;
	for (U32 i=0; i<nb; i++)
	{
		U32	element=list[i];
		if (element!=oldelement)
		{
			list[nbout++]=element;
			oldelement=element;
		}
	}
	return nbout;
}

#endif
