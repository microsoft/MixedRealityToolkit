// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#ifndef _MATH_QUAT_Z_H
#define _MATH_QUAT_Z_H

#include <BeginDef_Z.h>

//----------------------------------------------------------------
// Quaternion
//----------------------------------------------------------------

POSTALIGNED128_Z struct  Quat
{
	// Data
	Vec3f	v	ALIGNED128_Z;
	Float	w;

	// NOTE: Some compilers such like gcc would probably give better performance if the operator was directly returning a VecFloat4
	// However there's currently no way to create an union with a VecFloat4 since Vec3f is a type defined with a constructor.
	inline operator VecFloat4& ()  { return reinterpret_cast<VecFloat4&>(*this); }
	inline operator const VecFloat4& () const  { return reinterpret_cast<const VecFloat4&>(*this); }

	// Constructor
#ifdef _DEBUG
	Quat()
	{
		memset(this, 0xFF, sizeof(*this));
	}
#else
	Quat() {}
#endif
	Quat( const Float _w, const Float _x, const Float _y, const Float _z );
    Quat( const Float Angle, const Vec3f &Axis );
    explicit Quat(const Vec3f &RV);
    Quat(const Vec3f &V1,const Vec3f &V2);
    Quat(const Vec3f &V1,const Vec3f &V2,const Vec3f &V3);
    explicit Quat(const Mat3x3 &Matrix);
    explicit Quat(const Mat4x4 &Matrix);

	const Vec4f& xyzw() const;
	Vec4f& xyzw();

	const Vec3f& xyz() const;
	Vec3f& xyz();

	Quat& SetNull();
	void SetAngleAxis( const Float Angle,const Vec3f &Axis );
	Quat& SetInverse();
	void	Normalize();
	Bool	Maximize(Float f);
	Float	GetNorm2() const;
	Float	GetNorm() const;
	Vec4f	GetAxisAndAngle() const;
	Float	GetAngle() const;
	Float	GetSignedAngle() const;
	void	SetAngle(Float _ang);
	void	GetMatrix(Mat4x4 &Mat) const;
	void	GetMatrix(Mat3x3 &Mat) const;
	void	SetMatrix(const Mat4x4 &Mat);
	void	SetMatrix(const Mat3x3 &Mat);
	Quat& operator = ( const Vec3f &RV );
	Quat& operator = ( const Vec4f &RV );
	Quat  operator -  () const;
	Quat  operator +  ( const Quat& q ) const;
	Quat  operator -  ( const Quat& q ) const;
	Quat& operator += ( const Quat& q );
	Quat& operator -= ( const Quat& q );
	Quat  operator *  ( const Quat& q ) const;
	Quat& operator *= ( const Quat& q );
	Quat  operator *  ( const Float f ) const;
	Quat& operator *= ( const Float f );
	Quat  operator /  ( const Float f ) const;
	Quat& operator /= ( const Float f );
	Float DotProduct( const Quat& Q ) const;
	Vec3f	operator* (const Vec3f& p) const;
	Vec4f	operator* (const Vec4f& p) const;
	Bool operator == ( const Quat &_Quat ) const;
	Bool operator != ( const Quat& _Quat ) const;
	void	NegativeMul(const Vec3f &_v,Vec3f &_result) const;
	void	NegativeMul(const Vec4f &_v,Vec4f &_result) const;
	void	GetEular(Vec3f &Eular) const;
	void	SetEular(const Vec4f &Eular);

	// Distance between 2 quaternion.
	Float	GetDist( const Quat &Q);
};

inline Quat operator * ( const Float _f, const Quat &_q) { return  _q*_f; }

const EXTERN_Z Quat QUAT_NULL;
void	RVToQuat(Quat &q);
void	QuatToRV(Quat &q);
void RVToQuat(const Vec3f &_v,Quat &q);
void QuatToRV(const Quat &_q,Vec3f &_v);

Quat Slerp(const Quat &Q1,const Quat &_Q2, const Float t);

void ConvSquadTangents(const Quat &Q1,const Quat &Q2,const Quat &Q3,const Quat &Q4,Vec3f &D1,Vec3f &D2,Vec3f &D3);
void Squad(const Quat &Q1,const Vec3f &D1,const Vec3f &D2,const Vec3f &D3,Float t,Quat &result);

#include <Quat_Z.inl>

#endif //_MATH_QUAT_Z_H
