// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#ifndef _MATH_Z_H
#define _MATH_Z_H

#include <Assert_Z.h>
#include <DynArray_Z.h>
#include <math.h>

struct Vec2f;
struct Vec3f;
struct Vec4f;
struct Quat;
struct Mat3x3;
struct Mat4x4;
class BoneNode_Z;

#define ROL_Z(val,shift) ((val<<shift)|(val>>((sizeof(val)*8)-shift)))
#define ROR_Z(val,shift) ((val>>shift)|(val<<((sizeof(val)*8)-shift)))
#define TRUNCPOS_Z(a)   Max(a,0.f)

#define POS_Z(a)        Abs(a)
#define ABS_Z(a)		Abs(a)	
#define	SIGN_Z(a)		Sign(a)
#define	ISPOW2_Z(a)		((a&(a-1))?FALSE:TRUE)
#define	SQR_Z(a)		( (a)*(a) )

#undef	M_PI
#define M_PI		3.14159265358979323846f
#undef	M_PI_2
#define M_PI_2		1.57079632679489661923f
#undef	M_PI_4
#define M_PI_4		0.785398163397448309616f
#undef	M_2_PI
#define M_2_PI		6.283185307179586476925f
#undef	M_INV_2_PI
#define M_INV_2_PI  0.159154943091895335768f
#undef M_PI_DIV_180 
#define M_PI_DIV_180 0.01745329251994329576923690768489f
#undef M_180_DIV_PI
#define M_180_DIV_PI 57.295779513082320876798154814105f

FINLINE_Z	S32		iDIV( const S32 a, const S32 b ) { return a < 0 ? (a-b+1)/b : a/b; }
FINLINE_Z	S32		iMOD(const S32 a, const S32 b )  { return a < 0 ? a-(iDIV(a,b)*b) : a%b; }
FINLINE_Z	S32		iABS(const S32 a) { return a < 0 ? -a : a; }

FINLINE_Z	int		FTOL( Float a );
FINLINE_Z	int		FLOORINT( Float a );
FINLINE_Z	int		TRUNCINT( Float a );
FINLINE_Z	Float	FLOORF( Float a );
FINLINE_Z	Float	FRACTF( Float a );
FINLINE_Z	Float	CEILFF( Float a );


// UTILISER LE DEFINE CAR UN ENTIER NEGATIF EST ARRONDI VERS LE HAUT EN C STANDARD ET floorf() RAME
#define	FLOOR(a)		((a)>=0.f?((int)(a)):-((int)(0.9999999f-(a))))	// Laisser celui l� car arrondi CPU pas le m�me sur tous les projets.
#define ROUNDINT(a)		FLOORINT( a+0.5f)
#define ROUNDF(a)       FLOORF( a+0.5f)

#define DegToRad(_Deg)		((_Deg)*(M_PI_DIV_180))
#define RadToDeg(_Rad)		((_Rad)*(M_180_DIV_PI))
#define Sqr(_Val)		((_Val) * (_Val))

inline	Float Log(Float x)				{return logf(x);}
inline	Float Log2(Float x)				{return logf(x)*1.44269504f;}	// 1,4426950408889634073599246810023 = 1/ln(2)
inline	Float LogN(Float x,Float n)		{return logf(x)/logf(n);}

inline	S32	 GetPower2(Float a)	// R�cup�re l'exposant de la plus proche puissance de 2 de a
	{
		union _casthelper {
			U32		i;
			float	f;
		} fi;
		fi.f = a;
		return (fi.i>>23)-127;
    }

template<typename T>
T Windowize(T _Value,T _Win)
{
	if (_Value>-_Win && _Value<_Win)
		_Value=0.f;
	else if (_Value<0.f)
		_Value += _Win;
	else if (_Value>0.f)
		_Value -= _Win;
	return _Value;
}

inline float U16RangeToFloat(const U16 val, const Float min, const Float max )
{
	return ( (static_cast<float>(val)/65535.f) * (max - min) ) + min;
}

inline U16 FloatToU16Range( Float val, const Float min, const Float max )
{
	val = (((val - min) / (max - min)) * 65535.f) + 0.5f;
	if (val < 0.f) return 0;
	if (val > 65535.f) return 65535;
	return static_cast<U16>(val);
}

FINLINE_Z Bool Atof(const Char* _str, Float& _res)
{
	if (*_str)
	{
		Char *pEndPtr;
		Float tmpRes = (Float)strtod(_str, &pEndPtr);

		if (_str == pEndPtr)
			return FALSE;
		else if (*pEndPtr == '\0')
		{
			_res = tmpRes;
			return TRUE;
		}
	}
	return FALSE;
}
FINLINE_Z Bool Atoi(const Char* _str, S32& _res)
{
	if (*_str)
	{
		Char *pEndPtr;
		S32	tmpRes = strtol(_str, &pEndPtr, 10);

		if (_str == pEndPtr)
			return FALSE;
		else if (*pEndPtr == '\0')
		{
			_res = tmpRes;
			return TRUE;
		}
	}
	return FALSE;
}

inline	Float Sqrt(Float a);
inline	Float SSqrt(Float a) { if (a<0.f) return -Sqrt(-a); else return Sqrt(a); }
inline	Float InvSqrt(Float x,Float y);
inline	Float Atan2(Float a,Float b);
inline	Float Cos(Float a);
inline	Float Sin(Float a);
inline	void  SinCos(Vec2f& res,Float a);	// res.x = Sin(a); res.y = Cos(a)
inline	Float Tan(Float a);
inline	Float ACos(Float x);
inline	Float ASin(Float x);
inline	Float ATan(Float x);
inline	Float Powf(Float x,Float e);

inline Float fSel( Float comp, Float a, Float b )
{
	return comp < 0.f ? b : a;
}

template<typename T> static inline T Min( const T& _V1, const T& _V2)
{
	return _V1 < _V2 ? _V1 : _V2; 
}
template <> inline Float Min <Float> ( const Float& _V1, const Float& _V2)
{
	return _V1 < _V2 ? _V1 : _V2; 
}

template<typename T> static inline T Max( const T& _V1, const T& _V2)
{
	return _V1 > _V2 ? _V1 : _V2; 
}
template <> inline Float Max <Float> ( const Float& _V1, const Float& _V2)
{
	return _V1 > _V2 ? _V1 : _V2; 
}

template<typename T> static inline T Abs( const T _V )
{
	return _V > 0 ? _V : -_V; 
}
template <> inline Float Abs <Float> ( const Float _V )
{
	return fabsf( _V ); 
}

template <class T> inline T Square(T value)
{
    return value*value;
}

template<typename T> static inline T Sign( T _Value )
{
	return _Value == 0 ? 0 : ( _Value < 0 ? -1 : 1 );
}
template <> inline Float Sign <Float> ( Float _Value )
{
	return _Value == 0.f ? 0.f : fSel( _Value, 1.f, -1.f );
}

template<typename T> T Clamp( T _Value, T _Min, T _Max )
{
	return Min( _Max, Max(_Min,_Value) );
}

template <typename T> void Swap(T& _a, T& _b)
{
	T t = _a;
	_a = _b;
	_b = t;
}

template<typename T> T Threshold( const T _V1, const T _V2)
{
	if(_V1>_V2)	return _V1-_V2; 
	else if(_V1<-_V2)	return _V1+_V2;
	else return _V1-_V1;
}

template<typename T> T Max_Threshold( const T _V1, const T _V2, const T _V3)
{
	if(_V1>_V2)	return (_V1-_V2)/(_V3-_V2); 
	else if(_V1<-_V2)	return (_V1+_V2)/(_V3-_V2); 
	else return _V1-_V1;
}

template<typename T, typename S> T Lerp( const T& a, const T& b, const S& s )
{
	return ( b - a ) * s + a;
}

template<typename T> T SmoothStep( const T& a, const T& b, const T& s )
{
	T x = Clamp((s - a) / (b - a), 0.f, 1.f);
	return x * x * (3.f - 2.f * x);
}

//----------------------------------------------------------------
// Access for Aligned Structure
//----------------------------------------------------------------

union	UDummy {
	S16			i16[2];
	U16			u16[2];
	Float		f32;
	S32			i32;
	U32			u32;
};

#include <VectorLib.h>
#include <Vec2f_Z.h>
#include <Vec3f_Z.h>
#include <Vec4f_Z.h>
#include <Mat3x3_Z.h>
#include <Mat4x4_Z.h>
#include <Quat_Z.h>
#include <Color_Z.h>

//----------------------------------------------------------------
// VecXi
//----------------------------------------------------------------

struct	Vec2i
{
	S32	x,y;

	Vec2i()										{};
	Vec2i(S32 _x,S32 _y)						: x( _x ), y( _y ) {}
	Vec2i	&Set(S32 _x,S32 _y)					{x=_x;y=_y;return *this;}
	// Operator
	Vec2i	operator=(const Vec2i &_v)			{x=_v.x; y=_v.y; return *this;}
	Vec2i	operator+(const Vec2i &_v) const	{return Vec2i(x+_v.x,y+_v.y);}
	Vec2i	&operator+=(const Vec2i &_v)		{x+=_v.x;y+=_v.y;return *this;}
	Vec2i	operator+() const					{return *this;}
	Vec2i	operator-(const Vec2i &_v) const	{return Vec2i(x-_v.x,y-_v.y);}
	Vec2i	operator-(S32 _i) const				{return Vec2i(x-_i,y-_i);}
	Vec2i	operator+(S32 _i) const				{return Vec2i(x+_i,y+_i);}
	Vec2i	&operator-=(const Vec2i &_v)		{x-=_v.x;y-=_v.y; return *this;}
	Vec2i	operator-() const					{return Vec2i(-x,-y);}
	Vec2i	operator*(S32 _f)	const			{return Vec2i(x*_f,y*_f);}
	Vec2i	operator/(S32 _f)	const			{return Vec2i(x/_f,y/_f);}
	Vec2i	&operator*=(S32 _f)					{x*=_f;y*=_f;return *this;}
	Vec2i	&operator/=(S32 _f)					{x/=_f;y/=_f;return *this;}
	Vec2i	&operator-=(S32 _i)					{x-=_i;y-=_i;return *this;}
	Bool	operator==(const Vec2i& v)	const	{return x==v.x && y==v.y;}
};

struct  Vec3i
{
	S32	x,y,z;

	Vec3i()										{};
	Vec3i(S32 _x,S32 _y,S32 _z)					{x = _x;y = _y;z = _z;};
	Vec3i	&Set(S32 _x,S32 _y,S32 _z)			{x=_x;y=_y;z=_z;return *this;}
	Vec3i	&Set(const Vec3i &_v)				{x=_v.x;y=_v.y;z=_v.z; return *this;}		
	// Operator
	Vec3i	operator=(const Vec3i &_v)			{x=_v.x; y=_v.y; z=_v.z; return *this;}
	Vec3i	operator+(const Vec3i &_v) const	{return Vec3i(x+_v.x,y+_v.y,z+_v.z);}
	Vec3i	&operator+=(const Vec3i &_v)		{x+=_v.x;y+=_v.y;z+=_v.z;return *this;}
	Vec3i	operator+() const					{return *this;}
	Vec3i	operator-(const Vec3i &_v) const	{return Vec3i(x-_v.x,y-_v.y,z-_v.z);}
	Vec3i	&operator-=(const Vec3i &_v)		{x-=_v.x;y-=_v.y;z-=_v.z; return *this;}
	Vec3i	operator-() const					{return Vec3i(-x,-y,-z);}
	Vec3i	operator*(S32 _f)	const			{return Vec3i(x*_f,y*_f,z*_f);}
	Vec3i	&operator*=(S32 _f)					{x*=_f;y*=_f;z*=_f; return *this;}
	Bool	operator==(const Vec3i& v)	const	{return x==v.x && y==v.y && z==v.z;}
	S32&	operator[]  ( const int _i )		{return (&x)[_i];}
	const S32& operator[] ( const int _i ) const {return (&x)[_i];}
};
const EXTERN_Z Vec3i VEC3I_NULL;
const EXTERN_Z Vec3i VEC3I_ONE;
const EXTERN_Z Vec3i VEC3I_UP;
const EXTERN_Z Vec3i VEC3I_DOWN;
const EXTERN_Z Vec3i VEC3I_LEFT;
const EXTERN_Z Vec3i VEC3I_RIGHT;
const EXTERN_Z Vec3i VEC3I_FRONT;
const EXTERN_Z Vec3i VEC3I_BACK;

#include <ByteColor_Z.h>

inline Color::Color( const Vec3f& _v )
{
	r = _v.x;
	g = _v.y;
	b = _v.z;
	a = 1.0f;
}

inline Color::Color( const Vec4f& _v )
{
	vec128 = _v.vec128;
}


inline Vec4f::Vec4f( const Color& _rgba )
{
	vec128 = _rgba.vec128;
}

inline Vec2f::Vec2f( const Vec3f& _o ) : x( _o.x ), y( _o.z ) { }

inline Vec2f& Vec2f::operator=(const Vec3f &_v)	  {x=_v.x; y=_v.z; return *this;}


inline Vec3f::Vec3f( const Color& _c ) : x( _c.r ), y( _c.g ), z( _c.b )  { }
inline Vec3f::Vec3f( const Vec4f& _v ) : x( _v.x ), y( _v.y ), z( _v.z )  { }

inline Vec3f::Vec3f(const Quat &Q)
{
	Float	w=Q.w;

	if (w<-1.f) w=-1.f;
	if (w>1.f) w=1.f;
	Float	halfang=ACos(w);

	Float	s=Sqrt(1.f-w*w);

	if (s>Float_Eps) *this=Q.xyz()*(1.f/s);
	else
	{
		*this=VEC3F_NULL;
		return;
	}
	w=halfang*2.f;
	if (w<0.f)
	{
		*this=-*this;
		w=-w;
	}
	if (w>3.141592f)
	{
		*this=-*this;
		w=3.141592f*2.f-w;
	}
	*this*=w;
}

inline Vec4f::Vec4f(const Quat &Q)
{
	xyz() = Q;
	w = 1;
}

inline const Color& Vec4f::rgba() const
{
	return reinterpret_cast <const Color&> ( *this );
}

inline Color& Vec4f::rgba()
{
	return reinterpret_cast <Color&> ( *this );
}

inline const Vec4f& Color::xyzw() const
{
	return reinterpret_cast <const Vec4f&> ( *this );
}

inline Vec4f& Color::xyzw()
{
	return reinterpret_cast <Vec4f&> ( *this );
}

inline const Vec3f& Color::xyz() const
{
	return reinterpret_cast <const Vec3f&> ( *this );
}

inline Vec3f& Color::xyz()
{
	return reinterpret_cast <Vec3f&> ( *this );
}

inline Vec3f& Vec3f::operator = ( const Color& _c )
{
	x=_c.r;
	y=_c.g;
	z=_c.b;
	return *this;
}

inline Vec3f& Vec3f::operator = ( const Vec4f& _v )
{
	x=_v.x;
	y=_v.y;
	z=_v.z;
	return *this;
}

inline Vec3f& Vec3f::operator= (const Quat &Q)
{
	Float	w=Q.w;

	if (w<-1.f) w=-1.f;
	if (w>1.f) w=1.f;
	Float	halfang=ACos(w);

	Float	s=Sqrt(1.f-w*w);

	if (s>Float_Eps)*this=Q.xyz()*(1.f/s);
	else
	{
		*this=VEC3F_NULL;
		return *this;
	}
	w=halfang*2.f;
	if (w<0.f)
	{
		*this=-*this;
		w=-w;
	}
	if (w>Pi)
	{
		*this=-*this;
		w=Pi*2.f-w;
	}
	*this*=w;
	return *this;
}

inline Mat3x3::Mat3x3( const Mat4x4& m )    { *this = m.m3(); }

inline void Mat3x3::Set( const Mat4x4& Mat) { *this = Mat.m3(); }

inline void Mat4x4::SetTRS(const Vec3f &Trans,const Quat &Rot,const Vec3f &Scale)
{
	Rot.GetMatrix(*this);

	// Calc LocalMatrix
	m[0][0] *= Scale.x;	m[1][0] *= Scale.y;	m[2][0] *= Scale.z;	m[3][0] = Trans.x;
	m[0][1] *= Scale.x;	m[1][1] *= Scale.y;	m[2][1] *= Scale.z;	m[3][1] = Trans.y;
	m[0][2] *= Scale.x;	m[1][2] *= Scale.y;	m[2][2] *= Scale.z;	m[3][2] = Trans.z;
	m[0][3]  = 0.f;		m[1][3] = 0.f;		m[2][3]  = 0.f;		m[3][3] = 1.f;
}

inline void Mat4x4::SetTRS( const Vec3f &Trans, const Quat &Rot, const Float Scale )
{
	Rot.GetMatrix(*this);

	// Calc LocalMatrix
	m[0][0] *= Scale;	m[1][0] *= Scale;	m[2][0] *= Scale;	m[3][0] = Trans.x;
	m[0][1] *= Scale;	m[1][1] *= Scale;	m[2][1] *= Scale;	m[3][1] = Trans.y;
	m[0][2] *= Scale;	m[1][2] *= Scale;	m[2][2] *= Scale;	m[3][2] = Trans.z;
	m[0][3]  = 0.f;		m[1][3] = 0.f;		m[2][3]  = 0.f;		m[3][3] = 1.f;
}


inline void MinVec(const Vec4f &v1,const Vec4f &v2, Vec4f &result)
{
	Vec4_Min( result, v1, v2 );
}

inline void MaxVec(const Vec4f &v1,const Vec4f &v2, Vec4f &result)
{
	Vec4_Max( result, v1, v2 );
}

inline void MinVec(const Vec3f &v1,const Vec3f &v2, Vec3f &result)
{
	result.x = Min(v1.x,v2.x);
	result.y = Min(v1.y,v2.y);
	result.z = Min(v1.z,v2.z);
}

inline void MaxVec(const Vec3f &v1,const Vec3f &v2, Vec3f &result)
{
	result.x = Max(v1.x,v2.x);
	result.y = Max(v1.y,v2.y);
	result.z = Max(v1.z,v2.z);
}

inline void MinVec(const Vec2f &v1,const Vec2f &v2, Vec2f &result)
{
	result.x = Min(v1.x,v2.x);
	result.y = Min(v1.y,v2.y);
}

inline void MaxVec(const Vec2f &v1,const Vec2f &v2, Vec2f &result)
{
	result.x = Max(v1.x,v2.x);
	result.y = Max(v1.y,v2.y);
}

inline Vec4f MinVec(const Vec4f &v1,const Vec4f &v2)
{
	return VecFloatMin( v1, v2 );
}

inline Vec4f MaxVec(const Vec4f &v1,const Vec4f &v2)
{
	return VecFloatMax( v1, v2 );
}

inline Vec3f MinVec(const Vec3f &v1,const Vec3f &v2)
{
	Vec3f result;
	result.x = Min(v1.x,v2.x);
	result.y = Min(v1.y,v2.y);
	result.z = Min(v1.z,v2.z);
	return result;
}

inline Vec3f MaxVec(const Vec3f &v1,const Vec3f &v2)
{
	Vec3f result;
    MaxVec( v1, v2, result );
	return result;
}

inline Vec2f MinVec(const Vec2f &v1,const Vec2f &v2)
{
	Vec2f result;
	MinVec( v1, v2, result );
	return result;
}

inline Vec2f MaxVec(const Vec2f &v1,const Vec2f &v2)
{
	Vec2f result;
	MaxVec( v1, v2, result );
	return result;
}


//----------------------------------------------------------------
// EXTERNAL INLINES
//----------------------------------------------------------------

// CAUTION !!! ALIGNED ON 32 !!!
typedef DynArray_Z<Vec2f,32,FALSE,FALSE,32,TRUE>	Vec2fDA;
typedef DynArray_Z<Vec3f,32,FALSE,FALSE,32,TRUE>	Vec3fDA;
typedef DynArray_Z<Vec4f,32,FALSE,FALSE,32,TRUE>	Vec4fDA;
typedef DynArray_Z<Vec3i,32,FALSE,FALSE,32,TRUE>	Vec3iDA;

// CAUTION !!! ALIGNED ON 16 !!!
typedef DynArray_Z<Color,32,FALSE,FALSE,16,TRUE>	ColorDA;
typedef DynArray_Z<Mat4x4,32,FALSE,FALSE,16,TRUE>	Mat4x4DA;
typedef DynArray_Z<Mat3x3,32,FALSE,FALSE,16,TRUE>	Mat3x3DA;

typedef DynArray_Z<Quat,32,FALSE,FALSE,32,TRUE>		QuatDA;


//----------------------------------------------------------------
// Inlines Targets
//----------------------------------------------------------------

#include <MathInlinePc_Z.h>
#include <MathInlineEngine_Z.h>

FINLINE_Z void Vec4_Transform(Vec4f& _result, const Mat4x4& _m0, const Vec4f& _v0 )
{
	_result = VecFloat4x4Transform4( _m0.m128, _v0 );
}

FINLINE_Z void Vec4_Rotate(Vec4f& _result, const Mat4x4& _m0, const Vec4f& _v0 )
{
	Vec4_Rotate( _result, _m0.m3(), _v0 );
}

FINLINE_Z void Vec4_RotateTransp(Vec4f& _result, const Mat4x4& _m0, const Vec4f& _v0 )
{
	Vec4_RotateTransp( _result, _m0.m3(), _v0 );
}

FINLINE_Z void Vec4_Rotate(Vec4f& _result, const Mat3x3& _m0, const Vec4f& _v0 )
{
	_result = VecFloat3x3Transform( _m0.m.m128, _v0 );
}

FINLINE_Z void Vec4_RotateTransp(Vec4f& _result, const Mat3x3& _m0, const Vec4f& _v0 )
{
	Float	v0x, v0y, v0z;
	v0x = _v0.x; v0y = _v0.y; v0z = _v0.z;

	_result.x = _m0.m.m[0][0]*v0x + _m0.m.m[0][1]*v0y + _m0.m.m[0][2]*v0z;
	_result.y = _m0.m.m[1][0]*v0x + _m0.m.m[1][1]*v0y + _m0.m.m[1][2]*v0z;
	_result.z = _m0.m.m[2][0]*v0x + _m0.m.m[2][1]*v0y + _m0.m.m[2][2]*v0z;
	_result.w = 1.f;
}

FINLINE_Z void Vec4_Rotate(Vec4f& _result, const Quat& _q0, const Vec4f& _v0 )
{
	_result = VecFloatQuaternionRotate( _v0, _q0 );
	_result.w = 1.f;
}

FINLINE_Z void Vec4_NegRotate(Vec4f& _result, const Quat& _q0, const Vec4f& _v0 )
{
	_result = VecFloatQuaternionRotate( _v0, -_q0 );
	_result.w = 1.f;
}

FINLINE_Z void		Vec4_AddRot_Scale(Quat& _result,const Quat& _q0, const Float _s, const Vec4f& _v0)
{
	Quat	temp;

	// transformation de _v0 en quaternion temp
	Float len=Vec4_GetNorm(_v0);
	if (len<=0.f)
	{
		_result=_q0;
		return;
	}
	Float	hlen=_s*0.5f*len;
	Vec2f	sincos;
	SinCos(sincos,hlen);
	Vec4_Scale(*reinterpret_cast<Vec4f*>(&temp),sincos.x/len,_v0);
	temp.w=sincos.y;

	// Multiplication de quaterion
	_result=temp*_q0;
}

FINLINE_Z void		Vec4_AddRot(Quat& _result,const Quat& _q0, const Vec4f& _v0)
{
	Quat	temp;

	// transformation de _v0 en quaternion temp
	Float len=Vec4_GetNorm(_v0);
	if (len<=0.f)
	{
		_result=_q0;
		return;
	}
	Float	hlen=0.5f*len;
	Vec2f	Result;
	SinCos(Result,hlen);
	Vec4_Scale(*(Vec4f*)&temp,Result.x/len,_v0);
	temp.w=Result.y;

	// Multiplication de quaterion
	_result=temp*_q0;
}

inline ByteColor& ByteColor::operator*=( const Float _f)
{
	RGBA.BYTE.r=(U8)(FTOL(RGBA.BYTE.r*_f));
	RGBA.BYTE.g=(U8)(FTOL(RGBA.BYTE.g*_f));
	RGBA.BYTE.b=(U8)(FTOL(RGBA.BYTE.b*_f));
	return *this;
}

inline Vec3f::Vec3f( const VecFloat4& _v )
{
	Float4 res;
	VecFloatStoreAligned( &res, _v );
	memcpy( this, &res, sizeof(Vec3f) );
}

inline Vec3f& Vec3f::operator = ( const VecFloat4& _v )
{
	const Vec4f res = _v;
	memcpy( this, &res, sizeof(Vec3f) );
	return *this;
}

inline	U32	Vec3f::ToU32( const Float fHeight)const
{
	U32 r = FTOL( 127.0f * x + 128.0f );
	U32 g = FTOL( 127.0f * y + 128.0f );
	U32 b = FTOL( 127.0f * z + 128.0f );
	U32 a = FTOL( 255.0f * fHeight );
	return( (a<<24L) + (r<<16L) + (g<<8L) + (b<<0L) );
}

inline	U32	Vec3f::ToU32Swapped(const Float fHeight)const
{
	U32 r = FTOL( 127.0f * x + 128.0f );
	U32 g = FTOL( 127.0f * y + 128.0f );
	U32 b = FTOL( 127.0f * z + 128.0f );
	U32 a = FTOL( 255.0f * fHeight );
	return( (a<<0L) + (b<<8L) + (g<<16L) + (r<<24L) );
}

inline	U32	Vec3f::ToU3210(const Float fHeight)const
{
	U32 r = FTOL( 511.0f * x + 512.0f );
	U32 g = FTOL( 511.0f * y + 512.0f );
	U32 b = FTOL( 511.0f * z + 512.0f );
	U32 a = FTOL( 3.0f * fHeight );
    return( (a<<30L) + (b<<20L) + (g<<10L) + (r<<0L) );
}

inline	void Vec3f::FromU32( const U32 rgba, Float &fHeight)
{
	U32 r = (rgba>>16)&0xff;
    U32 g = (rgba>>8)&0xff;
    U32 b = (rgba)&0xff;
    U32 a = (rgba>>24)&0xff;
	x= ( static_cast<Float>(r)-128.f ) /127.f;
	y= ( static_cast<Float>(g)-128.f ) / 127.f;
	z= ( static_cast<Float>(b)-128.f ) / 127.f;
	fHeight= static_cast<Float>(a) / 255.f;
}


inline void ByteColor::Set(const Vec4f &_c)
{
      RGBA.rgbaColor = VecFloatPackColor8888( VecFloatSaturate(_c) ); 
}

inline void ByteColor::Get(Vec4f &_c)
{
	_c = VecFloatUnpackColor8888( RGBA.rgbaColor );
}

void MergeRects(S32& _outPx, S32& _outPy, S32& _outSx, S32& _outSy, S32 _inPx0, S32 _inPy0, S32 _inSx0, S32 _inSy0, S32 _inPx1, S32 _inPy1, S32 _inSx1, S32 _inSy1);

#endif //_MATH_Z_H
