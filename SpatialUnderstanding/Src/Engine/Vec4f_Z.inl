// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

inline Vec4f::Vec4f( const Float _x, const Float _y, const Float _z, const Float _w )
{
	vec128 = VecFloatLoad4( _x, _y, _z, _w );
}

inline Vec4f::Vec4f( const Float _xyzw )
{
	vec128 = VecFloatLoad1( _xyzw );
}

inline Vec4f::Vec4f( const Vec3f& _v, Float _w )
{
	vec128 = VecFloatLoad4( _v.x, _v.y, _v.z, _w );
}

inline Vec4f::Vec4f( const Vec2f &_xy, const Vec2f &_zw )
{
	vec128 = VecFloatLoad4( _xy.x, _xy.y, _zw.x, _zw.y );
}

inline Vec2f& Vec4f::xy()
{
	return reinterpret_cast <Vec2f&> ( *this );
}

inline const Vec2f& Vec4f::xy() const
{
	return reinterpret_cast <const Vec2f&> ( *this );
}

inline Vec2f& Vec4f::zw()
{
	return reinterpret_cast <Vec2f&> ( z );
}

inline const Vec2f& Vec4f::zw() const
{
	return reinterpret_cast <const Vec2f&> ( z );
}

inline Vec3f& Vec4f::xyz()
{
	return reinterpret_cast <Vec3f&> ( *this );
}

inline const Vec3f& Vec4f::xyz() const
{
	return reinterpret_cast <const Vec3f&> ( *this );
}

inline Vec4f& Vec4f::operator = ( const Float _f )
{
	vec128 = VecFloatLoad1( _f );
	return *this;
}

inline Vec4f& Vec4f::operator = ( const Vec3f& _v )
{
	vec128 = VecFloatLoad4( _v.x, _v.y, _v.z, 1.f );
	return *this;
}

inline Vec4f& Vec4f::operator = ( const Vec4f& _v )
{
	vec128 = _v.vec128;
	return *this;
}

inline Vec4f& Vec4f::operator = ( const VecFloat4& _v )
{
	vec128 = _v;
	return *this;
}

inline Vec4f& Vec4f::Set( const Float _x, const Float _y, const Float _z, const Float _w )
{
	vec128 = VecFloatLoad4( _x, _y, _z, _w );
	return *this;
}

inline Vec4f& Vec4f::SetNull()
{
	vec128 = VecFloatSplatZero();
	return *this;
}

inline Vec4f& Vec4f::SetDefault()
{
	vec128 = VecFloatSetIdentity();
	return *this;
}

inline Vec4f Vec4f::operator + ( const Vec4f &_v ) const
{
	return VecFloatAdd( vec128, _v.vec128 );
}

inline Vec4f& Vec4f::operator += ( const Vec4f &_v )
{
	vec128 = VecFloatAdd( vec128, _v.vec128 );
	return *this;
}

inline Vec4f Vec4f::operator + () const  { return *this; }

inline Vec4f Vec4f::operator - ( const Vec4f& _v ) const 
{ 
	return VecFloatSub( vec128, _v.vec128 );
}
 
inline Vec4f& Vec4f::operator -= ( const Vec4f& _v )
{
	vec128 = VecFloatSub( vec128, _v.vec128 );
	return *this;
}

inline Vec4f Vec4f::operator - () const
{
	return VecFloatNegate( vec128 );
}

inline Vec4f Vec4f::operator * ( const Float _f ) const
{
	return VecFloatScale( vec128, _f );
}

inline Vec4f& Vec4f::operator *= ( const Float _f )
{
	vec128 = VecFloatScale( vec128, _f );
	return *this;
}

inline Float Vec4f::operator * ( const Vec4f& _v ) const
{
	return VecFloatGetX( VecFloatScalarDot3(vec128,_v.vec128) );
}

inline Vec4f Vec4f::operator / ( const Float _f ) const
{
	ASSERT_Z(_f!=0.f);
	return VecFloatScale( vec128, 1.0f/_f );
}

inline Vec4f& Vec4f::operator /= ( const Float _f )
{
	vec128 = VecFloatScale( vec128, 1.0f/_f );
	return *this;
}

inline Float& Vec4f::operator[]  ( const int _i )
{
	ASSERT_Z( _i > -1 && _i < 4 );
	return (&x)[_i];
}

inline Bool	Vec4f::operator == ( const Vec4f& v ) const
{
	return VecFloatAllNearEqual( vec128, v.vec128, VEC4F_EPSILON.vec128 ) != 0;
}

inline Bool	Vec4f::operator != ( const Vec4f& v ) const
{
	VecFloat4 diff = VecFloatSub( vec128, v.vec128 );
	return VecFloatAnyLess( VEC4F_EPSILON,VecFloatAbs(diff) ) != 0;
}

inline Vec4f Vec4f::operator ^ ( const Vec4f &_v ) const
{
	Vec4f result = VecFloatCross3( vec128, _v.vec128 );
	result.w = 1.0f;
	return result;
}

inline Vec4f Vec4f::operator & ( const Vec4f &_v ) const
{
	Vec4f result;
	Vec4_Mul3( result, *this, _v );
	return result;
}

inline Vec4f& Vec4f::operator &= ( const Vec4f &_v )
{
	Vec4_Mul3( *this, *this, _v );
	return *this;
}

inline Vec4f Vec4f::operator / ( const Vec4f &_v ) const
{
	return VecFloatDiv( vec128, _v.vec128 );
}

inline Vec4f& Vec4f::operator /= ( const Vec4f &_v )
{
	vec128 = VecFloatDiv( vec128, _v.vec128 );
	return *this;
}

inline Vec4f::operator Float*() { return &x; }

inline Vec4f::operator const Float*() const  { return &x; }

inline Float Vec4f::GetNorm2() const
{
	return VecFloatGetX( VecFloatScalarDot3(vec128,vec128) );
}

inline Float Vec4f::GetNorm() const
{
	return VecFloatGetX( VecFloatLength3(vec128) );
}

inline void Vec4f::SetNorm(Float _norm)
{
	Float	norm=GetNorm();
	if (norm>Float_Eps)
		*this*=_norm/norm;
}

inline Float Vec4f::HGetNorm2() const	{ return x*x + z*z; }

inline Float Vec4f::HGetNorm() const    { return Sqrt(x*x + z*z); }

inline Vec4f& Vec4f::HNormalize()
{
	const Float hNorma( HGetNorm() );
	x /= hNorma;
	y = 0.f;
	z /= hNorma;
	w = 0.f; // 1.0f?
	return *this;
}

inline Vec4f& Vec4f::Normalize()
{
	vec128 = VecFloatNormalize3( vec128, 2 );
	return *this;
}

inline Bool Vec4f::CNormalize() // Conditional normalize (3-components length). Returns TRUE if could normalize...
{
	VecFloat4 sqNorma = VecFloatDot3( vec128, vec128 );
	if( VecFloatAllGreater(sqNorma,VEC4F_SQUARED_EPSILON) )
	{		
		vec128 = VecFloatMul( vec128, VecFloatRSqrt(sqNorma) );
		return TRUE;
	}
	return FALSE;
}

inline Bool	Vec4f::CMaximize( const Float maximum )
{
	const Float	n=(*this)*(*this);
	if( n >maximum*maximum )
	{
		(*this)*=InvSqrt( maximum, n );
		return TRUE;
	}
	return FALSE;
}




FINLINE_Z void Vec4_Add(Vec4f& _result, const Vec4f& _v0, const Vec4f& _v1 )
{
	_result = VecFloatAdd( _v0, _v1 );
}

FINLINE_Z void Vec4_Sub(Vec4f& _result, const Vec4f& _v0, const Vec4f& _v1 )
{
	_result = VecFloatSub( _v0, _v1 );
}


FINLINE_Z void Vec4_Mul4(Vec4f& _result, const Vec4f& _v0, const Vec4f& _v1 )
{
	_result = VecFloatMul( _v0, _v1 );
}

FINLINE_Z void Vec4_Mul3(Vec4f& _result, const Vec4f& _v0, const Vec4f& _v1 )
{
	const Float w( _result.w );
	_result = VecFloatMul( _v0, _v1 );
	_result.w = w;
}

FINLINE_Z void Vec4_Scale(Vec4f& _result, const Float _s, const Vec4f& _v0 )
{
	_result = VecFloatScale( _v0, _s );
}

FINLINE_Z void Vec4_Fabs(Vec4f& _result, const Vec4f& _v )
{
	_result = VecFloatAbs( _v );
}

FINLINE_Z void	Vec4_Lerp(Vec4f& _result, const Vec4f &_v0, const Vec4f &_v1, const Float _s)
{
	_result = VecFloatLerp( _v0, _v1, _s );
}

FINLINE_Z void Vec4_Scale_Add(Vec4f& _result, const Vec4f& _v0, const Vec4f& _v1, const Float _s )
{
	_result = VecFloatScale( VecFloatAdd(_v0,_v1), _s );
}

FINLINE_Z void Vec4_Add_Scale(Vec4f& _result, const Vec4f& _v0, const Float _s, const Vec4f& _v1 )
{
	// result = v0 + v1*s
	_result = VecFloatMadd( _v1, VecFloatLoad1(_s), _v0 );
}

//_result = _v1 * _s + _v0
FINLINE_Z void Vec4_Add_Scale4(Vec4f& _result, const Vec4f& _v0, const Float _s, const Vec4f& _v1 )
{
    _result = VecFloatMadd( _v1, VecFloatLoad1(_s), _v0 );
}

FINLINE_Z void Vec4_Sub_Scale(Vec4f& _result, const Vec4f& _v0, const Float _s, const Vec4f& _v1 )
{
	// result = v0 - v1*s
	_result = VecFloatSub( _v0, VecFloatScale(_v1,_s) );
}

FINLINE_Z void Vec4_Sub_Scale4(Vec4f& _result, const Vec4f& _v0, const Float _s, const Vec4f& _v1 )
{
	_result = VecFloatSub( _v0, VecFloatScale(_v1,_s) );
}

FINLINE_Z void Vec4_Mul_Scale(Vec4f& _result, const Vec4f& _v0, const Float _s, const Vec4f& _v1 )
{
	// result = v0 * v1*s
	_result = VecFloatMul( _v0, VecFloatScale(_v1,_s) );
}

FINLINE_Z void	Vec4_FabsSub(Vec4f& _result, const Vec4f& _v0, const Vec4f& _v1 )
{
  _result = VecFloatSub( VecFloatAbs(_v0), _v1 );
}

FINLINE_Z Float	Vec4_FabsDot(Vec4f& _vresult, const Vec4f& _v0, const Vec4f& _v1 )
{
	const VecFloat4 absV1 = VecFloatAbs( _v1 );
	_vresult = absV1;
	return VecFloatGetX( VecFloatScalarDot3(absV1,_v0) );
}

FINLINE_Z Float Vec4_Dot4(const Vec4f& _v0, const Vec4f& _v1 )
{
	return VecFloatGetX( VecFloatDot4(_v0,_v1) );
}

FINLINE_Z Float Vec4_Dot(const Vec4f& _v0, const Vec4f& _v1 )
{
    return VecFloatGetX( VecFloatScalarDot3(_v0,_v1) );
}

FINLINE_Z void Vec4_Cross(Vec4f& _result, const Vec4f& _v0, const Vec4f& _v1 )
{
	_result = VecFloatSetWOne( VecFloatCross3(_v0,_v1) );
}

FINLINE_Z void Vec4_PositiveCross(Vec4f& _result, const Vec4f& _v0, const Vec4f& _v1 )
{
	Float	v0x,v0y,v0z;
	Float	v1x,v1y,v1z;
	v0x = _v0.x; v0y = _v0.y; v0z = _v0.z;
	v1x = _v1.x; v1y = _v1.y; v1z = _v1.z;

	_result.x = v0y*v1z + v0z*v1y;
	_result.y = v0z*v1x + v0x*v1z;
	_result.z = v0x*v1y + v0y*v1x;
	_result = VecFloatSetWOne(_result);
}

FINLINE_Z Float Vec4_GetNorm2( const Vec4f& _v )
{
	const VecFloat4 v4 = _v.vec128;
	return VecFloatGetX( VecFloatScalarDot3(v4,v4) );
}

FINLINE_Z Float Vec4_GetNorm( const Vec4f& _v )
{
	return VecFloatGetX( VecFloatLength3(_v) );
}

FINLINE_Z void Vec4_Normalize( Vec4f& _result, const Vec4f& _v )
{
	_result = VecFloatNormalize3( _v, 2 );
}

FINLINE_Z Float	Vec4_Dist2(const Vec4f& _v0, const Vec4f& _v1)
{
	return VecFloatGetX( VecFloatSquareDistance3(_v0,_v1) );
}

FINLINE_Z Float	Vec4_Dist2Vector(Vec4f& _result, const Vec4f& _v0, const Vec4f& _v1)
{
	const VecFloat4 diff = VecFloatSub( _v0, _v1 );
	const VecFloat4 distance2 = VecFloatScalarDot3( diff, diff );
	_result = diff;
	_result.w=0.f;
	return VecFloatGetX( distance2 );
}

FINLINE_Z Float	Vec4_Dist(const Vec4f& _v0, const Vec4f& _v1)
{
	return VecFloatGetX( VecFloatDistance3(_v0,_v1) );
}

// _result = (_v0-_v1)*(_v0-_v1);
FINLINE_Z Float	Vec4_HDist2(const Vec4f& _v0, const Vec4f& _v1)
{
	VecFloat4 diff = VecFloatSub( _v0, _v1 );
	Vec4f _result = VecFloatMul( diff, diff );
	return _result.x+_result.z;
}

FINLINE_Z void Vec4_Max(Vec4f& _result, const Vec4f& _v0 , const Vec4f& _v1 )
{
	_result = VecFloatMax( _v0, _v1 );
}

FINLINE_Z void Vec4_Min(Vec4f& _result, const Vec4f& _v0 , const Vec4f& _v1 )
{
	_result = VecFloatMin( _v0, _v1 );
}

FINLINE_Z	Float Vec4_MinDot(const Vec4f& _v0 , const Vec4f& _v1 )
{
	VecFloat4 temp = VecFloatSub( VecFloatAbs(_v0), _v1 );
	temp = VecFloatMax( VecFloatSplatZero(), temp );
	return VecFloatGetX( VecFloatScalarDot3(temp,temp) );
}

FINLINE_Z	Float Vec4_MinDotRes(Vec4f& _result, const Vec4f& _v0 , const Vec4f& _v1 )
{
	VecFloat4 temp, temp2, v0( _v0.vec128 ), v1( _v1.vec128 );
	temp = VecFloatMax( v0, v1 );
	temp2 = VecFloatMin( v0, VecFloatNegate(v1) );
	temp = VecFloatAdd( temp, temp2 );
	_result = temp;
	return VecFloatGetX( VecFloatScalarDot3(temp,temp) );
}

FINLINE_Z void	Vec4_GetLocalSpeed(Vec4f& _result, const Vec4f& _v0, const Vec4f& _v1, const Vec4f& _v2, const Vec4f& _v3 )
{
	VecFloat4 h_delta = VecFloatCross3( _v2, VecFloatSub(_v1,_v0) );
	_result = VecFloatAdd( _v3, h_delta );
	_result.w = _v3.w;
}

FINLINE_Z void	Vec4_AddTorque(Vec4f& _result, const Vec4f& _v0, const Vec4f& _v1, const Vec4f& _v2 )
{
	VecFloat4 temp = VecFloatSub( _v0, _v1 );
	temp = VecFloatCross3( temp, _v2 );
	_result = VecFloatAdd( _result, temp );
}

