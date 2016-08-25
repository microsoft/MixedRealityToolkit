// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#ifndef _BNKDYNARRAY_Z_H
#define _BNKDYNARRAY_Z_H


//BnkDynArray_Z<T>
//----------------
//
//BnkDynArray_Z<T> manages a linked list of fixed-size chunks of elements (BnkDynArray_Z<T>::BnkDynArrayEle_Z).
//	- T* ptr = bnk.Add() -> ptr is ALWAYS VALID (no problems with resizing).
//	- There is no Release() for an element, we can only add elements or remove all of them.
//	- Empty() = remove all elements
//	- Flush() = Empty() + free memory

#if defined(_MSC_VER)
#pragma warning( push )
#pragma warning( disable : 4127 )
#endif

#include <DynArray_Z.h>

template<class T ,S32 Granularity=8,Bool DeleteObject = TRUE,Bool InitObject = TRUE,int align = _ALLOCDEFAULTALIGN, int BnkGranularity=16> class BnkDynArray_Z
{
protected:
	struct	BnkDynArrayEle_Z{
		DynArray_Z<T,Granularity,DeleteObject,InitObject,align>	DA;
	};

	U32					NbBank;
	U32					NbFreeBank;
	U16					ShiftGranularity;
	U16					MaskGranularity;
	BnkDynArrayEle_Z	**TabBankPtr;

	BnkDynArrayEle_Z *AddBank()
	{
		// Non... Cr�ation d'une nouvelle Bank.
		if (!NbFreeBank)
		{
			// R�allocation Tab Pointeur.
			NbFreeBank = BnkGranularity;
			TabBankPtr=(BnkDynArrayEle_Z **)ReallocAlign_Z(TabBankPtr,(NbFreeBank+NbBank) * sizeof(BnkDynArrayEle_Z*), sizeof(BnkDynArrayEle_Z*));
			memset(TabBankPtr+NbBank,0,BnkGranularity*sizeof(BnkDynArrayEle_Z*));

		}

		BnkDynArrayEle_Z *pLastBank = TabBankPtr[NbBank];
		if (!pLastBank)
		{
			pLastBank = (BnkDynArrayEle_Z *)New_Z BnkDynArrayEle_Z;
			ASSERT_Z(pLastBank!=0);
			TabBankPtr[NbBank] = pLastBank;
		}

		NbBank++;
		NbFreeBank--;
		return pLastBank;
	}
	void	FlushLastBank()
	{
		if(!NbBank)
			return;
		NbBank--;
		NbFreeBank++;
		if (TabBankPtr[NbBank])
		{
			Delete_Z	TabBankPtr[NbBank];
			TabBankPtr[NbBank] = 0;
		}
	}
public:
	BnkDynArray_Z()
	{
		EXCEPTIONC_Z(Granularity && ISPOW2_Z(Granularity),"Non Power of 2 unsupported %s",TYPEINFO_Z(this));
		EXCEPTIONC_Z(Granularity <= 65536,"Gran > 65536 non supported %s",TYPEINFO_Z(this));

		union FloatAsUInt {
			float		 f;
			unsigned int i;
		} c;

		//volatile float fp = static_cast<float>(Granularity);
		//volatile unsigned int * iptr = reinterpret_cast<volatile unsigned int*>(&fp);
		//ShiftGranularity = ((*iptr)>>23)-127;
		c.f = static_cast<float>(Granularity);
		ShiftGranularity = (c.i>>23)-127;
		MaskGranularity = Granularity -1;

		NbBank = 0;
		NbFreeBank = 0;
		TabBankPtr = NULL;
	}
	~BnkDynArray_Z()
	{
		Flush();
		if(TabBankPtr)
			Free_Z(TabBankPtr);
	}
	S32		GetGranularity()const
	{
		return Granularity;
	}
	S32		GetSize()const
	{
		if (NbBank)
		{
			register int LastBank = NbBank-1;
			return (LastBank<<ShiftGranularity) + TabBankPtr[LastBank]->DA.GetSize();
		}
		return 0;
	}
	T		&Get(int Id)
	{
		register U32	NumTab = Id>>ShiftGranularity;
		EXCEPTIONC_Z(NumTab<NbBank,"BnkDynArray_Z::Get %s",TYPEINFO_Z(this));
		return TabBankPtr[NumTab]->DA[Id&(unsigned int)(MaskGranularity)];
	}
	T		&operator[](int Id)
	{
		return 	Get(Id);
	}
	const T		&Get(int Id)const
	{
		register U32	NumTab = Id>>ShiftGranularity;
		EXCEPTIONC_Z(NumTab<NbBank,"BnkDynArray_Z::Get %s",TYPEINFO_Z(this));
		return TabBankPtr[NumTab]->DA[Id&(unsigned int)(MaskGranularity)];
	}
	const T		&operator[](int Id)const
	{
		return 	Get(Id);
	}
	void	Empty()
	{
		while (NbBank)
		{
			NbBank--;
			NbFreeBank++;
			TabBankPtr[NbBank]->DA.Empty();
		}
	}
	void	Realloc()
	{
		for (U32 i=0 ; i<(NbFreeBank+NbBank) ; i++)
		{
			if (!TabBankPtr[i]) break;
			TabBankPtr[i]->DA.Realloc();
		}
	}
	void	Minimize()
	{
		for (U32 i=NbBank ; i<(NbFreeBank+NbBank) ; i++)
		{
			if (!TabBankPtr[i]) break;
			Delete_Z	TabBankPtr[i];
			TabBankPtr[i] = 0;
		}
	}
	void	Flush()
	{
		NbFreeBank += NbBank;
		NbBank = 0;
		for (U32 i=0 ; i<NbFreeBank ; i++)
		{
			if (!TabBankPtr[i]) break;
			Delete_Z	TabBankPtr[i];
			TabBankPtr[i] = 0;
		}
	}
	Bool	RemoveLast()
	{
		if (!NbBank)
			return FALSE;

		BnkDynArrayEle_Z *pLastBank = TabBankPtr[NbBank-1];
		pLastBank->DA.Remove(pLastBank->DA.GetSize()-1);

		if (!pLastBank->DA.GetSize())
		{
			// Remove Empty Bank.
			NbBank--;
			NbFreeBank++;
		}
		return TRUE;
	}
	T		&Add()
	{
		// Un de libre dans derni�re Bank ?
		if (NbBank)
		{
			BnkDynArrayEle_Z *pLastBank = TabBankPtr[NbBank-1];
			if (pLastBank->DA.GetSize() != Granularity) return	pLastBank->DA[pLastBank->DA.Add()];
		}
		BnkDynArrayEle_Z *pLastBank=AddBank();
		return	pLastBank->DA[pLastBank->DA.Add()];
	}

	T		&AddRef(S32 &retId)
	{
		// Un de libre dans derni�re Bank ?
		if (NbBank)
		{
			register int CurBank = NbBank-1;
			BnkDynArrayEle_Z *pLastBank = TabBankPtr[CurBank];
			if (pLastBank->DA.GetSize() != Granularity)
			{
				register int CurId = pLastBank->DA.Add();
				retId = CurId + (CurBank << ShiftGranularity);
				return	pLastBank->DA[CurId];
			}
		}
		BnkDynArrayEle_Z *pLastBank=AddBank();
		register int CurId = pLastBank->DA.Add();
		retId = CurId + ((NbBank-1) << ShiftGranularity);
		return	pLastBank->DA[CurId];
	}
	S32		Find(const T *pEle)const
	{
		if(!pEle)	return -1;
		S32		RetId=0;
		for (U32 i=0 ; i<NbBank ; i++, RetId+=Granularity)
		{
			const T	*pFirstEle=TabBankPtr[i]->DA.GetArrayPtr();
			if(pEle<pFirstEle)
				continue;
			const T	*pLastEle=pFirstEle+Granularity;
			if(pEle>pLastEle)
				continue;
			return RetId+(S32)(pEle-pFirstEle);
		}
		return -1;
	}
	void	SetSize(S32 Size)
	{
		if(!Size)
		{
			Flush();
			return;
		}
		register U32	LastId=Size&(unsigned int)(MaskGranularity);
		register U32	NewBankNb=(Size>>ShiftGranularity)+(LastId?1:0);
		while(NbBank>NewBankNb)
			FlushLastBank();
		while(NbBank!=NewBankNb)
			AddBank()->DA.SetSize(Granularity);
		EXCEPTIONC_Z(NewBankNb==NbBank,"BnkDynArray_Z::SetSize %s",TYPEINFO_Z(this));
		if(LastId)		
			TabBankPtr[NewBankNb-1]->DA.SetSize(LastId,TRUE);	//TRUE no allocated data moved
	}	
	BnkDynArray_Z<T,Granularity,DeleteObject,InitObject,align>	&operator+=(const BnkDynArray_Z<T,Granularity,DeleteObject,InitObject,align> &Src)
	{
		const S32 n=Src.GetSize();
		for(int i=0;i<n;i++)
			Add()=Src[i];
		return *this;
	}
	BnkDynArray_Z<T,Granularity,DeleteObject,InitObject,align>	&operator=(const BnkDynArray_Z<T,Granularity,DeleteObject,InitObject,align> &Src)
	{
		if (this==&Src)
			return *this;
		Empty();

		// Realloc du tableau de pointeur ?
		if ((NbBank+NbFreeBank) < (Src.NbBank+Src.NbFreeBank))
		{
			// Oui...
			TabBankPtr=(BnkDynArrayEle_Z **)ReallocAlign_Z(TabBankPtr,(Src.NbBank+Src.NbFreeBank)* sizeof(BnkDynArrayEle_Z*),sizeof(BnkDynArrayEle_Z*));
			memset(TabBankPtr+NbBank+NbFreeBank,0,( (Src.NbBank+Src.NbFreeBank)-(NbBank+NbFreeBank) )* sizeof(BnkDynArrayEle_Z*));
			NbBank = Src.NbBank;
			NbFreeBank = Src.NbFreeBank;
		}
		else
		{
			// Non, on garde le m�me.
			NbFreeBank = NbBank + NbFreeBank - Src.NbBank;
			NbBank = Src.NbBank;
		}

		// Copie des donn�es.

		for (U32 i=0 ; i<NbBank ; i++)
		{
			// Alloc.
			BnkDynArrayEle_Z *pLastBank = TabBankPtr[i];
			if (!pLastBank)
			{
				pLastBank = New_Z BnkDynArrayEle_Z;
				ASSERT_Z(pLastBank);
				TabBankPtr[i] = pLastBank;
			}

			// Copie.
			pLastBank->DA=Src.TabBankPtr[i]->DA;
		}
		return *this;
	}
	int GetObjectSize()const
	{
		S32	Size = (NbBank + NbFreeBank) * sizeof(BnkDynArrayEle_Z*);
		for (U32 i = 0 ; i<(NbBank+NbFreeBank) ; i++)
			if(TabBankPtr[i])
				Size += sizeof(BnkDynArrayEle_Z) + TabBankPtr[i]->DA.GetObjectSize();
		return	Size;
	}
	void	Swap(BnkDynArray_Z<T, Granularity, DeleteObject, InitObject, align, BnkGranularity> &_Other)
	{
		if (this == &_Other)
			return;

		// Save Other
		U32	Copy_NbBank = _Other.NbBank;
		U32	Copy_NbFreeBank = _Other.NbFreeBank;
		U16	Copy_ShiftGranularity = _Other.ShiftGranularity;
		U16	Copy_MaskGranularity = _Other.MaskGranularity;
		BnkDynArrayEle_Z	**Copy_TabBankPtr = _Other.TabBankPtr;

		// Copy this to Other.
		_Other.NbBank = NbBank;
		_Other.NbFreeBank = NbFreeBank;
		_Other.ShiftGranularity = ShiftGranularity;
		_Other.MaskGranularity = MaskGranularity;
		_Other.TabBankPtr = TabBankPtr;

		// Copy Other to this.
		NbBank = Copy_NbBank;
		NbFreeBank = Copy_NbFreeBank;
		ShiftGranularity = Copy_ShiftGranularity;
		MaskGranularity = Copy_MaskGranularity;
		TabBankPtr = Copy_TabBankPtr;
		return;
	}

};

#if defined(_MSC_VER)
#pragma warning( pop )
#endif

#endif // _BNKDYNARRAY_Z_H
