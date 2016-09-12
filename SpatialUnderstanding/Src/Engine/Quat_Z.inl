// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

inline Quat::Quat( const Float _w, const Float _x, const Float _y, const Float _z )
{
	xyzw().Set( _x, _y, _z, _w );
}

inline Quat::Quat( const Float Angle, const Vec3f& Axis)
{
	SetAngleAxis( Angle, Axis );
}

inline Quat::Quat(const Mat3x3 &Matrix)
{
    SetMatrix(Matrix);
}

inline Quat::Quat(const Mat4x4 &Matrix)
{
    SetMatrix(Matrix);
}

inline Quat::Quat(const Vec3f &RV)
{
	*this = RV;
}

inline Quat	&Quat::operator= (const Vec4f &RV)
{
	*this = RV.xyz();
	return *this;
}

inline Quat& Quat::SetNull()
{
	xyzw().SetDefault();
	return *this;
}

inline void Quat::SetMatrix(const Mat4x4 &Matrix)
{
	SetMatrix( Matrix.m3() );
}

inline const Vec4f& Quat::xyzw() const 
{
	return reinterpret_cast <const Vec4f&> ( *this ); 
}

inline Vec4f& Quat::xyzw() 
{
	return reinterpret_cast <Vec4f&> ( *this ); 
}

inline const Vec3f& Quat::xyz() const 
{
	return reinterpret_cast <const Vec3f&> ( *this ); 
}

inline Vec3f& Quat::xyz() 
{
	return reinterpret_cast <Vec3f&> ( *this ); 
}

inline void	Quat::SetAngleAxis( const Float Angle,const Vec3f &Axis)
{
	Vec2f sc;
	SinCos(sc,Angle*0.5f);
	xyz()=Axis * sc.x;
	w=sc.y;
}

inline 	Quat Quat::operator-() const
{
	Quat res;
	res.xyzw() = VecFloatQuaternionConjugate( VecFloatLoadAligned(&v.x) );
	return res;
}

inline Quat& Quat::SetInverse()	 { xyz() =-xyz(); return *this; }

inline Quat Quat::operator + ( const Quat& q ) const { return (*this)*q; }	   // Somme de deux rotations

inline Quat	Quat::operator - ( const Quat& q ) const { return (*this)*(-q); }  // Difference de deux rotation

inline Quat& Quat::operator += ( const Quat& q ) { *this=(*this)*q; return *this; }	// Somme de deux rotations

inline Quat& Quat::operator -= ( const Quat& q ) { *this=(*this)*(-q); return *this; } // Difference de deux rotation

inline Quat Quat::operator / ( const Float f ) const { return *this*(1.f/f); }

inline Quat& Quat::operator /= ( const Float f ) { *this=*this*(1.f/f); return *this; }

inline Quat Quat::operator* (const Quat& Q) const
{
	Quat	t;
	t.xyzw() = VecFloatQuaternionMul( VecFloatLoadAligned(&v), VecFloatLoadAligned(&Q.v) );
	return t;
}

inline Quat &Quat::operator *=(const Quat &Q)
{
	xyzw() = VecFloatQuaternionMul( VecFloatLoadAligned(&v), VecFloatLoadAligned(&Q.v) );
	return *this;
}

inline Quat& Quat::operator *= ( const Float f )
{
	*this=*this*f;
	return *this;
}

inline Float Quat::DotProduct( const Quat& Q ) const
{
	register const VecFloat4 f = VecFloatDot4( xyzw(), Q.xyzw() );
	return VecFloatGetX( VecFloatSaturateExtended(f) );	
}

inline Bool	Quat::operator == ( const Quat &_Quat ) const
{
	return	_Quat.xyzw()==xyzw();
}

inline Bool Quat::operator != ( const Quat& _Quat ) const { return !operator==(_Quat); }

// 
// Vector By Quat (Unit Quaternions only)
inline Vec3f Quat::operator* (const Vec3f& p) const
{
	Vec4f	Temp = VecFloatSetWZero( VecFloatLoadUnaligned(&p) );
	Vec4_Rotate(Temp,*this,Temp);
	return Temp.xyz();
}

inline Vec4f Quat::operator* (const Vec4f& p) const
{
	Vec4f	Temp = VecFloatSetWZero( p );
	Vec4_Rotate(Temp,*this,Temp);
	return Temp;
}

// Does a 'rotation' by -quat => _result=(-quat)*_v

// * - Quat Value
inline void Quat::NegativeMul(const Vec3f &_v,Vec3f &_result) const
{
	VecFloat4 _v0 = VecFloatSetWZero( VecFloatLoadUnaligned(&_v) );
	VecFloat4 _q0 = VecFloatQuaternionConjugate( VecFloatLoadAligned(&v) );
	Vec4f _temp = VecFloatQuaternionRotate( _v0, _q0 );
	_result = _temp.xyz();
}

// Does a 'rotation' by -quat => _result=(-quat)*_v
inline void Quat::NegativeMul(const Vec4f &_v,Vec4f &_result) const
{
	VecFloat4 _v0 = VecFloatSetWZero( VecFloatLoadAligned(&_v) );
	VecFloat4 _q0 = VecFloatQuaternionConjugate( VecFloatLoadAligned(&v) );
	VecFloat4 _temp = VecFloatQuaternionRotate( _v0, _q0 );
	_result = VecFloatPermuteX0Y0Z0W1( _temp, _result );

}

inline void Quat::Normalize(void)
{
	// FAST Version of Normalize : precision < 1e-6 (most time < 1e-8)
	register VecFloat4 quat = VecFloatLoadAligned( &v.x );
	VecFloatStoreAligned( &v.x, VecFloatQuaternionCNormalize(quat) );
}

inline Quat	Slerp(const Quat &Q1,const Quat &Q2,Float t)
{
	Quat result;
	result.xyzw() = VecFloatQuatSlerp( Q1.xyzw(), Q2.xyzw(), VecFloatLoad1(t) );
	return result;
}

inline Float Quat::GetDist( const Quat &Q )
{
	register const VecFloat4 vQ = VecFloatLoadAligned( &Q.v.x );
	register const VecFloat4 thisV = VecFloatLoadAligned( &v.x );
	return VecFloatGetX ( VecFloatQuaternionDistance(vQ,thisV) );
}

inline Float Quat::GetNorm() const
{
	register const VecFloat4 length = VecFloatLength4( VecFloatLoadAligned(&v) );
	return VecFloatGetX( length );
}

inline Float Quat::GetNorm2() const
{
	register const VecFloat4 thisV = VecFloatLoadAligned( &v );
	register const VecFloat4 dot = VecFloatDot4( thisV, thisV );
	return VecFloatGetX( dot );
}

inline Vec4f Quat::GetAxisAndAngle() const
{
	return VecFloatQuaternionGetAxisAngle( VecFloatLoadAligned(&v) );
}

// Transform Quaternion to Matrix
inline void Quat::GetMatrix( Mat4x4 &Mat) const
{
	VecFloatQuaternionTo4x4( Mat.m[0], VecFloatLoadAligned(&v) );
}
