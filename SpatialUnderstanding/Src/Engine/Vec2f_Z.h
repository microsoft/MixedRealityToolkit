// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#ifndef _MATH_VECTOR2D_Z_H
#define _MATH_VECTOR2D_Z_H

#include <BeginDef_Z.h>

struct Vec3f;

//----------------------------------------------------------------
// Vec2f
//----------------------------------------------------------------

struct  Vec2f
{
	// Data
	Float x,y;
	// Init Method
#ifdef _DEBUG
	Vec2f()
	{
		// memset(this, 0xFF, sizeof(*this)); TROP LENT EN DEBUG !!!
		U32 *ptr = (U32*)this;
		*ptr++ = 0xFFFFFFFF;
		*ptr = 0xFFFFFFFF;
	}
#else
	Vec2f()								{}
#endif

			Vec2f( const Float _xy) : x(_xy ),y( _xy ) {}
			Vec2f( const Float _x, const Float _y) : x(_x ),y( _y ) {}
			Vec2f(const Vec3f &_o);
	Vec2f	&Set(const Float _x, const Float _y)				{x=_x;y=_y;return *this;}
	Vec2f	&Set(const Vec2f &_v)				  {x=_v.x;y=_v.y; return *this;}			
	// Operator
	Vec2f	&operator=(const Vec2f &_v)			  {x=_v.x; y=_v.y; return *this;}
	Vec2f	&operator=(const Vec3f &_v);
	Vec2f	operator+(const Vec2f &_v) const	  {return Vec2f(x+_v.x,y+_v.y);}
	Vec2f	&operator+=(const Vec2f &_v)		  {x+=_v.x;y+=_v.y; return *this;}
	Vec2f	operator+() const					  {return *this;}
	Vec2f	operator-(const Vec2f &_v) const	  {return Vec2f(x-_v.x,y-_v.y);}
	Vec2f	&operator-=(const Vec2f &_v)		  {x-=_v.x;y-=_v.y; return *this;}
	Vec2f	operator-() const					  {return Vec2f(-x,-y);}
	Vec2f	operator*(Float _f)	const			  {return Vec2f(x*_f,y*_f);}
	Vec2f	&operator*=(Float _f)				  {x*=_f;y*=_f; return *this;}
	Float	operator*(const Vec2f &_v) const	  {return x*_v.x+y*_v.y;}
	Vec2f	operator/(Float _f)	const			  {ASSERT_Z(_f!=0.f);float inv=1.f/_f;return Vec2f(x*inv,y*inv);}
	Vec2f	&operator/=(Float _f)				  {ASSERT_Z(_f!=0.f);float inv=1.f/_f;x*=inv;y*=inv;return *this;}
	Vec2f	operator/(const Vec2f &_v)	const	  {ASSERT_Z(_v.x!=0.f&&_v.y!=0.f);return Vec2f(x/_v.x,y/_v.y);}
	Vec2f	&operator/=(const Vec2f &_v)		  {ASSERT_Z(_v.x!=0.f&&_v.y!=0.f);x/=_v.x;y/=_v.y;return *this;}
	Float	operator^(const Vec2f &_v) const	  {return x*_v.y-y*_v.x;}
	Vec2f	operator&(const Vec2f &_v) const	  {return Vec2f(x*_v.x, y*_v.y);}
	Float	&operator[](int _i)					  {ASSERT_Z(_i>=0&&_i<2);return (&x)[_i];}	
	const Float	&operator[](int _i) const		  {ASSERT_Z(_i>=0&&_i<2);return (&x)[_i];}
	Bool	operator==(const Vec2f& v)	const	  {Vec2f Diff=*this-v;return (Abs(Diff.x)<Float_Eps)&&(Abs(Diff.y)<Float_Eps);}
	Bool	operator!=(const Vec2f& v)	const	  { return !operator==(v); };
	// Utils
	Float	GetNorm2() const					  {return (*this)*(*this);}
	Float	GetNorm() const						  {return Sqrt(GetNorm2());}
	Vec2f	&Normalize()						  {return (*this)/=GetNorm();}
	Bool	CNormalize(const Float NormValue = 1.f)	// J'ai modifi� le CNormalize pour que l'on puisse normer avec autre chose que 1... Plus rapide ! A faire en Vec3f
	{
		Float	n=(*this)*(*this);
		if (n > Float_Eps)	// Using Float_Eps generates precision problems
		{
			(*this)*=InvSqrt(NormValue,n);
			return TRUE;
		}
		return FALSE;
	}
};

inline Vec2f operator*(Float _f, const Vec2f &_v)		{return  _v*_f;}

EXTERN_Z Float	CalcArea2D(Vec2f *Shape,S32 NbPoints);
EXTERN_Z Float	SignedCalcArea2D(Vec2f *Shape,S32 NbPoints);
EXTERN_Z void	RotateVector2D(Vec2f &vec, Float Angle);

const EXTERN_Z Vec2f VEC2F_NULL;
const EXTERN_Z Vec2f VEC2F_HALF;
const EXTERN_Z Vec2f VEC2F_ONE;
const EXTERN_Z Vec2f VEC2F_RIGHT;
const EXTERN_Z Vec2f VEC2F_DOWN;


#endif //_MATH_VECTOR2D_Z_H
