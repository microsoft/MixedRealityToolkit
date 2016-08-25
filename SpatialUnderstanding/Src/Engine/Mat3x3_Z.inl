// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

inline Mat3x3::Mat3x3( const Mat3x3& _m )						
{
	VecFloat3x3Copy( m.m128, _m.m.m128 ); 
}

inline Mat3x3& Mat3x3::SetNull()						
{
	VecFloat3x3SplatZero( m.m128 );
	return *this;
}
	
inline Mat3x3& Mat3x3::SetIdentity()					
{
	VecFloat3x3SetIdentity( m.m128 );
	return *this;
}

inline Float Mat3x3::Determinant() const
{
	return VecFloatGetX( VecFloatDeterminantM3x3(m.m128) );
}

inline Mat3x3& Mat3x3::operator=( const Mat3x3& _m )						
{
	VecFloat3x3Copy( m.m128, _m.m.m128 ); 
	return *this;
}

inline Vec2f Mat3x3::operator*(const Vec2f &_v) const
{
	Vec2f	vec;

	vec.x=m.m[0][0]*_v.x + m.m[1][0]*_v.y + m.m[2][0];
	vec.y=m.m[0][1]*_v.x + m.m[1][1]*_v.y + m.m[2][1];
	
	return vec;
}

inline Vec3f Mat3x3::operator*(const Vec3f &_v) const
{
	Vec3f	vec;

	vec.x=m.m[0][0]*_v.x + m.m[1][0]*_v.y + m.m[2][0]*_v.z;
	vec.y=m.m[0][1]*_v.x + m.m[1][1]*_v.y + m.m[2][1]*_v.z;
	vec.z=m.m[0][2]*_v.x + m.m[1][2]*_v.y + m.m[2][2]*_v.z;
	
	return vec;
}

inline Vec4f Mat3x3::operator*(const Vec4f &_v) const
{
	Vec4f vec = VecFloat3x3Transform( m.m128, _v.vec128 );
	vec.w = 1.f; // c'est utile �a?
	return vec;
}

inline Mat3x3 &Mat3x3::operator *=(const Mat3x3 &_m)
{
	VecFloat3x3Multiply( m.m128, m.m128, _m.m.m128 );
	return *this;
}

inline Mat3x3 Mat3x3::operator *(const Mat3x3 &_m) const
{
	Mat3x3 Temp;
	VecFloat3x3Multiply( Temp.m.m128, m.m128, _m.m.m128 );
	return Temp;
}

inline void Mat3x3::Transp(Mat3x3 &_Out) const
{
	VecFloat3x3Transpose( _Out.m.m128, m.m128 );
}

inline Bool Mat3x3::operator ==(const Mat3x3 &_m) const
{
	if (fabs(m.m[0][0]-_m.m.m[0][0])>Float_Eps) return FALSE;
	if (fabs(m.m[1][0]-_m.m.m[1][0])>Float_Eps) return FALSE;	
	if (fabs(m.m[2][0]-_m.m.m[2][0])>Float_Eps) return FALSE;
	if (fabs(m.m[0][1]-_m.m.m[0][1])>Float_Eps) return FALSE;
	if (fabs(m.m[1][1]-_m.m.m[1][1])>Float_Eps) return FALSE;	
	if (fabs(m.m[2][1]-_m.m.m[2][1])>Float_Eps) return FALSE;
	if (fabs(m.m[0][2]-_m.m.m[0][2])>Float_Eps) return FALSE;
	if (fabs(m.m[1][2]-_m.m.m[1][2])>Float_Eps) return FALSE;	
	if (fabs(m.m[2][2]-_m.m.m[2][2])>Float_Eps) return FALSE;
	return	TRUE;
}

inline Bool Mat3x3::operator !=(const Mat3x3 &_m) const
{
	return !operator==(_m);
}

inline	const Vec4f& Mat3x3::GetRow( const int x) const
{
	ASSERT_Z(x>=0 && x<3);
	return reinterpret_cast<const Vec4f&>(m.m[x]);
}

inline	Vec4f& Mat3x3::GetRow( const int x)
{
	ASSERT_Z(x>=0 && x<3);
	return reinterpret_cast<Vec4f&>(m.m[x]);
}


inline Bool Inverse3x3(const Mat3x3& In,Mat3x3& Out)	// Real inversion (works for any 3x3 matrix)
{
	// Calculate the determinant of the 3x3 rotation matrix
	Float	Det( In.Determinant() );
	if (Det)
	{
		Det = 1.f/Det;
		// Invert the 3x3 rotation matrix
		Out.m.m[0][0] =  (In.m.m[1][1] * In.m.m[2][2] - In.m.m[2][1] * In.m.m[1][2]) * Det;
		Out.m.m[0][1] = -(In.m.m[0][1] * In.m.m[2][2] - In.m.m[2][1] * In.m.m[0][2]) * Det;
		Out.m.m[0][2] =  (In.m.m[0][1] * In.m.m[1][2] - In.m.m[1][1] * In.m.m[0][2]) * Det;
		Out.m.m[1][0] = -(In.m.m[1][0] * In.m.m[2][2] - In.m.m[2][0] * In.m.m[1][2]) * Det;
		Out.m.m[1][1] =  (In.m.m[0][0] * In.m.m[2][2] - In.m.m[2][0] * In.m.m[0][2]) * Det;
		Out.m.m[1][2] = -(In.m.m[0][0] * In.m.m[1][2] - In.m.m[1][0] * In.m.m[0][2]) * Det;
		Out.m.m[2][0] =  (In.m.m[1][0] * In.m.m[2][1] - In.m.m[2][0] * In.m.m[1][1]) * Det;
		Out.m.m[2][1] = -(In.m.m[0][0] * In.m.m[2][1] - In.m.m[2][0] * In.m.m[0][1]) * Det;
		Out.m.m[2][2] =  (In.m.m[0][0] * In.m.m[1][1] - In.m.m[1][0] * In.m.m[0][1]) * Det;
		return TRUE;
	}
	return FALSE;
}

inline void Inverse(const Mat3x3& In,Mat3x3& Out)	// Transpose (works for rotations)
{
	Out.m.m[0][0] = In.m.m[0][0];	Out.m.m[0][1] = In.m.m[1][0];	Out.m.m[0][2] = In.m.m[2][0];
	Out.m.m[1][0] = In.m.m[0][1];	Out.m.m[1][1] = In.m.m[1][1];	Out.m.m[1][2] = In.m.m[2][1];
	Out.m.m[2][0] = In.m.m[0][2];	Out.m.m[2][1] = In.m.m[1][2];	Out.m.m[2][2] = In.m.m[2][2];
}


FINLINE_Z void Mat3x3_Mul(Mat3x3& _result, const Mat3x3& _m0, const Mat3x3& _m1)
{
	VecFloat3x3Multiply( _result.m.m128, _m0.m.m128, _m1.m.m128 );
}