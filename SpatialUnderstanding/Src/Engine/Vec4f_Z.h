// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#ifndef _MATH_VECTOR4D_Z_H
#define _MATH_VECTOR4D_Z_H

#include <BeginDef_Z.h>

struct Mat3x3;
struct Mat4x4;
struct Color;
struct Vec4i;


POSTALIGNED128_Z struct  Vec4f
{
	union
	{
		struct
		{
			Float x ALIGNED128_Z;
			Float y,z,w;
		};
		VecFloat4	vec128;
	};

	inline operator VecFloat4& ()  { return vec128; }
	inline operator const VecFloat4& () const { return vec128; }

#ifdef _DEBUG
	Vec4f()
	{
		// memset(this, 0xFF, sizeof(*this)); TROP LENT EN DEBUG !!!
		U32 *ptr = (U32*)this;
		*ptr++ = 0xFFFFFFFF;
		*ptr++ = 0xFFFFFFFF;
		*ptr++ = 0xFFFFFFFF;
		*ptr = 0xFFFFFFFF;
	}
#else
	Vec4f()	{}
#endif
	Vec4f( const Vec4f& _v ) : vec128( _v.vec128 )  { }
	Vec4f( const VecFloat4& _v ) : vec128( _v )  { }
	Vec4f( const Vec3f& _v, Float _w = 1.0f );
	explicit Vec4f( const Float _xyzw );
	explicit Vec4f( const Color& _rgba );
	explicit Vec4f( const Quat& q );
	Vec4f( const Float _x, const Float _y, const Float _z, const Float _w );
	Vec4f( const Vec2f& _xy, const Vec2f& _zw );

	Vec2f& xy();
	const Vec2f& xy() const;

	Vec2f& zw();
	const Vec2f& zw() const;

	Vec3f& xyz();
	const Vec3f& xyz() const;

	const Color& rgba() const;
	Color& rgba();

	Vec4f& Set( const Float _x, const Float _y, const Float _z, const Float _w );	
	Vec4f& SetNull(); // xyzw = 0
	Vec4f& SetDefault(); // xyz = 0, w = 1
	Vec4f& operator =  ( const Float _f );
	Vec4f& operator =  ( const Vec3f& _v );
	Vec4f& operator =  ( const Vec4f& _v );
	Vec4f& operator =  ( const VecFloat4& _v );
	Vec4f  operator +  ( const Vec4f& _v ) const;
	Vec4f& operator += ( const Vec4f& _v );
	Vec4f  operator +  () const;
	Vec4f  operator -  ( const Vec4f& _v ) const;
	Vec4f& operator -= ( const Vec4f& _v );
	Vec4f  operator -  () const;
	Vec4f  operator * ( const Float _f ) const;  // Scale
	Vec4f& operator *= ( const Float _f );
	Float  operator *  ( const Vec4f& _v ) const; // Dot
	Vec4f  operator & ( const Vec4f &_v ) const;  // Multiplication composante par composante
	Vec4f& operator &= ( const Vec4f &_v );
	Vec4f  operator /  ( const Float _f ) const;  // Scale inverse
	Vec4f& operator /= ( const Float _f );       
	Vec4f  operator /  ( const Vec4f& _v ) const; // Division composante par composante
	Vec4f& operator /= ( const Vec4f& _v );
	Float& operator[]  ( const int _i );
	Bool   operator == ( const Vec4f& v ) const;
	Bool   operator != ( const Vec4f& v ) const;
	// il n'y pas d'op�rateur <, qu'est-ce ce qui est math�matiquement correct: GetSum(a) < GetSum(b) ou AllLess(a) < AllLess(b)?
	Vec4f  operator ^ ( const Vec4f &_v ) const;
	operator Float *();
	operator const Float *() const;
	// Utils
	Float  GetNorm2() const;
	Float  GetNorm() const;
	void	SetNorm(Float _norm);
	Float  HGetNorm2() const;
	Float  HGetNorm() const;
	Vec4f	GetNormal() const { Vec4f _temp=*this; _temp.CNormalize(); return _temp;} 
	Vec4f	GetHNormal() const { Vec4f _temp=*this; _temp.CHNormalize(); return _temp;}
	Vec4f& HNormalize();
	Vec4f& Normalize();
	Bool   CNormalize();
	Bool   CMaximize( const Float maximum );
	Bool   ANormalize();
	inline Bool	CHNormalize() // Conditional normalize. Returns TRUE if could normalize...
	{
		Float n( x*x + z*z );
		if( n > Float_Eps )
		{
			n=InvSqrt(1.f,n);
			x*=n; y=0.f; z*=n;
			return TRUE;
		}
		return FALSE;
	}

	// Construit une base de vecteur perpendiculaire
	void	Get2OrthoVector(Vec4f &_n1,Vec4f &_n2) const;
	void	Get2OrthoVectorFrom(Vec4f &_n1, Vec4f &_n2,const Vec4f &_oldn2) const;
};

inline Vec4f operator * ( const Float _f, const Vec4f &_v) { return  _v*_f; }


// Math operations

inline void	 Vec4_Normalize( Vec4f& _result, const Vec4f& _v );
inline void	 Vec4_Add( Vec4f& _result, const Vec4f& _v0, const Vec4f& _v1 );    // _result = _v0+_v1;
inline void  Vec4_Sub( Vec4f& _result, const Vec4f& _v0, const Vec4f& _v1 );    // _result = _v0-_v1;
inline void  Vec4_Mul3(Vec4f& _result, const Vec4f& _v0, const Vec4f& _v1 );    // _result = _v0&_v1;
inline void  Vec4_Scale( Vec4f& _result, const Float _s, const Vec4f& _v0 );    // _result = _s*_v0;
inline void  Vec4_Cross( Vec4f& _result, const Vec4f& _v0, const Vec4f& _v1 );  // _result = _v0^_v1;
inline Float Vec4_Dot( const Vec4f& _v0, const Vec4f& _v1 );                    // _result = _v0._v1, 3 components
inline Float Vec4_Dot4( const Vec4f& _v0, const Vec4f& _v1 );                   // _result = _v0._v1;

inline Float Vec4_GetNorm2( const Vec4f& _v ); // 3 components
inline Float Vec4_GetNorm( const Vec4f& _v );  // 3 components

inline Float Vec4_MinDot(const Vec4f& _v0 , const Vec4f& _v1 );
inline Float Vec4_MinDotRes(Vec4f& _result, const Vec4f& _v0 , const Vec4f& _v1 );
inline void	 Vec4_Max(Vec4f& _result, const Vec4f& _v0 , const Vec4f& _v1 );
inline void	 Vec4_Min(Vec4f& _result, const Vec4f& _v0 , const Vec4f& _v1 );
inline void	 Vec4_Fabs(Vec4f& _result, const Vec4f& _v );
inline void	 Vec4_FabsSub(Vec4f& _result, const Vec4f& _v0, const Vec4f& _v1 );
inline Float Vec4_FabsDot(Vec4f& _vresult, const Vec4f& _v0, const Vec4f& _v1 ); // 3 components
inline Float Vec4_Dist2(const Vec4f& _v0, const Vec4f& _v1);  // not unified
inline Float Vec4_HDist2(const Vec4f& _v0, const Vec4f& _v1);
inline Float Vec4_Dist2Vector(Vec4f& _result, const Vec4f& _v0, const Vec4f& _v1);
inline Float Vec4_Dist(const Vec4f& _v0, const Vec4f& _v1);  // not unified
inline void	 Vec4_GetLocalSpeed(Vec4f& _result, const Vec4f& _v0, const Vec4f& _v1, const Vec4f& _v2, const Vec4f& _v3 );
inline void	 Vec4_AddTorque(Vec4f& _result, const Vec4f& _v0, const Vec4f& _v1, const Vec4f& _v2 );
inline void	 Vec4_AddRot(Quat& _result,const Quat& _q0, const Vec4f& _v0);
inline void	 Vec4_AddRot_Scale(Quat& _result,const Quat& _q0, const Float _s, const Vec4f& _v0);
inline void	 Vec4_Mul_Scale(Vec4f& _result, const Vec4f& _v0, const Float _s, const Vec4f& _v1 ); // 3 components
inline void	 Vec4_Add_Scale(Vec4f& _result, const Vec4f& _v0, const Float _s, const Vec4f& _v1 ); // 3 components
inline void	 Vec4_Scale_Add(Vec4f& _result, const Vec4f& _v0, const Vec4f& _v1, const Float _s ); // 3 components
inline void	 Vec4_Sub_Scale(Vec4f& _result, const Vec4f& _v0, const Float _s, const Vec4f& _v1 ); // 3 components
inline void	 Vec4_Sub_Scale4(Vec4f& _result, const Vec4f& _v0, const Float _s, const Vec4f& _v1 );
inline void	 Vec4_PositiveCross(Vec4f& _result, const Vec4f& _v0, const Vec4f& _v1 );
inline void	 Vec4_Transform(Vec4f& _result, const Mat4x4& _m0, const Vec4f& _v0 );
inline void	 Vec4_Rotate(Vec4f& _result, const Mat4x4& _m0, const Vec4f& _v0 );
inline void	 Vec4_Rotate(Vec4f& _result, const Mat3x3& _m0, const Vec4f& _v0 );
inline void	 Vec4_Rotate(Vec4f& _result, const Quat& _q0, const Vec4f& _v0 );
inline void	 Vec4_NegRotate(Vec4f& _result, const Quat& _q0, const Vec4f& _v0 );
inline void	 Vec4_Lerp(Vec4f& _result, const Vec4f &_v0, const Vec4f &_v1, const Float _s);


const EXTERN_Z Vec4f VEC4F_127;
const EXTERN_Z Vec4f VEC4F_255;
const EXTERN_Z Vec4f VEC4F_ONEOVER255;
const EXTERN_Z Vec4f VEC4F_EPSILON;
const EXTERN_Z Vec4f VEC4F_SQUARED_EPSILON;
const EXTERN_Z Vec4f VEC4F_ONE;
const EXTERN_Z Vec4f VEC4F_HALF;
const EXTERN_Z Vec4f VEC4F_THIRD;
const EXTERN_Z Vec4f VEC4F_QUARTER;
const EXTERN_Z Vec4f VEC4F_TWO;
const EXTERN_Z Vec4f VEC4F_THREE;
const EXTERN_Z Vec4f VEC4F_FOUR;
const EXTERN_Z Vec4f VEC4F_EIGHT;
const EXTERN_Z Vec4f VEC4F_NULL;
const EXTERN_Z Vec4f VEC4F_UP;
const EXTERN_Z Vec4f VEC4F_DOWN;
const EXTERN_Z Vec4f VEC4F_LEFT;
const EXTERN_Z Vec4f VEC4F_RIGHT;
const EXTERN_Z Vec4f VEC4F_FRONT;
const EXTERN_Z Vec4f VEC4F_BACK;

#include <Vec4f_Z.inl>

#endif //_MATH_VECTOR4D_Z_H
