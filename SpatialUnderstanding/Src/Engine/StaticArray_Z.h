// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#ifndef _STATICARRAY_Z_H
#define _STATICARRAY_Z_H

#if defined(_MSC_VER)
#pragma warning( push )
#pragma warning( disable : 4127 )
#endif

#include <BitArray_Z.h>

template<class T,int ReservedSize,Bool DeleteObject=TRUE,Bool InitObject = TRUE, Bool bExceptionWhenFull=FALSE> class StaticArray_Z
{   
private:
	POSTALIGNED128_Z char	ArrayChar[ReservedSize * sizeof(T)]		ALIGNED128_Z;
	int		Size;
	#ifdef	_DEBUG
		typedef T	Value[ReservedSize];
		Value *DebugPtr;
	#endif
	T		&Get(int Id)
	{
		return 	*(T *)(ArrayChar+Id*sizeof(T));
	}

	const T	&Get(int Id) const 
	{
		return 	*(T *)(ArrayChar+Id*sizeof(T));
	}

public:
	StaticArray_Z()
	{
		Size=0;
		#ifdef	_DEBUG
			DebugPtr = reinterpret_cast <Value*>( ArrayChar );
		#endif
	}
	StaticArray_Z(T nValue[],S32 nSize)
	{
		Size=0;
		#ifdef	_DEBUG
			DebugPtr = reinterpret_cast <Value*>( ArrayChar );
		#endif
		for(int i=0;i<nSize;i++)
			Add(nValue[i]);
	}
	StaticArray_Z(const StaticArray_Z<T,ReservedSize> &Src)
	{
		Size=0;
		#ifdef	_DEBUG
			DebugPtr = reinterpret_cast <Value*>( ArrayChar );
		#endif
		for(int i=0;i<Src.GetSize();i++)
			Add(Src[i]);
	}

	~StaticArray_Z()
	{
		if(DeleteObject)
			for(int i=0;i<Size;i++)
				Get(i).~T();
	}

	T	*GetArrayPtr()
	{
		return (T *)ArrayChar;
	}
	const T	*GetArrayPtr()const
	{
		return (T *)ArrayChar;
	}

	void	Empty()
	{
		if (Size)
			SetSize(0);	
	}
	Bool	IsFull()
	{
		return	ReservedSize==Size?TRUE:FALSE;
	}
	void	Remove(int Id)
	{
        DYNARRAY_Z_EXP( (((U32)Id)<(U32)Size) );
		if(DeleteObject)
			Get(Id).~T();
		memmove(&Get(Id),&Get(Id+1),(Size-Id-1)*sizeof(T));
		Size--;
	}

	void	SetSize(int NewSize)
	{
		DYNARRAY_Z_EXP(NewSize<=ReservedSize);
		int i;
		if( NewSize > Size )
		{
			if (InitObject)
				for( i=Size; i<NewSize; i++ )
					new(&Get(i)) T;
		}
		else if( NewSize < Size )
		{
			if(DeleteObject)
				for(i=Size-1;i>=NewSize;i--)
					Get(i).~T();
		}
		Size = NewSize;
	}

	int		GetSize() const
	{
		return Size;
	}
	int		GetReservedSize() const
	{
		return ReservedSize;
	}
	int		Add()
	{
		DYNARRAY_Z_WARNING(Size<ReservedSize);
		
		if (Size>=ReservedSize)
		{
			DYNARRAY_Z_EXP(!bExceptionWhenFull);
			if(DeleteObject)
				Get(ReservedSize-1).~T();
			if (InitObject)
				new(&Get(ReservedSize-1)) T();
			return ReservedSize-1;
		}

		if (InitObject)
			new(&Get(Size++)) T();
		else
			Size++;
		return Size-1;
	}

	int		Add(const T &Ele)
	{
		if (Size>=ReservedSize)
		{
			DYNARRAY_Z_EXP(!bExceptionWhenFull);
			DYNARRAY_Z_WARNING(Size<ReservedSize);
			if (InitObject)
				new(&Get(ReservedSize-1)) T(Ele);
			return ReservedSize-1;
		}

		if (InitObject)
			new(&Get(Size++)) T(Ele);
		else
		{
			Get(Size)=Ele;
			Size++;
		}
		return Size-1;
	}

	void	Insert(int Id,const T &Ele)
	{
		if (Id==(int)Size)
		{
			Add(Ele);
			return;
		}
		DYNARRAY_Z_EXP(Id<(int)Size && Id>=0 );
		if (Size>=ReservedSize)
		{
			DYNARRAY_Z_EXP(!bExceptionWhenFull);
			DYNARRAY_Z_WARNING(Size<ReservedSize);
			return;
		}
		memmove(&Get(Id+1),&Get(Id),(Size-Id)*sizeof(T));
		if (InitObject)
			new(&Get(Id)) T(Ele);
		else
			Get(Id)=Ele;
		Size++;
	}

	void	Insert(int Id)
	{
		if (Id==(int)Size)
		{
			Add();
			return;
		}
		DYNARRAY_Z_EXP(Id<(int)Size && Id>=0 );
		if (Size>=ReservedSize)
		{
			DYNARRAY_Z_EXP(!bExceptionWhenFull);
			DYNARRAY_Z_WARNING(Size<ReservedSize);
			return;
		}
		memmove(&Get(Id+1),&Get(Id),(Size-Id)*sizeof(T));
		if (InitObject)
			new(&Get(Id)) T();
		Size++;
	}

	T		&operator[](int Id)
	{
		DYNARRAY_Z_EXP(Id<(int)Size && Id>=0 );
		return 	Get(Id);
	}

	S32 Contains(const T &val) const 
	{
		int k=0;
		while(k<Size && !(Get(k)==val))
			k++;
		if(k==Size)
			return -1;
		else 
			return k;
	}

	const T	&operator[](int Id) const
	{
        DYNARRAY_Z_EXP( (((U32)Id)<(U32)Size) );
		return 	Get(Id);
	}

	StaticArray_Z<T,ReservedSize,DeleteObject,InitObject,bExceptionWhenFull>	&operator=(const StaticArray_Z<T,ReservedSize,DeleteObject,InitObject,bExceptionWhenFull> &Src)
	{
		if (this==&Src)
			return *this;
		Empty();
		U32 NewSize = Src.GetSize();
		SetSize(NewSize);
		
		const T	*pSrc = Src.GetArrayPtr();
		T	*pDst = GetArrayPtr();
		T	*pEnd = pDst+NewSize;

		while (pDst < pEnd)
			*pDst++ = *pSrc++;

		return *this;
	}
	StaticArray_Z<T,ReservedSize,DeleteObject,InitObject,bExceptionWhenFull>	&operator+=(const StaticArray_Z<T,ReservedSize,DeleteObject,InitObject,bExceptionWhenFull> &Src)
	{
		if (this==&Src)
			return *this;
		for(int i=0;i<Src.GetSize();i++)
			Add(Src[i]);
		return *this;
	}
	int GetObjectSize()
	{
		return 0;
	}
};

template<class T,S32 ReservedSize,Bool DeleteObject=TRUE,Bool InitObject = TRUE> class SafeArray_Z
{   
private:
	POSTALIGNED128_Z char	ArrayChar[ReservedSize * sizeof(T)]		ALIGNED128_Z;
	#ifdef	_DEBUG
		typedef T	Value[ReservedSize];
		Value *DebugPtr;
	#endif
	T		&Get(int Id)
	{
		return 	*(T *)(ArrayChar+Id*sizeof(T));
	}

	const T	&Get(int Id) const 
	{
		return 	*(T *)(ArrayChar+Id*sizeof(T));
	}

public:
	SafeArray_Z()
	{
		#ifdef	_DEBUG
			DebugPtr = reinterpret_cast <Value*>( ArrayChar );
		#endif
		if (InitObject)
		{
			for(int i=0;i<ReservedSize;i++)
				new(&Get(i)) T();
		}
	}
	~SafeArray_Z()
	{
		if(DeleteObject)
			for(int i=0;i<ReservedSize;i++)
				Get(i).~T();
	}
	SafeArray_Z(const SafeArray_Z<T,ReservedSize,DeleteObject,InitObject> &Src)
	{
		#ifdef	_DEBUG
			DebugPtr = reinterpret_cast <Value*>( ArrayChar );
		#endif
		if(InitObject)
		{
			for(S32 i=0;i<ReservedSize;i++)
			{
				new(&Get(i)) T();
				Get(i)=Src[i];
			}
		}
		else
			memcpy(ArrayChar,Src.ArrayChar,ReservedSize * sizeof(T));
	}
	Bool operator==(const SafeArray_Z& _compare) const
	{
		if(GetSize() != _compare.GetSize())
			return FALSE;

		for(S32 i=0;i<GetSize();i++)
		{
			if(Get(i)!=_compare.Get(i))
				return FALSE;
		}			
		return TRUE;
	}
	Bool operator!=(const SafeArray_Z& _compare) const
	{
		return !(*this==_compare);
	}
	T	*GetArrayPtr()
	{
		return (T *)ArrayChar;
	}
	const T	*GetArrayPtr()const
	{
		return (T *)ArrayChar;
	}
	int		GetSize() const
	{
		return ReservedSize;
	}
	int		GetEleSize() const
	{
		return	sizeof(T);
	}
	T		&operator[](int Id)
	{
        DYNARRAY_Z_EXP( (((U32)Id)<(U32)ReservedSize) );
		return 	Get(Id);
	}
	const T	&operator[](int Id) const
	{
        DYNARRAY_Z_EXP( (((U32)Id)<(U32)ReservedSize) );
		return 	Get(Id);
	}
	S32 Contains(const T &val,S32 _NbElement) const 
	{
		DYNARRAY_Z_EXP(_NbElement <= GetSize());
		int k = 0;
		T *Ptr = (T *)(ArrayChar);
		while (k<_NbElement)
		{
			if (*Ptr++ == val)
				return k;
			k++;
		}
		return -1;
	}
	void	Null()
	{
		memset(ArrayChar,0,sizeof(T)*ReservedSize);
	}
	SafeArray_Z<T,ReservedSize,DeleteObject,InitObject>	&operator=(const SafeArray_Z<T,ReservedSize,DeleteObject,InitObject> &Src)
	{
		if (this==&Src)
			return *this;
		if(InitObject)
		{
			for(S32 i=0;i<ReservedSize;i++)
				Get(i)=Src[i];
		}
		else
			memcpy(ArrayChar,Src.ArrayChar,ReservedSize * sizeof(T));
		return *this;
	}

	int GetObjectSize()const
	{
		return	0;
	}
};


template<class T,int ReservedSize> class SafeArrayPtr_Z
{   
private:
	T*	ArrayChar[ReservedSize];
public:
	T	**GetArrayPtr()
	{
		return ArrayChar;
	}
	T*const *GetArrayPtr()const
	{
		return ArrayChar;
	}
	int		GetSize() const
	{
		return ReservedSize;
	}
	T*		&operator[](int Id)
	{
		DYNARRAY_Z_EXP( (((U32)Id)<(U32)ReservedSize) );
		return 	ArrayChar[Id];
	}

	T* const	&operator[](int Id) const
	{
		DYNARRAY_Z_EXP( (((U32)Id)<(U32)ReservedSize) );
		return 	ArrayChar[Id];
	}
	void	Null()
	{
		memset(ArrayChar,0,sizeof(ArrayChar) );
	}
	int GetObjectSize()const
	{
		return	0;
	}
};

template<class T, S32 ReservedSize, Bool DeleteObject = TRUE, Bool InitObject = TRUE> class AutoStackArray_Z
{
private:
	T		*Tab;
	S32		Size;
public:
	AutoStackArray_Z()
	{
		Size = 0;
		Tab = NULL;
	}
	~AutoStackArray_Z()
	{
		Flush();
	}
	T	*GetArrayPtr()
	{
		return Tab;
	}
	void	Flush()
	{
		if (DeleteObject)
			for (int i = 0; i<Size; i++)
				Tab[i].~T();

		Free_Z(Tab);
		Size = 0;
		Tab = NULL;
	}
	int		GetSize() const
	{
		return Size;
	}
	T		&operator[](int Id)
	{
		if (((U32)Id) >= Size)
		{
			DYNARRAY_Z_EXP((Id==Size));
			S32 NewSize = Size + ReservedSize;

			if (!Size)
				Tab = (T*)Alloc_Z(NewSize * sizeof(T));
			else
				Tab = (T*)Realloc_Z(Tab,NewSize * sizeof(T));

			if (InitObject)
			{
				for (int i = Size; i<NewSize; i++)
					new(&Tab[i]) T();
			}
			Size = NewSize;
		}
		return 	Tab[Id];
	}
};

#if defined(_MSC_VER)
#pragma warning( pop )
#endif

#endif //_STATICARRAY_Z_H
