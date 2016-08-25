// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.


inline Mat4x4::Mat4x4(const Mat3x3 &_Mat)
{
	VecFloat4x4CopyM3x3( m128, _Mat.m.m128 );
}

inline Mat4x4::Mat4x4( const Float _s )						
{
	VecFloat4x4Load1( m128, _s );
}

inline Mat4x4::Mat4x4( const Mat4x4& _Mat )						
{
	VecFloat4x4Copy( m128, _Mat.m128 );
}

inline Mat4x4 & Mat4x4::operator =(const Mat4x4 &_m)
{
	VecFloat4x4Copy( m128, _m.m128 );
	return *this;
}

inline const Mat3x3& Mat4x4::m3() const				
{
	return reinterpret_cast <const Mat3x3&> ( *this );
}

inline Mat3x3& Mat4x4::m3()				
{
	return reinterpret_cast <Mat3x3&> ( *this );
}

inline Mat4x4& Mat4x4::SetNull()						
{
	VecFloat4x4SplatZero( m128 );
	return *this;
}

inline Mat4x4& Mat4x4::SetIdentity()					
{
	VecFloat4x4SetIdentity( m128 );
	return *this;
}

inline void Mat4x4::Transp(Mat4x4 &_Out) const
{
	VecFloat4x4Transpose( _Out.m128, m128 );
}

inline Mat4x4::operator Float *() { return &m[0][0]; }

inline Mat4x4::operator const Float *() const { return &m[0][0]; }

inline Bool Mat4x4::operator ==(const Mat4x4 &_m) const
{
	const VecFloat4 epsilon( VEC4F_EPSILON );
	U32 result = VecFloatAllNearEqual( VecFloat4x4GetRow0(m128), VecFloat4x4GetRow0(_m.m128), epsilon )
		       + VecFloatAllNearEqual( VecFloat4x4GetRow1(m128), VecFloat4x4GetRow1(_m.m128), epsilon )
		       + VecFloatAllNearEqual( VecFloat4x4GetRow2(m128), VecFloat4x4GetRow2(_m.m128), epsilon )
		       + VecFloatAllNearEqual( VecFloat4x4GetRow3(m128), VecFloat4x4GetRow3(_m.m128), epsilon );
	return (result >> 2) != 0;
}

inline Bool Mat4x4::operator !=(const Mat4x4 &_m) const
{
	return ! (*this == _m);  
}

inline Mat4x4 &Mat4x4::operator *=(const Mat4x4 &_m)
{
	VecFloat4x4Multiply( m128, m128, _m.m128 );
	return	*this;
}

inline Mat4x4 Mat4x4::operator *(const Mat4x4 &_m) const
{
	Mat4x4 Temp;
	VecFloat4x4Multiply( Temp.m128, m128, _m.m128 );
	return	Temp;
}

inline	const Vec4f& Mat4x4::GetRow( const int x ) const
{
	ASSERT_Z(x>=0 && x<4);
	return reinterpret_cast<const Vec4f&>(m[x]);
}

inline	Vec4f& Mat4x4::GetRow( const int x )
{
	ASSERT_Z(x>=0 && x<4);
	return reinterpret_cast<Vec4f&>(m[x]);
}

inline U32 Mat4x4::GetCRC() const
{
	U32	*pCRC = (U32*)(&m);
	U32 CRC = *pCRC++;
	CRC ^= *pCRC++;CRC ^= *pCRC++;CRC ^= *pCRC++;
	CRC ^= *pCRC++;CRC ^= *pCRC++;CRC ^= *pCRC++;CRC ^= *pCRC++;
	CRC ^= *pCRC++;CRC ^= *pCRC++;CRC ^= *pCRC++;CRC ^= *pCRC++;
	CRC ^= *pCRC++;CRC ^= *pCRC++;CRC ^= *pCRC++;CRC ^= *pCRC++;

	return CRC;
}

inline Vec3f Mat4x4::operator*(const Vec3f &_v) const
{
	Vec4f vec = VecFloat4x4Transform3( m128, Vec4f(_v) );
	return vec.xyz();
}

inline Vec4f Mat4x4::operator*(const Vec4f &_v) const
{
	Vec4f vec = VecFloat4x4Transform3( m128, _v );
	vec.w = 1.f;
	return vec;
}

inline Vec3f Mat4x4::MulHomogenous(const Vec3f &_v) const
{
	Vec4f vec = VecFloat4x4TransformH3( m128, VecFloatLoadUnaligned(&_v.x) );
	return vec.xyz();
}

inline void	Mat4x4::MulWithoutTrans(const Vec3f &_v,Vec4f &_o) const
{
	_o = m3() * _v;
}

inline void	Mat4x4::MulWithoutTrans(const Vec3f &_v,Vec3f &_o) const
{
	_o = m3() * _v;
}

inline void	Mat4x4::MulWithoutTrans(const Vec4f &_v,Vec4f &_o) const
{
	_o = m3() * _v;
}

inline Float	Mat4x4::GetUniformScale() const
{
	return Vec3f(m[0][0],m[1][0],m[2][0]).GetNorm();
}

inline void	Mat4x4::GetScale(Vec3f &Scale) const
{
	Scale.x = reinterpret_cast <const Vec3f*> ( &m[0][0] )->GetNorm();
	Scale.y = reinterpret_cast <const Vec3f*> ( &m[1][0] )->GetNorm();
	Scale.z = reinterpret_cast <const Vec3f*> ( &m[2][0] )->GetNorm();
}

inline const Vec3f& Mat4x4::GetMatrixTrans() const  { return reinterpret_cast<const Vec3f&>(m[3][0]); }

inline const Vec4f& Mat4x4::GetMatrixTrans4() const { return reinterpret_cast<const Vec4f&>(m[3][0]); }

inline void Inverse4x3(const Mat4x4& In,Mat4x4& Out)
{
	// Calculate the determinant of the 3x3 rotation matrix
	Float Det1 = 1.0f / In.m3().Determinant();
	// Invert the 3x3 rotation matrix
	Out.m[0][0] =  (In.m[1][1] * In.m[2][2] - In.m[2][1] * In.m[1][2]) * Det1;
	Out.m[0][1] = -(In.m[0][1] * In.m[2][2] - In.m[2][1] * In.m[0][2]) * Det1;
	Out.m[0][2] =  (In.m[0][1] * In.m[1][2] - In.m[1][1] * In.m[0][2]) * Det1;
	Out.m[1][0] = -(In.m[1][0] * In.m[2][2] - In.m[2][0] * In.m[1][2]) * Det1;
	Out.m[1][1] =  (In.m[0][0] * In.m[2][2] - In.m[2][0] * In.m[0][2]) * Det1;
	Out.m[1][2] = -(In.m[0][0] * In.m[1][2] - In.m[1][0] * In.m[0][2]) * Det1;
	Out.m[2][0] =  (In.m[1][0] * In.m[2][1] - In.m[2][0] * In.m[1][1]) * Det1;
	Out.m[2][1] = -(In.m[0][0] * In.m[2][1] - In.m[2][0] * In.m[0][1]) * Det1;
	Out.m[2][2] =  (In.m[0][0] * In.m[1][1] - In.m[1][0] * In.m[0][1]) * Det1;

	Float Tx=In.m[3][0];
	Float Ty=In.m[3][1];
	Float Tz=In.m[3][2];
	// Translation
	Out.m[3][0] = -(Out.m[0][0] * Tx + Out.m[1][0] * Ty + Out.m[2][0] * Tz);
	Out.m[3][1] = -(Out.m[0][1] * Tx + Out.m[1][1] * Ty + Out.m[2][1] * Tz);
	Out.m[3][2] = -(Out.m[0][2] * Tx + Out.m[1][2] * Ty + Out.m[2][2] * Tz);

	Out.m[0][3] = 0.f;
	Out.m[1][3] = 0.f;
	Out.m[2][3] = 0.f;
	Out.m[3][3] = 1.f;
}

// Total inversion (use this for projection matrices)
inline void Inverse4x4(const Mat4x4& In,Mat4x4& Out)
{
	VecFloat4x4Inverse(Out.m128, In.m128);
}

inline void Inverse(const Mat4x4& In,Mat4x4& Out) // Affine inverse: invert length & angle preserving matrix
{
	// Transpose the 3x3 rotation matrix
	In.m3().Transp( Out.m3() );
	Float Tx=In.m[3][0];
	Float Ty=In.m[3][1];
	Float Tz=In.m[3][2];
	// Translation
	Out.m[3][0] = -(Out.m[0][0] * Tx + Out.m[1][0] * Ty + Out.m[2][0] * Tz);
	Out.m[3][1] = -(Out.m[0][1] * Tx + Out.m[1][1] * Ty + Out.m[2][1] * Tz);
	Out.m[3][2] = -(Out.m[0][2] * Tx + Out.m[1][2] * Ty + Out.m[2][2] * Tz);

	// Last line
	Out.m[0][3] = 0.f;
	Out.m[1][3] = 0.f;
	Out.m[2][3] = 0.f;
	Out.m[3][3] = 1.f;
}


FINLINE_Z  void Mat4x4_Mul(Mat4x4& _result, const Mat4x4& _m0, const Mat4x4& _m1)
{
	VecFloat4x4Multiply( _result.m128, _m1.m128, _m0.m128 );
}