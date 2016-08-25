// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#ifndef _MATH_VECTOR3D_Z_H
#define _MATH_VECTOR3D_Z_H

#include <Types_Z.h>
#include <BeginDef_Z.h>

struct Quat;
struct Color;
struct Vec4f;

//----------------------------------------------------------------
// Vec3f - Mainly used for storage
// Naive operations
//----------------------------------------------------------------

struct  Vec3f
{
	// Data
	Float x,y,z;

	// Init Method
#ifdef _DEBUG
	Vec3f()
	{
		// memset(this, 0xFF, sizeof(*this)); TROP LENT EN DEBUG !!!
		U32 *ptr = (U32*)this;
		*ptr++ = 0xFFFFFFFF;
		*ptr++ = 0xFFFFFFFF;
		*ptr = 0xFFFFFFFF;
	}
#else
	Vec3f()	{}
#endif
	explicit Vec3f( const Float _xyz );
	Vec3f( const Float _x, const Float _y, const Float _z );
	Vec3f( const VecFloat4&_v );
	////////////////////// � virer
	explicit Vec3f( const Quat &Q );
	explicit Vec3f( const Color &C );
	Vec3f( const Vec4f&_v );
	//////////////////////

	Vec2f& xy();
	const Vec2f& xy() const;

	Vec3f& Set( const Float _x, const Float _y, const Float _z );
	Vec3f& Set( const Vec3f &_v );	

	// Operator
	////////////////////// � virer
	Vec3f& operator = ( const Quat &Q );
	Vec3f& operator = ( const Vec4f&_v );
	Vec3f& operator = ( const Color &_c );

	//////////////////////
	Vec3f& operator = ( const VecFloat4&_v );

	Vec3f	operator +  ( const Vec3f &_v ) const;
	Vec3f& operator += ( const Vec3f &_v );
	Vec3f	operator +  () const;
	Vec3f	operator -  ( const Vec3f &_v ) const;
	Vec3f& operator -= ( const Vec3f &_v );
	Vec3f	operator -  () const;
	Vec3f	operator *  ( const Float _f ) const;
	Vec3f& operator *= ( const Float _f );
	Float   operator *  ( const Vec3f& _v ) const;
	Vec3f  operator &  ( const Vec3f& _v ) const;
	Vec3f  operator &= ( const Vec3f& _v );
	Vec3f	operator /  ( const Float _f ) const;
	Vec3f& operator /= ( const Float _f );
	Vec3f	operator /  ( const Vec3f& _v ) const;
	Vec3f	operator ^  ( const Vec3f& _v ) const;
	Float&  operator[]  ( const int _i );
	const Float& operator[] ( const int _i ) const;

	FINLINE_Z Bool operator == ( const Vec3f& v ) const;
	FINLINE_Z Bool operator != ( const Vec3f& v ) const;
	// Utils
	Float GetNorm2() const;
	Float GetNorm() const;
	void	SetNorm(Float _norm);
	Vec3f& Normalize( const Float lengthScale = 1.0f );

	////////////////////// � virer
	inline	U32	ToU32(Float fHeight)const;
	inline	U32	ToU32Swapped(Float fHeight)const;
	inline	U32	ToU3210(Float fHeight)const;
	inline	void FromU32(U32 rgba,Float &fHeight);

	Bool CNormalize(const Float NormValue = 1.f);
	Bool CMaximize( const Float maximum );
	Bool CSetLength( const Float length );
	Bool CTermMaximize( const Float maximum );
	Bool CMinimize( const Float minimum );
	Bool CThreshold( const Float threshold_val );

	Bool ANormalize();
	Vec3f	GetNormal() const { Vec3f _temp=*this; _temp.CNormalize(); return _temp;} 
	Vec3f	GetHNormal() const { Vec3f _temp=*this; _temp.CHNormalize(); return _temp;}
	Float  HGetNorm2() const;
	Float  HGetNorm() const;
	Float  DownHGetNorm2( const Vec3f &_down ) const;
	Float  DownHGetNorm( const Vec3f &_down ) const;
	Vec3f& HNormalize();
	Bool   CHNormalize();

	// Construit une base de vecteur perpendiculaire
	void	Get1OrthoVector(Vec3f &_n1) const;
	void	Get2OrthoVector(Vec3f &_n1,Vec3f &_n2) const;
	void	Get2OrthoVectorFrom(Vec3f &_n1, Vec3f &_n2,const Vec3f &_oldn2) const;

	//////////////////////

	void Lerp( const Vec3f &_v1, const Vec3f &_v2 , Float s  );
	
};
	//////////////////////


inline Vec3f operator * ( const Float _f, const Vec3f &_v )	{ return _v*_f; }

template <> inline Vec3f Min <Vec3f> ( const Vec3f& _V1, const Vec3f& _V2)
{
	Vec3f result;
	result.x = _V1.x < _V2.x ? _V1.x : _V2.x;
	result.y = _V1.y < _V2.y ? _V1.y : _V2.y;
	result.z = _V1.z < _V2.z ? _V1.z : _V2.z;
	return result; 
}

template <> inline Vec3f Max <Vec3f> ( const Vec3f& _V1, const Vec3f& _V2)
{
	Vec3f result;
	result.x = _V1.x > _V2.x ? _V1.x : _V2.x;
	result.y = _V1.y > _V2.y ? _V1.y : _V2.y;
	result.z = _V1.z > _V2.z ? _V1.z : _V2.z;
	return result; 
}

const EXTERN_Z Vec3f VEC3F_NULL;
const EXTERN_Z Vec3f VEC3F_ONE;
const EXTERN_Z Vec3f VEC3F_UP;
const EXTERN_Z Vec3f VEC3F_DOWN;
const EXTERN_Z Vec3f VEC3F_LEFT;
const EXTERN_Z Vec3f VEC3F_RIGHT;
const EXTERN_Z Vec3f VEC3F_FRONT;
const EXTERN_Z Vec3f VEC3F_BACK;
const EXTERN_Z Vec3f VEC3F_HALF;

#include <Vec3f_Z.inl>


#endif //_MATH_VECTOR3D_Z_H
