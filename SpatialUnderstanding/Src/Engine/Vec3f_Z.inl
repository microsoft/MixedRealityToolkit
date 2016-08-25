// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

inline Vec3f::Vec3f( const Float _x, const Float _y, const Float _z ) : x(_x ), y(_y ), z(_z ) { }

inline Vec3f::Vec3f( const Float _xzy ) : x(_xzy ), y(_xzy ), z(_xzy ) { }

inline Vec3f& Vec3f::Set( const Float _x, const Float _y, const Float _z )  { x=_x; y=_y; z=_z; return *this; }

inline Vec3f& Vec3f::Set( const Vec3f& _v )  { x=_v.x; y=_v.y; z=_v.z; return *this; }

inline Vec2f& Vec3f::xy()
{
	return reinterpret_cast <Vec2f&> ( *this );
}

inline const Vec2f& Vec3f::xy() const
{
	return reinterpret_cast <const Vec2f&> ( *this );
}

inline Vec3f Vec3f::operator + ( const Vec3f& _v ) const
{
	return Vec3f ( x + _v.x, y + _v.y, z + _v.z );
}

inline Vec3f& Vec3f::operator += ( const Vec3f& _v )
{
	x += _v.x;
	y += _v.y;
	z += _v.z;
	return *this;
}

inline Vec3f Vec3f::operator + () const	{ return *this; }

inline Vec3f Vec3f::operator - ( const Vec3f& _v ) const
{
	return Vec3f ( x - _v.x, y - _v.y, z - _v.z );
}

inline Vec3f& Vec3f::operator -= ( const Vec3f& _v )
{
	x -= _v.x;
	y -= _v.y;
	z -= _v.z;
	return *this;
}

inline Vec3f Vec3f::operator - () const   { return Vec3f(-x,-y,-z); }

inline Vec3f Vec3f::operator * ( const Float _f ) const	
{
	return Vec3f ( x*_f, y*_f, z*_f );
}

inline Vec3f& Vec3f::operator *= ( const Float _f )
{
	x *= _f;
	y *= _f;
	z *= _f;
	return *this;
}

inline Vec3f Vec3f::operator / ( const Float _f ) const
{
	ASSERT_Z( _f != 0.f );
	const Float invF( 1.0f / _f );
	return Vec3f ( x*invF, y*invF, z*invF );
}

inline Vec3f& Vec3f::operator /= ( const Float _f )
{
	ASSERT_Z( _f != 0.f );
	const Float invF( 1.0f / _f );
	x *= invF;
	y *= invF;
	z *= invF;
	return *this;
}

inline Vec3f Vec3f::operator / ( const Vec3f& _v ) const
{
	return Vec3f ( x / _v.x, y / _v.y, z / _v.z );
}

inline Vec3f Vec3f::operator ^ ( const Vec3f& _v ) const
{
    Vec3f result;
	result.x = y * _v.z - z * _v.y;
    result.y = z * _v.x - x * _v.z;
    result.z = x * _v.y - y * _v.x;
	return result;
}

inline Float Vec3f::operator * ( const Vec3f& _v ) const	
{
    return x*_v.x + y*_v.y + z*_v.z;
}

inline Vec3f Vec3f::operator & ( const Vec3f& _v ) const	
{
	return Vec3f ( x * _v.x, y * _v.y, z * _v.z );
}
inline Vec3f Vec3f::operator &= ( const Vec3f& _v )
{
	x*=_v.x;
	y*=_v.y;
	z*=_v.z;
	return *this;
}

inline Float& Vec3f::operator[] ( const int _i )
{
	ASSERT_Z( _i>-1 && _i<3 );
	return (&x)[_i];
}

inline const Float& Vec3f::operator[] ( const int _i ) const
{
	ASSERT_Z( _i>-1 && _i<3 );
	return (&x)[_i];
}

FINLINE_Z Bool Vec3f::operator == ( const Vec3f& v ) const
{
	const Vec3f Diff( *this-v );
	return Abs(Diff.x) < Float_Eps
		&& Abs(Diff.y) < Float_Eps
		&& Abs(Diff.z) < Float_Eps;
}

FINLINE_Z Bool Vec3f::operator != ( const Vec3f& v ) const
{
	return ! (v == *this);
}

inline Float Vec3f::GetNorm2() const
{
	return (*this)*(*this);
}

inline Float Vec3f::GetNorm() const
{
	return Sqrt ( GetNorm2() );
}

inline void Vec3f::SetNorm(Float _norm)
{
	Float	norm=GetNorm();
	if (norm>Float_Eps)
		*this*=_norm/norm;
}

inline Vec3f& Vec3f::Normalize( const Float lengthScale )
{
	const Float n( (*this)*(*this) );
	(*this) *= InvSqrt( lengthScale, n );
	return *this;
}

inline Bool	Vec3f::CNormalize(const Float NormValue)  // Conditional normalize. Returns TRUE if could normalize...
															 // J'ai modifi� le CNormalize pour que l'on puisse normer avec autre chose que 1... Plus rapide !
{
	const Float n( (*this)*(*this) );
	if ( n > Float_Eps_2 )	// Using Float_Eps generates precision problems
	{
		(*this) *= InvSqrt( NormValue, n );
		return TRUE;
	}
	return FALSE;
}

inline Bool	Vec3f::CMaximize( const Float maximum )  // Conditional maximize. Returns TRUE if did maximize...
{
	const Float	n( (*this)*(*this) );
	if ( n > maximum*maximum )
	{
		(*this) *= InvSqrt( maximum, n );
		return TRUE;
		}
	return FALSE;
}

inline Bool	Vec3f::CSetLength( const Float length )  // Conditional maximize. Returns TRUE if did maximize...
{
	Float	n( (*this)*(*this) );
	n=Sqrt(n);
	if ( n > Float_Eps )
	{
		(*this) *= length/n;
		return TRUE;
		}
	return FALSE;
}

inline Bool	Vec3f::CTermMaximize( const Float maximum )  // Conditional term maximize. Returns TRUE if did maximize...
{
	const Float	n = Max( Max( POS_Z(x), POS_Z(y) ), POS_Z(z) );
	if ( n > maximum )
	{
		(*this) *= maximum/n;
		return TRUE;
	}
	return FALSE;
}

inline Bool	Vec3f::CMinimize( const Float minimum ) // Conditional minimize. Returns TRUE if did minimize...
{
	const Float n( (*this)*(*this) );
	if( n < minimum*minimum )
	{
		if( n > Float_Eps )
		{
			(*this)*=InvSqrt(minimum,n);
			return TRUE;
		}
	}
	return FALSE;
}

inline Bool	Vec3f::CThreshold( const Float threshold_val ) // Conditional threshold. Returns TRUE if did threshold...
{
	const Float n( (*this)*(*this) );
	if( n < threshold_val*threshold_val )
	{
		(*this)*=0.f;
		return TRUE;
	}
	else
	{
		Float	sqrtn=Sqrt(n);
		(*this)*=(sqrtn-threshold_val)/sqrtn;
		return FALSE;
	}
}

inline Float Vec3f::HGetNorm2() const  { return x*x + z*z; }

inline Float Vec3f::HGetNorm() const   { return Sqrt( HGetNorm2() ); }

inline Float Vec3f::DownHGetNorm2( const Vec3f &_down ) const
{
    const Vec3f& me ( *this );
	const Vec3f HDepl( me - _down * me *_down );
	return HDepl.GetNorm2();
}

inline Float Vec3f::DownHGetNorm( const Vec3f &_down ) const
{
    const Vec3f& me ( *this );
	const Vec3f HDepl( me - _down * me *_down );
	return HDepl.GetNorm();
}

inline Vec3f& Vec3f::HNormalize()
{
	const Float hLength( HGetNorm() );
	x /= hLength;
	y = 0.f;
	z /= hLength;
	return *this;
}

inline Bool	Vec3f::CHNormalize() // Conditional normalize. Returns TRUE if could normalize...
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
