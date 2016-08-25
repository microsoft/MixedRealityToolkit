// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include <pch.h>
#include <Math_Z.h>

// ACCURATE NORMALIZE
// Un peu plus pr�cis que le normalize normal quand le vecteur est tr�s petit.

Bool Vec4f::ANormalize()
{
	Vec4f	Temp;
	Vec4_Fabs(Temp,*this);

	Float	l;
	if (Temp.x>Temp.y)
	{
		if (Temp.x>Temp.z)
		{
			Float	i=1.f/Temp.x;
			y*=i; z*=i;
			l=InvSqrt(1.f,y*y+z*z+1.f);
			x=(float)SIGN_Z(x);
		}
		else
		{
zlargest:
			Float	i=1.f/Temp.z;
			x*=i; y*=i;
			l=InvSqrt(1.f,x*x+y*y+1.f);
			z=(float)SIGN_Z(z);
		}
	}
	else
	{
		if (Temp.y>Temp.z)
		{
			Float	i=1.f/Temp.y;
			x*=i; z*=i;
			l=InvSqrt(1.f,x*x+z*z+1.f);
			y=(float)SIGN_Z(y);
		}
		else if (Temp.z<=0.f)
			return FALSE;
		else
			goto	zlargest;
	}
	Vec4_Scale(*this,l,*this);
	return TRUE;
}

void	Vec4f::Get2OrthoVector(Vec4f &_n1,Vec4f &_n2) const
{
	VecFloatBuildOrthoBase3( vec128, _n1.vec128, _n2.vec128 );
    _n1.w=0.f; // faut voir si c'est n�cessaire �a
	_n2.w=0.f;
}

void Vec4f::Get2OrthoVectorFrom(Vec4f &_n1, Vec4f &_n2,const Vec4f &_oldn2) const
{
	Vec4_Cross(_n1,*this,_oldn2);
	_n1.w=0.f;
	Vec4_Cross(_n2,_n1,*this);
	_n2.w=0.f;

	float norme1=Vec4_GetNorm(_n1);
	if (norme1<Float_Eps)
	{
		this->Get2OrthoVector(_n1,_n2);
		return;
	}
	float norme2=Vec4_GetNorm(_n2);
	if (norme2<Float_Eps)
	{
		this->Get2OrthoVector(_n1,_n2);
		return;
	}

	Vec4_Scale(_n1,1.f/norme1,_n1);
	Vec4_Scale(_n2,1.f/norme2,_n2);
}
