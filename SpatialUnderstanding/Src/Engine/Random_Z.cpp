// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include <pch.h>
#include <Math_Z.h>
#include <Random_Z.h>

// Global Object Random
static Random_Z	gRandom(DEFAULT_SEED);

// Static Random
StaticRandom_Z<0x1234ABCD,12>	g_StaticRandom;

S32	Alea(S32 min, S32 max)
{
	return gRandom.Alea(min,max);
}

Float	AleaF(Float vmin, Float vmax)
{
	return gRandom.AleaF(vmin,vmax);
}

Float	Random()
{
	return gRandom.Random();
}

#define IA 16807
#define IM 2147483647
#define AM (1.0f/IM)
#define IQ 127773
#define IR 2836
#define NDIV (1+(IM-1)/RANDOM_Z_NTAB)
#define EPSILON 1.2e-7f
#define RNMX (1.0f-EPSILON)
/*
"Minimal" random number generator of Park and Miller with Bays-Durham shu.e and added
safeguards. Returns a uniform random deviate between 0.0 and 1.0 (exclusive of the endpoint
values). Call with idum a negative integer to initialize; thereafter, do not alter idum between
successive deviates in a sequence. RNMX should approximate the largest floating value that is
less than 1.
*/
Float Random_Z::Ran1(S32 *idum)
{
	S32 j;
	S32 k;

	Float temp;

	if (*idum <= 0 || !m_iy)
	{
		if (-(*idum) < 1)
			*idum=1;
		else
			*idum = -(*idum);
		for (j=RANDOM_Z_NTAB+7;j>=0;j--)
		{
			k=(*idum)/IQ;
			*idum=IA*(*idum-k*IQ)-IR*k;
			if (*idum < 0)
				*idum += IM;
			if (j < RANDOM_Z_NTAB)
				m_iv[j] = *idum;
		}
		m_iy=m_iv[0];
	}
	k=(*idum)/IQ;
	*idum=IA*(*idum-k*IQ)-IR*k;
	if (*idum < 0)
		*idum += IM;
	j=m_iy/NDIV;
	m_iy=m_iv[j];
	m_iv[j] = *idum;
	if ((temp=(Float)(AM*m_iy)) > RNMX)
		return (Float)RNMX;
	else
		return temp;
}

Random_Z::Random_Z()
{
	m_iy=0;
	InitRandom(DEFAULT_SEED);
}
Random_Z::Random_Z(S32 aSeed)					
{
	m_iy=0;
	InitRandom(aSeed);
}

void Random_Z::InitRandom(S32 aSeed)
{
	ASSERT_Z(aSeed);
	if (aSeed < 0)
		m_SEED = aSeed;
	else
		m_SEED = -aSeed;

	Ran1(&m_SEED);
}

