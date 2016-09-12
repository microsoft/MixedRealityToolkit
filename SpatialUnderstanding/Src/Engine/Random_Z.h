// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#ifndef _RANDOM_Z_H
#define _RANDOM_Z_H

#include <Types_Z.h>
#include <Assert_Z.h>

#include <BeginDef_Z.h>

// Returns an S32 between min and max (included)
EXTERN_Z S32	Alea(S32 min, S32 max);

// Returns a Float between min and max (excluded)
EXTERN_Z Float	AleaF(Float vmin, Float vmax);

//Random: Returns a Float between 0 and 1 (excluded)
EXTERN_Z Float	Random();


#define RANDOM_Z_NTAB 32
#define DEFAULT_SEED 666

class  Random_Z
{
private:
	S32 m_iy;
	S32 m_iv[RANDOM_Z_NTAB];
	S32 m_SEED;

	Float	Ran1(S32 *idum);

public:
	Random_Z();
	Random_Z(S32 aSeed);

	void	InitRandom(S32 aSeed);

	inline S32	Alea(S32 min, S32 max)
	{
		Float	range = ((Float)(max - min))+0.9999f;
		Float	val=Ran1(&m_SEED) * range;
		return min + (S32)FLOOR(val);
	}

	inline Float Random()
	{
		return Ran1(&m_SEED);
	}

	inline Float AleaF(Float vmin, Float vmax)
	{
		return vmin + Random() * (vmax - vmin);
	}
};


// Class For (Use gData.StaticRandom
// FOR BASIC => USE g_StaticRandom
// => See declaration after this template.

template<U32 Seed = 0x1234ABCD,U32 ComplexityPowerOf2 = 12> class StaticRandom_Z
{
private:
	U32		CurRandom;
	Float	TabRandom[1<<ComplexityPowerOf2];
public:
	StaticRandom_Z()
	{
		CurRandom = 0;
		U32 i;
		U32	Size = 1<<ComplexityPowerOf2;

		// First : Fill the list with all possibilty from 0 to 0.9999
		for (i=0 ; i<Size ; i++)
			TabRandom[i] = (Float)i / (Float)Size;

		// Second : Mixing for having random.
		Random_Z MyRandom(Seed);

		//InitRandom(Seed);
		for (i=0 ; i<(Size*2) ; i++)
		{
			S32 i1 = MyRandom.Alea(0,Size-1);
			S32 i2 = MyRandom.Alea(0,Size-1);

			Float v1 = TabRandom[i1];
			TabRandom[i1] = TabRandom[i2];
			TabRandom[i2] = v1;
		}
	}

	Float	RandFromU32(U32 _val)			//Returns a Float between 0 and 1 (excluded)
	{
		return TabRandom[_val & ((1<<ComplexityPowerOf2)-1)];
	}
	Float	RandFromUnityFloat(Float _val)	//Returns a Float between 0 and 1 (excluded)
	{
		return RandFromU32((U32)_val*(1<<ComplexityPowerOf2));
	}
	Float	RandFromFloat(Float _val,Float _min,Float _max) //Returns a Float between 0 and 1 (excluded)
	{
		return RandFromU32((U32)((_val-_min)/(_max-_min))*(1<<ComplexityPowerOf2));
	}

	Float	Rand() // Returns a Float between 0 and 1 (excluded)
	{
		CurRandom = (CurRandom+1) & ((1<<ComplexityPowerOf2)-1);
		return TabRandom[CurRandom];
	}
	Float	Rand(U32 Step) // Returns a Float between 0 and 1 (excluded)
	{
		Step |= 0x1;
		CurRandom = (CurRandom+Step) & ((1<<ComplexityPowerOf2)-1);
		return TabRandom[CurRandom];
	}
};

extern StaticRandom_Z<0x1234ABCD,12>	g_StaticRandom;

#endif
