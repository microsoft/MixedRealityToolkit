// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include <pch.h>
#include <Math_Z.h>

// Accurate normalize:
//	len1 = length1(v)
//	if len1 == 0 
//		return FALSE
//	v' = v' / len1
//	v = v' / length2(v')
//	return TRUE
Bool Vec3f::ANormalize()
{
	Float	ax=POS_Z(x);
	Float	ay=POS_Z(y);
	Float	az=POS_Z(z);
	if (ax>ay)
	{
		if (ax>az)
		{
			Float	i=1.f/ax;
			y*=i; z*=i;
			Float	l=InvSqrt(1.f,y*y+z*z+1.f);
			x=(float)SIGN_Z(x); x*=l; y*=l; z*=l;
		}
		else
		{
zlargest:
			Float	i=1.f/az;
			x*=i; y*=i;
			Float	l=InvSqrt(1.f,x*x+y*y+1.f);
			z=(float)SIGN_Z(z); x*=l; y*=l; z*=l;
		}
	}
	else
	{
		if (ay>az)
		{
			Float	i=1.f/ay;
			x*=i; z*=i;
			Float	l=InvSqrt(1.f,x*x+z*z+1.f);
			y=(float)SIGN_Z(y); x*=l; y*=l; z*=l;
		}
		else if (az<=0.f)	// actually only az==0.0f could happen...
			return FALSE;
		else
			goto	zlargest;
	}
	return TRUE;
}

void	Vec3f::Get1OrthoVector(Vec3f &_n1) const
{
	if (y>0.99f || y<-0.99f)
	{
		_n1.x = 0.f;
		_n1.y = z;
		_n1.z = -y;

		_n1.Normalize();
	}
	else
	{
		_n1.x = z;
		_n1.y = 0.f;
		_n1.z = -x;

		_n1.Normalize();
	}
}

void	Vec3f::Get2OrthoVector(Vec3f &_n1,Vec3f &_n2) const
{
	Get1OrthoVector(_n1);

	_n2 = _n1^(*this);
	_n2.Normalize();
}

void Vec3f::Get2OrthoVectorFrom(Vec3f &_n1, Vec3f &_n2,const Vec3f &_oldn2) const
{
	_n1 = *this^_oldn2;
	_n2 = _n1^*this;

	float norme1=_n1.GetNorm();
	if (norme1<Float_Eps)
	{
		this->Get2OrthoVector(_n1,_n2);
		return;
	}
	float norme2=_n2.GetNorm();
	if (norme2<Float_Eps)
	{
		this->Get2OrthoVector(_n1,_n2);
		return;
	}
	_n1 *= (1.f/norme1);
	_n2 *= (1.f/norme2);
}

void Vec3f::Lerp(const Vec3f &_v1,const Vec3f &_v2,Float s)
{
	*this = _v1 + (_v2-_v1)*s;
}