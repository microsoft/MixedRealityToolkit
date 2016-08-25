// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#ifndef _DYNARRAY_Z_H
#define _DYNARRAY_Z_H

#include <Types_Z.h>
#include <Assert_Z.h>

#define	DYA_SIZEBITS		18						// Dynarray maxi: 256k �l�ments !!!
#define	DYA_RSVSIZEBITS		(32-DYA_SIZEBITS)
#define	DYA_SIZEMAX			((1<<DYA_SIZEBITS)-1)
#define	DYA_RSVSIZEMAX		((1<<DYA_RSVSIZEBITS)-1)	// Reserve maxi: 16384 �l�ments !!!

#define	DYNARRAY_Z_EXP(exp)			{EXCEPTIONC_Z(exp,"Array_Z %s %s (size %d)",#exp, TYPEINFO_Z(this), GetSize());}
#define	DYNARRAY_Z_WARNING(exp)		{ASSERTC_Z(exp,"Array_Z %s %s (size %d)",#exp, TYPEINFO_Z(this), GetSize());}

#if defined(_MSC_VER)
#pragma warning( push )
#pragma warning( disable : 4127 )
#endif

template<class T ,S32 Granularity=8,Bool DeleteObject = TRUE,Bool InitObject = TRUE,int align = _ALLOCDEFAULTALIGN,Bool IOAsBlock = FALSE> class DynArray_Z
{    
	// Attention, reservedSize est exprim� par rapport � Size !!!
	U32		ReservedSize:DYA_RSVSIZEBITS,Size:DYA_SIZEBITS;
	T		*ArrayPtr;
public:
	typedef	T*			TPtr;
	typedef	const T* 	TCPtr;
	DynArray_Z()
	{
		Size=0;
		ReservedSize=0;
		ArrayPtr=NULL;
	}

	DynArray_Z(const DynArray_Z<T,Granularity,DeleteObject,InitObject,align,IOAsBlock> &Src)
	{
		Size=0;
		ReservedSize=0;
		ArrayPtr=NULL;
		SetReserve(Src.GetSize());
		for(int i=0;i<Src.GetSize();i++)
			Add(Src[i]);
	}
	~DynArray_Z()
	{
		if(ArrayPtr)
		{
			if (DeleteObject)
			{
				for(U32 i=0;i<Size;i++)
					ArrayPtr[i].~T();
			}
			Free_Z(ArrayPtr);
			ArrayPtr = NULL;
		}
	}
	T	*GetArrayPtr()
	{
		return ArrayPtr;
	}
	const T	*GetArrayPtr()const
	{
		return ArrayPtr;
	}
	void	Empty()
	{
		SetSize(0,TRUE);	
	}
	void	Flush()
	{
		SetSize(0);	
	}
	void	Minimize(void)
	{
		Realloc(Size);
		ReservedSize=0;
	}
	void	Realloc()
	{
		Realloc(Size+ReservedSize);
	}
	void	Remove(int Id)
	{
        DYNARRAY_Z_EXP( (((U32)Id)<Size) );
		if (DeleteObject)
		{
			ArrayPtr[Id].~T();
		}
		if (((U32)Id)<Size-1)
		{
			memmove((void*)(ArrayPtr+Id),(void*)(ArrayPtr+Id+1),(Size-Id-1)*sizeof(T));
		}
		Size--;
		ReservedSize++;
		if (ReservedSize==DYA_RSVSIZEMAX)
			Minimize();
	}
	void	Remove(int Id, int Nb)
	{
		DYNARRAY_Z_EXP(Id<(int)Size && Id>=0 && Nb>0 && Id<=(int)(Size-Nb));
		if (DeleteObject)
		{
			for(int i=0;i<Nb;i++)
				ArrayPtr[Id+i].~T();
		}

		memmove((void*)(ArrayPtr+Id),(void*)(ArrayPtr+Id+Nb),(Size-Id-Nb)*sizeof(T));

		Size-=Nb;
		ReservedSize+=Nb;
		if (ReservedSize>=DYA_RSVSIZEMAX)
			Minimize();
	}
	void	SetReserve(int NewReservedSize)
	{
		if(((U32)NewReservedSize)<Size)
			SetSize(NewReservedSize);
		else
		{
			int	nr=NewReservedSize-Size;
			DYNARRAY_Z_EXP(nr<=DYA_RSVSIZEMAX);
			if (nr>DYA_RSVSIZEMAX)
				nr=DYA_RSVSIZEMAX;
			NewReservedSize=Size+nr;
			Realloc(NewReservedSize);
			ReservedSize = nr;
		}
	}
	void	SetSize(int NewSize,Bool ResizeOnlyIfGreater = FALSE)
	{
		DYNARRAY_Z_EXP(NewSize<=DYA_SIZEMAX);
		DYNARRAY_Z_EXP(NewSize>=0);
		int i;
		if( ((U32)NewSize) > Size )
		{
			if( (NewSize-Size>ReservedSize) || !ResizeOnlyIfGreater )
			{
				Realloc(NewSize);
				ReservedSize = 0;
			}
			else if( (NewSize-Size<=ReservedSize))
			{
				ReservedSize-=NewSize-Size;
			}
			if(InitObject)
				for( i=Size; i<NewSize; i++ )
					new(&ArrayPtr[i]) T;

			Size = NewSize;
		}
		else if( (U32)NewSize < Size )
		{
			if (DeleteObject)
			{
				for(i=Size-1;i>=NewSize;i--)
					ArrayPtr[i].~T();
			}
			if(ResizeOnlyIfGreater&&(ReservedSize+(Size-NewSize) <= DYA_RSVSIZEMAX))
			{
				DYNARRAY_Z_EXP( ReservedSize+(Size-NewSize) <= DYA_RSVSIZEMAX);
				ReservedSize+=Size-NewSize;
				Size = NewSize;
			}
			else
			{
				Realloc(NewSize);
				Size = NewSize;
				ReservedSize = 0;
			}
		}
		else if (!NewSize && ReservedSize && !ResizeOnlyIfGreater)
		{
			Realloc(NewSize);
			Size = NewSize;
			ReservedSize = 0;
		}
	}
	int		GetSize() const
	{
		return Size;
	}
	int		GetReserved() const
	{
		return ReservedSize;
	}
	int		GetReservedSize() const	// Tu touches pas !!!
	{
		return ReservedSize+Size;
	}
	int		GetEleSize() const
	{
		return	sizeof(T);
	}
	int		GetGranularity()const
	{
		return Granularity;
	}
	int		GetSizeMax()const
	{
		return DYA_SIZEMAX;
	}
	int		Add()
	{
		if(!ReservedSize)
		{
			DYNARRAY_Z_EXP(Granularity<=DYA_RSVSIZEMAX);
			ReservedSize=Granularity;
			Realloc(ReservedSize+Size);
		}
		if(InitObject)
			new(&ArrayPtr[Size]) T();
		DYNARRAY_Z_EXP(Size<DYA_SIZEMAX);
		Size++;
		ReservedSize--;
		return Size-1;
	}

	
	S32 Contains(const T &val,U32 Id=0) const 
	{
		for(U32 i=Id;i<Size;i++)
			if(ArrayPtr[i]==val)
				return (S32)i;
		return -1;
	}

	int		Add(const T &Ele)
	{
		DYNARRAY_Z_EXP(ArrayPtr==NULL || !(&Ele>=ArrayPtr && &Ele<ArrayPtr+(INT_PTR_Z)GetSize()));
		if(!ReservedSize)
		{
			DYNARRAY_Z_EXP(Granularity<=DYA_RSVSIZEMAX);
			ReservedSize=Granularity;
			Realloc(ReservedSize+Size);
		}
		if(InitObject)
			new(&ArrayPtr[Size]) T(Ele);
		else
			ArrayPtr[Size]=Ele;
		DYNARRAY_Z_EXP(Size<DYA_SIZEMAX);
		Size++;
		ReservedSize--;
		return Size-1;
	}
	void	Insert(int Id,const T &Ele)
	{
		DYNARRAY_Z_EXP(ArrayPtr==NULL || !(&Ele>=ArrayPtr && &Ele<ArrayPtr+(INT_PTR_Z)GetSize()));
		
		if (Id==(int)Size)
		{
			Add(Ele);
			return;
		}
		DYNARRAY_Z_EXP(Id<(int)Size && Id>=0 );
		if(!ReservedSize)
		{
			DYNARRAY_Z_EXP(Granularity<=DYA_RSVSIZEMAX);
			ReservedSize=Granularity;
			Realloc(ReservedSize+Size);
		}
		memmove((void*)&ArrayPtr[Id+1],(void*)&ArrayPtr[Id],(Size-Id)*sizeof(T));
		if(InitObject)
			new(&ArrayPtr[Id]) T(Ele);
		else
			ArrayPtr[Id]=Ele;
		DYNARRAY_Z_EXP(Size<DYA_SIZEMAX);
		Size++;
		ReservedSize--;
	}
	void	Insert(int Id)
	{
		if (Id==(int)Size)
		{
			Add();
			return;
		}
		DYNARRAY_Z_EXP(Id<(int)Size && Id>=0 );
		if(!ReservedSize)
		{
			DYNARRAY_Z_EXP(Granularity<=DYA_RSVSIZEMAX);
			ReservedSize=Granularity;
			Realloc(ReservedSize+Size);
		}
		memmove(&ArrayPtr[Id+1],&ArrayPtr[Id],(Size-Id)*sizeof(T));
		if(InitObject)
			new(&ArrayPtr[Id]) T();
		DYNARRAY_Z_EXP(Size<DYA_SIZEMAX);
		Size++;
		ReservedSize--;
	}
	T		&Get(int Id)
	{
        DYNARRAY_Z_EXP( (((U32)Id)<Size) );
		return 	ArrayPtr[Id];
	}
	const T	&Get(int Id) const 
	{
        DYNARRAY_Z_EXP( (((U32)Id)<Size) );
		return 	ArrayPtr[Id];
	}
	T		&operator[](int Id)
	{
        DYNARRAY_Z_EXP( (((U32)Id)<Size) );
		return 	ArrayPtr[Id];
	}
	const T	&operator[](int Id) const
	{
        DYNARRAY_Z_EXP( (((U32)Id)<Size) );
		return 	ArrayPtr[Id];
	}

	DynArray_Z<T,Granularity,DeleteObject,InitObject,align,IOAsBlock>	&operator=(const DynArray_Z<T,Granularity,DeleteObject,InitObject,align,IOAsBlock> &Src)
	{
		if (this==&Src)
			return *this;
		Flush();
		SetSize(Src.GetSize());
		if(InitObject)
		{
			for(int i=0;i<Src.GetSize();i++)
				new(&ArrayPtr[i]) T(Src[i]);
		}
		else
		{
			for(int i=0;i<Src.GetSize();i++)
				ArrayPtr[i]=Src[i];
		}
		return *this;
	}
	DynArray_Z<T,Granularity,DeleteObject,InitObject,align,IOAsBlock>	&operator+=(const DynArray_Z<T,Granularity,DeleteObject,InitObject,align,IOAsBlock> &Src)
	{
		for(int i=0;i<Src.GetSize();i++)
			Add(Src[i]);
		return *this;
	}
	template <class OT ,S32 OGranularity,Bool ODeleteObject,Bool OInitObject,int Oalign,Bool OIOAsBlock>
	DynArray_Z<T,Granularity,DeleteObject,InitObject,align,IOAsBlock> &operator += ( const DynArray_Z<OT,OGranularity,ODeleteObject,OInitObject,Oalign,OIOAsBlock> &Src)
	{
		for(int i=0;i<Src.GetSize();i++)
			Add(Src[i]);
		return *this;
	}

	int GetObjectSize()const
	{
		return	(ReservedSize+Size)*sizeof(T);
	}
	void Null()
	{
		memset(ArrayPtr, 0, GetObjectSize());
	}
	void	Swap(DynArray_Z<T,Granularity,DeleteObject,InitObject,align,IOAsBlock> &_Other)
	{
		if (this==&_Other)
			return;

		// Save Other
		U32	Copy_ReservedSize = _Other.ReservedSize;
		U32 Copy_Size = _Other.Size;
		T	*Copy_ArrayPtr = _Other.ArrayPtr;

		// Copy this to Other.
		_Other.ReservedSize = ReservedSize;
		_Other.Size = Size;
		_Other.ArrayPtr = ArrayPtr;

		// Copy Other to this.
		ReservedSize = Copy_ReservedSize;
		Size = Copy_Size;
		ArrayPtr = Copy_ArrayPtr;
		return;
	}
private:
	void	Realloc(int	NewNbElement)
	{
		if(NewNbElement)
		{
			if(ArrayPtr)
				ArrayPtr=(T *)ReallocAlign_Z(ArrayPtr, NewNbElement*sizeof(T),align);
			else
				ArrayPtr=(T *)AllocAlign_Z(NewNbElement*sizeof(T),align);
			DYNARRAY_Z_EXP(ArrayPtr!=NULL);
		}
		else
		{
			if(ArrayPtr)
			{
				Free_Z(ArrayPtr);
				ArrayPtr = NULL;
			}
		}
	}
};

#if defined(_MSC_VER)
#pragma warning( pop )
#endif

// CAUTION !!! ALIGNED ON 4 !!!
typedef	DynArray_Z<S32,32,FALSE,FALSE,4,TRUE>	S32DA;
typedef	DynArray_Z<S16,32,FALSE,FALSE,4,TRUE>	S16DA;
typedef	DynArray_Z<S8,32,FALSE,FALSE,4,TRUE>	S8DA;

typedef	DynArray_Z<U32,32,FALSE,FALSE,4,TRUE>	U32DA;
typedef	DynArray_Z<U16,32,FALSE,FALSE,4,TRUE>	U16DA;
typedef	DynArray_Z<U8,32,FALSE,FALSE,4,TRUE>	U8DA;
typedef	DynArray_Z<Bool,32,FALSE,FALSE,4,TRUE>	BoolDA;

typedef	DynArray_Z<Float,32,FALSE,FALSE,4,TRUE>	FloatDA;

typedef	DynArray_Z<S32DA,32,TRUE,TRUE,4>		S32DADA;
typedef	DynArray_Z<S16DA,32,TRUE,TRUE,4>		S16DADA;
typedef	DynArray_Z<U16DA,32,TRUE,TRUE,4>		U16DADA;

#endif // DYNARRAY_Z_H
