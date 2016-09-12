// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#ifndef _MATH_MATRIX4x4_Z_H
#define _MATH_MATRIX4x4_Z_H

#include <BeginDef_Z.h>

struct Mat4x4;

inline void Mat4x4_Mul(Mat4x4& _result, const Mat4x4& _m0, const Mat4x4& _m1); // _result = _m0 * _m1;
inline void Inverse4x3(const Mat4x4& In,Mat4x4& Out);	// Inversion for a matrix with any 3x3 rotation/scale and a translation
inline void Inverse4x4(const Mat4x4& In,Mat4x4& Out);	// Inversion for any 4x4 matrix
inline void Inverse(const Mat4x4& In,Mat4x4& Out);		// Inversion for a matrix with a 3x3 rotation without scale and a translation

POSTALIGNED128_Z struct  Mat4x4
{
	union
	{
		Float		m[4][4]	ALIGNED128_Z;
		VecFloat4x4 m128;
	};

	inline operator VecFloat4x4& ()  { return m128; }
	inline Mat4x4 & operator =(const Mat4x4 &_m);

	Mat4x4() {}
	explicit Mat4x4( const Float _s );
	explicit Mat4x4(const Vec3f &Trans, const Quat &Rot,const Vec3f& Scale)
	{
		SetTRS(Trans,Rot,Scale);
	}	
	explicit Mat4x4(const Vec3f &Trans, const Quat &Rot,const Float Scale)
	{
		SetTRS(Trans,Rot,Scale);
	}	
	Mat4x4( const Mat3x3& _Mat );
	Mat4x4( const Mat4x4& _Mat );

	const Mat3x3& m3() const;
	Mat3x3& m3();
	
	Mat4x4& SetNull();
	Mat4x4& SetIdentity();
	Mat4x4& Inverse4x3() { ::Inverse4x3(*this,*this); return *this; }	// Inversion for a matrix with any 3x3 rotation/scale and a translation
	Mat4x4& Inverse4x4() { ::Inverse4x4(*this,*this); return *this; }	// Inversion for any 4x4 matrix
	Mat4x4& Inverse()	 { ::Inverse(*this,*this); return *this; }		// Inversion for a matrix with a 3x3 rotation without scale and a translation

	// We count of Return-Value Optimization to avoid the unnecessary copy when building the optimized version ( https://en.wikipedia.org/wiki/Return_value_optimization )
	Mat4x4 GetInverse4x3()	const { Mat4x4 mtx; ::Inverse4x3(*this,mtx);	return mtx; }	// Inversion for a matrix with any 3x3 rotation/scale and a translation
	Mat4x4 GetInverse4x4()	const { Mat4x4 mtx; ::Inverse4x4(*this,mtx);	return mtx; }	// Inversion for any 4x4 matrix
	Mat4x4 GetInverse()		const { Mat4x4 mtx; ::Inverse(*this,mtx);		return mtx; }	// Inversion for a matrix with a 3x3 rotation without scale and a translation

	// Operator
	Vec3f operator*(const Vec3f &_v) const;
	Vec4f operator*(const Vec4f &_v) const;

	Vec3f MulHomogenous(const Vec3f &_v) const;

    Mat4x4& operator *=(const Mat4x4 &_m);
    Mat4x4 operator *(const Mat4x4 &_m) const;

	operator Float *();
	operator const Float *() const;

	Bool operator ==(const Mat4x4 &_m) const;
	Bool operator !=(const Mat4x4 &_m) const;

	const Vec4f& GetRow( const int x ) const;
    Vec4f& GetRow( const int x );

	void MulWithoutTrans(const Vec3f &_v,Vec4f &_o) const;
    void MulWithoutTrans(const Vec3f &_v,Vec3f &_o) const;
    void MulWithoutTrans(const Vec4f &_v,Vec4f &_o) const;

	void Transp(Mat4x4 &_Out) const;

	Float GetUniformScale() const;
	void GetScale(Vec3f &Scale) const;
	const Vec3f& GetMatrixTrans() const;
	const Vec4f& GetMatrixTrans4()const;

    void SetTRS( const Vec3f &Trans, const Quat &Rot,const Vec3f& Scale );
    void SetTRS( const Vec3f &Trans, const Quat &Rot, const Float Scale );

	U32	GetCRC() const;
};

const EXTERN_Z Mat4x4 MAT4X4_ZERO;
const EXTERN_Z Mat4x4 MAT4X4_IDENTITY;

#include <Mat4x4_Z.inl>

#endif //_MATH_MATRIX4x4_Z_H
