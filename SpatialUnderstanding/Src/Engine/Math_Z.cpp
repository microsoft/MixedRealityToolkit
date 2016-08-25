// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include <pch.h>
#include <Math_Z.h>
#include <Random_Z.h>

// --------------------------------------------------------
// Const
// --------------------------------------------------------
const Vec3i VEC3I_NULL = Vec3i(0,0,0);
const Vec3i VEC3I_ONE = Vec3i(1, 1, 1);
const Vec3i VEC3I_UP = Vec3i(0, 1, 0);
const Vec3i VEC3I_DOWN = Vec3i(0, -1, 0);
const Vec3i VEC3I_LEFT = Vec3i(1, 0, 0);
const Vec3i VEC3I_RIGHT = Vec3i(-1, 0, 0);
const Vec3i VEC3I_FRONT = Vec3i(0, 0, 1);
const Vec3i VEC3I_BACK = Vec3i(0, 0, -1);

// Vec2f constants
const Vec2f VEC2F_NULL			= Vec2f(0.f,0.f);
const Vec2f VEC2F_ONE			= Vec2f(1.f,1.f);
const Vec2f VEC2F_HALF			= Vec2f(0.5f,0.5f);
const Vec2f VEC2F_LEFT			= Vec2f(-1.f,0.f);
const Vec2f VEC2F_RIGHT		= Vec2f(1.f,0.f);
const Vec2f VEC2F_UP				= Vec2f(0.f,-1.f);
const Vec2f VEC2F_DOWN		= Vec2f(0.f,1.f);

// Vec3f constants
const Vec3f VEC3F_NULL = Vec3f(0.f,0.f,0.f);
const Vec3f VEC3F_ONE  = Vec3f(1.f,1.f,1.f);
const Vec3f VEC3F_HALF = Vec3f(0.5f,0.5f,0.5f);
const Vec3f VEC3F_UP   = Vec3f(0.f,1.f,0.f);
const Vec3f VEC3F_DOWN = Vec3f(0.f,-1.f,0.f);
const Vec3f VEC3F_LEFT= Vec3f(1.f,0.f,0.f);
const Vec3f VEC3F_RIGHT = Vec3f(-1.f,0.f,0.f);
const Vec3f VEC3F_FRONT= Vec3f(0.f,0.f,1.f);
const Vec3f VEC3F_BACK = Vec3f(0.f,0.f,-1.f);

// Vec4f constants
const Vec4f VEC4F_127  = Vec4f(127.f,127.f,127.f,127.f);
const Vec4f VEC4F_255  = Vec4f(255.f,255.f,255.f,255.f);
const Vec4f VEC4F_ONEOVER255  = Vec4f(1.f/255.f,1.f/255.f,1.f/255.f,1.f/255.f);
const Vec4f VEC4F_NULL = Vec4f(0.f,0.f,0.f,0.f);
const Vec4f VEC4F_ONE  = Vec4f(1.f,1.f,1.f,1.f);
const Vec4f VEC4F_HALF = Vec4f(0.5f,0.5f,0.5f,0.5f);
const Vec4f VEC4F_THIRD= Vec4f(0.333333f,0.333333f,0.333333f,0.333333f);
const Vec4f VEC4F_QUARTER= Vec4f(0.25f,0.25f,0.25f,0.25f);
const Vec4f VEC4F_TWO  = Vec4f(2.f,2.f,2.f,2.f);
const Vec4f VEC4F_THREE= Vec4f(3.f,3.f,3.f,3.f);
const Vec4f VEC4F_FOUR = Vec4f(4.f,4.f,4.f,4.f);
const Vec4f VEC4F_EIGHT = Vec4f(8.f,8.f,8.f,8.f);
const Vec4f VEC4F_UP   = Vec4f(0.f,1.f,0.f,0.f);
const Vec4f VEC4F_DOWN = Vec4f(0.f,-1.f,0.f,0.f);
const Vec4f VEC4F_LEFT= Vec4f(1.f,0.f,0.f,0.f);
const Vec4f VEC4F_RIGHT = Vec4f(-1.f,0.f,0.f,0.f);
const Vec4f VEC4F_FRONT= Vec4f(0.f,0.f,1.f,0.f);
const Vec4f VEC4F_BACK = Vec4f(0.f,0.f,-1.f,0.f);
const Vec4f VEC4F_EPSILON = Vec4f(Float_Eps);
const Vec4f VEC4F_SQUARED_EPSILON = Vec4f(Float_Eps*Float_Eps);

const Mat4x4 MAT4X4_ZERO = Mat4x4(0.0f);
const Mat4x4 MAT4X4_IDENTITY = Mat4x4().SetIdentity();

// Color constants
const Color COLOR_WHITE=Color(1.f,1.f,1.f,1.f);
const Color COLOR_FULLWHITE=Color(1.f,1.f,1.f,1.f);
const Color COLOR_BLACK=Color(0.f,0.f,0.f,1.f);
const Color COLOR_GREY=Color(0.5f,0.5f,0.5f,1.f);
const Color COLOR_LIGHTGREY=Color(0.66f,0.66f,0.66f,1.f);
const Color COLOR_DARKGREY=Color(0.33f,0.33f,0.33f,1.f);
const Color COLOR_RED=Color(1.f,0.f,0.f,1.f);
const Color COLOR_GREEN=Color(0.f,1.f,0.f,1.f);
const Color COLOR_BLUE=Color(0.f,0.f,1.f,1.f);
const Color COLOR_YELLOW=Color(1.f,1.f,0.f,1.f);
const Color COLOR_CYAN=Color(0.f,1.f,1.f,1.f);
const Color COLOR_MAGENTA=Color(1.f,0.f,1.f,1.f);
const Color COLOR_ORANGE=Color(1.f,0.5f,0.f,1.f);
const Color COLOR_BROWN=Color(145.f/255.f,89.0f/255.f,60.f/255.f,1.f);
const Color COLOR_LIGHTBLUE=Color(0.5f,0.5f,1.f,1.f);
const Color COLOR_LIGHTGREEN=Color(0.5f,1.f,0.5f,1.f);
const Color COLOR_LIGHTRED=Color(1.f,0.5f,0.5f,1.f);
const Color COLOR_DARKBLUE=Color(0.0f,0.0f,0.5f,1.f);
const Color COLOR_DARKGREEN=Color(0.0f,0.5f,0.0f,1.f);
const Color COLOR_DARKRED=Color(0.5f,0.0f,0.0f,1.f);
const Color COLOR_PINK=Color(1.f, 174.f/255.f, 201.f/255.f, 1.f);
const Color COLOR_PURPLE=Color(.5f, 0.f, .5f, 1.f);
const Color COLOR_NULL=Color(0.f,0.f,0.f,0.f);
const ByteColor BCOLOR_WHITE=ByteColor( U8(255), U8(255), U8(255), U8(255) );
const ByteColor BCOLOR_NULL=ByteColor( (U8)0, (U8)0, (U8)0, (U8)0 );

const Char* SCOLOR_WHITE="^999";
const Char* SCOLOR_BLACK="^000";
const Char* SCOLOR_GREY="^444";
const Char* SCOLOR_LIGHTGREY="^666";
const Char* SCOLOR_DARKGREY="^333";
const Char* SCOLOR_RED="^900";
const Char* SCOLOR_GREEN="^090";
const Char* SCOLOR_BLUE="^009";
const Char* SCOLOR_YELLOW="^990";
const Char* SCOLOR_CYAN="^099";
const Char* SCOLOR_MAGENTA="^909";
const Char* SCOLOR_ORANGE="^940";
const Char* SCOLOR_BROWN="^532";
const Char* SCOLOR_LIGHTBLUE="^449";
const Char* SCOLOR_LIGHTGREEN="^494";
const Char* SCOLOR_LIGHTRED="^944";
const Char* SCOLOR_DARKBLUE="^004";
const Char* SCOLOR_DARKGREEN="^040";
const Char* SCOLOR_DARKRED="^400";
const Char* SCOLOR_PINK="^967";
const Char* SCOLOR_PURPLE="^404";


Bool CheckFloat(Float v)
{
	struct FComponent
	{
	#if defined(__BIG_ENDIAN__) && !defined(_FAKE)
		U32 sign	 : 1;
		U32 exponent : 8;
		U32 mantissa : 23;
	#else
		U32 mantissa : 23;
		U32 exponent : 8;
		U32 sign	 : 1;
	#endif
	};
		
	union FType
	{
		Float 		f;
		FComponent 	f2;

	};
	FType fType;
	fType.f = v;
	return (fType.f2.exponent!=255) && (Abs(v)<1.e7f);
}

Bool	TESTFTOLMODE()
{
#if defined(_PC)
	int	a=FTOL(-21.1f);
	if (a==-22)
		return TRUE;
	return FALSE;
#else
	return TRUE;
#endif
}

void	CHECKFTOLMODE()
{
#ifndef _EDITION_Z
	EXCEPTIONC_Z(TESTFTOLMODE(),"Pas le bon mode d'arrondi du processeur");
#endif
}

Bool CheckQuat(const Quat &q)
{
	Float	Norm=Sqrt(q.v*q.v + q.w*q.w);
	if (!CheckFloat(Norm))
		return FALSE;
	Float	d=POS_Z(1.f-Norm);
	return (d<=0.25f);
}

Bool CheckVec4f(const Vec4f &q)
{
	return CheckFloat(q.x) && CheckFloat(q.y) && CheckFloat(q.z) && CheckFloat(q.w);
}

Bool CheckVec3f(const Vec3f &q)
{
	return CheckFloat(q.x) && CheckFloat(q.y) && CheckFloat(q.z);
}

// PROTECTION DES VECTEURS DE POSITION QUI SORTIRAIENT DU MONDE 3D, ils peuvent changer la taille maxi autoris�e du monde
static const Float	ProtectWorldMaxSize=110000.f;
Bool CheckVecSize(const Vec3f &_vec)
{
	return POS_Z(_vec.x)<ProtectWorldMaxSize && POS_Z(_vec.y)<ProtectWorldMaxSize && POS_Z(_vec.z)<ProtectWorldMaxSize;
}
Bool CheckVecSizeH(const Vec3f &_vec)
{
	return POS_Z(_vec.x)<ProtectWorldMaxSize && POS_Z(_vec.z)<ProtectWorldMaxSize;
}

void MergeRects(S32& _outPx, S32& _outPy, S32& _outSx, S32& _outSy, S32 _inPx0, S32 _inPy0, S32 _inSx0, S32 _inSy0, S32 _inPx1, S32 _inPy1, S32 _inSx1, S32 _inSy1)
{
	const S32	left0	= _inPx0;
	const S32	right0	= _inPx0+_inSx0;
	const S32	top0	= _inPy0;
	const S32	bottom0	= _inPy0+_inSy0;

	const S32	left1	= _inPx1;
	const S32	right1	= _inPx1+_inSx1;
	const S32	top1	= _inPy1;
	const S32	bottom1	= _inPy1+_inSy1;

	const S32	finalLeft	= Min<S32>(left0	, left1		);
	const S32	finalRight	= Max<S32>(right0	, right1	);
	const S32	finalTop	= Min<S32>(top0		, top1		);
	const S32	finalBottom	= Max<S32>(bottom0	, bottom1	);

	_outPx	= finalLeft;
	_outPy	= finalTop;
	_outSx	= finalRight-finalLeft;
	_outSy	= finalBottom-finalTop;
}
