// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include <pch.h>
#include <Math_Z.h>

const Quat QUAT_NULL = Quat(1.f,0.f,0.f,0.f);

Quat::Quat(const Vec3f &V1,const Vec3f &V2)
{
	Quat	q;

	// normer les 2 vecteurs et sortir si un vecteur est trop petit et qu'on a pas r�ussi � normer

	Float	n1=V1.GetNorm();
	Float	n2=V2.GetNorm();
	if ((n1<Float_Eps) || (n2<Float_Eps))
	{
		// Error Quat !!!
		xyzw().SetDefault();
		Normalize();
		return;
	}

	Vec3f	v1=V1* (1.f/n1);
	Vec3f	v2=V2* (1.f/n2);

	if (v1*v2 >= 0.f)
	{
		// Angle < 90
		Vec3f	v3=v1+v2;

		v3.Normalize();
		v = v1^v3;
		Float	Sin2 = v*v;

		//Should be Float_Eps*Float_Eps? 
		if (Sin2 < Float_Eps)
		{
			// Pas de rotation
			xyzw().SetDefault();
			Normalize();
			return;
		}

		Float	tempf=1.f-Sin2;
		if (tempf<0.f) tempf=0.f;
		if (tempf>1.f) tempf=1.f;
		w = Sqrt(tempf);
		Normalize();
		return;
	}

	// Angle > 90
	
	// On travaille avec le vecteur oppos�.

	Vec3f	v3=v2-v1;

	v3.CNormalize();
	v = v1^v3;
	Float	Sin2 = v*v;

	if (Sin2 < Float_Eps)
	{
		// Rotation 180.

		Float x = Abs(v1.x);
		Float y = Abs(v1.y);
		Float z = Abs(v1.z);

		if ((v1.y > -0.707106f) && (v1.y < 0.707106f))
//		if ((y < x) && (y < z))
		{
			v3.x = v1.z;
			v3.y = v1.y;
			v3.z = v1.x;
		}
		else
		{
			v3.x = v1.z;
			v3.y = v1.x;
			v3.z = v1.y;
		}

		v = v1^v3;
		v.CNormalize();
		w=0.f;
		Normalize();
		return;
	}

	w = Sqrt(Sin2);
	if (w)
		v *= Sqrt(1.f-Sin2) / w;

	Normalize();
}

Quat::Quat(const Vec3f &V1,const Vec3f &V2,const Vec3f &V3)
{
	Quat	aQuat=Quat(V2,V1);

	Float	ang=Atan2(V1.x,V1.z);

	Quat	aQuat2=Quat(ang,V3);

	*this=aQuat*aQuat2;
}

Quat	&Quat::operator= (const Vec3f &RV)
{
	Float	_w=RV.GetNorm();
	if (_w<Float_Eps)
	{
		xyzw().SetDefault();
		return *this;
	}

	Vec2f	Toto;
	SinCos(Toto,_w*0.5f);
	xyz()=RV*(Toto.x/_w);
	w=Toto.y;
	return *this;
}

void RVToQuat(const Vec3f &_v,Quat &q)
{
	q.v=_v;
	q.w=_v.GetNorm();
	q.v.CNormalize();

	q.w*=0.5f;
	q.xyz()*=Sin(q.w);			//Don't try to use sinf here...we already tried, it breaks the quaternion...Need to investigate
	q.w=Cos(q.w);				//Don't try to use cosf here...we already tried, it breaks the quaternion...Need to investigate
}


void RVToQuat(Quat &q)
{
	q.w*=0.5f;
	q.xyz()*=Sin(q.w);			//Don't try to use sinf here...we already tried, it breaks the quaternion...Need to investigate
	q.w=Cos(q.w);				//Don't try to use cosf here...we already tried, it breaks the quaternion...Need to investigate
}

void QuatToRV(const Quat &_q,Vec3f &_v)
{
	Quat	q=_q;
	if (q.w<-1.f) q.w=-1.f;
	if (q.w>1.f) q.w=1.f;
	Float	halfang=ACos(q.w);
	Float	s=Sin(halfang);		//Don't try to use sinf here...we already tried, it breaks the quaternion...Need to investigate
	if (s>Float_Eps)
	{
		q.v*=(1.f/s);
	}
	q.w=halfang*2.f;

	_v=q.v,
	_v.CNormalize();
	_v*=q.w;
}



void QuatToRV(Quat &q)
{
	if (q.w<-1.f) q.w=-1.f;
	if (q.w>1.f) q.w=1.f;
	Float	halfang=ACos(q.w);
	Float	s=Sin(halfang);		//Don't try to use sinf here...we already tried, it breaks the quaternion...Need to investigate
	if (s>Float_Eps)
	{
		q.v*=(1.f/s);
	}
	q.w=halfang*2.f;
}

void	ConvSquadTangents(const Quat &Q1,const Quat &Q2,const Quat &Q3,const Quat &Q4,Vec3f &D1,Vec3f &D2,Vec3f &D3)
{
	D1=Q2-Q1;
	D2=Q4-Q1;
	D3=Q3-Q2;
}

void	Squad(const Quat &Q1,const Vec3f &D1,const Vec3f &D2,const Vec3f &D3,Float t,Quat &result)
{
	Vec3f	delta1=t*D2;
	Vec3f	delta2=D1+t*D3;
	
	Float	quadk=2.f*t*(1.f-t);

	Vec3f	delta=delta1*(1.f-quadk)+delta2*quadk;

	result=Q1+Quat(delta);
}

// 3x3 SetMatrix
void Quat::SetMatrix(const Mat3x3 &Matrix)
{
	VecFloatStoreAligned( &v, VecFloat3x3ToQuaternion(Matrix.m.m128) );
}

void Quat::GetMatrix(Mat3x3 &Mat) const
{
	Float X=v.x, Y=v.y, Z=v.z, W=w;
	Float X2=2.f*X*X, Y2=2.f*Y*Y, Z2=2.f*Z*Z;
	Float XY=2.f*X*Y, ZY=2.f*Z*Y, XZ=2.f*X*Z;
	Float WX=2.f*W*X, WY=2.f*W*Y, WZ=2.f*W*Z;
	Mat.m.m[0][0]=(1.f-Y2-Z2);	Mat.m.m[1][0]=XY-WZ;		Mat.m.m[2][0]=XZ+WY;
	Mat.m.m[0][1]=(XY+WZ);		Mat.m.m[1][1]=1.f-X2-Z2;	Mat.m.m[2][1]=ZY-WX;
	Mat.m.m[0][2]=(XZ-WY);		Mat.m.m[1][2]=ZY+WX;		Mat.m.m[2][2]=1.f-X2-Y2;
}

Quat Quat::operator* ( const Float f) const
{
	Float	w2=w*w;
	if (w2<(1.f-Float_Eps))
	{
		Float	s=Sqrt(1.f-w2);
		Vec2f	Toto;
		ASSERTC_Z((w <= 1.f) && (w >= -1.f),"ACOS will bug with val >1.f");
		SinCos(Toto,ACos(w)*f);
		Quat	r;
		r.v=v*(Toto.x/s);
		r.w=Toto.y;
		return r;
	}
	return *this;
}

Float	Quat::GetAngle() const
{
	Float	_w=w;
	if (_w<-1.f) _w=-1.f;
	else if (_w>1.f) _w=1.f;
	Float	s=Sqrt(1.f-_w*_w);

	return s>Float_Eps ? ACos(_w)*2.f : 0.f;
}

Float	Quat::GetSignedAngle() const
{
	Float fAngle = GetAngle();
	if(fAngle>Pi)
	{
		fAngle -= 2.f*Pi;
	}
	return fAngle;
}

void	Quat::SetAngle(Float _ang)
{
	QuatToRV(*this);
	w=_ang;
	RVToQuat(*this);
}

Bool	Quat::Maximize( Float f)
{
	if (w<-1.f) w=-1.f;
	if (w>1.f) w=1.f;
	Float	s=Sqrt(1.f-w*w);

	if (s>Float_Eps)
	{
		f*=0.5f;
		Vec2f	Toto;
		Float	a=ACos(w);
		if (a<-f) a=-f;
		else if (a>f) a=f;
		else return FALSE;
		SinCos(Toto,a);
		v*=(1.f/s)*Toto.x;
		w=Toto.y;
		return TRUE;
	}
	return FALSE;
}

/*inline*/ void	Quat::GetEular(Vec3f &Eular) const
{
	Mat3x3	mat;
	GetMatrix(mat);
	mat.GetEular(Eular);
}

/*inline*/ void	Quat::SetEular(const Vec4f &Eular)
{
	Vec2f	sincos;
	Quat	QWork;

	// Rotation en Z.
	Vec4f vSin = VEC4F_NULL; 
	Vec4f vCos = VEC4F_NULL;
	Vec4f vHalfEular = VecFloatMul( Eular, VEC4F_HALF );
	VecFloatSinCos( vSin, vCos, vHalfEular);
	xyzw().Set( 0.f, 0.f, vSin.z, vCos.z );
	// Rotation en Y.
	QWork.xyzw().Set( 0.f, vSin.y, 0.f, vCos.y );
	(*this) *= QWork;
	// Rotation en X.
	QWork.xyzw().Set( vSin.x, 0.f, 0.f, vCos.x );
	(*this) *= QWork;
}
