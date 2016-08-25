// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#ifndef __MATH_INLINE_ENGINE_Z_HH__
#define __MATH_INLINE_ENGINE_Z_HH__

#include <Types_Z.h>

#include <BeginDef_Z.h>

EXTERN_Z	Bool	TESTFTOLMODE();
EXTERN_Z	void	CHECKFTOLMODE();

FINLINE_Z	U32	UFTOL(float a)
{
#ifdef	_PC
	U32	b;
#ifndef _M_X64
	__asm fld a;
	__asm fistp b;
#else
	b = _mm_cvtss_si32( _mm_load_ss( &a ) );
#endif
	return b;
#endif
}

///////////////////// en attendant les optim pour chaque plate-forme...
FINLINE_Z void Vec3_Normalize( Vec3f& _result, const Vec3f& _v )
{
	const float length = sqrtf( _v.x * _v.x + _v.y * _v.y + _v.z * _v.z );
	_result.x = _v.x / length;
	_result.y = _v.y / length;
	_result.z = _v.z / length;

}

// _result = _v0+_v1;
FINLINE_Z void Vec3_Add( Vec3f& _result, const Vec3f& _v0, const Vec3f& _v1 )
{
	_result.x = _v0.x + _v1.x;
	_result.y = _v0.y + _v1.y;
	_result.z = _v0.z + _v1.z;
}

// _result = _v0-_v1;
FINLINE_Z void Vec3_Sub( Vec3f& _result, const Vec3f& _v0, const Vec3f& _v1 )
{
	_result.x = _v0.x - _v1.x;
	_result.y = _v0.y - _v1.y;
	_result.z = _v0.z - _v1.z;
}

// _result = _v0*_v1;
FINLINE_Z void Vec3_Mul( Vec3f& _result, const Vec3f& _v0, const Vec3f& _v1 )
{
	_result.x = _v0.x * _v1.x;
	_result.y = _v0.y * _v1.y;
	_result.z = _v0.z * _v1.z;
}

// _result = _v0*_s;
FINLINE_Z void Vec3_Scale( Vec3f& _result, const Float _s, const Vec3f& _v0 )
{
	_result.x = _v0.x * _s;
	_result.y = _v0.y * _s;
	_result.z = _v0.z * _s;
}

// _result = _v0/_v1;
FINLINE_Z void Vec3_Div( Vec3f& _result, const Vec3f& _v0, const Vec3f& _v1 )
{
	_result.x = _v0.x / _v1.x;
	_result.y = _v0.y / _v1.y;
	_result.z = _v0.z / _v1.z;
}

// _result = _v0^_v1;
FINLINE_Z void Vec3_Cross( Vec3f& _result, const Vec3f& _v0, const Vec3f& _v1 )
{
    _result.x = _v0.y * _v1.z - _v0.z * _v1.y;
    _result.y = _v0.z * _v1.x - _v0.x * _v1.z;
    _result.z = _v0.x * _v1.y - _v0.y * _v1.x;
}

// _result = _v0._v1;
FINLINE_Z Float Vec3_Dot( const Vec3f& _v0, const Vec3f& _v1 )
{
	return _v0.x * _v1.x + _v0.y * _v1.y + _v0.z * _v1.z;
}

// Fast Math functions
// -------------------
FINLINE_Z Float Vec3_GetNorm2( const Vec3f& _v )
{
	return _v.x * _v.x + _v.y * _v.y + _v.z * _v.z;
}

FINLINE_Z Float Vec3_GetNorm( const Vec3f& _v )
{
	return sqrtf( _v.x * _v.x + _v.y * _v.y + _v.z * _v.z );
}
//////////////////

// Implements all platform independent math inline functions/method

FINLINE_Z	Float	PushDownPos(Float val)
{
	EXCEPTION_Z(val>-Float_Eps);
	return val*val;
}

FINLINE_Z	Float	PushDown(Float val)
{
	return val*Abs(val);
}
FINLINE_Z	Float	PushDown2(Float val)
{
	return val*val*val*Abs(val);
}
FINLINE_Z	Float	PushDown2Pos(Float val)
{
	EXCEPTION_Z(val>-Float_Eps);
	val*=val;
	return val*val;
}
FINLINE_Z	Float	PushDown3(Float val)
{
	Float	val2=val*val;
	return val2*val2*val2*val*Abs(val);
}

FINLINE_Z	Float	PushDown3Pos(Float val)
{
	EXCEPTION_Z(val>-Float_Eps);
	val*=val;
	val*=val;
	return val*val;
}
FINLINE_Z	Float	PushUp(Float val)
{
	return val * ( 2.f - Abs ( val ) );
}

FINLINE_Z	Float	FlattenUp(Float val)
{
	if (val<0.f)
	{
		val=-val;
		val=val/(1.f+(val*val));
		val=-val;
	}
	else
		val=val/(1.f+(val*val));
	return val;
}

FINLINE_Z	Float	PushUpPos(Float val)
{
	EXCEPTION_Z(val>-Float_Eps);
	return val * ( 2.f - val );
}

FINLINE_Z	Float	PushUp2(Float val)
{
	Float	aval=Abs(val);
	return val*(4.f-6.f*aval+val*val*(4.f-aval));
}

FINLINE_Z	Float	PushUp2Pos(Float val)
{
	EXCEPTION_Z(val>-Float_Eps);
	return val*(4.f-6.f*val+val*val*(4.f-val));
}

FINLINE_Z	Float	PushUp3(Float val)
{
	if (val>0.f)
	{
		val=1.f-val;
		val*=val;
		val*=val;
		val*=val;
		val=1.f-val;
	}
	else
	{
		val=1.f+val;
		val*=val;
		val*=val;
		val*=val;
		val=val-1.f;
	}
	return val;
}

FINLINE_Z	Float	PushUpY(Float val,Float y)
{
	if (val>0.f)
	{
		val=1.f-val;
		val=powf(Max(0.f,val),y);
		val=1.f-val;
	}
	else
	{
		val=1.f+val;
		val=powf(Max(0.f,val),y);
		val=val-1.f;
	}
	return val;
}

FINLINE_Z	Float	PPowf(Float x,Float y)
{
	if (x<0.f)
		return -Powf(-x,y);
	return Powf(x,y);
}

FINLINE_Z	Float	PushDownY(Float val,Float y)
{
	return PPowf(val,y);
}

FINLINE_Z	Float	PushEdgeY(Float h,Float y)
{
	h*=2.f; h-=1.f;
	h=PushUpY(h,y);
	h*=0.5f; h+=0.5f;
	return h;
}

FINLINE_Z	Float	PushEdge(Float h)
{
	h*=2.f; h-=1.f;
	h=PushUp(h);
	h*=0.5f; h+=0.5f;
	return h;
}

FINLINE_Z	Float	PushMid(Float h)
{
	h*=2.f; h-=1.f;
	h=PushDown(h);
	h*=0.5f; h+=0.5f;
	return h;
}

FINLINE_Z	Float	PushEdge2(Float h)
{
	h*=2.f; h-=1.f;
	h=PushUp2(h);
	h*=0.5f; h+=0.5f;
	return h;
}

FINLINE_Z	Float	PushEdge3(Float h)
{
	h*=2.f; h-=1.f;
	h=PushUp3(h);
	h*=0.5f; h+=0.5f;
	return h;
}

#endif
