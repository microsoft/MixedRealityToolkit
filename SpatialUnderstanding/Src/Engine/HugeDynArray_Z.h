// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#ifndef _HUGEDYNARRAY_Z_H
#define _HUGEDYNARRAY_Z_H

#if defined(_MSC_VER)
#pragma warning( push )
#pragma warning( disable : 4127 )
#endif

#define	HUGEDYNARRAY_Z_SIZEMAX		((U32)(1<<31))

#define	HUGEDYNARRAY_Z_EXP(exp)		{EXCEPTIONC_Z(exp,"HugeDynArray_Z %s %s",#exp, TYPEINFO_Z(this));}

template<class T ,int Granularity=8,Bool DeleteObject = TRUE,Bool InitObject = TRUE,int align = _ALLOCDEFAULTALIGN,Bool IOAsBlock = FALSE> class HugeDynArray_Z
{    
	U32		ReservedSize;
	U32		Size;
	T		*ArrayPtr;
public:
	HugeDynArray_Z()
	{
		Size=0;
		ReservedSize=0;
		ArrayPtr=NULL;
	}
	HugeDynArray_Z(const HugeDynArray_Z<T,Granularity,DeleteObject,InitObject,align,IOAsBlock> &Src)
	{
		Size=0;
		ReservedSize=0;
		ArrayPtr=NULL;
		SetReserve(Src.GetSize());
		for(int i=0;i<Src.GetSize();i++)
			Add(Src[i]);
	}
	~HugeDynArray_Z()
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
	void	Remove(int Ele)
	{
        HUGEDYNARRAY_Z_EXP( (((U32)Ele)<Size) );
		if (DeleteObject)
		{
			ArrayPtr[Ele].~T();
		}
		if (((U32)Ele)<Size-1)
		{
			memmove(ArrayPtr+Ele,ArrayPtr+Ele+1,(Size-Ele-1)*sizeof(T));
		}
		Size--;
		ReservedSize++;
	}
	void	SetReserve(int NewReservedSize)
	{
		HUGEDYNARRAY_Z_EXP((U32)NewReservedSize<=HUGEDYNARRAY_Z_SIZEMAX);
		if(((U32)NewReservedSize)<Size)
			SetSize(NewReservedSize);
		else
		{
			Realloc(NewReservedSize);
			ReservedSize = NewReservedSize-Size;
		}
	}
	void	SetSize(int NewSize,Bool ResizeOnlyIfGreater = FALSE)
	{
		HUGEDYNARRAY_Z_EXP(NewSize>=0 && ((U32)NewSize)<=HUGEDYNARRAY_Z_SIZEMAX);
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
			if(ResizeOnlyIfGreater)
			{
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
	int		GetReservedSize() const
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
		return HUGEDYNARRAY_Z_SIZEMAX;
	}
	int		Add()
	{
		if(!ReservedSize)
		{
			ReservedSize=Granularity;
			Realloc(Size+ReservedSize);
		}
		if(InitObject)
			new(&ArrayPtr[Size]) T();
		Size++;
		HUGEDYNARRAY_Z_EXP(Size<=HUGEDYNARRAY_Z_SIZEMAX);
		ReservedSize--;
		return Size-1;
	}
	int		Add(const T &Ele)
	{
		HUGEDYNARRAY_Z_EXP(ArrayPtr==NULL || !(&Ele>=ArrayPtr && &Ele<ArrayPtr+(INT_PTR_Z)GetSize()));

		if(!ReservedSize)
		{
			ReservedSize=Granularity;
			Realloc(ReservedSize+Size);
		}
		if(InitObject)
			new(&ArrayPtr[Size]) T(Ele);
		else
			ArrayPtr[Size]=Ele;
		Size++;
		HUGEDYNARRAY_Z_EXP(Size<=HUGEDYNARRAY_Z_SIZEMAX);
		ReservedSize--;
		return Size-1;
	}
	void	Insert(int Id,const T &Ele)
	{
		HUGEDYNARRAY_Z_EXP(ArrayPtr==NULL || !(&Ele>=ArrayPtr && &Ele<ArrayPtr+(INT_PTR_Z)GetSize()));

		if (Id==(int)Size)
		{
			Add(Ele);
			return;
		}
		HUGEDYNARRAY_Z_EXP(((U32)Id)<Size && Id>=0);
		if(!ReservedSize)
		{
			ReservedSize=Granularity;
			Realloc(ReservedSize+Size);
		}
		memmove(&ArrayPtr[Id+1],&ArrayPtr[Id],(Size-Id)*sizeof(T));
		if(InitObject)
			new(&ArrayPtr[Id]) T(Ele);
		else
			ArrayPtr[Id]=Ele;
		Size++;
		HUGEDYNARRAY_Z_EXP(Size<=HUGEDYNARRAY_Z_SIZEMAX);
		ReservedSize--;
	}
	void	Insert(int Id)
	{
		if (Id==(int)Size)
		{
			Add();
			return;
		}
		HUGEDYNARRAY_Z_EXP(((U32)Id)<Size && Id>=0);
		if(!ReservedSize)
		{
			ReservedSize=Granularity;
			Realloc(ReservedSize+Size);
		}
		memmove(&ArrayPtr[Id+1],&ArrayPtr[Id],(Size-Id)*sizeof(T));
		if(InitObject)
			new(&ArrayPtr[Id]) T();
		Size++;
		HUGEDYNARRAY_Z_EXP(Size<=HUGEDYNARRAY_Z_SIZEMAX);
		ReservedSize--;
	}
	void	MultipleInsert(int Id,U32 Nb)
	{
		HUGEDYNARRAY_Z_EXP(((U32)Id)<=Size && Id>=0);
		if(ReservedSize < Nb)
		{
			U32 TheSizeGran = Granularity;
			if (TheSizeGran < Nb) TheSizeGran = Nb;
			ReservedSize=TheSizeGran;
			Realloc(ReservedSize+Size);
		}
		memmove(&ArrayPtr[Id+Nb],&ArrayPtr[Id],(Size-Id)*sizeof(T));
		if(InitObject)
		{
			for (U32 i=0 ; i<Nb ; i++)
				new(&ArrayPtr[Id+i]) T();
		}
		Size+=Nb;
		HUGEDYNARRAY_Z_EXP(Size<=HUGEDYNARRAY_Z_SIZEMAX);
		ReservedSize-=Nb;
	}
	T		&Get(int Id)
	{
        HUGEDYNARRAY_Z_EXP( (((U32)Id)<Size) );
		return 	ArrayPtr[Id];
	}
	const T	&Get(int Id) const 
	{
        HUGEDYNARRAY_Z_EXP( (((U32)Id)<Size) );
		return 	ArrayPtr[Id];
	}
	T		&operator[](int Id)
	{
        HUGEDYNARRAY_Z_EXP( (((U32)Id)<Size) );
		return 	ArrayPtr[Id];
	}
	const T	&operator[](int Id) const
	{
        HUGEDYNARRAY_Z_EXP( (((U32)Id)<Size) );
		return 	ArrayPtr[Id];
	}

	HugeDynArray_Z<T,Granularity,DeleteObject,InitObject,align,IOAsBlock>	&operator=(const HugeDynArray_Z<T,Granularity,DeleteObject,InitObject,align,IOAsBlock> &Src)
	{
		Empty();
		SetReserve(Src.GetSize());
		for(int i=0;i<Src.GetSize();i++)
			Add(Src[i]);
		return *this;
	}
	HugeDynArray_Z<T,Granularity,DeleteObject,InitObject,align,IOAsBlock>	&operator+=(const HugeDynArray_Z<T,Granularity,DeleteObject,InitObject,align,IOAsBlock> &Src)
	{
		for(int i=0;i<Src.GetSize();i++)
			Add(Src[i]);
		return *this;
	}

	void Sort(int (__CDECL *compareProc)(const void *elem1,const void *elem2))
	{
		qsort(ArrayPtr,(U32) Size,sizeof(T),compareProc);
	}

	int GetObjectSize()const
	{
		return	(ReservedSize+Size)*sizeof(T);
	}
	void Null()
	{
		memset(ArrayPtr, 0, GetObjectSize());
	}
	void	Swap(HugeDynArray_Z<T, Granularity, DeleteObject, InitObject, align, IOAsBlock> &_Other)
	{
		if (this == &_Other)
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
	void	Realloc(U32	NewNbElement)
	{
		HUGEDYNARRAY_Z_EXP(NewNbElement<=HUGEDYNARRAY_Z_SIZEMAX);
		if(NewNbElement)
		{
			if(ArrayPtr)
				ArrayPtr=(T *)ReallocAlign_Z(ArrayPtr,NewNbElement*sizeof(T),align);
			else
			{
				ArrayPtr=(T *)AllocAlign_Z(NewNbElement*sizeof(T),align);
			}
			HUGEDYNARRAY_Z_EXP(ArrayPtr!=NULL);
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

typedef	HugeDynArray_Z<Float,32,FALSE,FALSE,4,TRUE>	HFloatDA;
typedef	HugeDynArray_Z<S8,32,FALSE,FALSE,4,TRUE>	HS8DA;
typedef	HugeDynArray_Z<U8,32,FALSE,FALSE,4,TRUE>	HU8DA;
typedef	HugeDynArray_Z<S16,32,FALSE,FALSE,4,TRUE>	HS16DA;
typedef	HugeDynArray_Z<U16,32,FALSE,FALSE,4,TRUE>	HU16DA;
typedef	HugeDynArray_Z<S32,32,FALSE,FALSE,4,TRUE>	HS32DA;
typedef	HugeDynArray_Z<U32,32,FALSE,FALSE,4,TRUE>	HU32DA;

typedef	HugeDynArray_Z<HS32DA,32,TRUE,TRUE,4>		HS32DADA;
typedef	HugeDynArray_Z<HU16DA,32,TRUE,TRUE,4>		HU16DADA;

#endif // HUGEDYNARRAY_Z_H
