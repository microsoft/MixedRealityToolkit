// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include <DirectXMath.h>

#ifndef _M_X64
	// Not compatible X64 :(
	#define _USE_SQRT_DIRECT_CALL	1		// pb du _exit_math => 5x à 6x plus rapide comme ça.
#endif

#ifdef _USE_SQRT_DIRECT_CALL
double __forceinline __fastcall Sqrt(double n)
{
	register double result;
	_asm fld n
	_asm fsqrt
	_asm fstp result
	return result;
}

	FINLINE_Z	Float Sqrt(Float a)				{ return (Float)Sqrt((double)a); }
	FINLINE_Z	Float InvSqrt(Float x, Float y)	{ return x / (Float)Sqrt((double)y); }
#else
	FINLINE_Z	Float Sqrt(Float a)				{ return sqrtf(a); }
	FINLINE_Z	Float InvSqrt(Float x, Float y)	{ return x / sqrtf(y); }
#endif

FINLINE_Z	Float Atan2(Float a,Float b)	{return atan2f(a,b);}

FINLINE_Z	Float Cos(Float x)
{
#if ( _MSC_VER >= 1700 ) // __libm_sse2_cosf is buggy!
    return DirectX::XMScalarCos( x );
#else
	return cosf(x);
#endif
}

FINLINE_Z	Float Sin(Float x)
{
#if ( _MSC_VER >= 1700 ) // __libm_sse2_sinf is buggy!
    return DirectX::XMScalarSin( x );
#else
	return sinf(x);
#endif
}

FINLINE_Z	void  SinCos(Vec2f& r,Float x)	
{
    return DirectX::XMScalarSinCos( &r.x, &r.y, x );
}

FINLINE_Z	Float Tan(Float a)				{return Sin(a)/Cos(a);}

FINLINE_Z	Float ACos(Float value)
{
#ifndef	_MASTER
	EXCEPTIONC_Z(value<=1.f,"ACos(%f)", value);
	EXCEPTIONC_Z(value>=-1.f,"ACos(%f)", value);
#endif
	return DirectX::XMScalarACos( value );
}

FINLINE_Z	Float ASin(Float value)
{
#ifndef	_MASTER
	EXCEPTIONC_Z(value<=1.f,"ASin(%f)",value);
	EXCEPTIONC_Z(value>=-1.f,"ASin(%f)",value);
#endif
	return DirectX::XMScalarASin( value );
}

FINLINE_Z	Float ATan(Float x)				{return atanf(x);}
FINLINE_Z	Float Powf(Float x,Float e)		{return powf(x,e);}

#ifdef _M_X64
#include <xmmintrin.h>
#endif

FINLINE_Z	int	FTOL(float a)
{
	int	b;
#ifndef _M_X64
	__asm fld a;
	__asm fistp b;
#else
	b = _mm_cvtss_si32( _mm_load_ss( &a ) );
#endif
	return b;
}

FINLINE_Z	int	TRUNCINT(float a)
{
	int	b;
#ifndef _M_X64
	b = ( a >= 0 ) ? FTOL(a) : -FTOL(-a);
#else
	b = _mm_cvttss_si32( _mm_load_ss( &a ) );
#endif
	return b;
}

FINLINE_Z	int	FLOORINT(float a)
{
	return FTOL( a );
}

FINLINE_Z	float	FLOORF(float a)
{
	return static_cast <float> ( FLOORINT(a) );
}

FINLINE_Z	float	FRACTF(float a)
{
	return a - FLOORF( a );
}
FINLINE_Z	float	CEILFF(float a)
{
	float c = FLOORF( a );
	if (a>c)
		c++;
	return c;
}
FINLINE_Z	void	FLOORF_CEILFF(float a,float &c,float &d)
{
	d=c=FLOORF(a);
	if (a>c)
		d++;
}
FINLINE_Z	float	TRUNCRANGE(float a,float d,float invd)
{
	return a-FLOORF(a*invd)*d;
}
FINLINE_Z	Float	ClampVal(Float val)
{
	if (val>1.f) val=1.f;
	if (val<0.f) val=0.f;
	return val;
}

