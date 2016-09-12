// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#ifndef _BITARRAY_Z_H
#define _BITARRAY_Z_H

#include <DynArray_Z.h>
#include <HugeDynArray_Z.h>
#include <BeginDef_Z.h>

class  BitArray_Z
{
private:
	HU32DA	Bits;
	S32		Size;
public:
	BitArray_Z();
	BitArray_Z(S32 Size);
	BitArray_Z(S32 Size,Bool bAllBitsOn);
	virtual ~BitArray_Z();
	BitArray_Z(const BitArray_Z &_Rhs);
	BitArray_Z& operator=(const BitArray_Z &_Rhs);

	inline	HU32DA &GetDynArray()	{return Bits;}
	inline	void	SetDABits(const S32DA &DA,S32 _Size,Bool bClearAllBits=TRUE)
			{
				SetSize(_Size);
				if(bClearAllBits)	ClearAllBits();
				for(int i=0;i<DA.GetSize();i++)
					SetBit(DA[i]);
			}
	inline	void	SetDABits(const U16DA &DA,S32 _Size,Bool bClearAllBits=TRUE)
			{
				SetSize(_Size);
				if(bClearAllBits)	ClearAllBits();
				for(int i=0;i<DA.GetSize();i++)
					SetBit(DA[i]);
			}
	inline	S32		GetDABits(S32DA &DA,Bool bState=TRUE)
			{
				DA.Empty();
				S32		Id=FindFirstBit(bState);
				while(Id>=0)
				{
					DA.Add(Id);
					Id=FindFirstBit(bState,++Id);
				}
				return	DA.GetSize();
			}

	inline	S32 GetNbElements() const	{return Size;}
	inline	void	Minimize(void)
			{
				Bits.Minimize();
			}
	inline	void	Flush(void)
			{
				Bits.Flush();
				Size=0;
			}

	inline	void	SetSize(S32 _Size)
			{
				#ifndef _MASTER
				S32 oldSize = Size;
				#endif
				Size = _Size;
				S32 intSize = (_Size >> 5)+1;
				if (_Size && intSize > Bits.GetSize())
				{
					Bits.SetSize( intSize );
					#ifndef _MASTER
					//This is to make BoundsChecker happy
					if( oldSize == 0 )
					{
						for( S32 i=0 ; i < intSize ; ++i )
							Bits[i] = 0;
					}
					#endif
				}
			}
	inline	S32		GetSize(void)const
			{
				return Size;
			}

	// Bit Handling
	inline	U32 GetBit(S32	_Index) const
			{
				DYNARRAY_Z_EXP(_Index < Size);
				return Bits[_Index>>5] & (1 << (_Index & 0x1f));
			}

	// If Bits is On, set It and return 1L otherwise return 0L
	inline	U32	IsCleared(S32 _Index)
			{
				DYNARRAY_Z_EXP(_Index < Size);
				S32		BitsIndex=_Index>>5;
				S32		BitCpt=(1<<(_Index&0x1f));
				U32	&	Value=Bits[BitsIndex];
				if(Value&BitCpt)
				{
					Value^=BitCpt;
					return	1L;
				}
				return	0L;
			}
	//Inverse of IsCleared
	inline	U32	IsSet(S32 _Index)
			{
				DYNARRAY_Z_EXP(_Index < Size);
				S32		BitsIndex=_Index>>5;
				S32		BitCpt=(1<<(_Index&0x1f));
				U32	&	Value=Bits[BitsIndex];
				if(Value&BitCpt)
					return	0L;
				Value|=BitCpt;
				return	1L;
			}

			void RemoveBit(S32 _Index);

	inline	void SetBit(S32	_Index)
			{
				DYNARRAY_Z_EXP(_Index < Size);
				Bits[_Index>>5] |= (U32) (1 << (_Index & 0x1f));
			}

	inline	void ClearBit(S32 _Index)
			{
				DYNARRAY_Z_EXP(_Index < Size);
				Bits[_Index>>5] &=~ (U32) (1 << (_Index & 0x1f));
			}

	inline	void ToggleBit(S32	_Index)
			{
				DYNARRAY_Z_EXP(_Index < Size);
				Bits[_Index>>5] ^= (U32) (1 << (_Index & 0x1f));
			}

	inline	void ChangeBit(S32	_Index,Bool	_NewState)
			{
				DYNARRAY_Z_EXP(_Index < Size);
			
				if (_NewState)	SetBit(_Index);
				else			ClearBit(_Index);
			}

			void	SetBitRange(S32 _StartIndex, S32 _EndIndex);

			void	ClearBitRange(S32 _StartIndex, S32 _EndIndex);

			void	ToggleBitRange(S32 _StartIndex, S32 _EndIndex);

	inline	void	ChangeBitRange(S32 _StartIndex, S32 _EndIndex, Bool _NewState)
			{
				DYNARRAY_Z_EXP(_StartIndex < Size);
				DYNARRAY_Z_EXP(_EndIndex < Size);
				DYNARRAY_Z_EXP(_StartIndex <= _EndIndex);

				if (_NewState)	SetBitRange(_StartIndex,_EndIndex);
				else			ClearBitRange(_StartIndex,_EndIndex);
			}

			void	SetAllBits();
			void	ClearAllBits();
			void	ToggleAllBits();

	inline	void	ChangeAllBits(Bool _NewState)
			{
				// Exit if there are no bits.
				if (!Size) return;
				if (_NewState) SetAllBits();
				else ClearAllBits();
			}

			S32		CountSetBitsInRange(S32 _StartIndex, S32 _EndIndex) const;

			S32		CountSetBits() const;

			S32		FindFirstBit(Bool _State=TRUE,S32 _FirstBitToCheck=0) const;
			S32		FindLastBit(Bool _State,S32 _FirstBitToCheck) const;
	inline	S32		GetObjectSize()const		{ return Bits.GetObjectSize(); }
};
typedef	DynArray_Z<BitArray_Z,32>	BitArray_ZDA;
#endif
