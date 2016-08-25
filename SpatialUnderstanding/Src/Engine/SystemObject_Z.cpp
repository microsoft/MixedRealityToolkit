// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include <pch.h>
#include <SystemObject_Z.h>

void Sphere_Z::Set(const Segment_Z *VecPtr,S32 NbSeg)
{
	int i;
	Vec3f	VMin,VMax;
	MinVec(VecPtr[0].Org+VecPtr[0].Dir*VecPtr[0].Len,VecPtr[0].Org,VMin);
	MaxVec(VecPtr[0].Org+VecPtr[0].Dir*VecPtr[0].Len,VecPtr[0].Org,VMax);
	for(i = 1; i < NbSeg ; i++)
	{
		MinVec(VMin,VecPtr[i].Org,VMin);
		MaxVec(VMax,VecPtr[i].Org,VMax);
		MinVec(VMin,VecPtr[i].Org+VecPtr[i].Dir*VecPtr[i].Len,VMin);
		MaxVec(VMax,VecPtr[i].Org+VecPtr[i].Dir*VecPtr[i].Len,VMax);
	}

	Center = (VMax+VMin)/2.f;

	Float	Longest = (Center - VecPtr[0].Org).GetNorm2();

	for(i = 1; i < NbSeg ; i++)
	{
		Longest = Max(Longest,(Center - VecPtr[i].Org).GetNorm2());
		Longest = Max(Longest,(Center - (VecPtr[i].Org+VecPtr[i].Dir*VecPtr[i].Len)).GetNorm2());
	}
	Radius = Sqrt(Longest);
}

void Sphere_Z::Set(const Vec3f *VecPtr,S32 NbPoint)
{
	S32		i;
	if (!NbPoint) return;
	Vec3f	VMin,VMax;
	VMin = VMax = VecPtr[0];
	for(i = 1; i < NbPoint ; i++)
	{
		MinVec(VMin,VecPtr[i],VMin);
		MaxVec(VMax,VecPtr[i],VMax);
	}

	Center = (VMax+VMin)/2.f;

	Float	Longest = (Center - VecPtr[0]).GetNorm2();

	for(i = 1; i < NbPoint ; i++)
		Longest = Max(Longest,(Center - VecPtr[i]).GetNorm2());

	Radius = Sqrt(Longest);
}

static inline Sphere_Z& test( Sphere_Z& res, const Sphere_Z& Sphere )
{
	Vec4f	SphereDir;
	Vec4_Sub(SphereDir,*reinterpret_cast<const Vec4f*>(&Sphere.Center),*reinterpret_cast<const Vec4f*>(&res.Center) ); //cast Vec3f->Vec4f possible car pad pr�sent apr�s Center
	Float		Len=Vec4_GetNorm2(SphereDir);

	if(Len>Float_Eps*Float_Eps)
	{
		Len=Sqrt(Len);
		if (res.Radius>Len+Sphere.Radius)				// this contient Sphere
			return res;
		else if (Sphere.Radius>res.Radius+Len)			// Sphere contient this
		{
			res.Center=Sphere.Center;
			res.Radius=Sphere.Radius;
		}
		else
		{
			Float	NewRadius=(Len+Sphere.Radius+res.Radius)*0.5f;
			Vec4_Add_Scale(*reinterpret_cast<Vec4f*>(&res.Center),*reinterpret_cast<Vec4f*>(&res.Center),(NewRadius-res.Radius)/Len,SphereDir);
			res.Radius=NewRadius;
		}
	}
	else
		res.Radius=Max(Sphere.Radius,res.Radius);

	return res;
}

Sphere_Z operator *(const Mat4x4 &Mat,const Sphere_Z &Sphere)
{
	Sphere_Z	S;
	register VecFloat4 result;
	register VecFloat4 radius;
    register const VecFloat4x4& vMat( Mat.m128 );
	register const VecFloat4 length = VecFloatLength3( VecFloat4x4GetRow0(vMat) );
    register const VecFloat4 vSphere = VecFloatLoadAligned( &Sphere.Center.x );

    result = VecFloat4x4Transform3( vMat, vSphere );
	radius = VecFloatScaleW( length, vSphere );
    result = VecFloatPermuteX0Y0Z0X1( result, radius );
    VecFloatStoreAligned( &S.Center.x, result );
	return S;
}

Sphere_Z &  Sphere_Z::operator +=(const Sphere_Z &Sphere)
{
	VecFloat4 thisCenter = *this;
	const VecFloat4 center( Sphere );
	const VecFloat4 thisRadius = VecFloatSplatW(thisCenter);
	const VecFloat4 radius = VecFloatSplatW(center);
	const VecFloat4 sqFloatEps = VecFloatLoad1(Float_Eps*Float_Eps);

	VecFloat4 SphereDir = VecFloatSub( center, thisCenter );
	VecFloat4 Len = VecFloatDot3( SphereDir, SphereDir );

	if( VecFloatAllGreater(Len,sqFloatEps) )
	{
		Len = VecFloatSqrt( Len );
		if( VecFloatAllGreater(thisRadius,VecFloatAdd(Len,radius) ) )
		{
			return *this;
		}
		else if( VecFloatAllGreater(radius,VecFloatAdd(Len,thisRadius) ) )
			thisCenter = center;
		else
		{
			VecFloat4 middle = VecFloatAdd( Len, VecFloatAdd(radius,thisRadius) );
			VecFloat4 NewRadius = VecFloatMul( middle, VEC4F_HALF );
			VecFloat4 scale = VecFloatDiv( VecFloatSub( NewRadius, thisRadius), Len );
			VecFloat4 scaled = VecFloatMul(SphereDir,scale);
			thisCenter = VecFloatAdd(thisCenter,scaled);
			thisCenter = VecFloatPermuteX0Y0Z0W1( thisCenter, NewRadius );
		}
		VecFloatStoreAligned( &Center, thisCenter );
	}
	else 
		Radius = Max( Sphere.Radius,Radius);

	return *this;
}

Sphere_Z Sphere_Z::operator +(const Box_Z &Box) const
{
	Sphere_Z	Sph(*this);
	Vec3f	lVtx[8];	
	Box.GetVtx(lVtx);
	for(S32 i=0;i<8;i++)
		Sph.Add(lVtx[i]);
	return	Sph;
}

void Sphere_Z::Add(const Vec3f &Vec)
{
	*this=*this+Sphere_Z(Vec,0.f);
}


//
// Build Box

/*
	  3_________2         y
	 /|        /|         |
   0/_| ______/ |         | 
	| |     1|  |         |
	| |      |  |   z_____|
	| |      |  |        /
	|7|______|__|6      /
	| /      | /       /
	|/_______|/       x
	4         5 
*/

//
// Normal & Index & Flag must respect same face order !
Vec3f	CubeNormal[3] =
{
	Vec3f( 0.f, 1.f, 0.f),	//Top
	Vec3f( 1.f, 0.f, 0.f),	//Near
	Vec3f( 0.f, 0.f, 1.f)	//Left
};

//
// Support Only Mat4x4 with Uniform Scale (isotropic)
Box_Z Box_Z::operator *(const Mat4x4 &_Transform) const
{
	const VecFloat4 one = VecFloatSplatOne();
	const VecFloat4 mScale = VecFloatLength3( VecFloat4x4GetRow0(_Transform.m128) );
	const VecFloat4 mInvScale = VecFloatSetWOne( VecFloatReciprocal(mScale) );

	Box_Z B;
	VecFloat4x4 transposedTransform;
	VecFloat4 matTransposedColumn3, dotSoA[4];
		
    dotSoA[0] = VecFloatPermuteZ0Z1W0W1( Mat.m.m128[0], Mat.m.m128[1] );
	dotSoA[1] = VecFloatPermuteZ0Z1W0W1( Mat.m.m128[2], one );
	matTransposedColumn3 = VecFloatPermuteZ0W0Z1W1( dotSoA[0], dotSoA[1] ); // = { Mat.m[0][3], Mat.m[1][3], Mat.m[2][3], 1 }

	VecFloat4x4Transpose( transposedTransform, _Transform.m128 );
	
	dotSoA[0] = VecFloatDot3( transposedTransform[0], Mat.m.m128[0] );
	dotSoA[1] = VecFloatDot3( transposedTransform[1], Mat.m.m128[0] );
	dotSoA[2] = VecFloatDot3( transposedTransform[2], Mat.m.m128[0] );
	dotSoA[3] = VecFloatDot4( transposedTransform[0], matTransposedColumn3 );
	B.Mat.m.m128[0] = VecFloatMul( VecFloatTransposeColumnsX(dotSoA), mInvScale );
	
	dotSoA[0] = VecFloatDot3( transposedTransform[0], Mat.m.m128[1] );
	dotSoA[1] = VecFloatDot3( transposedTransform[1], Mat.m.m128[1] );
	dotSoA[2] = VecFloatDot3( transposedTransform[2], Mat.m.m128[1] );
	dotSoA[3] = VecFloatDot4( transposedTransform[1], matTransposedColumn3 );
	B.Mat.m.m128[1] = VecFloatMul( VecFloatTransposeColumnsX(dotSoA), mInvScale );
	
	dotSoA[0] = VecFloatDot3( transposedTransform[0], Mat.m.m128[2] );
	dotSoA[1] = VecFloatDot3( transposedTransform[1], Mat.m.m128[2] );
	dotSoA[2] = VecFloatDot3( transposedTransform[2], Mat.m.m128[2] );
	dotSoA[3] = VecFloatDot4( transposedTransform[2], matTransposedColumn3 );
	B.Mat.m.m128[2] = VecFloatMul( VecFloatTransposeColumnsX(dotSoA), mInvScale );

	VecFloat4 scaledValues = VecFloatMul( VecFloatLoadAligned(&Scale), mScale );
	VecFloatStoreAligned( &B.Scale, scaledValues );

	return B;
}

Box_Z::Box_Z(const Quat &_Rot,const Vec3f &_Trans,const Vec3f &_Size)  
{
	Set( _Rot, _Trans, _Size );
}

Box_Z	&Box_Z::operator =(const Box_Z &_Box)
{
	Mat = _Box.Mat;
	Scale=_Box.Scale;
	Ray=_Box.Ray;
	return *this;
}



void Box_Z::Set(const Vec4f &_Right,const Vec4f &_Up,const Vec4f &_Front,const Vec3f &_Trans,const Vec3f &_Size)
{
	Mat.m.m[0][0]=_Right.x;
	Mat.m.m[0][1]=_Right.y;
	Mat.m.m[0][2]=_Right.z;
	Mat.m.m[1][0]=_Up.x;
	Mat.m.m[1][1]=_Up.y;
	Mat.m.m[1][2]=_Up.z;
	Mat.m.m[2][0]=_Front.x;
	Mat.m.m[2][1]=_Front.y;
	Mat.m.m[2][2]=_Front.z;
	Scale=_Size;
	UpdateScale();
	SetTranslation(_Trans);	//Centered on Y Axis
}

void Box_Z::Set(const Vec3f &_Right,const Vec3f &_Up,const Vec3f &_Front,const Vec3f &_Trans,const Vec3f &_Size)
{
	Mat.m.m[0][0]=_Right.x;
	Mat.m.m[0][1]=_Right.y;
	Mat.m.m[0][2]=_Right.z;
	Mat.m.m[1][0]=_Up.x;
	Mat.m.m[1][1]=_Up.y;
	Mat.m.m[1][2]=_Up.z;
	Mat.m.m[2][0]=_Front.x;
	Mat.m.m[2][1]=_Front.y;
	Mat.m.m[2][2]=_Front.z;
	Scale=_Size;
	UpdateScale();
	SetTranslation(_Trans);	//Centered on Y Axis
}

void Box_Z::Set(const Quat &_Rot,const Vec3f &_Trans,const Vec3f &_Size)	
{
	_Rot.GetMatrix(Mat);
	Scale=_Size&Vec3f(0.5f,0.5f,0.5f);
	UpdateScale();
	SetTranslation(_Trans+Mat*Vec3f(0.f,_Size.y*0.5f,0.f));	//Centered on Y Axis
}

void Box_Z::Set(const Sphere_Z &_Sph)
{
	Scale.x=Scale.y=Scale.z=_Sph.Radius;
	UpdateScale();
	Mat.SetIdentity();
	SetTranslation(_Sph.Center);
}

void  Box_Z::GetVtx(Vec3f *_8Vtx) const
{
	Vec3f	Trans;	GetTranslation(Trans);

	Vec3f	Normals[6];
	Normals[0]=Mat*CubeNormal[1] * Scale.x; // ! Normals in CubeNormal are not in XYZ order
	Normals[1]=Mat*CubeNormal[0] * Scale.y;
	Normals[2]=Mat*CubeNormal[2] * Scale.z;
	Normals[3]=-Normals[0]; // For less LHS below
	Normals[4]=-Normals[1];
	Normals[5]=-Normals[2];

	*_8Vtx++ = Vec3f(Normals[0] + Normals[1] + Normals[2]) + Trans;
	*_8Vtx++ = Vec3f(Normals[0] + Normals[1] + Normals[5]) + Trans;
	*_8Vtx++ = Vec3f(Normals[3] + Normals[1] + Normals[5]) + Trans;
	*_8Vtx++ = Vec3f(Normals[3] + Normals[1] + Normals[2]) + Trans;
	*_8Vtx++ = Vec3f(Normals[0] + Normals[4] + Normals[2]) + Trans;
	*_8Vtx++ = Vec3f(Normals[0] + Normals[4] + Normals[5]) + Trans;
	*_8Vtx++ = Vec3f(Normals[3] + Normals[4] + Normals[5]) + Trans;
	*_8Vtx   = Vec3f(Normals[3] + Normals[4] + Normals[2]) + Trans;
}

Box_Z	&Box_Z::operator +=(const Box_Z &_Box)
{
	Vec3f	lVtx[16];	
	GetVtx(lVtx);
	_Box.GetVtx(lVtx+8);
	Vec3f	VMin,VMax;
	VMin = VMax = lVtx[0];
	int	i;
	for(i = 1; i < 16 ; i++)
	{
		MinVec(VMin,lVtx[i],VMin);
		MaxVec(VMax,lVtx[i],VMax);
	}
	Vec3f	Center=(VMax+VMin)/2.f;
	Vec3f	YAxis(0.f,1.f,0.f);
	Build(Center,YAxis,lVtx,16);
	return	*this;
}
Box_Z	&Box_Z::operator +=(const Sphere_Z &_Sph)
{
	Vec3f	lVtx[16];	
	GetVtx(lVtx);
	lVtx[8]=_Sph.GetCenter()	+Vec3f ( _Sph.Radius, _Sph.Radius, _Sph.Radius);
	lVtx[9]=_Sph.GetCenter()	+Vec3f (-_Sph.Radius, _Sph.Radius, _Sph.Radius);
	lVtx[10]=_Sph.GetCenter()	+Vec3f ( _Sph.Radius,-_Sph.Radius, _Sph.Radius);
	lVtx[11]=_Sph.GetCenter()	+Vec3f (-_Sph.Radius,-_Sph.Radius, _Sph.Radius);
	lVtx[12]=_Sph.GetCenter()	+Vec3f ( _Sph.Radius, _Sph.Radius, -_Sph.Radius);
	lVtx[13]=_Sph.GetCenter()	+Vec3f (-_Sph.Radius, _Sph.Radius, -_Sph.Radius);
	lVtx[14]=_Sph.GetCenter()	+Vec3f ( _Sph.Radius,-_Sph.Radius, -_Sph.Radius);
	lVtx[15]=_Sph.GetCenter()	+Vec3f (-_Sph.Radius,-_Sph.Radius, -_Sph.Radius);
	Vec3f	VMin,VMax;
	VMin = VMax = lVtx[0];
	int	i;
	for(i = 1; i < 16 ; i++)
	{
		MinVec(VMin,lVtx[i],VMin);
		MaxVec(VMax,lVtx[i],VMax);
	}
	Vec3f	Center=(VMax+VMin)/2.f;
	Vec3f	YAxis(0.f,1.f,0.f);
	Build(Center,YAxis,lVtx,16);
	return	*this;
}
// Note: If you modify Box_Z::Build( Vec3f...) also report modifications to Box_Z::Build( VecFloat4...)
void Box_Z::Build(const Vec3f &_Center,const Vec3f &_YAxis,const Vec3f *_PointList,U32 _NumPoint, Bool _bAllowTinyBoxes)
{
	Vec3f PlaneCenter = _Center;
	Vec3f XAxis,YAxis,ZAxis;

	YAxis = _YAxis;
	
	if ((YAxis.x != 0.f) || (YAxis.z != 0.f))
	{
		XAxis.x = YAxis.z;
		XAxis.y = 0;
		XAxis.z = -YAxis.x;
	}
	else
	{
		XAxis.x = 1.f;
		XAxis.y = 0.f;
		XAxis.z = 0.f;
	}

	ZAxis = (XAxis ^ YAxis);
	ZAxis.Normalize();

	XAxis = YAxis ^ ZAxis;
	XAxis.Normalize();

	Vec2f x_axis;
	Vec2f y_axis;
	Vec3f NewCenter = PlaneCenter;

	const Float fMinScale = _bAllowTinyBoxes ? Float_Eps : 0.05f;
	Float x_scale=fMinScale,y_scale=fMinScale,z_scale=fMinScale;

	Float surface = 10E+10f;		
	Float z_min = 10E+10f;
	Float z_max = -10E+10f;

	x_axis.x = 1.f;
	x_axis.y = 0.f;

	y_axis.x = 0.f;
	y_axis.y = 1.f;

	for (S32 j=0;j<90;j += 10) 
	{
		Vec2f x_axis2,y_axis2;
		Vec3f X2,Y2;

		Float angle = ((Float)j) * Pi / 180.f;

		x_axis2.x = Cos(angle);
		x_axis2.y = Sin(angle);

		y_axis2.x = -x_axis2.y;
		y_axis2.y = x_axis2.x;

		
		X2 = (XAxis * x_axis2.x) + (ZAxis * x_axis2.y);
		X2.Normalize();

		Y2 = (XAxis * y_axis2.x) + (ZAxis * y_axis2.y);
		Y2.Normalize();

		Float x_max = -10E+10f,x_min = 10E+10f;
		Float y_max = -10E+10f,y_min = 10E+10f;

		for (U32 pi=0;pi<_NumPoint;pi++)
		{
			
			Vec3f L = _PointList[pi] - PlaneCenter;

			Float val = X2 * L;
			if (val > x_max)	x_max = val;
			if (val < x_min)	x_min = val;

			val = Y2 * L;
			if (val > y_max)	y_max = val;
			if (val < y_min)	y_min = val;

			val = YAxis * L;
			if (val < z_min) z_min = val;
			if (val > z_max) z_max = val;
							
		}
		
		// build surface area of bounding box
		Float surface2 = (Float)Abs((x_max - x_min) * (y_max - y_min));

		if (surface2 < surface)
		{
			surface = surface2;

			x_axis = x_axis2;
			y_axis = y_axis2;

			NewCenter =  PlaneCenter + 
						(X2 * ((x_max + x_min) * 0.5f)) + 
						(Y2 * ((y_max + y_min) * 0.5f)) +
						(YAxis * ((z_max + z_min) * 0.5f));

			x_scale = (x_max - x_min) * 0.5f;
			y_scale = (y_max - y_min) * 0.5f;
			z_scale = (z_max - z_min) * 0.5f;
		}

	}

	Vec3f X = XAxis;
	Vec3f Y = ZAxis;

	XAxis = (X * x_axis.x) + (Y * x_axis.y);
	Scale.x = x_scale+Float_Eps;

	Scale.y = z_scale+Float_Eps;

	ZAxis = (X * y_axis.x) + (Y * y_axis.y);
	Scale.z = y_scale+Float_Eps;

	XAxis.Normalize();
	YAxis.Normalize();
	ZAxis.Normalize();

	UpdateScale();

	const Float fMin_Scale_Box	= _bAllowTinyBoxes ? Float_Eps : 0.025f;
	if (Scale.x < fMin_Scale_Box) 		Scale.x = fMin_Scale_Box;
	if (Scale.y < fMin_Scale_Box) 		Scale.y = fMin_Scale_Box;
	if (Scale.z < fMin_Scale_Box) 		Scale.z = fMin_Scale_Box;

	// Pure Rotation
	Mat.m.m[0][0] = XAxis.x;	Mat.m.m[1][0] = YAxis.x;	Mat.m.m[2][0] = ZAxis.x;
	Mat.m.m[0][1] = XAxis.y;	Mat.m.m[1][1] = YAxis.y;	Mat.m.m[2][1] = ZAxis.y;
	Mat.m.m[0][2] = XAxis.z;	Mat.m.m[1][2] = YAxis.z;	Mat.m.m[2][2] = ZAxis.z;

	SetTranslation(NewCenter);
}

// Note: If you modify Box_Z::Build( VecFloat4...) also report modifications to Box_Z::Build( Vec3f...)
void Box_Z::Build(const VecFloat4 &_Center,const VecFloat4 &_YAxis,const VecFloat4 *_PointList,U32 _NumPoint, Bool _bAllowTinyBoxes)
{
	const VecFloat4 vHalf = VecFloatLoad1(0.5f);
	const VecFloat4 vZero = VecFloatSplatZero();
	VecFloat4 PlaneCenter = _Center;
	VecFloat4 XAxis,YAxis,ZAxis;

	YAxis = _YAxis;
	
	XAxis = VEC4F_LEFT;

	const Float Yaxis_z = VecFloatGetZ(YAxis);
	const Float Yaxis_x = VecFloatGetX(YAxis);
	if (( Yaxis_x != 0.f) || ( Yaxis_z != 0.f))
	{
		VecFloatSetX( XAxis, Yaxis_z );
		VecFloatSetX( XAxis,-Yaxis_x );
	}

	ZAxis = VecFloatNormalize3( VecFloatCross3( XAxis, YAxis) );
	XAxis = VecFloatNormalize3( VecFloatCross3( YAxis, ZAxis) );

	VecFloat4 x_axis_Cos = VecFloatSplatOne();
	VecFloat4 x_axis_Sin = VecFloatSplatZero();
	VecFloat4 y_axis_Cos = x_axis_Sin;
	VecFloat4 y_axis_Sin = x_axis_Cos;
	VecFloat4 NewCenter = PlaneCenter;

	const VecFloat4 vMax = VecFloatLoad1( 10E+10f);
	const VecFloat4 vMin = VecFloatLoad1(-10E+10f);

	const Float fMinScale = _bAllowTinyBoxes ? Float_Eps : 0.05f;
	VecFloat4 x_scale = VecFloatLoad1(fMinScale);
	VecFloat4 y_scale=x_scale, z_scale=x_scale;

	VecFloat4 vSurface = vMax;
	VecFloat4 vZmax = vMin, vZmin = vMax;

	for (S32 j=0;j<90;j += 10) 
	{
		const Float angle = ((Float)j) * Pi / 180.f;
		const VecFloat4 vAngle = VecFloatLoad1( angle );
				
		const VecFloat4 x_axis_Cos2 = VecFloatCos(vAngle);
		const VecFloat4 x_axis_Sin2 = VecFloatSin(vAngle);
		const VecFloat4 y_axis_Cos2 = VecFloatNegate(x_axis_Sin2);
		const VecFloat4 y_axis_Sin2 = x_axis_Cos2;

		const VecFloat4 X2 = VecFloatNormalize3(VecFloatAdd(VecFloatMul(XAxis, x_axis_Cos2), VecFloatMul(ZAxis, x_axis_Sin2)));
		const VecFloat4 Y2 = VecFloatNormalize3(VecFloatAdd(VecFloatMul(XAxis, y_axis_Cos2), VecFloatMul(ZAxis, y_axis_Sin2)));


		VecFloat4 vXmax = vMin, vXmin = vMax;
		VecFloat4 vYmax = vMin, vYmin = vMax;

		for (U32 pi=0;pi<_NumPoint;pi++)
		{			
			const VecFloat4 L = VecFloatSub(_PointList[pi], PlaneCenter);
			VecFloat4 vDot;

			vDot = VecFloatDot3(X2, L);
			vXmax = VecFloatSelectGreater(vDot, vXmax, vDot, vXmax);
			vXmin = VecFloatSelectLess(vDot, vXmin, vDot, vXmin);
			
			vDot = VecFloatDot3(Y2, L);
			vYmax = VecFloatSelectGreater(vDot, vYmax, vDot, vYmax);
			vYmin = VecFloatSelectLess(vDot, vYmin, vDot, vYmin);
			
			vDot = VecFloatDot3(YAxis, L);
			vZmax = VecFloatSelectGreater(vDot, vZmax, vDot, vZmax);
			vZmin = VecFloatSelectLess(vDot, vZmin, vDot, vZmin);
							
		}
		
		// build surface area of bounding box
		const VecFloat4 vDiffx = VecFloatSub(vXmax,vXmin);
		const VecFloat4 vDiffy = VecFloatSub(vYmax,vYmin);
		const VecFloat4 vSurface2 =  VecFloatAbs(VecFloatMul( vDiffx, vDiffy));
		
		if ( VecFloatAllLess(vSurface2, vSurface) )
		{
			vSurface = vSurface2;

			x_axis_Cos = x_axis_Cos2;
			x_axis_Sin = x_axis_Sin2;
			y_axis_Cos = y_axis_Cos2;
			y_axis_Sin = y_axis_Sin2;

			const VecFloat4 vX = VecFloatMul(X2,	VecFloatMul(VecFloatAdd(vXmax, vXmin), vHalf));
			const VecFloat4 vY = VecFloatMul(Y2,	VecFloatMul(VecFloatAdd(vYmax, vYmin), vHalf));
			const VecFloat4 vZ = VecFloatMul(YAxis, VecFloatMul(VecFloatAdd(vZmax, vZmin), vHalf));
			NewCenter =  VecFloatAdd(PlaneCenter, VecFloatAdd(vX, VecFloatAdd(vY, vZ)));

			const VecFloat4 vDiffz = VecFloatSub(vZmax,vZmin);
			x_scale = VecFloatMul( vDiffx, vHalf );
			y_scale = VecFloatMul( vDiffy, vHalf );
			z_scale = VecFloatMul( vDiffz, vHalf );
		}

	}

	const VecFloat4 X = XAxis;
	const VecFloat4 Y = ZAxis;

	XAxis = VecFloatAdd(VecFloatMul(X, x_axis_Cos), VecFloatMul(Y, x_axis_Sin));
	Scale.x = VecFloatGetX(x_scale)+Float_Eps;

	YAxis = YAxis;
	Scale.y = VecFloatGetX(z_scale)+Float_Eps;

	ZAxis = VecFloatAdd(VecFloatMul(X, y_axis_Cos), VecFloatMul(Y, y_axis_Sin));
	Scale.z = VecFloatGetX(y_scale)+Float_Eps;

	XAxis = VecFloatNormalize3(XAxis);
	YAxis = VecFloatNormalize3(YAxis);
	ZAxis = VecFloatNormalize3(ZAxis);

	UpdateScale();

	const Float fMin_Scale_Box = _bAllowTinyBoxes ? Float_Eps : 0.025f;
	if (Scale.x < fMin_Scale_Box) 		Scale.x = fMin_Scale_Box;
	if (Scale.y < fMin_Scale_Box) 		Scale.y = fMin_Scale_Box;
	if (Scale.z < fMin_Scale_Box) 		Scale.z = fMin_Scale_Box;

	// Pure Rotation
	Mat.m.m[0][0] = VecFloatGetX(XAxis);	Mat.m.m[1][0] = VecFloatGetX(YAxis);	Mat.m.m[2][0] = VecFloatGetX(ZAxis);
	Mat.m.m[0][1] = VecFloatGetY(XAxis);	Mat.m.m[1][1] = VecFloatGetY(YAxis);	Mat.m.m[2][1] = VecFloatGetY(ZAxis);
	Mat.m.m[0][2] = VecFloatGetZ(XAxis);	Mat.m.m[1][2] = VecFloatGetZ(YAxis);	Mat.m.m[2][2] = VecFloatGetZ(ZAxis);

	SetTranslation(Vec4f(NewCenter));
}

Float	Segment_Z::DistToPoint(const Vec3f &ToPt,Vec3f *pPt0) const
{
	Float	uv=(ToPt-Org)*Dir;

	Vec3f	point;
	if (uv<0.f)
		point = Org;
	else
	{
		point = Org + Min(uv, Len) * Dir;
	}	

	if (pPt0)
		*pPt0=point;
    return (point-ToPt).GetNorm();   // return the closest distance
}



