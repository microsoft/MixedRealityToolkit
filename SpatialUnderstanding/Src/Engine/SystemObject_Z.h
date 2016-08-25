// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#ifndef _SYSTEM_OBJECT_Z_H
#define _SYSTEM_OBJECT_Z_H

#include <Math_Z.h>
#include <StaticArray_Z.h>

#include <BeginDef_Z.h>

struct   Sphere_Z;
struct 	 Box_Z;

//---------------------------------------------------------------------
// Box_Z (OBB)
//---------------------------------------------------------------------
typedef DynArray_Z<Box_Z,8,FALSE,FALSE> Box_ZDA;
struct 	 Box_Z				//Assume Box_Z is not aligned, for SurfaceCache struct
{
	// Data
	Mat3x3		Mat;		//Mat3x3 is [3][4]
							//3x3 is pure Rotation [0][3],[1][3],[2][3] is Center
	Vec3f		Scale;

	// In Fact Ray is the RADIUS, wrong translation from french, sorry...
	Float		Ray/*radius in fact*/;		// = Scale.GerNorm();///
	
	// Method
				Box_Z(): Scale(1.f, 1.f, 1.f), Ray(1.f) {Mat.SetIdentity();}
				Box_Z(const Quat &_Rot,const Vec3f &_Trans,const Vec3f &_Size);
			void		Set(const Quat &_Rot,const Vec3f &_Trans,const Vec3f &_Size);
			void		Set(const Vec4f &_Right,const Vec4f &_Up,const Vec4f &_Front,const Vec3f &_Trans,const Vec3f &_Size);
			void		Set(const Vec3f &_Right,const Vec3f &_Up,const Vec3f &_Front,const Vec3f &_Trans,const Vec3f &_Size);
			void		Set(const Sphere_Z &_Sph);
			Box_Z		&operator =(const Box_Z &_Box);
			Box_Z		&operator +=(const Box_Z &_Box);
			Box_Z		&operator +=(const Sphere_Z &_Sph);
			Box_Z		operator *(const Mat4x4 &_Mat) const;
			void		Build(const Vec3f &_Center,const Vec3f &_YAxis,const Vec3f *_PointList,U32 _NumPoint, Bool _bAllowTinyBoxes=FALSE);
			void		Build(const VecFloat4 &_Center,const VecFloat4 &_YAxis,const VecFloat4 *_PointList,U32 _NumPoint, Bool _bAllowTinyBoxes=FALSE);
			void		GetVtx(Vec3f *_8Vtx) const;

			void		UpdateScale(){Ray=Scale.GetNorm();}
			void		UpdateScale(const Vec3f &newscale){Scale=newscale;Ray=Scale.GetNorm();}

			Vec3f		GetCenter() const	{ return Vec3f( Mat.m.m[0][3], Mat.m.m[1][3], Mat.m.m[2][3] );  }

	// Computes Box's dimension on 3 axes (from center)
	// GetTranslation() + GetFrameDim() => BOX'S MAX on 3 axes
	// GetTranslation() - GetFrameDim() => BOX'S MIN on 3 axes
	// AVOIDS DOIN A GetVtx AND A for (i=0; i<8; i++) min max...
	inline	void		GetFrameDim(Vec3f &dim) const
	{
		dim.x=POS_Z(Mat.m.m[0][0])*Scale.x+POS_Z(Mat.m.m[1][0])*Scale.y+POS_Z(Mat.m.m[2][0])*Scale.z;
		dim.y=POS_Z(Mat.m.m[0][1])*Scale.x+POS_Z(Mat.m.m[1][1])*Scale.y+POS_Z(Mat.m.m[2][1])*Scale.z;
		dim.z=POS_Z(Mat.m.m[0][2])*Scale.x+POS_Z(Mat.m.m[1][2])*Scale.y+POS_Z(Mat.m.m[2][2])*Scale.z;
	}
	inline	void		GetTranslation(Vec3f &Trans) const {Trans.x=Mat.m.m[0][3],Trans.y=Mat.m.m[1][3],Trans.z=Mat.m.m[2][3];}
	inline	void		SetTranslation(const Vec3f &Trans) {Mat.m.m[0][3]=Trans.x,Mat.m.m[1][3]=Trans.y,Mat.m.m[2][3]=Trans.z;}
	inline	void		GetTranslation(Vec4f &Trans) const {Trans.x=Mat.m.m[0][3],Trans.y=Mat.m.m[1][3],Trans.z=Mat.m.m[2][3],Trans.w=0.f;}
	inline	void		SetTranslation(const Vec4f &Trans) {Mat.m.m[0][3]=Trans.x,Mat.m.m[1][3]=Trans.y,Mat.m.m[2][3]=Trans.z;}
	
	typedef StaticArray_Z<Vec2f,8> Poly2D;
};

//---------------------------------------------------------------------
// Segment
//---------------------------------------------------------------------
struct POSTALIGNED128_Z  Segment_Z			//aligned
{
	// Data
	Vec3f	Org ALIGNED128_Z;
	Float	Len;
	Vec3f	Dir;
	Float	pad;			// alignment padding !
		
	// Method
	Segment_Z() {pad = 0.f;}
	Segment_Z(const Vec3f &_Org,const Vec3f &_Dir,Float _Len) { Org = _Org ; Dir = _Dir ; Len = _Len ; pad = 0.f;}
	Segment_Z(const Vec3f &_P1,const Vec3f &_P2)			
	{	
		Org=_P1;
		Dir=_P2-_P1;
		Len= Dir.GetNorm();
		ASSERTC_Z(Len>Float_Eps,"Segment_Z::Dir Invalid");
		if(Len>Float_Eps)
			Dir/=Len;
		pad = 0.f;
	}
	Float	DistToPoint(const Vec3f &ToPt,Vec3f *pPt0=NULL) const;
};

inline Segment_Z operator *(const Mat4x4 &Mat,const Segment_Z &Seg)
{
	Segment_Z	S;
	S.Org  = Mat*Seg.Org;
	Mat.MulWithoutTrans(Seg.Dir,S.Dir);
	Float  Scale  = S.Dir.GetNorm();
	S.Len  = Scale * Seg.Len;
	if (Scale>Float_Eps)
		S.Dir /= Scale;
	return S;
}

//---------------------------------------------------------------------
// Sphere
//---------------------------------------------------------------------
POSTALIGNED128_Z struct  Sphere_Z
{
	Vec3f	Center	ALIGNED128_Z;
	Float	Radius;

						inline operator const VecFloat4& () const	{ return reinterpret_cast <const VecFloat4&>(Center); }

						Sphere_Z() {}
						Sphere_Z( const VecFloat4& centerRadius ) { VecFloatStoreAligned(&Center,centerRadius); }
						Sphere_Z(const Vec3f &_Center,Float _Radius)
						{
							Center = _Center; 
							Radius = _Radius;
						}

	inline	const Vec3f	&GetCenter()const{return Center;}
	inline	Float		GetRadius()const{return Radius;}
	inline	void		SetCenter(const Vec3f &c){Center=c;}
	inline	void		SetRadius(Float r){Radius=r;}
			void		Set(const Segment_Z *VecPtr,S32 NbSeg);
			void		Set(const Vec3f *VecPtr,S32 NbPoint);
			void		Add(const Vec3f &Vec);
			Sphere_Z	&operator =(const Sphere_Z &Sphere)				{Center = Sphere.Center; Radius = Sphere.Radius; return *this;}
			Sphere_Z	operator +(const Box_Z &Box) const;
	inline	Sphere_Z 	operator +(const Sphere_Z &Sphere) const;
			Sphere_Z &  operator +=(const Sphere_Z &Sphere);
};

EXTERN_Z Sphere_Z operator *(const Mat4x4 &Mat,const Sphere_Z &Sphere);



inline	Sphere_Z  Sphere_Z::operator +(const Sphere_Z &Sphere) const
{
	Sphere_Z	aSph=*this;
	aSph+=Sphere;
	return	aSph;
}

#endif