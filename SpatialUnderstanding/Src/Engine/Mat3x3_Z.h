// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#ifndef _MATH_MATRIX3x3_Z_H
#define _MATH_MATRIX3x3_Z_H

#include <BeginDef_Z.h>

struct Mat4x4;

//----------------------------------------------------------------
// Mat3x3
//----------------------------------------------------------------

POSTALIGNED128_Z struct   Mat3x3
{
	// Data Union for last columm access
		union	mat
		{
			float			m[3][4];
			VecFloat3x3     m128;

			//Row2
				struct	{
					float		f[2];
					UDummy		dummy;
					float		v;
				}m02;
				struct	{
					float		f[6];
					UDummy		dummy;
					float		v;
				}m12;
				struct	{
					float		f[10];
					UDummy		dummy;
					float		v;
				}m22;

			//Row3
				struct	{
					float		f[3];
					UDummy		dummy;
				}m03;
				struct	{
					float		f[7];
					UDummy		dummy;
				}m13;
				struct	{
					float		f[11];
					UDummy		dummy;
				}m23;
		}m ALIGNED128_Z;

		inline operator VecFloat3x3& ()  { return m.m128; }

		Mat3x3()						{}
		Mat3x3( const Mat3x3& m );
		Mat3x3( const Mat4x4& m );
		Mat3x3& SetNull();
		Mat3x3& SetIdentity();
			void	Transp(Mat3x3 &_Out) const;
	// Operator
		Mat3x3  &operator =(const Mat3x3 &_m);
		Mat3x3  &operator *=(const Mat3x3 &_m);
		Mat3x3	operator *(const Mat3x3 &_m) const;

		Float Determinant() const;
		
        void Set ( const Mat4x4& _m);
inline	Vec2f	operator*(const Vec2f &_v) const;

inline	Vec3f	operator*(const Vec3f &_v) const;
inline	Vec4f	operator*(const Vec4f &_v) const;
		Bool	operator==(const Mat3x3 &_m) const;
		Bool	operator!=(const Mat3x3 &_m) const;
        const Vec4f&  GetRow( const int x) const;
		Vec4f&  GetRow( const int x);
		void	GetEular(Vec3f &Eular);
};

inline Bool Inverse3x3(const Mat3x3& In,Mat3x3& Out);	// Real inversion (works for any 3x3 matrix)
inline void Inverse(const Mat3x3& In,Mat3x3& Out);		// Transpose (works for rotations)

#include <Mat3x3_Z.inl>

#endif //_MATH_MATRIX3x3_Z_H
