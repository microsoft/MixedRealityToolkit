// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include <pch.h>
#include <BitArray_Z.h>

BitArray_Z::BitArray_Z()
{
	Size = 0;
}

BitArray_Z::~BitArray_Z()
{
}

BitArray_Z::BitArray_Z(S32 _Size)
{
	Size = 0;
	SetSize(_Size);
}
BitArray_Z::BitArray_Z(S32 _Size,Bool bAllBitsOn)
{
	Size = 0;
	SetSize(_Size);
	if(bAllBitsOn)	
		SetAllBits();
	else
		ClearAllBits();
}

BitArray_Z::BitArray_Z(const BitArray_Z &_Rhs)
{
	DYNARRAY_Z_EXP(&_Rhs!=this);
	Bits = _Rhs.Bits;
	Size = _Rhs.Size;
}

BitArray_Z& BitArray_Z::operator=(const BitArray_Z &_Rhs)
{
	DYNARRAY_Z_EXP(&_Rhs!=this);
	Bits = _Rhs.Bits;
	Size = _Rhs.Size;
	return *this;
}

void BitArray_Z::SetBitRange(S32 _StartIndex,S32 _EndIndex)
{
	DYNARRAY_Z_EXP(_StartIndex < Size);
	DYNARRAY_Z_EXP(_EndIndex < Size);
	DYNARRAY_Z_EXP(_StartIndex <= _EndIndex);

	S32 BitsIndex		=_StartIndex>>5;
	S32	BitCpt			=_StartIndex&31;
	U32	Bit				=(1<<BitCpt)-1;
	S32 BitsIndexE		=_EndIndex>>5;
	S32	BitCptE			=_EndIndex&31;
	U32	BitE			=~(((1<<BitCptE)<<1)-1);
	if(BitsIndex==BitsIndexE)
	{
		Bits[BitsIndex]|=~(Bit|BitE);
		return;
	}
	Bits[BitsIndex]|=~Bit;
	if (BitsIndex<BitsIndexE)
	{
		BitsIndex++;
		for (;BitsIndex<BitsIndexE;Bits[BitsIndex]=0xffffffff,BitsIndex++);
	}
	Bits[BitsIndexE]|=~BitE;
}

void BitArray_Z::ClearBitRange(S32	_StartIndex,S32	_EndIndex)
{
	DYNARRAY_Z_EXP(_StartIndex < Size);
	DYNARRAY_Z_EXP(_EndIndex < Size);
	DYNARRAY_Z_EXP(_StartIndex <= _EndIndex);

	S32 BitsIndex		=_StartIndex>>5;
	S32	BitCpt			=_StartIndex&31;
	U32	Bit				=(1<<BitCpt)-1;
	S32 BitsIndexE		=_EndIndex>>5;
	U32	BitCptE			=_EndIndex&31;
	U32	BitE			=~(((1<<BitCptE)<<1)-1);
	if(BitsIndex==BitsIndexE)
	{
		Bits[BitsIndex]&=Bit|BitE;
		return;
	}
	Bits[BitsIndex]&=Bit;
	if (BitsIndex<BitsIndexE)
	{
		BitsIndex++;
		for (;BitsIndex<BitsIndexE;Bits[BitsIndex]=0,BitsIndex++);
	}
	Bits[BitsIndexE]&=BitE;
}

void BitArray_Z::ToggleBitRange(S32 _StartIndex,S32 _EndIndex)
{
	DYNARRAY_Z_EXP(_StartIndex < Size);
	DYNARRAY_Z_EXP(_EndIndex < Size);
	DYNARRAY_Z_EXP(_StartIndex <= _EndIndex);

	S32 BitsIndex		=_StartIndex>>5;
	S32	BitCpt			=_StartIndex&31;
	U32	Bit				=(1<<BitCpt)-1;
	S32 BitsIndexE		=_EndIndex>>5;
	S32	BitCptE			=_EndIndex&31;
	U32	BitE			=~(((1<<BitCptE)<<1)-1);
	if(BitsIndex==BitsIndexE)
	{
		Bits[BitsIndex]^=~(Bit|BitE);
		return;
	}
	Bits[BitsIndex]^=~Bit;
	if (BitsIndex<BitsIndexE)
	{
		BitsIndex++;
		for (;BitsIndex<BitsIndexE;Bits[BitsIndex]^=0xffffffff,BitsIndex++);
	}
	Bits[BitsIndexE]^=~BitE;
}

S32 BitArray_Z::CountSetBitsInRange(S32	_StartIndex,S32	_EndIndex) const
{
	DYNARRAY_Z_EXP(_StartIndex < Size);
	DYNARRAY_Z_EXP(_EndIndex < Size);
	DYNARRAY_Z_EXP(_StartIndex <= _EndIndex);

	S32 NrSetBits = 0;
	for(S32 Index = _StartIndex; Index <= _EndIndex; Index++)
		if (GetBit(Index)) NrSetBits++;

	return NrSetBits;
}

S32 BitArray_Z::CountSetBits() const
{
	S32 NrSetBits = 0;

	for(S32 Index = 0; Index < Size; Index++)
		if (GetBit(Index)) NrSetBits++;

	return NrSetBits;
}

void BitArray_Z::SetAllBits()
{
	U32	*Ptr=Bits.GetArrayPtr();
	S32	Nb=Bits.GetSize();
	while(Nb--)
		*Ptr++=0xffffffff;
}

void BitArray_Z::ClearAllBits()
{
	U32	*Ptr=Bits.GetArrayPtr();
	S32	Nb=Bits.GetSize();
	while(Nb--)
		*Ptr++=0L;
}

void BitArray_Z::ToggleAllBits()
{
	for(S32 Index=0;Index<Bits.GetSize();Index++)
		Bits[Index]^=-1;
}

S32	BitArray_Z::FindFirstBit(Bool _State,S32 _FirstBitToCheck) const
{
	DYNARRAY_Z_EXP(_FirstBitToCheck <= Size);
	if(_FirstBitToCheck>=Size)	return	-1;

	int Index;
	S32 BitsIndex		=_FirstBitToCheck>>5;
	S32	BitCpt			=_FirstBitToCheck&31;
	int	CurBits;	
	int	s=Bits.GetSize();
	if (BitCpt)
	{
		U32	Bit				=1<<BitCpt;
		CurBits=Bits[BitsIndex];
		if (!_State)
			CurBits=~CurBits;
		CurBits&=~(Bit-1);
		if (CurBits)
		{
			int x=( CurBits) & (-( CurBits));	// premier bit � "1"
			int a=0;
			if (x & 0xFFFF0000) a += 16; 
			if (x & 0xFF00FF00) a += 8; 
			if (x & 0xF0F0F0F0) a += 4; 
			if (x & 0xCCCCCCCC) a += 2; 
			if (x & 0xAAAAAAAA) a += 1;
			Index=(BitsIndex<<5)+a;
			if(Index>=Size)	return	-1;
			return	Index;
		}
		BitsIndex++;
	}
	if (_State)
	{
		for(;(BitsIndex<s)&&(!(CurBits=Bits[BitsIndex]));BitsIndex++);
		if (BitsIndex<s)
		{
			int x=( CurBits) & (-( CurBits));	// premier bit � "1"
			int a=0;
			if (x & 0xFFFF0000) a += 16; 
			if (x & 0xFF00FF00) a += 8; 
			if (x & 0xF0F0F0F0) a += 4; 
			if (x & 0xCCCCCCCC) a += 2; 
			if (x & 0xAAAAAAAA) a += 1;
			Index=(BitsIndex<<5)+a;
			if(Index>=Size)	return	-1;
			return	Index;
		}
	}
	else
	{
		for(;(BitsIndex<s)&&(!(CurBits=~Bits[BitsIndex]));BitsIndex++);
		if (BitsIndex<s)
		{
			int x=( CurBits) & (-( CurBits));	// premier bit � "1"
			int a=0;
			if (x & 0xFFFF0000) a += 16; 
			if (x & 0xFF00FF00) a += 8; 
			if (x & 0xF0F0F0F0) a += 4; 
			if (x & 0xCCCCCCCC) a += 2; 
			if (x & 0xAAAAAAAA) a += 1;
			Index=(BitsIndex<<5)+a;
			if(Index>=Size)	return	-1;
			return	Index;
		}
	}
	return -1;
}

S32	BitArray_Z::FindLastBit(Bool _State,S32 _FirstBitToCheck) const
{
	DYNARRAY_Z_EXP(_FirstBitToCheck <= Size);
	while(_FirstBitToCheck>=0)
	{
		if(_State)
		{
			if(GetBit(_FirstBitToCheck))
				break;
		}
		else
		{
			if(!GetBit(_FirstBitToCheck))
				break;
		}
		_FirstBitToCheck--;
	}
	return	_FirstBitToCheck;
}

void BitArray_Z::RemoveBit(S32 _Index)
{
	DYNARRAY_Z_EXP(_Index < Size);

	S32 Index=_Index>>5;
	S32	BitIndex=_Index&0x1f;

	S32 Bit=(1<<BitIndex);

	S32	Low= BitIndex ? (Bits[Index]& ( Bit-1 ) ) : 0;
	S32	Hi = (BitIndex==0x1f) ? 0 : Bits[Index]& ( (Bit|(Bit-1)) ^0xFFFFFFFF);
		Hi &=~0x8000000;

	Bits[Index]=(Hi>>1)|Low;

	for(++Index ;Index < Bits.GetSize(); Index++)
	{
		if(Bits[Index]&1)	
			Bits[Index-1]|=1<<31;

		Bits[Index]>>=1;
	}
	SetSize(GetSize()-1);
}
