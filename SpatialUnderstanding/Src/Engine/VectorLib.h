// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#ifndef _VECTOR_LIB_Z_H
#define _VECTOR_LIB_Z_H

/** \file
 * Vector math Library
 * This library stores atomic operations and common math that can be processed using vector register intrinsics.
 * Float, Float4, Int4 and so on are used as fallback structures when no intrinsic type has been defined for the target platform (see Types_Z.h).
 *
 * * VecFloat4 is a native floating-point vector type intended to be used for any intensive vector/matrix/quaternion/color computation.
 *   Operations such like dot exist in two versions, where the second one replicates the result in all components.
 *
 * * VecFloat3x3 is a set of three 4D native vectors that behaves as a 3x3 matrix. Never assume their 4th component holds a valid value. 
 *
 * * VecFloat4x4 is a native 4x4 matrix type, useful for any 4D transformation.
 *
 * * VecInt4, VecShort8, VecUShort8, VecUChar16 are native integer vector types. They optimize many basic integer arithmetic operations and conversions.
 *
 * IMPORTANT NOTICE
 *
 * After performing 3D vector operations such like Cross, Rotate or 3x3 matrix operations,
 * you must assume that the w component of the resulting elements may be anything but 1.
 * It is your responsability to reset w if ever used later in a 4D operation.
 * Have a look to the description of these functions to know whether the resulting w value is invalidated.
 */

#if defined( _PC_SSE )
	#define _ALWAYS_INLINE_BASE_FUNCTIONS_
#endif

// ++ All vectors ++

/// Precomputed permutation
enum VectorPermute
{
	VPERM_X0 = 0,
	VPERM_Y0 = 1,
	VPERM_Z0 = 2,
	VPERM_W0 = 3,
	VPERM_X1 = 4,
	VPERM_Y1 = 5,
	VPERM_Z1 = 6,
	VPERM_W1 = 7
};

enum VectorSwizzle
{
	VSWIZZLE_X = 0,
	VSWIZZLE_Y = 1,
	VSWIZZLE_Z = 2,
	VSWIZZLE_W = 3
};

namespace VectorConstantsPrivate
{
	static const U16 gs_uRandmax = (U16)(0x7fff);

	static const VecFloat4		vConjugateMask									= { -1.0f, -1.0f, -1.0f, 1.0f };

	#if defined( _PC_SSE3 )
	static const UChar16		vUInt2UshortMask0								= { 0, 1, 4, 5, 8, 9, 12, 13, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80 };
	static const UChar16		vUInt2UshortMask1								= { 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0, 1, 4, 5, 8, 9, 12, 13 };
	static const UChar16		vSwizzleMaskACEGACEG							= { 0,1, 4,5, 8,9, 12,13, 0,1, 4,5, 8,9, 12,13 };
	static const UChar16		vSwizzleMaskABCMDEFNGHIOJKLP					= { 0, 1, 2, 12, 3, 4, 5, 13, 6, 7, 8, 14, 9, 10, 11, 15 };
	static const UChar16		vSwizzleMaskACEGIKMOBDFHJLNP					= { 0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15 };
	static const UChar16		vSwizzleMaskCBAMFEDNIHGOLKJP					= { 2, 1, 0, 12, 5, 4, 3, 13, 8, 7, 6, 14, 11, 10, 9, 15 };
	static const UChar16		vEvenMask										= { 0, 0x80, 2, 0x80, 4, 0x80, 6, 0x80, 8, 0x80, 10, 0x80, 12, 0x80, 14, 0x80 };
	static const UChar16		vOddMask										= { 1, 0x80, 3, 0x80, 5, 0x80, 7, 0x80, 9, 0x80, 11, 0x80, 13, 0x80, 15, 0x80 };
	static const UChar16		vRGBAToRGBMask									= { 0,1,2, 4,5,6, 8,9,10, 12,13,14, 0,1,2,4 };
	static const UInt4			vInfinity										= { 0x47FFEFFF, 0x47FFEFFF, 0x47FFEFFF, 0x47FFEFFF };
	static const UInt4			vDenormal										= { 0x38800000, 0x38800000, 0x38800000, 0x38800000 };
	static const UInt4			vFixup											= { 0x48000000, 0x48000000, 0x48000000, 0x48000000 };
	static const UInt4			vRound1											= { 0x00000001, 0x00000001, 0x00000001, 0x00000001 };
	static const UInt4			vRound2											= { 0x00000FFF, 0x00000FFF, 0x00000FFF, 0x00000FFF };
	static const UInt4			vSign											= { 0x80000000, 0x80000000, 0x80000000, 0x80000000 };
	static const UInt4			vBase											= { 0x00007FFF, 0x00007FFF, 0x00007FFF, 0x00007FFF };
	static const UInt4			vInteger										= { 0x52000000, 0x52000000, 0x52000000, 0x52000000 };
	#endif

	#if defined (_PC_SSE2 )
	static const Int4			vUnpack8888_0									= { (S32)0x80808003,(S32)0x80808000,(S32)0x80808001,(S32)0x80808002 };
	static const Int4			vUnpack8888_1									= { (S32)0x80808007,(S32)0x80808004,(S32)0x80808005,(S32)0x80808006 };
	static const Int4			vUnpack8888_2									= { (S32)0x8080800B,(S32)0x80808008,(S32)0x80808009,(S32)0x8080800A };
	static const Int4			vUnpack8888_3									= { (S32)0x8080800F,(S32)0x8080800C,(S32)0x8080800D,(S32)0x8080800E };
	static const Int4			vShiftXMask										= { (S32)0xffffffff, (S32)0,		(S32)0,			(S32)0 };
	static const UShort8		vUShortCompareAdd								= { 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000, 0x8000 };
	static const UChar16		vUCharCompareAdd								= { 0x80,0x80,0x80,0x80, 0x80,0x80,0x80,0x80, 0x80,0x80,0x80,0x80, 0x80,0x80,0x80,0x80 };
	#endif

	#if defined (_PC_SSE )
	static const Int4			vAbsMask										= { 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff };
	#endif

	static const VecFloat4		vZeroF											= { 0.f, 0.f, 0.f, 0.f };
	static const VecFloat4		vEpsF											= { Float_Eps, Float_Eps, Float_Eps, Float_Eps };
	static const VecFloat4		vEpsSquareF										= { Float_Eps*Float_Eps, Float_Eps*Float_Eps, Float_Eps*Float_Eps, Float_Eps*Float_Eps };
	static const VecFloat4		vHalfF											= { 0.5f, 0.5f, 0.5f, 0.5f };
	static const VecFloat4		vNegHalfF										= { -0.5f, -0.5f, -0.5f, -0.5f };
	static const VecFloat4		v099F											= { 0.99f, 0.99f, 0.99f, 0.99f };
	static const VecFloat4		vOneF											= { 1.f, 1.f, 1.f, 1.f };
	static const VecFloat4		vTwoF											= { 2.f, 2.f, 2.f, 2.f };
	static const VecFloat4		vThreeF											= { 3.f, 3.f, 3.f, 3.f };
	static const VecFloat4		v127F											= { 127.f, 127.f, 127.f, 127.f };
	static const VecFloat4		v255F											= { 255.0f, 255.0f, 255.0f, 255.0f };
	static const VecFloat4		vInv255F										= { 1.0f/255.0f, 1.0f/255.0f, 1.0f/255.0f, 1.0f/255.0f };
	static const VecFloat4		vInv127F										= { 1.0f/127.f, 1.0f/127.f, 1.0f/127.f, 1.0f/127.f };
	static const VecFloat4		vHalfAndThreeHalf								= { 0.5f,0.5f,1.5f,1.5f };
	static const VecFloat4		vExp2ConstantsDegree2							= { 6.5763628e-1f, 1.0017247f, 3.3718944e-1f, 1.f };
	static const VecFloat4		vLog2ConstantsDegree3							= { -1.04913055217340124191f, 2.28330284476918490682f, 0.204446009836232697516f, 1.f };

	static const UInt4			vOneF_IEEE754									= { 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000 };

	static const UInt4			vUCharMask888									= { 0, 0, 0, 0xFFFFFFFF };

	static const VecFloat4		vSlerpEpsilon									= { 1.f - 1.e-5f, 1.f - 1.e-5f, 1.f - 1.e-5f, 1.f - 1.e-5f };

	static const VecFloat4		vPi												= { 3.141592654f, 3.141592654f, 3.141592654f, 3.141592654f };
	static const VecFloat4		vPiDiv2											= { 1.570796327f, 1.570796327f, 1.570796327f, 1.570796327f };
	static const VecFloat4		vTwoPi											= { Pi_Mul_2, Pi_Mul_2, Pi_Mul_2, Pi_Mul_2 };
	static const VecFloat4		vReciprocalTwoPi								= { 0.15915494309189f, 0.15915494309189f, 0.15915494309189f, 0.15915494309189f };

	static const VecFloat4		vCosCoefficients0								= { 1.0f, -0.5f, 4.166666667e-2f, -1.388888889e-3f };
	static const VecFloat4		vCosCoefficients1								= { 2.480158730e-5f, -2.755731922e-7f, 2.087675699e-9f, -1.147074560e-11f };
	static const VecFloat4		vCosCoefficients2								= { 4.779477332e-14f, -1.561920697e-16f, 4.110317623e-19f, -8.896791392e-22f };

	static const VecFloat4		vSinCoefficients0								= { 1.0f, -0.166666667f, 8.333333333e-3f, -1.984126984e-4f };
	static const VecFloat4		vSinCoefficients1								= { 2.755731922e-6f, -2.505210839e-8f, 1.605904384e-10f, -7.647163732e-13f };
	static const VecFloat4		vSinCoefficients2								= { 2.811457254e-15f, -8.220635247e-18f, 1.957294106e-20f, -3.868170171e-23f };

	static const VecFloat4		vAcosCoefficients0								= { -0.0012624911f, 0.0066700901f, -0.0170881256f, 0.0308918810f };
	static const VecFloat4		vAcosCoefficients1								= { -0.0501743046f, 0.0889789874f, -0.2145988016f, 1.5707963050f };

	static const Int4			vNegMask										= { (S32)0x80000000, (S32)0x80000000, (S32)0x80000000, (S32)0x80000000 };
	static const Int4			vPackSign										= { (S32)1, (S32)2, (S32)4, (S32)8 };
	static const Int4			vInfinite										= { (S32)0x7f800000, (S32)0x7f800000, (S32)0x7f800000, (S32)0x7f800000 };

	static const UInt4			vRandomMul										= { (S32)214013, (S32)17405, (S32)214013, (S32)69069 };
	static const UInt4			vRandomAdd										= { (S32)2531011, (S32)10395331, (S32)13737667, (S32)1 };
	static const UInt4			vRandMax										= { gs_uRandmax, gs_uRandmax, gs_uRandmax, gs_uRandmax };
}


// ++ Floating-point vectors ++


/// @name Load op (induce a LHS)
///@{

FINLINE_Z VecFloat4 VecFloatLoad1 ( const Float scalar );
FINLINE_Z VecFloat4 VecFloatLoad4 ( const Float x, const Float y, const Float z, const Float w );
#ifndef _ALWAYS_INLINE_BASE_FUNCTIONS_
FINLINE_Z VecFloat4 VecFloatLoadAligned ( const void* alignedMemory );
#endif
FINLINE_Z VecFloat4 VecFloatLoadUnaligned ( const void* unalignedMemory );

FINLINE_Z void VecFloatSetX ( register VecFloat4& a, const Float x );
FINLINE_Z void VecFloatSetY ( register VecFloat4& a, const Float y );
FINLINE_Z void VecFloatSetZ ( register VecFloat4& a, const Float z );
FINLINE_Z void VecFloatSetW ( register VecFloat4& a, const Float w );

FINLINE_Z void VecFloat3x3Load1 ( register VecFloat3x3& result, const Float scalar );
FINLINE_Z void VecFloat3x3LoadRow0 ( register VecFloat3x3& result, const Float x, const Float y, const Float z, const Float w );
FINLINE_Z void VecFloat3x3LoadRow1 ( register VecFloat3x3& result, const Float x, const Float y, const Float z );
FINLINE_Z void VecFloat3x3LoadRow2 ( register VecFloat3x3& result, const Float x, const Float y, const Float z );

FINLINE_Z void VecFloat4x4Load1 ( register VecFloat4x4& result, const Float scalar );
FINLINE_Z void VecFloat4x4LoadRow0 ( register VecFloat4x4& result, const Float x, const Float y, const Float z, const Float w );
FINLINE_Z void VecFloat4x4LoadRow1 ( register VecFloat4x4& result, const Float x, const Float y, const Float z, const Float w );
FINLINE_Z void VecFloat4x4LoadRow2 ( register VecFloat4x4& result, const Float x, const Float y, const Float z, const Float w );
FINLINE_Z void VecFloat4x4LoadRow3 ( register VecFloat4x4& result, const Float x, const Float y, const Float z, const Float w );

///@}
/// @name Set op (no LHS)
///@{

FINLINE_Z VecFloat4 VecFloatSplatZero ( );
FINLINE_Z VecFloat4 VecFloatSplatOne ( );
FINLINE_Z VecFloat4 VecFloatSetIdentity ( );

FINLINE_Z VecFloat4 VecFloatSetXZero ( register const VecFloat4& a );
FINLINE_Z VecFloat4 VecFloatSetXOne ( register const VecFloat4& a );
FINLINE_Z VecFloat4 VecFloatSetWZero ( register const VecFloat4& a );
FINLINE_Z VecFloat4 VecFloatSetWOne ( register const VecFloat4& a );

FINLINE_Z void VecFloat3x3Copy ( register VecFloat3x3& result, register const VecFloat3x3& m0 );
FINLINE_Z void VecFloat3x3Copy4x4 ( register VecFloat3x3& result, register const VecFloat4x4& m0 );
FINLINE_Z void VecFloat3x3CopyRows ( register VecFloat4* result, register const VecFloat3x3& m0 );
FINLINE_Z void VecFloat3x3CopyArray ( register VecFloat3x3& result, register const VecFloat4* rows );

FINLINE_Z void VecFloat3x3SplatZero ( register VecFloat3x3& result );
FINLINE_Z void VecFloat3x3SplatOne ( register VecFloat3x3& result );
FINLINE_Z void VecFloat3x3SetIdentity ( register VecFloat3x3& result );
FINLINE_Z void VecFloat3x3SetIdentityScale ( register VecFloat3x3& result, register const VecFloat4& scale );
FINLINE_Z void VecFloat3x3SetRow0 ( register VecFloat3x3& result, const register VecFloat4& row0 );
FINLINE_Z void VecFloat3x3SetRow1 ( register VecFloat3x3& result, const register VecFloat4& row1 );
FINLINE_Z void VecFloat3x3SetRow2 ( register VecFloat3x3& result, const register VecFloat4& row1 );

FINLINE_Z void VecFloat4x4Copy ( register VecFloat4x4& result, register const VecFloat4x4& m0 );
FINLINE_Z void VecFloat4x4CopyRows( register VecFloat4* result, register const VecFloat4x4& m0 );

FINLINE_Z void VecFloat4x4SplatZero ( register VecFloat4x4& result );
FINLINE_Z void VecFloat4x4SplatOne ( register VecFloat4x4& result );
FINLINE_Z void VecFloat4x4SetIdentity ( register VecFloat4x4& result );
FINLINE_Z void VecFloat4x4SetIdentityTrans ( register VecFloat4x4& result, register const VecFloat4& translation );
FINLINE_Z void VecFloat4x4SetIdentityScale ( register VecFloat4x4& result, register const VecFloat4& scale );
FINLINE_Z void VecFloat4x4SetScale ( register VecFloat4x4& result, register const VecFloat4& scale );
FINLINE_Z void VecFloat4x4SetRow0 ( register VecFloat4x4& result, const register VecFloat4& row0 );
FINLINE_Z void VecFloat4x4SetRow1 ( register VecFloat4x4& result, const register VecFloat4& row1 );
FINLINE_Z void VecFloat4x4SetRow2 ( register VecFloat4x4& result, const register VecFloat4& row1 );
FINLINE_Z void VecFloat4x4SetRow3 ( register VecFloat4x4& result, const register VecFloat4& row1 );
FINLINE_Z void VecFloat4x4BuildFrom3x3Trans( register VecFloat4x4& result, register const VecFloat3x3& mat3x3, register const VecFloat4& translation );

///@}
/// @name Access op (no LHS)
///@{

FINLINE_Z VecFloat4 VecFloat3x3GetRow0 ( register const VecFloat3x3& m );
FINLINE_Z VecFloat4 VecFloat3x3GetRow1 ( register const VecFloat3x3& m );
FINLINE_Z VecFloat4 VecFloat3x3GetRow2 ( register const VecFloat3x3& m );

FINLINE_Z VecFloat4 VecFloat4x4GetRow0 ( register const VecFloat4x4& m );
FINLINE_Z VecFloat4 VecFloat4x4GetRow1 ( register const VecFloat4x4& m );
FINLINE_Z VecFloat4 VecFloat4x4GetRow2 ( register const VecFloat4x4& m );
FINLINE_Z VecFloat4 VecFloat4x4GetRow3 ( register const VecFloat4x4& m );

///@}
/// @name Store op (induce a LHS)
///@{

FINLINE_Z Float VecFloatGetX ( register const VecFloat4& a );
FINLINE_Z Float VecFloatGetY ( register const VecFloat4& a );
FINLINE_Z Float VecFloatGetZ ( register const VecFloat4& a );
FINLINE_Z Float VecFloatGetW ( register const VecFloat4& a );

#ifndef _ALWAYS_INLINE_BASE_FUNCTIONS_
FINLINE_Z void VecFloatStoreAligned ( void* dest, register const VecFloat4& source );
#endif
FINLINE_Z void VecFloatStreamStoreAligned ( void* dest, register const VecFloat4& source );
FINLINE_Z void VecFloatStoreUnaligned ( void* dest, register const VecFloat4& source );

///@}
/// @name Select op
///@{

FINLINE_Z VecFloat4 VecFloatSelectMask( register const VecFloat4& a, register const VecFloat4& b, register const VecSelMask& mask );

FINLINE_Z VecFloat4 VecFloatSelectLess ( register const VecFloat4& a, register const VecFloat4& b, register const VecFloat4& success, register const VecFloat4& fail );
FINLINE_Z VecFloat4 VecFloatSelectLessOrEqual ( register const VecFloat4& a, register const VecFloat4& b, register const VecFloat4& success, register const VecFloat4& fail );
FINLINE_Z VecFloat4 VecFloatSelectGreater ( register const VecFloat4& a, register const VecFloat4& b, register const VecFloat4& success, register const VecFloat4& fail );
FINLINE_Z VecFloat4 VecFloatSelectGreaterOrEqual ( register const VecFloat4& a, register const VecFloat4& b, register const VecFloat4& success, register const VecFloat4& fail );
FINLINE_Z VecFloat4 VecFloatSelectEqual ( register const VecFloat4& a, register const VecFloat4& b, register const VecFloat4& success, register const VecFloat4& fail );
FINLINE_Z VecFloat4 VecFloatSelectNotEqual ( register const VecFloat4& a, register const VecFloat4& b, register const VecFloat4& success, register const VecFloat4& fail );
FINLINE_Z VecFloat4 VecFloatSelectBounds ( register const VecFloat4& a, register const VecFloat4& b, register const VecFloat4& success, register const VecFloat4& fail );

///@}
/// @name Logical op
///@{

FINLINE_Z VecFloat4 VecFloatAnd( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z VecFloat4 VecFloatOr( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z VecFloat4 VecFloatXor( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z VecFloat4 VecFloatAndNot( register const VecFloat4& a, register const VecFloat4& b );

///@}
/// @name Arithmetic op
///@{

FINLINE_Z VecFloat4 VecFloatNegate ( register const VecFloat4& a );
FINLINE_Z VecFloat4 VecFloatOneComplement( register const VecFloat4& a );
#ifndef _ALWAYS_INLINE_BASE_FUNCTIONS_
FINLINE_Z VecFloat4 VecFloatAdd ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z VecFloat4 VecFloatSub ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z VecFloat4 VecFloatMul ( register const VecFloat4& a, register const VecFloat4& b );
#endif
FINLINE_Z VecFloat4 VecFloatDivEst ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z VecFloat4 VecFloatDiv ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z VecFloat4 VecFloatCDiv ( register const VecFloat4& a, register const VecFloat4& b, register const VecFloat4& c );
FINLINE_Z VecFloat4 VecFloatReciprocal ( register const VecFloat4& a );
FINLINE_Z VecFloat4 VecFloatReciprocalAccurate ( register const VecFloat4& a, U32 refinements = 2 );
#ifndef _ALWAYS_INLINE_BASE_FUNCTIONS_
FINLINE_Z VecFloat4 VecFloatMadd ( register const VecFloat4& a, register const VecFloat4& b, register const VecFloat4& c );
#endif
FINLINE_Z VecFloat4 VecFloatMsub ( register const VecFloat4& a, register const VecFloat4& b, register const VecFloat4& c );
FINLINE_Z VecFloat4 VecFloatNegMsub ( register const VecFloat4& a, register const VecFloat4& b, register const VecFloat4& c );
FINLINE_Z VecFloat4 VecFloatHAdd ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z VecFloat4 VecFloatHSub ( register const VecFloat4& a, register const VecFloat4& b );

FINLINE_Z VecFloat4 VecFloatSign ( register const VecFloat4& a );
FINLINE_Z VecFloat4 VecFloatSign ( register const VecFloat4& a, register const VecFloat4& b );

FINLINE_Z VecFloat4 VecFloatScale ( register const VecFloat4& a, Float scalar );
FINLINE_Z VecFloat4 VecFloatScaleX ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z VecFloat4 VecFloatScaleY ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z VecFloat4 VecFloatScaleZ ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z VecFloat4 VecFloatScaleW ( register const VecFloat4& a, register const VecFloat4& b );

FINLINE_Z VecFloat4 VecFloatAbs ( register const VecFloat4& a );
FINLINE_Z VecFloat4 VecFloatMin ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z VecFloat4 VecFloatMinComponent ( register const VecFloat4& a );
FINLINE_Z VecFloat4 VecFloatMinComponent3 ( register const VecFloat4& a );
FINLINE_Z VecFloat4 VecFloatMax ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z VecFloat4 VecFloatMaxComponent ( register const VecFloat4& a );
FINLINE_Z VecFloat4 VecFloatMaxComponent3 ( register const VecFloat4& a );
FINLINE_Z VecFloat4 VecFloatClamp ( register const VecFloat4& a, register const VecFloat4& min, register const VecFloat4& max );
FINLINE_Z VecFloat4 VecFloatSaturate ( register const VecFloat4& a );
FINLINE_Z VecFloat4 VecFloatSaturateExtended ( register const VecFloat4& a );

FINLINE_Z VecFloat4 VecFloatSqrt ( register const VecFloat4& a );
FINLINE_Z VecFloat4 VecFloatRSqrt ( register const VecFloat4& a );
FINLINE_Z VecFloat4 VecFloatRSqrtAccurate ( register const VecFloat4& a, U32 refinements = 2 );

FINLINE_Z VecFloat4 VecFloatCos ( register const VecFloat4& a );
FINLINE_Z VecFloat4 VecFloatSin ( register const VecFloat4& a );
FINLINE_Z VecFloat4 VecFloatTan ( register const VecFloat4& a );
FINLINE_Z VecFloat4 VecFloatACos ( register const VecFloat4& a );
FINLINE_Z VecFloat4 VecFloatASin ( register const VecFloat4& a );
FINLINE_Z VecFloat4 VecFloatSinCos1 ( const Float a );
FINLINE_Z void VecFloatSinCos ( register VecFloat4& s, register VecFloat4& c, register const VecFloat4& a );

FINLINE_Z VecFloat4 VecFloatExp ( register const VecFloat4& a );
FINLINE_Z VecFloat4 VecFloatExp2Est ( register const VecFloat4& a );
FINLINE_Z VecFloat4 VecFloatExp2 ( register const VecFloat4& a );
FINLINE_Z VecFloat4 VecFloatLog ( register const VecFloat4& a );
FINLINE_Z VecFloat4 VecFloatLog2Est ( register const VecFloat4& a );
FINLINE_Z VecFloat4 VecFloatLog2 ( register const VecFloat4& a );
FINLINE_Z VecFloat4 VecFloatLog10 ( register const VecFloat4& a );
FINLINE_Z VecFloat4 VecFloatPowEst ( register const VecFloat4& a, register const VecFloat4& power );

///@}
/// @name Swizzle op
///@{

FINLINE_Z VecFloat4 VecFloatSplatX ( register const VecFloat4& a );
FINLINE_Z VecFloat4 VecFloatSplatY ( register const VecFloat4& a );
FINLINE_Z VecFloat4 VecFloatSplatZ ( register const VecFloat4& a );
FINLINE_Z VecFloat4 VecFloatSplatW ( register const VecFloat4& a );

template <U32 ElementCount>
FINLINE_Z VecFloat4 VecFloatRotateLeft ( register const VecFloat4& a );

template <VectorSwizzle SrcPos, VectorSwizzle DestPos>
FINLINE_Z VecFloat4 VecFloatZeroInsert ( register const VecFloat4& a );

FINLINE_Z VecFloat4 VecFloatPermuteX0Y0Z0X1 ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z VecFloat4 VecFloatPermuteX0Y0Z0W1 ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z VecFloat4 VecFloatPermuteX0Y0X1W0 ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z VecFloat4 VecFloatPermuteX0Y0Z1W0 ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z VecFloat4 VecFloatPermuteX0Z0X1Z1 ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z VecFloat4 VecFloatPermuteX0X1Z0W0 ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z VecFloat4 VecFloatPermuteX0Y1Z0W0 ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z VecFloat4 VecFloatPermuteX0Y1Z1W1 ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z VecFloat4 VecFloatPermuteY0W0Y1W1 ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z VecFloat4 VecFloatPermuteX1Y0Z0W0 ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z VecFloat4 VecFloatPermuteZ1Y0Z0W0 ( register const VecFloat4& a, register const VecFloat4& b );

///@}
/// @name Compare op
///@{

FINLINE_Z U32 VecFloatAllZero ( register const VecFloat4& a );
FINLINE_Z U32 VecFloatAllEqual ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z U32 VecFloatAllNotEqual( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z U32 VecFloatAllNearEqual ( register const VecFloat4& a, register const VecFloat4& b, register const VecFloat4& epsilon );
FINLINE_Z U32 VecFloatAllLess ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z U32 VecFloatAllLessOrEqual ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z U32 VecFloatAllGreater ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z U32 VecFloatAllGreaterOrEqual ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z U32 VecFloatAllInBounds( register const VecFloat4& a, register const VecFloat4& b ) ;

FINLINE_Z U32 VecFloatAll3Equal ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z U32 VecFloatAll3LessOrEqual ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z U32 VecFloatAll3Greater ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z U32 VecFloatAll3GreaterOrEqual( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z U32 VecFloatAll3InBounds( register const VecFloat4& a, register const VecFloat4& b );

FINLINE_Z U32 VecFloatAnyEqual ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z U32 VecFloatAnyNotEqual ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z U32 VecFloatAnyLess( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z U32 VecFloatAnyGreater( register const VecFloat4& a, register const VecFloat4& b );

FINLINE_Z U32 VecFloatPackSign( register const VecFloat4& v );

FINLINE_Z U32 VecFloat4x4IsEqual( register const VecFloat4x4& a, register const VecFloat4x4& b );

///@}
/// @name Rounding
///@{

FINLINE_Z VecFloat4 VecFloatSplitParts ( register const VecFloat4& a, register VecFloat4& intPart );
FINLINE_Z VecFloat4 VecFloatMod ( register const VecFloat4& a, register const VecFloat4& b );

FINLINE_Z VecFloat4 VecFloatFloor ( register const VecFloat4& a );
FINLINE_Z VecFloat4 VecFloatCeil ( register const VecFloat4& a );
FINLINE_Z VecFloat4 VecFloatNearest ( register const VecFloat4& a );
FINLINE_Z VecFloat4 VecFloatTruncate ( register const VecFloat4& a );

///@}

// Common math

/// @name 4D vector results
///@{

FINLINE_Z VecFloat4 VecFloatLerp ( register const VecFloat4& a, register const VecFloat4& b, const Float scalar );
FINLINE_Z VecFloat4 VecFloatLerp ( register const VecFloat4& a, register const VecFloat4& b, register const VecFloat4& c );

FINLINE_Z VecFloat4 VecFloatQuatSlerp( register const VecFloat4& a, register const VecFloat4& b, register const VecFloat4& t );

FINLINE_Z VecFloat4 VecFloatAddComponents( register const VecFloat4& a );

FINLINE_Z VecFloat4 VecFloatCross3 ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z void VecFloatCross3_SoA ( register VecFloat4* RESTRICT_Z results, register const VecFloat4* RESTRICT_Z a, register const VecFloat4* RESTRICT_Z b );

FINLINE_Z VecFloat4 VecFloatScalarDot3 ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z VecFloat4 VecFloatDot2 ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z VecFloat4 VecFloatDot3 ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z VecFloat4 VecFloatDot4 ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z VecFloat4 VecFloatDot3_SoA ( register const VecFloat4* a, register const VecFloat4* b );
FINLINE_Z VecFloat4 VecFloatDot4_SoA ( register const VecFloat4* a, register const VecFloat4* b );
FINLINE_Z VecFloat4 VecFloatDot3x4 ( register const VecFloat4* a, register const VecFloat4& b );
FINLINE_Z VecFloat4 VecFloatDot4x4 ( register const VecFloat4* a, register const VecFloat4* b );

FINLINE_Z VecFloat4 VecFloatLength3 ( register const VecFloat4& a );
FINLINE_Z VecFloat4 VecFloatLength4 ( register const VecFloat4& a );
FINLINE_Z VecFloat4 VecFloatLength3x3 ( register const VecFloat4* v );
FINLINE_Z VecFloat4 VecFloatLength3x4 ( register const VecFloat4* v );
FINLINE_Z VecFloat4 VecFloatLength3_SoA ( register const VecFloat4* source );
FINLINE_Z VecFloat4 VecFloatLength4_SoA ( register const VecFloat4* source );
FINLINE_Z VecFloat4 VecFloatSquaredLength3 ( register const VecFloat4& a );
FINLINE_Z VecFloat4 VecFloatSquaredLength4 ( register const VecFloat4& a );
FINLINE_Z VecFloat4 VecFloatSquaredLength3x3 ( register const VecFloat4* v );
FINLINE_Z VecFloat4 VecFloatSquaredLength3x4 ( register const VecFloat4* v );
FINLINE_Z VecFloat4 VecFloatRLength3 ( register const VecFloat4& a, U32 refinements = 0 );
FINLINE_Z VecFloat4 VecFloatRLength4 ( register const VecFloat4& a, U32 refinements = 0 );

FINLINE_Z VecFloat4 VecFloatSquareDistance3 ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z VecFloat4 VecFloatDistance3 ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z VecFloat4 VecFloatSquareDistance4 ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z VecFloat4 VecFloatDistance4 ( register const VecFloat4& a, register const VecFloat4& b );

FINLINE_Z VecFloat4 VecFloatNormalize3 ( register const VecFloat4& a, U32 refinements = 2 );
FINLINE_Z VecFloat4 VecFloatNormalize4 ( register const VecFloat4& a, U32 refinements = 2 );
FINLINE_Z VecFloat4 VecFloatCNormalize3 ( register const VecFloat4& a, U32 refinements = 2 );
FINLINE_Z VecFloat4 VecFloatCNormalize4 ( register const VecFloat4& a, U32 refinements = 2 );
FINLINE_Z void VecFloatNormalize3_SoA ( register VecFloat4* results, register const VecFloat4* source, U32 refinements = 1 );
FINLINE_Z void VecFloatNormalize4_SoA ( register VecFloat4* results, register const VecFloat4* source, U32 refinements = 1 );

FINLINE_Z VecFloat4 VecFloatStep ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z VecFloat4 VecFloatSmoothstep ( register const VecFloat4& min, register const VecFloat4& max, register const VecFloat4& x );
FINLINE_Z VecFloat4 VecFloatBezier( register const VecFloat4& p1, register const VecFloat4& p2, register const VecFloat4& p3, register const VecFloat4& p4, register const VecFloat4& x );

FINLINE_Z VecFloat4 VecFloatModAngles ( register const VecFloat4& angles );

FINLINE_Z VecFloat4 VecFloatAngleBetweenNormals3 ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z VecFloat4 VecFloatAngleBetweenNormals4 ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z VecFloat4 VecFloatAngleBetweenVectors3 ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z VecFloat4 VecFloatAngleBetweenVectors4 ( register const VecFloat4& a, register const VecFloat4& b );

FINLINE_Z VecFloat4 VecFloatQuaternionDistance ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z VecFloat4 VecFloatDeterminantM3x3 ( register const VecFloat3x3& mat3x3 );
FINLINE_Z VecFloat4 VecFloatDeterminantM4x4 ( register const VecFloat4x4& mat4x4 );

FINLINE_Z VecFloat4 VecFloatReflect3 ( register const VecFloat4& incident, register const VecFloat4& normal );
FINLINE_Z VecFloat4 VecFloatReflect4 ( register const VecFloat4& incident, register const VecFloat4& normal );

FINLINE_Z void VecFloatBuildOrthoBase3 ( const register VecFloat4& v, register VecFloat4& n1, register VecFloat4& n2 );
FINLINE_Z void VecFloatOrthonormalize3 ( register const VecFloat4& n, register VecFloat4& t, register VecFloat4& b );

FINLINE_Z VecFloat4 VecFloatQuaternionGetAxisAngle ( register const VecFloat4& q );
FINLINE_Z VecFloat4 VecFloatQuaternionRotate ( register const VecFloat4& v, register const VecFloat4& q );

FINLINE_Z VecFloat4 VecFloat3x3Transform ( register const VecFloat3x3& mat3x3, register const VecFloat4& v );
FINLINE_Z VecFloat4 VecFloat4x4TransformH3 ( register const VecFloat4x4& mat4x4, register const VecFloat4& v );
FINLINE_Z VecFloat4 VecFloat4x4Transform3 ( register const VecFloat4x4& mat4x4, register const VecFloat4& v );
FINLINE_Z VecFloat4 VecFloat4x4Transform4 ( register const VecFloat4x4& mat4x4, register const VecFloat4& v );
FINLINE_Z void VecFloat4x4TransformH3_SoA ( register VecFloat4* RESTRICT_Z results, register const VecFloat4x4& mat4x4, register const VecFloat4* RESTRICT_Z v, const U32 groups );
FINLINE_Z void VecFloat4x4Transform3_SoA ( register VecFloat4* RESTRICT_Z results, register const VecFloat4x4& mat4x4, register const VecFloat4* RESTRICT_Z v, const U32 groups );
FINLINE_Z void VecFloat4x4Transform4_SoA ( register VecFloat4* RESTRICT_Z results, register const VecFloat4x4& mat4x4, register const VecFloat4* RESTRICT_Z v, const U32 groups );


///@}
/// @name 3x3 matrix results
///@{

FINLINE_Z void VecFloat3x3Scale( register VecFloat3x3& result, register const VecFloat3x3& m0, const Float scalar );
FINLINE_Z void VecFloat3x3Multiply ( register VecFloat3x3& result, register const VecFloat3x3& m1, register const VecFloat3x3& m2 );
FINLINE_Z void VecFloat3x3MultiplyN ( register VecFloat3x3* RESTRICT_Z results, register const VecFloat3x3* RESTRICT_Z m, register const VecFloat3x3& n, const U32 count );
FINLINE_Z void VecFloat3x3Transpose ( register VecFloat3x3& transposed, register const VecFloat3x3& mat3x3 );
FINLINE_Z void VecFloat3x3MultiplyTransposeN ( register VecFloat3x3* RESTRICT_Z results, register const VecFloat3x3* RESTRICT_Z m, register const VecFloat3x3& n, const U32 count );
FINLINE_Z void VecFloat3x3MultiplyTranspose ( register VecFloat3x3& result, register const VecFloat3x3& m1, register const VecFloat3x3& m2 );
FINLINE_Z void VecFloat3x3Inverse ( register VecFloat3x3& inverse, register const VecFloat3x3& mat3x3 );


///@}
/// @name 4x4 matrix results
///@{

FINLINE_Z void VecFloat4x4Scale( register VecFloat4x4& result, register const VecFloat4x4& m0, const Float scalar );
FINLINE_Z void VecFloat4x4Add ( register VecFloat4x4& result, register const VecFloat4x4& m1, register const VecFloat4x4& m2 );
FINLINE_Z void VecFloat4x4Multiply ( register VecFloat4x4& result, register const VecFloat4x4& m1, register const VecFloat4x4& m2 );
FINLINE_Z void VecFloat4x4MultiplyN ( register VecFloat4x4* RESTRICT_Z results, register const VecFloat4x4* RESTRICT_Z m, register const VecFloat4x4& n, const U32 count );
FINLINE_Z void VecFloat4x4Transpose ( register VecFloat4x4& transposed, register const VecFloat4x4& mat4x4 );
FINLINE_Z void VecFloat4x4MultiplyTranspose ( register VecFloat4x4& result, register const VecFloat4x4& mat4x4 );
FINLINE_Z void VecFloat4x4Inverse ( register VecFloat4x4& result, register const VecFloat4x4& m0 );

///@}
/// @name Quaternion results
///@{

FINLINE_Z VecFloat4 VecFloatQuaternionBuildAngleAxis ( const Float angle, register const VecFloat4& normalizedAxis );
FINLINE_Z void VecFloatQuaternionBuildAnglesAxes3 ( register VecFloat3x3& quaternions, register const VecFloat4& angles, register const VecFloat3x3& normalizedAxes );
FINLINE_Z void VecFloatQuaternionBuildAnglesAxes4 ( register VecFloat4x4& quaternions, register const VecFloat4& angles, register const VecFloat4x4& normalizedAxes );
FINLINE_Z void VecFloatQuaternionBuildAnglesAxes_SoA ( register VecFloat4* quaternions, register const VecFloat4& angles, register const VecFloat4* normalizedAxes );

FINLINE_Z VecFloat4 VecFloatQuaternionConjugate( register const VecFloat4& q );
FINLINE_Z VecFloat4 VecFloatQuaternionCNormalize ( register const VecFloat4& q, const U32 refinements = 2 );
FINLINE_Z VecFloat4 VecFloatQuaternionMul ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z void VecFloatQuaternionMul_SoA ( VecFloat4* RESTRICT_Z results, register const VecFloat4* RESTRICT_Z a, register const VecFloat4* RESTRICT_Z b );

FINLINE_Z VecFloat4 VecFloatQuaternionToPitchYawRoll ( register const VecFloat4& a );



// ++ 32bits integer vectors ++


///@}
/// @name Load op (induce a LHS)
///@{VecUC

FINLINE_Z VecInt4 VecIntLoad1 ( const S32 _xyzw );
FINLINE_Z VecInt4 VecIntLoad4 ( const S32 x, const S32 y, const S32 z, const S32 w );
FINLINE_Z VecInt4 VecIntLoadAligned ( const void* alignedMemory );
FINLINE_Z VecInt4 VecIntLoadUnaligned ( const void* unalignedMemory );

///@}
/// @name Set op (no LHS)
///@{

FINLINE_Z VecInt4 VecIntSplatZero ( );
FINLINE_Z VecInt4 VecIntSplatOne ( );

///@}
/// @name Store op (induce a LHS)
///@{

FINLINE_Z S32 VecIntGetX ( register const VecInt4& a );
FINLINE_Z S32 VecIntGetY ( register const VecInt4& a );
FINLINE_Z S32 VecIntGetZ ( register const VecInt4& a );
FINLINE_Z S32 VecIntGetW ( register const VecInt4& a );

FINLINE_Z void VecIntStoreAligned ( void* dest, register const VecInt4& source );
FINLINE_Z void VecIntStreamStoreAligned ( void* dest, register const VecInt4& source );
FINLINE_Z void VecIntStoreUnaligned ( void* dest, register const VecInt4& source );

///@}
/// @name Select op
///@{

FINLINE_Z VecInt4 VecIntSelectMask ( register const VecInt4& a, register const VecInt4& b, register const VecSelMask& mask );

FINLINE_Z VecInt4 VecIntSelectLess ( register const VecInt4& a, register const VecInt4& b, register const VecInt4& success, register const VecInt4& fail );
FINLINE_Z VecInt4 VecIntSelectLessOrEqual ( register const VecInt4& a, register const VecInt4& b, register const VecInt4& success, register const VecInt4& fail );
FINLINE_Z VecInt4 VecIntSelectGreater ( register const VecInt4& a, register const VecInt4& b, register const VecInt4& success, register const VecInt4& fail );
FINLINE_Z VecInt4 VecIntSelectGreaterOrEqual ( register const VecInt4& a, register const VecInt4& b, register const VecInt4& success, register const VecInt4& fail );
FINLINE_Z VecInt4 VecIntSelectEqual ( register const VecInt4& a, register const VecInt4& b, register const VecInt4& success, register const VecInt4& fail );

///@}
/// @name Logical op
///@{

FINLINE_Z VecInt4 VecIntAnd ( register const VecInt4& a, register const VecInt4& b );
FINLINE_Z VecInt4 VecIntAndNot ( register const VecInt4& a, register const VecInt4& b );
FINLINE_Z VecInt4 VecIntOr ( register const VecInt4& a, register const VecInt4& b );
FINLINE_Z VecInt4 VecIntXor ( register const VecInt4& a, register const VecInt4& b );

///@}
/// @name Arithmetic op
///@{

FINLINE_Z VecInt4 VecIntNegate ( register const VecInt4& a );
FINLINE_Z VecInt4 VecIntAdd ( register const VecInt4& a, register const VecInt4& b );
FINLINE_Z VecInt4 VecIntSub ( register const VecInt4& a, register const VecInt4& b );
FINLINE_Z VecInt4 VecIntMul ( register const VecInt4& a, register const VecInt4& b );
FINLINE_Z VecInt4 VecIntHAdd ( register const VecInt4& a, register const VecInt4& b );
FINLINE_Z VecInt4 VecIntHSub ( register const VecInt4& a, register const VecInt4& b );

FINLINE_Z VecInt4 VecIntSign ( register const VecInt4& a );
FINLINE_Z VecInt4 VecIntSign ( register const VecInt4& a, register const VecInt4& b );

FINLINE_Z VecInt4 VecIntAbs ( register const VecInt4& a );
FINLINE_Z VecInt4 VecIntMin ( register const VecInt4& a, register const VecInt4& b );
FINLINE_Z VecInt4 VecIntMinComponent ( register const VecInt4& a );
FINLINE_Z VecInt4 VecIntMinComponent3 ( register const VecInt4& a );
FINLINE_Z VecInt4 VecIntMax ( register const VecInt4& a, register const VecInt4& b );
FINLINE_Z VecInt4 VecIntMaxComponent ( register const VecInt4& a );
FINLINE_Z VecInt4 VecIntMaxComponent3 ( register const VecInt4& a );
FINLINE_Z VecInt4 VecIntClamp ( register const VecInt4& a, register const VecInt4& min, register const VecInt4& max );

FINLINE_Z VecInt4 VecIntShiftLeft ( register const VecInt4& a, const U32 offset );
FINLINE_Z VecInt4 VecIntShiftLeftX ( register const VecInt4& a, register const VecInt4& offset );
FINLINE_Z VecInt4 VecIntShiftLeft ( register const VecInt4& a, register const VecInt4& offsets );

FINLINE_Z VecInt4 VecIntShiftRight ( register const VecInt4& a, const U32 offset );
FINLINE_Z VecInt4 VecIntShiftRightPositive ( register const VecInt4& a, const U32 offset );
FINLINE_Z VecInt4 VecIntShiftRightPositiveX ( register const VecInt4& a, register const VecInt4& offset );
FINLINE_Z VecInt4 VecIntShiftRightPositive ( register const VecInt4& a, register const VecInt4& offsets );

///@}
/// @name Swizzle op
///@{

FINLINE_Z VecInt4 VecIntSplatX ( register const VecInt4& a );
FINLINE_Z VecInt4 VecIntSplatY ( register const VecInt4& a );
FINLINE_Z VecInt4 VecIntSplatZ ( register const VecInt4& a );
FINLINE_Z VecInt4 VecIntSplatW ( register const VecInt4& a );

template <U32 ElementCount>
FINLINE_Z VecInt4 VecIntRotateLeft ( register const VecInt4& a );

template <VectorSwizzle x, VectorSwizzle y, VectorSwizzle z, VectorSwizzle w> FINLINE_Z VecInt4 VecIntSwizzle ( register const VecInt4& a );
template <VectorSwizzle SrcPos, VectorSwizzle DestPos> FINLINE_Z VecInt4 VecIntZeroInsert ( register const VecInt4& a );
FINLINE_Z VecInt4 VecIntSwizzleXXZZ ( register const VecInt4& a );
FINLINE_Z VecInt4 VecIntSwizzleYYWW ( register const VecInt4& a );

FINLINE_Z VecInt4 VecIntPermuteX0Z0X1Z1 ( register const VecInt4& a, register const VecInt4& b );
FINLINE_Z VecInt4 VecIntPermuteY0W0Y1W1 ( register const VecInt4& a, register const VecInt4& b );

///@}
/// @name Compare op
///@{

FINLINE_Z U32 VecIntAllZero ( register const VecInt4& a );
FINLINE_Z U32 VecIntAllEqual ( register const VecInt4& a, register const VecInt4& b );
FINLINE_Z U32 VecIntAnyNotEqual ( register const VecInt4& a, register const VecInt4& b );
FINLINE_Z U32 VecIntAllLess ( register const VecInt4& a, register const VecInt4& b );
FINLINE_Z U32 VecIntAllLessOrEqual ( register const VecInt4& a, register const VecInt4& b );
FINLINE_Z U32 VecIntAllGreater ( register const VecInt4& a, register const VecInt4& b );
FINLINE_Z U32 VecIntAllGreaterOrEqual ( register const VecInt4& a, register const VecInt4& b );

FINLINE_Z U32 VecIntAll3Equal ( register const VecInt4& a, register const VecInt4& b );

///@}
/// @name Common math
///@{

FINLINE_Z VecInt4 VecIntAddComponents ( register const VecInt4& a );

FINLINE_Z VecInt4 VecIntAverage ( register const VecInt4& a, register const VecInt4& b );



// ++ 32bits unsigned integer vectors ++


///@}
/// @name Load op (induce a LHS)
///@{

FINLINE_Z VecUInt4 VecUIntLoad1 ( const U32 _xyzw );
FINLINE_Z VecUInt4 VecUIntLoad4 ( const U32 x, const U32 y, const U32 z, const U32 w );
FINLINE_Z VecUInt4 VecUIntLoadAligned ( const void* alignedMemory );
FINLINE_Z VecUInt4 VecUIntLoadUnaligned ( const void* unalignedMemory );

///@}
/// @name Store op (induce a LHS)
///@{

FINLINE_Z void VecUIntStoreAligned ( void* dest, register const VecUInt4& source );
FINLINE_Z void VecUIntStreamStoreAligned ( void* dest, register const VecUInt4& source );
FINLINE_Z void VecUIntStoreUnaligned ( void* dest, register const VecUInt4& source );

///@}
/// @name Logical op
///@{

FINLINE_Z VecUInt4 VecUIntAnd ( register const VecUInt4& a, register const VecUInt4& b );
FINLINE_Z VecUInt4 VecUIntAndNot ( register const VecUInt4& a, register const VecUInt4& b );
FINLINE_Z VecUInt4 VecUIntOr ( register const VecUInt4& a, register const VecUInt4& b );
FINLINE_Z VecUInt4 VecUIntXor ( register const VecUInt4& a, register const VecUInt4& b );

///@}
/// @name Arithmetic op
///@{

FINLINE_Z VecUInt4 VecUIntAdd ( register const VecUInt4& a, register const VecUInt4& b );
FINLINE_Z VecUInt4 VecUIntSub ( register const VecUInt4& a, register const VecUInt4& b );
FINLINE_Z VecUInt4 VecUIntMul ( register const VecUInt4& a, register const VecUInt4& b );
FINLINE_Z VecULongLong2 VecUIntMul64( register const VecUInt4& a, register const VecUInt4& b );

FINLINE_Z VecUInt4 VecUIntShiftLeft ( register const VecUInt4& a, const U32 offset );
FINLINE_Z VecUInt4 VecUIntShiftRight ( register const VecUInt4& a, const U32 offset );


// ++ 16bits signed integer vectors ++

///@}
/// @name Load op (induce a LHS)
///@{

FINLINE_Z VecShort8 VecShortLoad1 ( const S16 a );
FINLINE_Z VecShort8 VecShortLoad8 ( S16 a, S16 b, S16 c, S16 d, S16 e, S16 f, S16 g, S16 h );
FINLINE_Z VecShort8 VecShortLoadAligned ( const void* alignedMemory );
FINLINE_Z VecShort8 VecShortLoadUnaligned ( const void* unalignedMemory );

///@}
/// @name Set op (no LHS)
///@{

FINLINE_Z VecShort8 VecShortSplatZero ( );
FINLINE_Z VecShort8 VecShortSplatOne ( );

///@}
/// @name Store op (induce a LHS)
///@{

template <U8 index> FINLINE_Z S16 VecShortGet ( register const VecShort8& a );
template <U8 index> FINLINE_Z S32 VecShortGetInt ( register const VecShort8& a );

FINLINE_Z void VecShortStoreAligned ( void* dest, register const VecShort8& source );
FINLINE_Z void VecShortStreamStoreAligned ( void* dest, register const VecShort8& source );
FINLINE_Z void VecShortStoreUnaligned ( void* dest, register const VecShort8& source );

///@}
/// @name Select op
///@{

FINLINE_Z VecShort8 VecShortSelectMask ( register const VecShort8& a, register const VecShort8& b, register const VecSelMask& mask );
FINLINE_Z VecShort8 VecShortSelectBitMask ( register const VecShort8& a, register const VecShort8& b, register const VecShort8& bitMask );

FINLINE_Z VecShort8 VecShortSelectGreater ( register const VecShort8& a, register const VecShort8& b, register const VecShort8& success, register const VecShort8& fail );

///@}
/// @name Logical op
///@{

FINLINE_Z VecUShort8 VecUShortAnd ( register const VecUShort8& a, register const VecUShort8& b );
FINLINE_Z VecUShort8 VecUShortOr ( register const VecUShort8& a, register const VecUShort8& b );

///@}
/// @name Arithmetic op
///@{

FINLINE_Z VecShort8 VecShortAdd ( register const VecShort8& a, register const VecShort8& b );
FINLINE_Z VecShort8 VecShortSub ( register const VecShort8& a, register const VecShort8& b );

FINLINE_Z VecShort8 VecShortMin ( register const VecShort8& a, register const VecShort8& b );
FINLINE_Z VecShort8 VecShortMax ( register const VecShort8& a, register const VecShort8& b );


// ++ 16bits unsigned integer vectors ++

///@}
/// @name Load op (induce a LHS)
///@{

FINLINE_Z VecUShort8 VecUShortLoad1 ( const U16 a );
FINLINE_Z VecUShort8 VecUShortLoad8 ( U16 a, U16 b, U16 c, U16 d, U16 e, U16 f, U16 g, U16 h );
FINLINE_Z VecUShort8 VecUShortLoadAligned ( const void* alignedMemory );
FINLINE_Z VecUShort8 VecUShortLoadUnaligned ( const void* unalignedMemory );

///@}
/// @name Set op (no LHS)
///@{

FINLINE_Z VecUShort8 VecUShortSplatZero ( );
FINLINE_Z VecUShort8 VecUShortSplatOne ( );

///@}
/// @name Store op (induce a LHS)
///@{

template <U8 index> FINLINE_Z U16 VecUShortGet ( register const VecUShort8& a );
template <U8 index> FINLINE_Z S32 VecUShortGetInt ( register const VecUShort8& a );

FINLINE_Z void VecUShortStoreAligned ( void* dest, register const VecUShort8& source );
FINLINE_Z void VecUShortStreamStoreAligned ( void* dest, register const VecUShort8& source );
FINLINE_Z void VecUShortStoreUnaligned ( void* dest, register const VecUShort8& source );

///@}
/// @name Select op
///@{

FINLINE_Z VecUShort8 VecUShortSelectMask ( register const VecUShort8& a, register const VecUShort8& b, register const VecSelMask& mask );
FINLINE_Z VecUShort8 VecUShortSelectBitMask ( register const VecUShort8& a, register const VecUShort8& b, register const VecUShort8& bitMask );

FINLINE_Z VecUShort8 VecUShortSelectEqual ( register const VecUShort8& a, register const VecUShort8& b, register const VecUShort8& success, register const VecUShort8& fail );
FINLINE_Z VecUShort8 VecUShortSelectGreater ( register const VecUShort8& a, register const VecUShort8& b, register const VecUShort8& success, register const VecUShort8& fail );

///@}
/// @name Logical op
///@{

FINLINE_Z VecUShort8 VecUShortAnd ( register const VecUShort8& a, register const VecUShort8& b );
FINLINE_Z VecUShort8 VecUShortOr ( register const VecUShort8& a, register const VecUShort8& b );

///@}
/// @name Arithmetic op
///@{

FINLINE_Z VecUShort8 VecUShortAdd ( register const VecUShort8& a, register const VecUShort8& b );
FINLINE_Z VecUShort8 VecUShortSub ( register const VecUShort8& a, register const VecUShort8& b );
FINLINE_Z VecUShort8 VecUShortSubSat ( register const VecUShort8& a, register const VecUShort8& b );

FINLINE_Z VecUShort8 VecUShortMin ( register const VecUShort8& a, register const VecUShort8& b );
FINLINE_Z VecUShort8 VecUShortMax ( register const VecUShort8& a, register const VecUShort8& b );

///@}
/// @name Swizzle op
///@{

template <U16 ElementCount> VecUShort8
FINLINE_Z VecUShortRotateLeft ( register const VecUShort8& a );

FINLINE_Z VecUShort8 VecUShortSwizzleABCDFFGH ( register const VecUShort8& a );
FINLINE_Z VecUShort8 VecUShortSwizzleACEGACEG ( register const VecUShort8& a );
FINLINE_Z VecUShort8 VecUShortSwizzleBDFHBDFH ( register const VecUShort8& a );

FINLINE_Z VecUShort8 VecUShortPermuteA0A1B0B1C0C1D0D1 ( const VecUShort8& a, const VecUShort8& b );
FINLINE_Z VecUShort8 VecUShortPermuteE0E1F0F1G0G1H0H1 ( const VecUShort8& a, const VecUShort8& b );


// ++ 8bits unsigned char vectors ++

///@}
/// @name Load op (induce a LHS)
///@{

FINLINE_Z VecUChar16 VecUCharLoad1 ( const U8 a );
FINLINE_Z VecUChar16 VecUCharLoad16 ( U8 u0, U8 u1, U8 u2, U8 u3, U8 u4, U8 u5, U8 u6, U8 u7, U8 u8, U8 u9, U8 u10, U8 u11, U8 u12, U8 u13, U8 u14, U8 u15 );
FINLINE_Z VecUChar16 VecUCharLoadAligned ( const void* alignedMemory );
FINLINE_Z VecUChar16 VecUCharLoadUnaligned ( const void* unalignedMemory );

///@}
/// @name Set op (no LHS)
///@{

FINLINE_Z VecUChar16 VecUCharSplatZero ( );
FINLINE_Z VecUChar16 VecUCharSplatOne ( );

///@}
/// @name Store op (induce a LHS)
///@{

template <U8 index> FINLINE_Z U8 VecUCharGet ( register const VecUChar16& a );
template <U8 index> FINLINE_Z S32 VecUCharGetInt ( register const VecUChar16& a );

FINLINE_Z void VecUCharStoreAligned ( void* dest, register const VecUChar16& source );
FINLINE_Z void VecUCharStreamStoreAligned ( void* dest, register const VecUChar16& source );
FINLINE_Z void VecUCharStoreUnaligned ( void* dest, register const VecUChar16& source );

///@}
/// @name Select op
///@{

FINLINE_Z VecUChar16 VecUCharSelectMask ( register const VecUChar16& a, register const VecUChar16& b, register const VecSelMask& mask );
FINLINE_Z VecUChar16 VecUCharSelectBitMask ( register const VecUChar16& a, register const VecUChar16& b, register const VecUChar16& bitMask );

FINLINE_Z VecUChar16 VecUCharSelectEqual ( register const VecUChar16& a, register const VecUChar16& b, register const VecUChar16& success, register const VecUChar16& fail );
FINLINE_Z VecUChar16 VecUCharSelectGreater ( register const VecUChar16& a, register const VecUChar16& b, register const VecUChar16& success, register const VecUChar16& fail );

///@}
/// @name Logical op
///@{

FINLINE_Z VecUChar16 VecUCharAnd ( register const VecUChar16& a, register const VecUChar16& b );
FINLINE_Z VecUChar16 VecUCharOr ( register const VecUChar16& a, register const VecUChar16& b );

///@}
/// @name Arithmetic op
///@{

FINLINE_Z VecUChar16 VecUCharAdd ( register const VecUChar16& a, register const VecUChar16& b );
FINLINE_Z VecUChar16 VecUCharSub ( register const VecUChar16& a, register const VecUChar16& b );
FINLINE_Z VecUChar16 VecUCharSubS ( register const VecUChar16& a, register const VecUChar16& b );

FINLINE_Z VecUChar16 VecUCharAbsDiff ( register const VecUChar16& a, register const VecUChar16& b );
FINLINE_Z VecUChar16 VecUCharMin ( register const VecUChar16& a, register const VecUChar16& b );
FINLINE_Z VecUChar16 VecUCharMax ( register const VecUChar16& a, register const VecUChar16& b );

FINLINE_Z VecUShort8 VecUCharSad ( register const VecUChar16& a, register const VecUChar16& b );

///@}
/// @name Swizzle op
///@{

FINLINE_Z VecUChar16 VecUCharSwizzleABCMDEFNGHIOJKLP ( register const VecUChar16& a );
FINLINE_Z VecUChar16 VecUCharSwizzleACEGIKMOBDFHJLNP ( register const VecUChar16& a );
FINLINE_Z VecUChar16 VecUCharSwizzleCBAMFEDNIHGOLKJP ( register const VecUChar16& a );

FINLINE_Z VecUChar16 VecUCharPermuteLow ( const VecUChar16& a, const VecUChar16& b );
FINLINE_Z VecUChar16 VecUCharPermuteHigh ( const VecUChar16& a, const VecUChar16& b );



// ++ Select mask vectors ++

///@}
/// @name Set op (no LHS)
///@{

FINLINE_Z VecSelMask VecSelMaskSplatTrue();
FINLINE_Z VecSelMask VecSelMaskSplatFalse();

///@}
/// @name Compare op
///@{

FINLINE_Z VecSelMask VecFloatCompareLT ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z VecSelMask VecFloatCompareLE ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z VecSelMask VecFloatCompareGT ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z VecSelMask VecFloatCompareGE ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z VecSelMask VecFloatCompareEQ ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z VecSelMask VecFloatCompareBounds ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z VecSelMask VecFloatCompareSign ( register const VecFloat4& a, register const VecFloat4& b );
FINLINE_Z VecSelMask VecFloatCompareNanOrInfinite ( register const VecFloat4& a );

FINLINE_Z VecSelMask VecIntCompareLT ( register const VecInt4& a, register const VecInt4& b );
FINLINE_Z VecSelMask VecIntCompareLE ( register const VecInt4& a, register const VecInt4& b );
FINLINE_Z VecSelMask VecIntCompareGT ( register const VecInt4& a, register const VecInt4& b );
FINLINE_Z VecSelMask VecIntCompareGE( register const VecInt4& a, register const VecInt4& b );
FINLINE_Z VecSelMask VecIntCompareEQ ( register const VecInt4& a, register const VecInt4& b );

FINLINE_Z VecSelMask VecUShortCompareGT ( register const VecUShort8& a, register const VecUShort8& b );
FINLINE_Z VecSelMask VecUShortCompareEQ ( register const VecUShort8& a, register const VecUShort8& b );

FINLINE_Z VecSelMask VecShortCompareGT ( register const VecShort8& a, register const VecShort8& b );

FINLINE_Z VecSelMask VecUCharCompareGT ( register const VecUChar16& a, register const VecUChar16& b );
FINLINE_Z VecSelMask VecUCharCompareEQ ( register const VecUChar16& a, register const VecUChar16& b );

FINLINE_Z U32 VecSelMaskPackBits( register const VecSelMask& v );

///@}
/// @name Logical op
///@{

FINLINE_Z VecSelMask VecSelMaskAnd ( register const VecSelMask& a, register const VecSelMask& b );
FINLINE_Z VecSelMask VecSelMaskOr ( register const VecSelMask& a, register const VecSelMask& b );
FINLINE_Z VecSelMask VecSelMaskXor ( register const VecSelMask& a, register const VecSelMask& b );
FINLINE_Z VecSelMask VecSelMaskAndNot ( register const VecSelMask& a, register const VecSelMask& b );

FINLINE_Z U32 VecSelMaskAllTrue( register const VecSelMask& a );
FINLINE_Z U32 VecSelMaskAll3True( register const VecSelMask& a );
FINLINE_Z U32 VecSelMaskAnyTrue( register const VecSelMask& a );

FINLINE_Z U32 VecSelMaskAllFalse( register const VecSelMask& a );
FINLINE_Z U32 VecSelMaskAll3False( register const VecSelMask& a );
FINLINE_Z U32 VecSelMaskAnyFalse( register const VecSelMask& a );


// ++ Vector convert ++

///@}
/// @name Vector to vector (no LHS)
///@{

FINLINE_Z VecInt4 VecFloatAsInt( register const VecFloat4& a );
FINLINE_Z VecFloat4 VecIntAsFloat( register const VecInt4& a );

FINLINE_Z VecUInt4 VecFloatAsUInt( register const VecFloat4& a );
FINLINE_Z VecFloat4 VecUIntAsFloat( register const VecUInt4& a );

FINLINE_Z VecUInt4 VecULongLongAsUInt( register const VecULongLong2& a );
FINLINE_Z VecULongLong2 VecUIntAsULongLong( register const VecUInt4& a );

FINLINE_Z VecSelMask VecFloatAsSelMask( register const VecFloat4& a );
FINLINE_Z VecFloat4 VecSelMaskAsFloat( register const VecSelMask& a );

FINLINE_Z VecSelMask VecIntAsSelMask( register const VecInt4& a );
FINLINE_Z VecInt4 VecSelMaskAsInt( register const VecSelMask& a );

FINLINE_Z VecUInt4 VecIntAsUInt( register const VecInt4& a );
FINLINE_Z VecInt4 VecUIntAsInt( register const VecUInt4& a );

FINLINE_Z VecInt4 VecFloatToIntFloor ( register const VecFloat4& a );
FINLINE_Z VecInt4 VecFloatToIntCeil ( register const VecFloat4& a );
FINLINE_Z VecInt4 VecFloatToIntNearest ( register const VecFloat4& a );
FINLINE_Z VecInt4 VecFloatToIntNearestPositive ( register const VecFloat4& a );
FINLINE_Z VecInt4 VecFloatToInt ( register const VecFloat4& a );
#define VecFTOI VecFloatToIntFloor
FINLINE_Z VecUShort8 VecFloatToUShortSat ( register const VecFloat4* v );
template <U16 LeftShift> FINLINE_Z VecUShort8 VecFloatToUShortSat ( register const VecFloat4* v );
FINLINE_Z VecUChar16 VecFloatToUCharSat ( register const VecFloat4* v );

FINLINE_Z VecFloat4 VecIntToFloat ( register const VecInt4& a );
FINLINE_Z VecShort8 VecIntToShortSat ( register const VecInt4* v );
FINLINE_Z VecUShort8 VecIntToUShortSat ( register const VecInt4* v );
FINLINE_Z VecUShort8 VecIntToUShortModulo ( register const VecInt4* v );
FINLINE_Z VecUChar16 VecIntToUCharSat ( register const VecInt4* v );

FINLINE_Z VecUShort8 VecUIntToUShortSat ( register const VecUInt4* v );
FINLINE_Z VecUShort8 VecUIntToUShortModulo ( register const VecUInt4* v );

FINLINE_Z void VecShortToFloat ( VecFloat4* results, const VecShort8& v );

template <U16 RightShift> FINLINE_Z void VecUShortToFloat ( VecFloat4* results, const VecUShort8& v );
FINLINE_Z void VecUShortToInt ( VecInt4* results, const VecUShort8& a );
FINLINE_Z VecUChar16 VecUShortToUCharSat ( register const VecUShort8* v );

FINLINE_Z void VecUCharToFloat ( VecFloat4* results, const VecUChar16& a );
FINLINE_Z void VecUCharToInt ( VecInt4* results, const VecUChar16& a );
FINLINE_Z void VecUCharToUShort ( VecUShort8* results, const VecUChar16& a );
FINLINE_Z void VecUCharToUShortEvenOdd ( register VecUShort8& even, register VecUShort8& odd, register const VecUChar16& a );

FINLINE_Z VecInt4 VecFloatPackColors8888 ( register const VecFloat4 colors[4] );
FINLINE_Z void VecFloatUnpackColors8888 ( VecFloat4 unpackedColors[4], const VecInt4& packedColors );

FINLINE_Z VecUChar16 VecUCharConvertR8G8B8ToR8G8B8A8 ( register const VecUChar16& a );
FINLINE_Z VecUChar16 VecUCharConvertB8G8R8ToR8G8B8A8 ( register const VecUChar16& a );
FINLINE_Z void VecUCharConvertR8G8B8A8ToR8G8B8 ( VecUChar16* destRGB, register const VecUChar16* srcRGBA );

FINLINE_Z VecFloat4 VecFloatTransposeColumnsX3( register const VecFloat4* columns );
FINLINE_Z VecFloat4 VecFloatTransposeColumnsX( register const VecFloat4* columns );
FINLINE_Z VecFloat4 VecFloatTransposeColumnsY( register const VecFloat4* columns );
FINLINE_Z VecFloat4 VecFloatTransposeColumnsZ( register const VecFloat4* columns );
FINLINE_Z VecFloat4 VecFloatTransposeColumnsW( register const VecFloat4* columns );
FINLINE_Z void VecFloatTranspose3SoA ( register VecFloat4* dest, register const VecFloat4* source );
FINLINE_Z void VecFloatTranspose4SoA ( register VecFloat4* dest, register const VecFloat4* source );

///@}
/// @name Load op (induce a LHS)
///@{

FINLINE_Z VecFloat4 VecFloatFromShort4 ( const S16* s );

template <U16 RightShift> FINLINE_Z VecFloat4 VecFloatFromUShort4 ( const U16* u );

//FINLINE_Z VecFloat4 VecFloatFromHalf2 ( const Half* h );
//FINLINE_Z VecFloat4 VecFloatFromHalf4 ( const Half* h );

FINLINE_Z VecFloat4 VecFloatUnpack2FromS16 ( const S16* source );
FINLINE_Z VecFloat4 VecFloatUnpack4FromS16 ( const S16* source );

FINLINE_Z VecFloat4 VecFloatUnpackSnorm8888( const U32 packedNormal );

FINLINE_Z VecFloat4 VecFloatUnpackColor8888 ( const U32 color );

///@}
/// @name Store op (induce a LHS)
///@{

//FINLINE_Z void VecFloatToHalf2 ( Half* result, register const VecFloat4& a );
//FINLINE_Z void VecFloatToHalf4 ( Half* result, register const VecFloat4& a );

FINLINE_Z void VecFloatPack2ToS16NormedU ( S16* dest, register const VecFloat4& a );
FINLINE_Z void VecFloatPack4ToS16NormedU ( S16* dest, register const VecFloat4& a );
FINLINE_Z void VecFloatPack4ToS16NormedA ( S16* dest, register const VecFloat4& a );

FINLINE_Z U32 VecFloatPackSnorm8888( register const VecFloat4& normal );

FINLINE_Z U32 VecFloatPackColor8888 ( register const VecFloat4& color );

FINLINE_Z void VecFloatPack16ToU8A ( U8* dest, register const VecFloat4* source );

FINLINE_Z void VecIntPack4ToS16 ( S16* dest, register const VecInt4& a );


///@}
/// @name ++ Matrix convert ++
///@{

FINLINE_Z void VecFloat4x4CopyM3x3 ( register VecFloat4x4& result, register const VecFloat3x3& m0 );


///@}
/// @name ++ Quaternion convert ++
///@{

FINLINE_Z void VecFloatQuaternionTo3x3 ( register VecFloat3x3& matrix, register const VecFloat4& quat );

FINLINE_Z void VecFloatQuaternionTo4x4 ( Float* matrix, register const VecFloat4& quat );
FINLINE_Z void VecFloatQuaternionTo4x4 ( register VecFloat4x4& matrix, register const VecFloat4& quat );

FINLINE_Z VecFloat4 VecFloat3x3ToQuaternion( register const VecFloat3x3& m );


#ifndef _PC_SSE4
	#ifdef _PC_SSE
		static __m128 _mm_sel_ps( __m128 a, __m128 b, __m128 mask )
		{
			//return _mm_or_ps( _mm_andnot_ps(mask,a), _mm_and_ps(b,mask) );
			return _mm_xor_ps( a, _mm_and_ps( mask, _mm_xor_ps(b,a) ) );
		}
	#endif

	#ifdef _PC_SSE2
		static __m128i _mm_sel_epi8( __m128i a, __m128i b, __m128i mask )
		{
			return _mm_or_si128( _mm_andnot_si128(mask,a), _mm_and_si128(b,mask) );
		}
	#endif
#else
	#define	_mm_sel_ps		_mm_blendv_ps
	#define	_mm_sel_epi8	_mm_blendv_epi8
#endif


#ifdef _ALWAYS_INLINE_BASE_FUNCTIONS_

#if defined (_PC_SSE )
	#define VecFloatLoadAligned(alignedMem)		_mm_load_ps( reinterpret_cast<const float*>((alignedMem)) )
	#define VecFloatStoreAligned(dest,src)		_mm_store_ps( reinterpret_cast<float*>((dest)), src )
	#define VecFloatAdd(a,b)					_mm_add_ps( (a), (b) )
	#define VecFloatSub(a,b)					_mm_sub_ps( (a), (b) )
	#define VecFloatMul(a,b)					_mm_mul_ps( (a), (b) )
	#define VecFloatMadd(a,b,c)					VecFloatAdd( VecFloatMul(a,b), (c) )
#endif	

#endif



///////////////// Floating-point vectors /////////////////


// ---------------------- Load operations

/// xyzw = scalar
FINLINE_Z VecFloat4 VecFloatLoad1 ( const Float scalar )
{
#if defined (_PC_SSE )
	return _mm_set_ps1( scalar );
#else
	VecFloat4 result;
	result.x = result.y = result.z = result.w = scalar;
	return result;
#endif
}

/// xyzw = { x y z w }
FINLINE_Z VecFloat4 VecFloatLoad4 ( const Float x, const Float y, const Float z, const Float w )
{
#if defined (_PC_SSE )
	return _mm_set_ps( w, z, y, x );
#else
	VecFloat4 result;
	result.x = x;
	result.y = y;
	result.z = z;
	result.w = w;
	return result;
#endif
}

#ifndef _ALWAYS_INLINE_BASE_FUNCTIONS_

/// Load xyzw from a 16-byte aligned memory address
FINLINE_Z VecFloat4 VecFloatLoadAligned ( const void* alignedMemory )
{
#if defined (_PC_SSE )
	return _mm_load_ps( reinterpret_cast<const float*>(alignedMemory) );
#else
	VecFloat4 result;
	memcpy( &result, alignedMemory, sizeof(result) );
	return result;
#endif
}

#endif

/// Load xyzw from any memory address
FINLINE_Z VecFloat4 VecFloatLoadUnaligned ( const void* unalignedMemory )
{
#if defined (_PC_SSE )
	return _mm_loadu_ps( reinterpret_cast<const float*>(unalignedMemory) );
#else
	VecFloat4 result;
	memcpy( &result, unalignedMemory, sizeof(result) );
	return result;
#endif
}

/// a.x = x
FINLINE_Z void VecFloatSetX ( register VecFloat4& a, const Float x )
{
	a = VecFloatPermuteX0Y1Z1W1( VecFloatLoad1(x), a );
}

/// a.y = y
FINLINE_Z void VecFloatSetY ( register VecFloat4& a, const Float y )
{
	a = VecFloatPermuteX0X1Z0W0( a, VecFloatLoad1(y) );
}

/// a.z = z
FINLINE_Z void VecFloatSetZ ( register VecFloat4& a, const Float z )
{
	a = VecFloatPermuteX0Y0X1W0( a, VecFloatLoad1(z) );
}

FINLINE_Z void VecFloatSetW ( register VecFloat4& a, const Float w ) /// a.w = w
{
	a = VecFloatPermuteX0Y0Z0X1( a, VecFloatLoad1(w) );
}

FINLINE_Z void VecFloat3x3Load1( register VecFloat3x3& result, const Float scalar ) /// result{0-2}.xyzw = scalar
{
	result[0] = result[1] = result[2] = VecFloatLoad1( scalar );
}

FINLINE_Z void VecFloat3x3LoadRow0 ( register VecFloat3x3& result, const Float x, const Float y, const Float z ) /// result[0].xyz = { x, y, z }
{
	result[0] = VecFloatLoad4( x, y, z, 0.f );
}

FINLINE_Z void VecFloat3x3LoadRow1 ( register VecFloat3x3& result, const Float x, const Float y, const Float z ) /// result[1].xyz = { x, y, z }
{
	result[1] = VecFloatLoad4( x, y, z, 0.f );
}

FINLINE_Z void VecFloat3x3LoadRow2 ( register VecFloat3x3& result, const Float x, const Float y, const Float z ) /// result[2].xyz = { x, y, z }
{
	result[2] = VecFloatLoad4( x, y, z, 0.f );
}

FINLINE_Z void VecFloat4x4Load1( register VecFloat4x4& result, const Float scalar ) /// result{0-3}.xyzw = scalar
{
	result[0] = result[1] = result[2] = result[3] = VecFloatLoad1( scalar );
}

FINLINE_Z void VecFloat4x4LoadRow0 ( register VecFloat4x4& result, const Float x, const Float y, const Float z, const Float w ) /// result[0].xyzw = { x, y, z, w }
{
	result[0] = VecFloatLoad4( x, y, z, w );
}

FINLINE_Z void VecFloat4x4LoadRow1 ( register VecFloat4x4& result, const Float x, const Float y, const Float z, const Float w ) /// result[1].xyzw = { x, y, z, w }
{
	result[1] = VecFloatLoad4( x, y, z, w );
}

FINLINE_Z void VecFloat4x4LoadRow2 ( register VecFloat4x4& result, const Float x, const Float y, const Float z, const Float w ) /// result[2].xyzw = { x, y, z, w }
{
	result[2] = VecFloatLoad4( x, y, z, w );
}

FINLINE_Z void VecFloat4x4LoadRow3 ( register VecFloat4x4& result, const Float x, const Float y, const Float z, const Float w ) /// result[3].xyzw = { x, y, z, w }
{
	result[3] = VecFloatLoad4( x, y, z, w );
}

// ---------------------- Set operations

FINLINE_Z VecFloat4 VecFloatSplatZero () /// xyzw = 0
{
#if defined (_PC_SSE )
	return _mm_setzero_ps();
#else
	return VectorConstantsPrivate::vZeroF;
#endif
}

FINLINE_Z VecFloat4 VecFloatSplatOne () /// xyzw = 1
{
	return VectorConstantsPrivate::vOneF;
}

FINLINE_Z VecFloat4 VecFloatSetIdentity ( ) /// xyzw = { 0, 0, 0, 1 }
{
#if defined (_PC_SSE )
	__m128 idtInverse = _mm_cvtsi32_ss( _mm_setzero_ps(), 1 );
	return _mm_shuffle_ps( idtInverse, idtInverse, _MM_SHUFFLE(0,1,2,3) );
#else
	return VecFloatLoad4( 0.f, 0.f, 0.f, 1.f );
#endif
}

FINLINE_Z VecFloat4 VecFloatSetXZero ( register const VecFloat4& a ) /// xyzw = float4( 0.f, a.yzw )
{
#if defined ( _PC_SSE )
	return _mm_cvtsi32_ss( a, 0 );
#else
	return VecFloatPermuteX1Y0Z0W0( a, VecFloatSplatZero() );
#endif
}

FINLINE_Z VecFloat4 VecFloatSetXOne ( register const VecFloat4& a ) /// xyzw = float4( 1.f, a.yzw )
{
#if defined ( _PC_SSE )
	return _mm_cvtsi32_ss( a, 1 );
#else
	return VecFloatPermuteX1Y0Z0W0( a, VecFloatSplatOne() );
#endif
}

FINLINE_Z VecFloat4 VecFloatSetWZero ( register const VecFloat4& a ) /// xyzw = float4( a.xyz, 0.f )
{
#if defined ( _PC_SSE4 )
	return _mm_insert_ps( a, a, 0x08 );
#elif defined ( _PC_SSE )
	return _mm_shuffle_ps( a, _mm_cvtsi32_ss(a,0), _MM_SHUFFLE(0,2,1,0) );
#else
	return VecFloatPermuteX0Y0Z0X1( a, VecFloatSplatZero() );
#endif
}

FINLINE_Z VecFloat4 VecFloatSetWOne ( register const VecFloat4& a ) /// xyzw = float4( a.xyz, 1.f )
{
#if defined ( _PC_SSE )
	return _mm_shuffle_ps( a, _mm_cvtsi32_ss(a,1), _MM_SHUFFLE(0,2,1,0) );
#else
	return VecFloatPermuteX0Y0Z0X1( a, VecFloatSplatOne() );
#endif
}

FINLINE_Z void VecFloat3x3Copy( register VecFloat3x3& result, register const VecFloat3x3& m0 ) /// result = m0
{
	result[0] = m0[0];
	result[1] = m0[1];
	result[2] = m0[2];
}

FINLINE_Z void VecFloat3x3Copy4x4( register VecFloat3x3& result, register const VecFloat4x4& m0 ) /// result = m0
{
	result[0] = m0[0];
	result[1] = m0[1];
	result[2] = m0[2];
}

FINLINE_Z void VecFloat3x3CopyRows( register VecFloat4* result, register const VecFloat3x3& m0 ) /// result = m0
{
	result[0] = m0[0];
	result[1] = m0[1];
	result[2] = m0[2];
}

FINLINE_Z void VecFloat3x3CopyArray ( register VecFloat3x3& result, register const VecFloat4* rows ) /// result = rows
{
	result[0] = rows[0];
	result[1] = rows[1];
	result[2] = rows[2];
}

FINLINE_Z void VecFloat3x3SplatZero( register VecFloat3x3& result ) /// result{0-2}.xyzw = 0
{
	result[0] = result[1] = result[2] = VecFloatSplatZero();
}

FINLINE_Z void VecFloat3x3SplatOne( register VecFloat3x3& result ) ///< result{0-2}.xyzw = 1
{
	result[0] = result[1] = result[2] = VecFloatSplatOne();
}

FINLINE_Z void VecFloat3x3SetIdentity ( register VecFloat3x3& result ) ///< result = identity matrix
{
#if defined( _PC_SSE )
	const __m128 zero = _mm_xor_ps( result[0], result[0] );
	result[0] = _mm_cvtsi32_ss( zero, 1 );
    result[1] = _mm_shuffle_ps( result[0], result[0], _MM_SHUFFLE(1,1,0,1) );
    result[2] = _mm_shuffle_ps( result[0], result[0], _MM_SHUFFLE(1,0,1,1) );
#else
	result[0] = VecFloatLoad4( 1.f, 0.f, 0.f, 0.f );
    result[1] = VecFloatLoad4( 0.f, 1.f, 0.f, 0.f );
    result[2] = VecFloatLoad4( 0.f, 0.f, 1.f, 0.f );
#endif
}

FINLINE_Z void VecFloat3x3SetIdentityScale ( register VecFloat3x3& result, register const VecFloat4& scale ) ///< result = identity + scale (xyz)
{
#if defined( _PC_SSE4 )
	result[0] = _mm_insert_ps( result[0], scale, 0x0E ); ///< scale.x(00) -> r.x(00), x pass (1110)
    result[1] = _mm_insert_ps( result[1], scale, 0x5D ); ///< scale.y(01) -> r.y(01), y pass (1101)
    result[2] = _mm_insert_ps( result[2], scale, 0xAB ); ///< scale.z(10) -> r.z(10), z pass (1011)
#elif defined( _PC_SSE )
	const __m128 zero = _mm_xor_ps( scale, scale );
	const __m128 scaleZero = VecFloatPermuteX0Y0Z0W1( scale, zero );
	result[0] = _mm_shuffle_ps( scaleZero, scaleZero, _MM_SHUFFLE(3,3,3,0) );
    result[1] = _mm_shuffle_ps( scaleZero, scaleZero, _MM_SHUFFLE(3,3,1,3) );
    result[2] = _mm_shuffle_ps( scaleZero, scaleZero, _MM_SHUFFLE(3,2,3,3) );
#else
	result[0] = VecFloatLoad4( scale.x, 0.f, 0.f, 0.f );
    result[1] = VecFloatLoad4( 0.f, scale.y, 0.f, 0.f );
    result[2] = VecFloatLoad4( 0.f, 0.f, scale.z, 0.f );
#endif
}

FINLINE_Z void VecFloat3x3SetRow0 ( register VecFloat3x3& result, const register VecFloat4& row0 ) ///< result[0] = row0
{
	result[0] = row0;
}

FINLINE_Z void VecFloat3x3SetRow1 ( register VecFloat3x3& result, const register VecFloat4& row1 ) ///< result[1] = row1
{
	result[1] = row1;
}

FINLINE_Z void VecFloat3x3SetRow2 ( register VecFloat3x3& result, const register VecFloat4& row2 ) ///< result[2] = row2
{
	result[2] = row2;
}

FINLINE_Z void VecFloat4x4Copy( register VecFloat4x4& result, register const VecFloat4x4& m0 ) ///< result = m0
{
	result[0] = m0[0];
	result[1] = m0[1];
	result[2] = m0[2];
	result[3] = m0[3];
}

FINLINE_Z void VecFloat4x4CopyRows( register VecFloat4* result, register const VecFloat4x4& m0 ) ///< result = m0
{
	result[0] = m0[0];
	result[1] = m0[1];
	result[2] = m0[2];
	result[3] = m0[3];
}

FINLINE_Z void VecFloat4x4SplatZero( register VecFloat4x4& result ) ///< result{0-3}.xyzw = 0
{
	result[0] = result[1] = result[2] = result[3] = VecFloatSplatZero();
}

FINLINE_Z void VecFloat4x4SplatOne( register VecFloat4x4& result ) ///< result{0-3}.xyzw = 1
{
	result[0] = result[1] = result[2] = result[3] = VecFloatSplatOne();
}

FINLINE_Z void VecFloat4x4SetIdentity( register VecFloat4x4& result ) ///< result = identity matrix
{
#if defined( _PC_SSE )
	const __m128 zero = _mm_xor_ps( result[0], result[0] );
	result[0] = _mm_cvtsi32_ss( zero, 1 );
    result[1] = _mm_shuffle_ps( result[0], result[0], _MM_SHUFFLE(1,1,0,1) );
    result[2] = _mm_shuffle_ps( result[0], result[0], _MM_SHUFFLE(1,0,1,1) );
    result[3] = _mm_shuffle_ps( result[0], result[0], _MM_SHUFFLE(0,1,1,1) );
#else
	result[0] = VecFloatLoad4( 1.f, 0.f, 0.f, 0.f );
    result[1] = VecFloatLoad4( 0.f, 1.f, 0.f, 0.f );
    result[2] = VecFloatLoad4( 0.f, 0.f, 1.f, 0.f );
    result[3] = VecFloatLoad4( 0.f, 0.f, 0.f, 1.f );
#endif
}

FINLINE_Z void VecFloat4x4SetIdentityTrans ( register VecFloat4x4& result, register const VecFloat4& translation ) ///< result = identity + translation (xyzw)
{
#if defined( _PC_SSE )
	const __m128 zero = _mm_xor_ps( result[0], result[0] );
	result[0] = _mm_cvtsi32_ss( zero, 1 );
    result[1] = _mm_shuffle_ps( result[0], result[0], _MM_SHUFFLE(1,1,0,1) );
    result[2] = _mm_shuffle_ps( result[0], result[0], _MM_SHUFFLE(1,0,1,1) );
    result[3] = translation;
#else
	result[0] = VecFloatLoad4( 1.f, 0.f, 0.f, 0.f );
    result[1] = VecFloatLoad4( 0.f, 1.f, 0.f, 0.f );
    result[2] = VecFloatLoad4( 0.f, 0.f, 1.f, 0.f );
    result[3] = translation;
#endif
}

FINLINE_Z void VecFloat4x4SetIdentityScale ( register VecFloat4x4& result, register const VecFloat4& scale ) ///< result = identity + scale (xyz)
{
#if defined( _PC_SSE4 )
	const __m128 v1scale = _mm_cvtsi32_ss( scale, 1 );
	result[0] = _mm_insert_ps( scale, scale, 0x0E ); // scale.x(00) -> r.x(00), x pass (1110)
    result[1] = _mm_insert_ps( scale, scale, 0x5D ); // scale.y(01) -> r.y(01), y pass (1101)
    result[2] = _mm_insert_ps( scale, scale, 0xAB ); // scale.z(10) -> r.z(10), z pass (1011)
	result[3] = _mm_insert_ps( v1scale, v1scale, 0x37 ); // v1scale.x(00) -> r.w(11), w pass (0111)
#elif defined( _PC_SSE )
	const __m128 zero = _mm_xor_ps( scale, scale );
	const __m128 v1000 = _mm_cvtsi32_ss( zero, 1 );
	const __m128 scaleZero = VecFloatPermuteX0Y0Z0W1( scale, zero );
	result[0] = _mm_shuffle_ps( scaleZero, scaleZero, _MM_SHUFFLE(3,3,3,0) );
    result[1] = _mm_shuffle_ps( scaleZero, scaleZero, _MM_SHUFFLE(3,3,1,3) );
    result[2] = _mm_shuffle_ps( scaleZero, scaleZero, _MM_SHUFFLE(3,2,3,3) );
    result[3] = _mm_shuffle_ps( v1000, v1000, _MM_SHUFFLE(0,1,1,1) );
#else
	result[0] = VecFloatLoad4( scale.x, 0.f, 0.f, 0.f );
    result[1] = VecFloatLoad4( 0.f, scale.y, 0.f, 0.f );
    result[2] = VecFloatLoad4( 0.f, 0.f, scale.z, 0.f );
    result[3] = VecFloatLoad4( 0.f, 0.f, 0.f, 1.f );
#endif
}

FINLINE_Z void VecFloat4x4SetScale ( register VecFloat4x4& result, register const VecFloat4& scale ) ///< set scale values (xyz) in result
{
	result[0] = VecFloatPermuteX1Y0Z0W0( result[0], scale );
    result[1] = VecFloatPermuteX0Y1Z0W0( result[1], scale );
    result[2] = VecFloatPermuteX0Y0Z1W0( result[2], scale );
}

FINLINE_Z void VecFloat4x4SetRow0 ( register VecFloat4x4& result, const register VecFloat4& row0 ) ///< result[0] = row0
{
	result[0] = row0;
}

FINLINE_Z void VecFloat4x4SetRow1 ( register VecFloat4x4& result, const register VecFloat4& row1 ) ///< result[1] = row1
{
	result[1] = row1;
}

FINLINE_Z void VecFloat4x4SetRow2 ( register VecFloat4x4& result, const register VecFloat4& row2 ) ///< result[2] = row2
{
	result[2] = row2;
}

FINLINE_Z void VecFloat4x4SetRow3 ( register VecFloat4x4& result, const register VecFloat4& row3 ) ///< result[3] = row3
{
	result[3] = row3;
}

FINLINE_Z void VecFloat4x4BuildFrom3x3Trans( register VecFloat4x4& result, register const VecFloat3x3& mat3x3, register const VecFloat4& translation ) ///< result = mat3x3 + translation.xyz
{
	result[0] = VecFloatSetWZero( mat3x3[0] );
	result[1] = VecFloatSetWZero( mat3x3[1] );
	result[2] = VecFloatSetWZero( mat3x3[2] );
	result[3] = VecFloatSetWOne( translation );
}

// ---------------------- Save operations

FINLINE_Z Float VecFloatGetX ( register const VecFloat4& a ) ///< Store a.x in CPU register
{
#if defined (_PC_SSE )
	return _mm_cvtss_f32( a );
#else
	return a.x;
#endif
}

FINLINE_Z Float VecFloatGetY ( register const VecFloat4& a ) ///< Store a.y in CPU register
{
#if defined (_PC_SSE)
	const Float* pA = reinterpret_cast <const Float*> ( &a );
	return pA[1];
#else
	return a.y;
#endif
}

FINLINE_Z Float VecFloatGetZ ( register const VecFloat4& a ) ///< Store a.z in CPU register
{
#if defined (_PC_SSE)
	const Float* pA = reinterpret_cast <const Float*> ( &a );
	return pA[2];
#else
	return a.z;
#endif
}

FINLINE_Z Float VecFloatGetW ( register const VecFloat4& a ) ///< Store a.w in CPU register
{
#if defined (_PC_SSE)
	const Float* pA = reinterpret_cast <const Float*> ( &a );
	return pA[3];
#else
	return a.w;
#endif
}

FINLINE_Z VecFloat4 VecFloat3x3GetRow0 ( register const VecFloat3x3& m ) ///< xyzw = m[0]
{
	return m[0];
}

FINLINE_Z VecFloat4 VecFloat3x3GetRow1 ( register const VecFloat3x3& m ) ///< xyzw = m[1]
{
	return m[1];
}

FINLINE_Z VecFloat4 VecFloat3x3GetRow2 ( register const VecFloat3x3& m ) ///< xyzw = m[2]
{
	return m[2];
}

FINLINE_Z VecFloat4 VecFloat4x4GetRow0 ( register const VecFloat4x4& m ) ///< xyzw = m[0]
{
	return m[0];
}

FINLINE_Z VecFloat4 VecFloat4x4GetRow1 ( register const VecFloat4x4& m ) ///< xyzw = m[1]
{
	return m[1];
}

FINLINE_Z VecFloat4 VecFloat4x4GetRow2 ( register const VecFloat4x4& m ) ///< xyzw = m[2]
{
	return m[2];
}

FINLINE_Z VecFloat4 VecFloat4x4GetRow3 ( register const VecFloat4x4& m ) ///< xyzw = m[3]
{
	return m[3];
}

#ifndef _ALWAYS_INLINE_BASE_FUNCTIONS_

FINLINE_Z void VecFloatStoreAligned ( void* dest, register const VecFloat4& source ) ///< dest = source, with dest a 16-byte aligned memory address
{
#if defined (_PC_SSE )
	_mm_store_ps( reinterpret_cast<float*>(dest), source );
#else
	memcpy( dest, &source, sizeof(source) );
#endif
}

#endif

FINLINE_Z void VecFloatStreamStoreAligned ( void* dest, register const VecFloat4& source ) ///< dest = source, with dest a 16-byte aligned memory address. This store doesn't pollute the cache, it is slower when dest points to data already in cache, and faster otherwise
{
#if defined (_PC_SSE )
	return _mm_stream_ps( reinterpret_cast<float*>(dest), source );
#else
	VecFloatStoreAligned( dest, source );
#endif
}

FINLINE_Z void VecFloatStoreUnaligned ( void* dest, register const VecFloat4& source ) ///< dest = source. No specific memory alignement is required for dest, but prefer an aligned store when possible.
{
#if defined (_PC_SSE )
	return _mm_storeu_ps( reinterpret_cast<float*>(dest), source );
#else
	memcpy( dest, &source, sizeof(source) );
#endif
}

// ---------------------- Select operations

FINLINE_Z VecSelMask VecFloatCompareLT ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = ( a.xyzw < b.xyzw ) ? # : 0
{
#if defined (_PC_SSE )
	return _mm_cmplt_ps( a, b );
#else
	VecSelMask result;
	for( U32 i = 0; i < 4; ++i )
		result.xyzw[i] = ( a.xyzw[i] < b.xyzw[i] ) ? 0xFFFFFFFF : 0;
	return result;
#endif
}

FINLINE_Z VecSelMask VecFloatCompareLE ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = ( a.xyzw <= b.xyzw ) ? # : 0
{
	return VecFloatCompareGE( b, a );
}

FINLINE_Z VecSelMask VecFloatCompareGT ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = ( a.xyzw > b.xyzw ) ? # : 0
{
#if defined (_PC_SSE )
	return _mm_cmpgt_ps( a, b );
#else
	VecSelMask result;
	for( U32 i = 0; i < 4; ++i )
		result.xyzw[i] = ( a.xyzw[i] > b.xyzw[i] ) ? 0xFFFFFFFF : 0;
	return result;
#endif
}

FINLINE_Z VecSelMask VecFloatCompareGE ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = ( a.xyzw >= b.xyzw ) ? # : 0
{
#if defined (_PC_SSE )
	return _mm_cmpge_ps( a, b );
#else
	VecSelMask result;
	for( U32 i = 0; i < 4; ++i )
		result.xyzw[i] = ( a.xyzw[i] >= b.xyzw[i] ) ? 0xFFFFFFFF : 0;
	return result;
#endif
}

FINLINE_Z VecSelMask VecFloatCompareEQ ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = ( a.xyzw == b.xyzw ) ? # : 0
{
#if defined (_PC_SSE )
	return _mm_cmpeq_ps( a, b );
#else
	VecSelMask result;
	for( U32 i = 0; i < 4; ++i )
		result.xyzw[i] = ( a.xyzw[i] == b.xyzw[i] ) ? 0xFFFFFFFF : 0;
	return result;
#endif
}

FINLINE_Z VecSelMask VecFloatCompareBounds ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = ( a.xyzw <= b.xyzw && a.xyzw >= -b.xyzw ) ? # : 0
{
#if defined (_PC_SSE )
	register const __m128 negB = VecFloatNegate( b );
    return _mm_and_ps( _mm_cmple_ps(a,b), _mm_cmpge_ps(a,negB) );
#else
	VecSelMask result;
	for( U32 i = 0; i < 4; ++i )
		result.xyzw[i] = ( a.xyzw[i] <= b.xyzw[i] && a.xyzw[i] >= -b.xyzw[i] ) ? 0xFFFFFFFF : 0;
	return result;
#endif
}

FINLINE_Z VecSelMask VecFloatCompareSign ( register const VecFloat4& a, register const VecFloat4& b )   ///< res.xyzw = sign(a.xyzw) == sign(b.xyzw) ? # : 0
{
#if defined( _PC_SSE4 )
	register const __m128 vSignA = _mm_blendv_ps( a, b, a );
	register const __m128 vSignB = _mm_blendv_ps( a, b, b );
	return _mm_cmpeq_ps( vSignA, vSignB );
#else
	register const VecInt4 vSignMask = VecIntLoadAligned( &VectorConstantsPrivate::vNegMask );
	register const VecInt4 vSignA = VecIntAnd( VecFloatAsInt(a), vSignMask );
	register const VecInt4 vSignB = VecIntAnd( VecFloatAsInt(b), vSignMask );
	return VecIntCompareEQ( vSignA, vSignB );
#endif
}

FINLINE_Z VecSelMask VecFloatCompareNanOrInfinite ( register const VecFloat4& a ) ///< xyzw = isNan(a.xyzw) || isInfinite(a.xyzw) ? # : 0
{
	const VecFloat4 infinite = VecFloatLoadAligned( &VectorConstantsPrivate::vInfinite );
	return VecFloatCompareEQ( infinite, VecFloatAnd(a,infinite) );
}

FINLINE_Z VecFloat4 VecFloatSelectMask ( register const VecFloat4& a, register const VecFloat4& b, register const VecSelMask& mask ) ///< for i � [0,4] : res[i] = ( mask[i] == 0 ) ? a[i] : b[i] => Not a bitwise selection!
{
#if defined (_PC_SSE )
	return _mm_sel_ps( a, b, mask );
#else
	VecFloat4 result;
	for( U32 i = 0; i < 4; ++i )
		result.xyzw[i] = ( mask.xyzw[i] == 0 ) ? a.xyzw[i] : b.xyzw[i];
	return result;
#endif
}

FINLINE_Z VecFloat4 VecFloatSelectLess ( register const VecFloat4& a, register const VecFloat4& b, register const VecFloat4& success, register const VecFloat4& fail ) ///< xyzw = ( a.xyzw < b.xyzw ) ? success.xyzw : fail.xyzw
{
	return VecFloatSelectGreaterOrEqual( a, b, fail, success );
}

FINLINE_Z VecFloat4 VecFloatSelectGreater ( register const VecFloat4& a, register const VecFloat4& b, register const VecFloat4& success, register const VecFloat4& fail ) ///< xyzw = ( a.xyzw > b.xyzw ) ? success.xyzw : fail.xyzw
{
	return VecFloatSelectLessOrEqual( a, b, fail, success );
}

FINLINE_Z VecFloat4 VecFloatSelectLessOrEqual ( register const VecFloat4& a, register const VecFloat4& b, register const VecFloat4& success, register const VecFloat4& fail ) ///< xyzw = ( a.xyzw <= b.xyzw ) ? success.xyzw : fail.xyzw
{
#if defined (_PC_SSE )
	return VecFloatSelectMask( fail, success, VecFloatCompareLE(a,b) );
#else
	VecFloat4 result;
	for( U32 i = 0; i < 4; ++i )
		result.xyzw[i] = ( a.xyzw[i] <= b.xyzw[i] ) ? success.xyzw[i] : fail.xyzw[i];
	return result;
#endif
}

FINLINE_Z VecFloat4 VecFloatSelectGreaterOrEqual ( register const VecFloat4& a, register const VecFloat4& b, register const VecFloat4& success, register const VecFloat4& fail ) ///< xyzw = ( a.xyzw >= b.xyzw ) ? success.xyzw : fail.xyzw
{
#if defined (_PC_SSE )
	return VecFloatSelectMask( fail, success, VecFloatCompareGE(a,b) );
#else
	VecFloat4 result;
	for( U32 i = 0; i < 4; ++i )
		result.xyzw[i] = ( a.xyzw[i] >= b.xyzw[i] ) ? success.xyzw[i] : fail.xyzw[i];
	return result;
#endif
}

FINLINE_Z VecFloat4 VecFloatSelectEqual ( register const VecFloat4& a, register const VecFloat4& b, register const VecFloat4& success, register const VecFloat4& fail ) ///< xyzw = ( a.xyzw == b.xyzw ) ? success.xyzw : fail.xyzw
{
#if defined (_PC_SSE )
	return VecFloatSelectMask( fail, success, VecFloatCompareEQ(a,b) );
#else
	VecFloat4 result;
	for( U32 i = 0; i < 4; ++i )
		result.xyzw[i] = ( a.xyzw[i] == b.xyzw[i] ) ? success.xyzw[i] : fail.xyzw[i];
	return result;
#endif
}

FINLINE_Z VecFloat4 VecFloatSelectNotEqual ( register const VecFloat4& a, register const VecFloat4& b, register const VecFloat4& success, register const VecFloat4& fail ) ///< xyzw = ( a.xyzw == b.xyzw ) ? success.xyzw : fail.xyzw
{
	return VecFloatSelectEqual( a, b, fail, success );
}

FINLINE_Z VecFloat4 VecFloatSelectBounds ( register const VecFloat4& a, register const VecFloat4& b, register const VecFloat4& success, register const VecFloat4& fail ) ///< xyzw = ( a.xyzw <= b.xyzw && a.xyzw >= -b.xyzw ) ? success.xyzw : fail.xyzw
{
#if defined (_PC_SSE )
	return VecFloatSelectMask( fail, success, VecFloatCompareBounds(a,b) );
#else
	VecFloat4 result;
	for( U32 i = 0; i < 4; ++i )
		result.xyzw[i] = ( a.xyzw[i] <= b.xyzw[i] && a.xyzw[i] >= -b.xyzw[i] ) ? success.xyzw[i] : fail.xyzw[i];
	return result;
#endif
}


// ---------------------- Logical operations

FINLINE_Z VecFloat4 VecFloatAnd ( register const VecFloat4& a, register const VecFloat4& b ) ///< return xyzw = a.xyzw & b.xyzw (bitwise)
{
#if defined (_PC_SSE )
	return _mm_and_ps( a, b );
#else
	Float4 result; 
	const Int4& aI = reinterpret_cast <const Int4&> ( a );
	const Int4& bI = reinterpret_cast <const Int4&> ( b );
	Int4& rI = reinterpret_cast <Int4&> ( result );
	for( U32 i = 0; i < 4; ++i )
		rI.xyzw[i] = aI.xyzw[i] & bI.xyzw[i];
	return result;
#endif
}

FINLINE_Z VecFloat4 VecFloatOr ( register const VecFloat4& a, register const VecFloat4& b ) ///< return xyzw = a.xyzw | b.xyzw (bitwise)
{
#if defined (_PC_SSE )
	return _mm_or_ps( a, b );
#else
	Float4 result; 
	const Int4& aI = reinterpret_cast <const Int4&> ( a );
	const Int4& bI = reinterpret_cast <const Int4&> ( b );
	Int4& rI = reinterpret_cast <Int4&> ( result );
	for( U32 i = 0; i < 4; ++i )
		rI.xyzw[i] = aI.xyzw[i] | bI.xyzw[i];
	return result;
#endif
}

FINLINE_Z VecFloat4 VecFloatXor ( register const VecFloat4& a, register const VecFloat4& b ) ///< return xyzw = a.xyzw ^ b.xyzw (bitwise)
{
#if defined (_PC_SSE )
	return _mm_xor_ps( a, b );
#else
	Float4 result; 
	const Int4& aI = reinterpret_cast <const Int4&> ( a );
	const Int4& bI = reinterpret_cast <const Int4&> ( b );
	Int4& rI = reinterpret_cast <Int4&> ( result );
	for( U32 i = 0; i < 4; ++i )
		rI.xyzw[i] = aI.xyzw[i] ^ bI.xyzw[i];
	return result;
#endif
}

FINLINE_Z VecFloat4 VecFloatAndNot ( register const VecFloat4& a, register const VecFloat4& b ) ///< return xyzw = a.xyzw & ~b.xyzw (bitwise)
{
#if defined (_PC_SSE )
	return _mm_andnot_ps( b, a );
#else
	Float4 result; 
	const Int4& aI = reinterpret_cast <const Int4&> ( a );
	const Int4& bI = reinterpret_cast <const Int4&> ( b );
	Int4& rI = reinterpret_cast <Int4&> ( result );
	for( U32 i = 0; i < 4; ++i )
		rI.xyzw[i] = aI.xyzw[i] & (~bI.xyzw[i]);
	return result;
#endif
}


// ---------------------- Arithmetic operations

FINLINE_Z VecFloat4 VecFloatNegate ( register const VecFloat4& a ) ///< xyzw = -xyzw
{
#if defined (_PC_SSE )
	return _mm_xor_ps( a, VecFloatLoadAligned(&VectorConstantsPrivate::vNegMask) );
#else
	VecFloat4 result;
	result.x = -a.x;
	result.y = -a.y;
	result.z = -a.z;
	result.w = -a.w;
	return result;
#endif
}

FINLINE_Z VecFloat4 VecFloatOneComplement( register const VecFloat4& a ) ///< xyzw = 1.0f - a.xyzw
{
	return VecFloatSub( VecFloatSplatOne(), a );
}

#ifndef _ALWAYS_INLINE_BASE_FUNCTIONS_

FINLINE_Z VecFloat4 VecFloatAdd ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = a.xyzw + b.xyzw
{
#if defined (_PC_SSE )
	return _mm_add_ps( a, b );
#else
	VecFloat4 result;
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	result.z = a.z + b.z;
	result.w = a.w + b.w;
	return result;
#endif
}

FINLINE_Z VecFloat4 VecFloatSub ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = a.xyzw - b.xyzw
{
#if defined (_PC_SSE )
	return _mm_sub_ps( a, b );
#else
	VecFloat4 result;
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	result.z = a.z - b.z;
	result.w = a.w - b.w;
	return result;
#endif
}

FINLINE_Z VecFloat4 VecFloatMul ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = a.xyzw * b.xyzw
{
#if defined (_PC_SSE )
	return _mm_mul_ps( a, b );
#else
	VecFloat4 result;
	result.x = a.x * b.x;
	result.y = a.y * b.y;
	result.z = a.z * b.z;
	result.w = a.w * b.w;
	return result;
#endif
}

#endif

FINLINE_Z VecFloat4 VecFloatDivEst ( register const VecFloat4& a, register const VecFloat4& b )  ///< xyzw = a.xyzw / b.xyzw (estimation)
{
	return VecFloatMul( a, VecFloatReciprocal(b) );
}

FINLINE_Z VecFloat4 VecFloatDiv ( register const VecFloat4& a, register const VecFloat4& b )  ///< xyzw = a.xyzw / b.xyzw (accurate)
{
#if defined (_PC_SSE )
	return _mm_div_ps( a, b );
#else
	VecFloat4 result;
	result.x = a.x / b.x;
	result.y = a.y / b.y;
	result.z = a.z / b.z;
	result.w = a.w / b.w;
	return result;
#endif
}

FINLINE_Z VecFloat4 VecFloatCDiv ( register const VecFloat4& a, register const VecFloat4& b, register const VecFloat4& c )  ///< xyzw = ( b.xyzw == 0.f ) ? c.xyzw : a.xyzw / b.xyzw (accurate)
{
	return VecFloatSelectNotEqual( b, VecFloatSplatZero(), VecFloatDiv(a,b), c );
}

FINLINE_Z VecFloat4 VecFloatReciprocal ( register const VecFloat4& a )  ///< xyzw = 1.f / a.xyzw (estimation)
{
#if defined (_PC_SSE )
	return _mm_rcp_ps( a );
#else
	VecFloat4 result;
	result.x = 1.f / a.x;
	result.y = 1.f / a.y;
	result.z = 1.f / a.z;
	result.w = 1.f / a.w;
	return result;
#endif
}

FINLINE_Z VecFloat4 VecFloatReciprocalAccurate ( register const VecFloat4& a, U32 refinements ) ///< result.xyzw = 1 / a.xyzw (accurate)
{
#if !defined(_PC_SSE)
	VecFloat4 aa = VecFloatSelectGreaterOrEqual( VecFloatAbs(a), VectorConstantsPrivate::vEpsF, a, VectorConstantsPrivate::vEpsF );
#else
	register const VecFloat4& aa = a;
#endif
	// Perform n refinements using Newton's method.
	register const VecFloat4 vOne = VecFloatSplatOne();
    register VecFloat4 Y0 = VecFloatReciprocal( aa );

	do {
	    // y0 = y0 + y0 * (1.0 - x * y0)   
		Y0 = VecFloatMadd( Y0, VecFloatNegMsub(a,Y0,vOne), Y0 );
		refinements--;
	} while( refinements > 0 );

	return Y0;
}

#ifndef _ALWAYS_INLINE_BASE_FUNCTIONS_

FINLINE_Z VecFloat4 VecFloatMadd ( register const VecFloat4& a, register const VecFloat4& b, register const VecFloat4& c ) ///< xyzw = a.xyzw * b.xyzw + c.xyzw
{
	return VecFloatAdd( VecFloatMul(a,b), c );
}

#endif

FINLINE_Z VecFloat4 VecFloatMsub ( register const VecFloat4& a, register const VecFloat4& b, register const VecFloat4& c ) ///< xyzw = a.xyzw * b.xyzw - c.xyzw
{
	return VecFloatSub( VecFloatMul(a,b), c );
}

FINLINE_Z VecFloat4 VecFloatNegMsub ( register const VecFloat4& a, register const VecFloat4& b, register const VecFloat4& c ) ///< xyzw = - ( a.xyzw * b.xyzw - c.xyzw )
{
	return VecFloatSub( c, VecFloatMul(a,b) );
}

FINLINE_Z VecFloat4 VecFloatHAdd ( register const VecFloat4& a, register const VecFloat4& b )  ///< xyzw = { a.x+a.y, a.z+a.w, b.x+b.y, b.z+b.w }
{
#if defined (_PC_SSE3 )
	return _mm_hadd_ps( a, b );
#else
	return VecFloatAdd( VecFloatPermuteX0Z0X1Z1(a,b), VecFloatPermuteY0W0Y1W1(a,b) );
#endif
}

FINLINE_Z VecFloat4 VecFloatHSub ( register const VecFloat4& a, register const VecFloat4& b )  ///< xyzw = { a.x-a.y, a.z-a.w, b.x-b.y, b.z-b.w }
{
#if defined (_PC_SSE3 )
	return _mm_hsub_ps( a, b );
#else
	return VecFloatSub( VecFloatPermuteX0Z0X1Z1(a,b), VecFloatPermuteY0W0Y1W1(a,b) );
#endif
}

FINLINE_Z VecFloat4 VecFloatSign ( register const VecFloat4& a )   ///< res.xyzw = a.xyzw == 0.f ? 0.f : ( a.xyzw > 0.f ? 1.f : -1.f )
{
	return VecFloatSign( a, VecFloatSplatOne() );
}

FINLINE_Z VecFloat4 VecFloatSign ( register const VecFloat4& a, register const VecFloat4& b )   ///< res.xyzw = a.xyzw == 0.f ? 0.f : ( a.xyzw > 0.f ? b.xyzw : -b.xyzw )
{
#if defined( _PC_SSE4 )
	register const VecFloat4 zero = _mm_andnot_ps( a, a );
	register const VecFloat4 signNotNull = _mm_blendv_ps( b, VecFloatNegate(b), a );
	return _mm_blendv_ps( signNotNull, zero, _mm_cmpeq_ps(a,zero) );
#else
	register const VecFloat4 zero = VecFloatSplatZero();
	register const VecFloat4 signNotNull = VecFloatSelectGreater( a, zero, b, VecFloatNegate(b) );
    return VecFloatSelectEqual( a, zero, zero, signNotNull );
#endif
}

FINLINE_Z VecFloat4 VecFloatScale ( register const VecFloat4& a, Float scalar )  ///< xyzw = a.xyzw * scalar
{
#if defined (_PC_SSE )
	return _mm_mul_ps( a, _mm_set_ps1(scalar) );
#else
	VecFloat4 result;
	result.x = a.x * scalar;
	result.y = a.y * scalar;
	result.z = a.z * scalar;
	result.w = a.w * scalar;
	return result;
#endif
}

FINLINE_Z VecFloat4 VecFloatScaleX ( register const VecFloat4& a, register const VecFloat4& b )  ///< xyzw = a.xyzw * b.x
{
	return VecFloatMul( a, VecFloatSplatX(b) );
}

FINLINE_Z VecFloat4 VecFloatScaleY ( register const VecFloat4& a, register const VecFloat4& b )  ///< xyzw = a.xyzw * b.y
{
	return VecFloatMul( a, VecFloatSplatY(b) );
}

FINLINE_Z VecFloat4 VecFloatScaleZ ( register const VecFloat4& a, register const VecFloat4& b )  ///< xyzw = a.xyzw * b.z
{
	return VecFloatMul( a, VecFloatSplatZ(b) );
}

FINLINE_Z VecFloat4 VecFloatScaleW ( register const VecFloat4& a, register const VecFloat4& b )  ///< xyzw = a.xyzw * b.w
{
	return VecFloatMul( a, VecFloatSplatW(b) );
}

FINLINE_Z VecFloat4 VecFloatAbs ( register const VecFloat4& a )  ///< xyzw = abs ( a.xyzw )
{
#if defined (_PC_SSE )
    return _mm_and_ps( a, VecFloatLoadAligned(&VectorConstantsPrivate::vAbsMask) );
#else
	VecFloat4 result;
	for( U32 i = 0; i < 4; ++i ) 
		result.xyzw[i] = Abs( a.xyzw[i] );
	return result;
#endif
}

FINLINE_Z VecFloat4 VecFloatMin ( register const VecFloat4& a, register const VecFloat4& b )  ///< xyzw = min ( a.xyzw, b.xyzw )
{
#if defined (_PC_SSE )
	return _mm_min_ps( a, b );
#else
	VecFloat4 result;
	for( U32 i = 0; i < 4; ++i ) 
		result.xyzw[i] = Min( a.xyzw[i], b.xyzw[i] );
	return result;
#endif
}

FINLINE_Z VecFloat4 VecFloatMinComponent ( register const VecFloat4& a )  ///< xyzw = min ( a.x, min( a.y, min(a.z,a.w) ) )
{
#if defined (_PC_SSE )
	VecFloat4 x = VecFloatSplatX( a );
	VecFloat4 y = VecFloatSplatY( a );
	VecFloat4 z = VecFloatSplatZ( a );
	VecFloat4 w = VecFloatSplatW( a );
	return VecFloatMin( x, VecFloatMin( y, VecFloatMin(z,w) ) );
#else
	VecFloat4 result;
	result.x = result.y = result.z = result.w = Min( a.x, Min(a.y, Min(a.z,a.w) ) );
	return result;
#endif
}

FINLINE_Z VecFloat4 VecFloatMinComponent3 ( register const VecFloat4& a )  ///< xyzw = min ( a.x, min(a.y,a.z) )
{
#if defined (_PC_SSE )
	VecFloat4 x = VecFloatSplatX( a );
	VecFloat4 y = VecFloatSplatY( a );
	VecFloat4 z = VecFloatSplatZ( a );
	return VecFloatMin( x, VecFloatMin(y,z) );
#else
	VecFloat4 result;
	result.x = result.y = result.z = result.w = Min( a.x, Min(a.y,a.z) );
	return result;
#endif
}

FINLINE_Z VecFloat4 VecFloatMax ( register const VecFloat4& a, register const VecFloat4& b )  ///< xyzw = max ( a.xyzw, b.xyzw )
{
#if defined (_PC_SSE )
	return _mm_max_ps( a, b );
#else
	VecFloat4 result;
	for( U32 i = 0; i < 4; ++i ) 
		result.xyzw[i] = Max( a.xyzw[i], b.xyzw[i] );
	return result;
#endif
}

FINLINE_Z VecFloat4 VecFloatMaxComponent ( register const VecFloat4& a )  ///< xyzw = max ( a.x, max( a.y, max(a.z,a.w) ) )
{
#if defined (_PC_SSE )
	VecFloat4 x = VecFloatSplatX( a );
	VecFloat4 y = VecFloatSplatY( a );
	VecFloat4 z = VecFloatSplatZ( a );
	VecFloat4 w = VecFloatSplatW( a );
	return VecFloatMax( x, VecFloatMax( y, VecFloatMax(z,w) ) );
#else
	VecFloat4 result;
	result.x = result.y = result.z = result.w = Max( a.x, Max(a.y, Max(a.z,a.w) ) );
	return result;
#endif
}

FINLINE_Z VecFloat4 VecFloatMaxComponent3 ( register const VecFloat4& a )  ///< xyzw = max ( a.x, max(a.y,a.z) )
{
#if defined (_PC_SSE )
	VecFloat4 x = VecFloatSplatX( a );
	VecFloat4 y = VecFloatSplatY( a );
	VecFloat4 z = VecFloatSplatZ( a );
	return VecFloatMax( x, VecFloatMax(y,z) );
#else
	VecFloat4 result;
	result.x = result.y = result.z = result.w = Max( a.x, Max(a.y,a.z) );
	return result;
#endif
}

FINLINE_Z VecFloat4 VecFloatClamp ( register const VecFloat4& a, register const VecFloat4& min, register const VecFloat4& max )  ///< xyzw = clamp ( a.xyzw, min.xyzw, max.xyzw )
{
	return VecFloatMin( max, VecFloatMax(min,a) );
}

FINLINE_Z VecFloat4 VecFloatSaturate ( register const VecFloat4& a ) ///< result.xyzw = saturate ( a.xyzw ) [0,1]
{
	return VecFloatClamp( a, VecFloatSplatZero(), VecFloatSplatOne() );
}

FINLINE_Z VecFloat4 VecFloatSaturateExtended ( register const VecFloat4& a ) ///< result.xyzw = saturate ( a.xyzw ) [-1,1]
{
	VecFloat4 one = VecFloatSplatOne();
	return VecFloatClamp( a, VecFloatNegate(one), one );
}

FINLINE_Z VecFloat4 VecFloatSqrt ( register const VecFloat4& a )  ///< xyzw = sqrt ( a.xyzw )
{
#if defined (_PC_SSE )
	return _mm_sqrt_ps( a );
#else
	VecFloat4 res;
	res.x = Sqrt(a.x);
	res.y = Sqrt(a.y);
	res.z = Sqrt(a.z);
	res.w = Sqrt(a.w);
	return res;
#endif
}

FINLINE_Z VecFloat4 VecFloatRSqrt ( register const VecFloat4& a ) ///< result.xyzw = 1 / sqrt( a.xyzw ). This estimate is very fast but has generally no more than 12 bits of precision.
{
#if defined (_PC_SSE )
	return _mm_rsqrt_ps( a );
#else
	VecFloat4 res;
	res.x = InvSqrt( 1.f, a.x );
	res.y = InvSqrt( 1.f, a.y );
	res.z = InvSqrt( 1.f, a.z );
	res.w = InvSqrt( 1.f, a.w );
	return res;
#endif
}

FINLINE_Z VecFloat4 VecFloatRSqrtAccurate ( register const VecFloat4& a, U32 refinements ) ///< result.xyzw = 1 / sqrt( a.xyzw ). Perform n refinements using Newton's method.
{
#if !defined(_PC_SSE) // No vector optimization: avoid division by 0, no need to refine  
	VecFloat4 aa = VecFloatSelectGreaterOrEqual( VecFloatAbs(a), VectorConstantsPrivate::vEpsSquareF, a, VectorConstantsPrivate::vEpsSquareF );
	return VecFloatRSqrt( aa ); 
#else
	register VecFloat4 Y0 = VecFloatRSqrt( a );

	if( refinements > 0 )
	{
		register VecFloat4 half, halfA, Y0Y0, refine, threeHalf;

		refine = VectorConstantsPrivate::vHalfAndThreeHalf;
		half = VecFloatSplatX( refine );
		threeHalf = VecFloatSplatZ( refine );
		halfA = VecFloatMul( a, half );
		do {
			// y0 = y0 * ( (3-a*y0*y0) / 2 ) = y0 * ( 1.5f-(a/2)*y0*y0 )
			Y0Y0 = VecFloatMul( Y0, Y0 );
			refine = VecFloatNegMsub( halfA, Y0Y0, threeHalf );
			Y0 = VecFloatMul( refine, Y0 );
			refinements--;
		} while( refinements );
	}

	return Y0;
#endif
}

FINLINE_Z VecFloat4 VecFloatCos ( register const VecFloat4& a ) ///< result.xyzw = cos( a.xyzw )
{
	VecFloat4 V1, V2, V4, V6, V8, V10, V12, V14, V16, V18, V20, V22;
    VecFloat4 C1, C2, C3, C4, C5, C6, C7, C8, C9, C10, C11;
    VecFloat4 Result;

	register const VecFloat4 cosCoefficients0 = VectorConstantsPrivate::vCosCoefficients0;
	register const VecFloat4 cosCoefficients1 = VectorConstantsPrivate::vCosCoefficients1;
	register const VecFloat4 cosCoefficients2 = VectorConstantsPrivate::vCosCoefficients2;

    V1 = VecFloatModAngles( a );

    // cos(V) ~= 1 - V^2 / 2! + V^4 / 4! - V^6 / 6! + V^8 / 8! - V^10 / 10! + V^12 / 12! - 
    //           V^14 / 14! + V^16 / 16! - V^18 / 18! + V^20 / 20! - V^22 / 22! (for -π <= a < π)
    V2 = VecFloatMul( V1, V1 );
    V4 = VecFloatMul( V2, V2 );
    V6 = VecFloatMul( V4, V2 );
    V8 = VecFloatMul( V4, V4 );
    V10 = VecFloatMul( V6, V4 );
    V12 = VecFloatMul( V6, V6 );
    V14 = VecFloatMul( V8, V6 );
    V16 = VecFloatMul( V8, V8 );
    V18 = VecFloatMul( V10, V8 );
    V20 = VecFloatMul( V10, V10 );
    V22 = VecFloatMul( V12, V10 );

    C1 = VecFloatSplatY( cosCoefficients0 );
    C2 = VecFloatSplatZ( cosCoefficients0 );
    C3 = VecFloatSplatW( cosCoefficients0 );
    C4 = VecFloatSplatX( cosCoefficients1 );
    C5 = VecFloatSplatY( cosCoefficients1 );
    C6 = VecFloatSplatZ( cosCoefficients1 );
    C7 = VecFloatSplatW( cosCoefficients1 );
    C8 = VecFloatSplatX( cosCoefficients2 );
    C9 = VecFloatSplatY( cosCoefficients2 );
    C10 = VecFloatSplatZ( cosCoefficients2 );
    C11 = VecFloatSplatW( cosCoefficients2 );

    Result = VecFloatMadd( C1, V2, VectorConstantsPrivate::vOneF );
    Result = VecFloatMadd( C2, V4, Result );
    Result = VecFloatMadd( C3, V6, Result );
    Result = VecFloatMadd( C4, V8, Result );
    Result = VecFloatMadd( C5, V10, Result );
    Result = VecFloatMadd( C6, V12, Result );
    Result = VecFloatMadd( C7, V14, Result );
    Result = VecFloatMadd( C8, V16, Result );
    Result = VecFloatMadd( C9, V18, Result );
    Result = VecFloatMadd( C10, V20, Result );
    return VecFloatMadd( C11, V22, Result );
}

FINLINE_Z VecFloat4 VecFloatSin ( register const VecFloat4& a ) ///< result.xyzw = sin( a.xyzw )
{
	VecFloat4 V1, V2, V3, V5, V7, V9, V11, V13, V15, V17, V19, V21, V23;
    VecFloat4 S1, S2, S3, S4, S5, S6, S7, S8, S9, S10, S11;
    VecFloat4 Result;

	register const VecFloat4 sinCoefficients0 = VectorConstantsPrivate::vSinCoefficients0;
	register const VecFloat4 sinCoefficients1 = VectorConstantsPrivate::vSinCoefficients1;
	register const VecFloat4 sinCoefficients2 = VectorConstantsPrivate::vSinCoefficients2;

    V1 = VecFloatModAngles( a );

    /* sin(a) ~= a - a^3 / 3! + a^5 / 5! - a^7 / 7! + a^9 / 9! - a^11 / 11! + a^13 / 13! - 
               a^15 / 15! + a^17 / 17! - a^19 / 19! + a^21 / 21! - a^23 / 23! (for -π <= a < π) */
    V2  = VecFloatMul( V1, V1 );
    V3  = VecFloatMul( V2, V1 );
    V5  = VecFloatMul( V3, V2 );
    V7  = VecFloatMul( V5, V2 );
    V9  = VecFloatMul( V7, V2 );
    V11 = VecFloatMul( V9, V2 );
    V13 = VecFloatMul( V11, V2 );
    V15 = VecFloatMul( V13, V2 );
    V17 = VecFloatMul( V15, V2 );
    V19 = VecFloatMul( V17, V2 );
    V21 = VecFloatMul( V19, V2 );
    V23 = VecFloatMul( V21, V2 );

    S1  = VecFloatSplatY( sinCoefficients0 );
    S2  = VecFloatSplatZ( sinCoefficients0 );
    S3  = VecFloatSplatW( sinCoefficients0 );
    S4  = VecFloatSplatX( sinCoefficients1 );
    S5  = VecFloatSplatY( sinCoefficients1 );
    S6  = VecFloatSplatZ( sinCoefficients1 );
    S7  = VecFloatSplatW( sinCoefficients1 );
    S8  = VecFloatSplatX( sinCoefficients2 );
    S9  = VecFloatSplatY( sinCoefficients2 );
    S10 = VecFloatSplatZ( sinCoefficients2 );
    S11 = VecFloatSplatW( sinCoefficients2 );

    Result = VecFloatMadd( S1, V3, V1 );
    Result = VecFloatMadd( S2, V5, Result );
    Result = VecFloatMadd( S3, V7, Result );
    Result = VecFloatMadd( S4, V9, Result );
    Result = VecFloatMadd( S5, V11, Result );
    Result = VecFloatMadd( S6, V13, Result );
    Result = VecFloatMadd( S7, V15, Result );
    Result = VecFloatMadd( S8, V17, Result );
    Result = VecFloatMadd( S9, V19, Result );
    Result = VecFloatMadd( S10, V21, Result );
    return VecFloatMadd( S11, V23, Result );
}

FINLINE_Z VecFloat4 VecFloatTan ( register const VecFloat4& a ) ///< result.xyzw = tan( a.xyzw )
{
	return VecFloatLoad4( tanf(VecFloatGetX(a)), tanf(VecFloatGetY(a)), tanf(VecFloatGetZ(a)), tanf(VecFloatGetW(a)) );
}

FINLINE_Z VecFloat4 VecFloatACos ( register const VecFloat4& a ) ///< result.xyzw = acos( a.xyzw ) with -1 <= a.xyzw <= 1
{
	// Code from DirectX::XMScalarACos
	const VecFloat4 zero = VecFloatSplatZero();

	// Clamp input to [-1,1].
    VecFloat4 x = VecFloatAbs( a );
    VecFloat4 omx = VecFloatSub( VecFloatSplatOne(), x );
    omx = VecFloatSelectLess( omx, zero, zero, omx );
    VecFloat4 root = VecFloatSqrt( omx );

    // 7-degree minimax approximation
	VecFloat4 result = VecFloatMadd( VecFloatSplatX(VectorConstantsPrivate::vAcosCoefficients0), x, VecFloatSplatY(VectorConstantsPrivate::vAcosCoefficients0) );
	result = VecFloatMadd( result, x, VecFloatSplatZ(VectorConstantsPrivate::vAcosCoefficients0) );
	result = VecFloatMadd( result, x, VecFloatSplatW(VectorConstantsPrivate::vAcosCoefficients0) );
	result = VecFloatMadd( result, x, VecFloatSplatX(VectorConstantsPrivate::vAcosCoefficients1) );
	result = VecFloatMadd( result, x, VecFloatSplatY(VectorConstantsPrivate::vAcosCoefficients1) );
	result = VecFloatMadd( result, x, VecFloatSplatZ(VectorConstantsPrivate::vAcosCoefficients1) );
    result = VecFloatMadd( result, x, VecFloatSplatW(VectorConstantsPrivate::vAcosCoefficients1) );
    result = VecFloatMul( result, root );

    // acos(x) = pi - acos(-x) when x < 0
    return VecFloatSelectGreaterOrEqual( a, zero, result, VecFloatSub(VectorConstantsPrivate::vPi,result) );
}

FINLINE_Z VecFloat4 VecFloatASin ( register const VecFloat4& a ) ///< result.xyzw = asin( a.xyzw ) with -1 <= a.xyzw <= 1
{
	// Code from DirectX::XMScalarASin
	const VecFloat4 zero = VecFloatSplatZero();

	// Clamp input to [-1,1].
    VecFloat4 x = VecFloatAbs( a );
    VecFloat4 omx = VecFloatSub( VecFloatSplatOne(), x );
    omx = VecFloatSelectLess( omx, zero, zero, omx );
    VecFloat4 root = VecFloatSqrt( omx );

    // 7-degree minimax approximation
    VecFloat4 result = VecFloatMadd( VecFloatSplatX(VectorConstantsPrivate::vAcosCoefficients0), x, VecFloatSplatY(VectorConstantsPrivate::vAcosCoefficients0) );
	result = VecFloatMadd( result, x, VecFloatSplatZ(VectorConstantsPrivate::vAcosCoefficients0) );
	result = VecFloatMadd( result, x, VecFloatSplatW(VectorConstantsPrivate::vAcosCoefficients0) );
	result = VecFloatMadd( result, x, VecFloatSplatX(VectorConstantsPrivate::vAcosCoefficients1) );
	result = VecFloatMadd( result, x, VecFloatSplatY(VectorConstantsPrivate::vAcosCoefficients1) );
	result = VecFloatMadd( result, x, VecFloatSplatZ(VectorConstantsPrivate::vAcosCoefficients1) );
    result = VecFloatMadd( result, x, VecFloatSplatW(VectorConstantsPrivate::vAcosCoefficients1) );
	result = VecFloatMul( result, root );  // acos(|x|)

    // acos(x) = pi - acos(-x) when x < 0, asin(x) = pi/2 - acos(x)
    return VecFloatSelectGreaterOrEqual( a, zero, VecFloatSub(VectorConstantsPrivate::vPiDiv2,result), VecFloatSub(result,VectorConstantsPrivate::vPiDiv2) );
}

FINLINE_Z VecFloat4 VecFloatSinCos1 ( const Float a ) ///< xyzw = { sin(a), cos(a), undefined, undefined }
{
	return VecFloatLoad4( Sin(a), Cos(a), a, a );
}

FINLINE_Z void VecFloatSinCos ( register VecFloat4& s, register VecFloat4& c, register const VecFloat4& a ) ///< s.xyzw = sin( a.xyzw ), c = cos( a.xyzw ) with -1 <= a.xyzw <= 1
{
	s = VecFloatSin( a );
	c = VecFloatCos( a );
}

FINLINE_Z VecFloat4 VecFloatExp ( register const VecFloat4& a ) ///< result.xyzw = exp( a.xyzw )
{
	return VecFloatLoad4( expf(VecFloatGetX(a)), expf(VecFloatGetY(a)), expf(VecFloatGetZ(a)), expf(VecFloatGetW(a)) );
}

FINLINE_Z VecFloat4 VecFloatExp2Est ( register const VecFloat4& a ) ///< result.xyzw = exp2( a.xyzw ) (estimation)
{
#if defined (_PC_SSE2 )
	// Code adapted from http://jrfonseca.blogspot.fr/2008/09/fast-sse2-pow-tables-or-polynomials.html
	const U32 ExpPolyDegree = 2;

	VecInt4 ipart;
	VecFloat4 x, fpart, expipart, expfpart, vCoefficients[ExpPolyDegree];

	register const VecFloat4 vConstants0 = VecFloatLoad4( 129.0f, -126.99999f, 0.5f, 8.9893397e-3f );
	register const VecFloat4 vConstants1 = VectorConstantsPrivate::vExp2ConstantsDegree2;
	register const VecFloat4 vMax = VecFloatSplatX( vConstants0 );
	register const VecFloat4 vMin = VecFloatSplatY( vConstants0 );
	vCoefficients[0] = VecFloatSplatX( vConstants1 );
	vCoefficients[1] = VecFloatSplatY( vConstants1 );

	x = VecFloatClamp( a, vMin, vMax );

	/* ipart = floor( x ) */
	ipart = VecFloatToIntFloor( x );
	/* fpart = x - ipart */
	fpart = VecFloatSub( x, VecIntToFloat(ipart) );
	/* expipart = (float) (1 << ipart) */
	VecInt4 exp2i = VecIntAdd( ipart, VecIntLoad1(127) );
	exp2i = VecIntShiftLeft ( exp2i, 23 );
	expipart = VecIntAsFloat( exp2i );
	/* minimax polynomial fit of 2**x, in range [-0.5, 0.5[ */
	//expfpart = POLY2(fpart, 1.0017247f, 6.5763628e-1f, 3.3718944e-1f);
	expfpart = VecFloatSplatZ( vConstants1 );
	for( U32 i = 0; i < ExpPolyDegree; ++i )
	   expfpart = VecFloatMadd( expfpart, fpart, vCoefficients[i] );
	
	return VecFloatMul( expipart, expfpart );
#else
	return VecFloatExp2( a );
#endif
}

FINLINE_Z VecFloat4 VecFloatExp2 ( register const VecFloat4& a ) ///< result.xyzw = exp2( a.xyzw ) (accurate)
{
#if defined (_PC_SSE2 )
    // Code adapted from http://jrfonseca.blogspot.fr/2008/09/fast-sse2-pow-tables-or-polynomials.html
	const U32 ExpPolyDegree = 5;

	VecInt4 ipart;
	VecFloat4 x, fpart, expipart, expfpart, vCoefficients[ExpPolyDegree];

	register const VecFloat4 vConstants0 = VecFloatLoad4( 129.0f, -126.99999f, 1.8775767e-3f, 8.9893397e-3f );
	register const VecFloat4 vConstants1 = VecFloatLoad4( 5.5826318e-2f, 2.4015361e-1f, 6.9315308e-1f, 9.9999994e-1f );
	register const VecFloat4 vMax = VecFloatSplatX( vConstants0 );
	register const VecFloat4 vMin = VecFloatSplatY( vConstants0 );
	vCoefficients[0] = VecFloatSplatW( vConstants0 );
	vCoefficients[1] = VecFloatSplatX( vConstants1 );
	vCoefficients[2] = VecFloatSplatY( vConstants1 );
	vCoefficients[3] = VecFloatSplatZ( vConstants1 );
	vCoefficients[4] = VecFloatSplatW( vConstants1 );
	
	x = VecFloatClamp( a, vMin, vMax );
	
	/* ipart = floor( x ) */
	ipart = VecFloatToIntFloor( x );
	/* fpart = x - ipart */
	fpart = VecFloatSub(x, VecIntToFloat(ipart));
	/* expipart = (float) (1 << ipart) */
	VecInt4 exp2i = VecIntAdd( ipart, VecIntLoad1(127) );
	exp2i = VecIntShiftLeft ( exp2i, 23 );
	expipart = VecIntAsFloat( exp2i );
	/* minimax polynomial fit of 2**x, in range [-0.5, 0.5[ 
	expfpart = POLY5( fpart, 9.9999994e-1f, 6.9315308e-1f, 2.4015361e-1f, 5.5826318e-2f, 8.9893397e-3f, 1.8775767e-3f); */
	expfpart = VecFloatSplatZ( vConstants0 );
	for( U32 i = 0; i < ExpPolyDegree; ++i )
	   expfpart = VecFloatMadd( expfpart, fpart, vCoefficients[i] );
	
	return VecFloatMul( expipart, expfpart );
#else
	return VecFloatLoad4( powf(2.0f,VecFloatGetX(a)), powf(2.0f,VecFloatGetY(a)), powf(2.0f,VecFloatGetZ(a)), powf(2.0f,VecFloatGetW(a)) );
#endif
}

FINLINE_Z VecFloat4 VecFloatLog ( register const VecFloat4& a ) ///< result.xyzw = log( a.xyzw )
{
	return VecFloatLoad4( logf(VecFloatGetX(a)), logf(VecFloatGetY(a)), logf(VecFloatGetZ(a)), logf(VecFloatGetW(a)) );
}

FINLINE_Z VecFloat4 VecFloatLog2Est ( register const VecFloat4& a ) ///< result.xyzw = log2( a.xyzw ) (estimation)
{
#if defined (_PC_SSE2 )
   // Code adapted from http://jrfonseca.blogspot.fr/2008/09/fast-sse2-pow-tables-or-polynomials.html
	const U32 LogPolyDegree = 3;

	VecFloat4 e, m, p, vCoefficients[LogPolyDegree-1];
	VecInt4 eI;

	register const VecInt4 vConstantsI = VecIntLoad4( 0x7F800000, 0x007FFFFF, 127, 0 );
	register const VecFloat4 vConstantsF = VectorConstantsPrivate::vLog2ConstantsDegree3;
	register const VecInt4 exp = VecIntSplatX( vConstantsI );
	register const VecInt4 mantissa = VecIntSplatY( vConstantsI );
	register const VecInt4 v127 = VecIntSplatZ( vConstantsI );
	register const VecFloat4 one = VecFloatSplatW( vConstantsF );
	register const VecInt4 i = VecFloatAsInt( a );

	vCoefficients[0] = VecFloatSplatX( vConstantsF );
	vCoefficients[1] = VecFloatSplatY( vConstantsF );

	eI = VecIntAnd( i, exp );
	eI = VecIntShiftRightPositive ( eI, 23 );
	eI = VecIntSub( eI, v127 );
	e = VecIntToFloat( eI );

	eI = VecIntAnd( i, mantissa );
	m = VecFloatOr( VecIntAsFloat(eI), one );

	/* Minimax polynomial fit of log2(x)/(x - 1), for x in range [1, 2[ 
	p = POLY2(m, 2.28330284476918490682f, -1.04913055217340124191f, 0.204446009836232697516f); */
	p = VecFloatSplatZ( vConstantsF );
	for( U32 degree = 0; degree < (LogPolyDegree-1); ++degree )
		p = VecFloatMadd( p, m, vCoefficients[degree] );

	/* This effectively increases the polynomial degree by one, but ensures that log2(1) == 0*/
	return VecFloatMadd( p, VecFloatSub(m,one), e );
#else
	return VecFloatLog2( a );
#endif
}

FINLINE_Z VecFloat4 VecFloatLog2 ( register const VecFloat4& a ) ///< result.xyzw = log2( a.xyzw ) (accurate)
{
#if defined (_PC_SSE2 )
	// Code adapted from http://jrfonseca.blogspot.fr/2008/09/fast-sse2-pow-tables-or-polynomials.html
	const U32 LogPolyDegree = 5;

	VecFloat4 e, m, p, vCoefficients[LogPolyDegree-1];
	VecInt4 eI;

	register const VecInt4 vConstantsI = VecIntLoad4( 0x7F800000, 0x007FFFFF, 127, 0 );
	register const VecFloat4 vConstantsF = VecFloatLoad4( -0.465725644288844778798f, 1.48116647521213171641f, -2.52074962577807006663f, 2.8882704548164776201f );
	register const VecInt4 exp = VecIntSplatX( vConstantsI );
	register const VecInt4 mantissa = VecIntSplatY( vConstantsI );
	register const VecInt4 v127 = VecIntSplatZ( vConstantsI );
	register const VecFloat4 one = VecFloatSplatOne();
	register const VecInt4 i = VecFloatAsInt( a );

	vCoefficients[0] = VecFloatSplatX( vConstantsF );
	vCoefficients[1] = VecFloatSplatY( vConstantsF );
	vCoefficients[2] = VecFloatSplatZ( vConstantsF );
	vCoefficients[3] = VecFloatSplatW( vConstantsF );

	eI = VecIntAnd( i, exp );
	eI = VecIntShiftRightPositive ( eI, 23 );
	eI = VecIntSub( eI, v127 );
	e = VecIntToFloat( eI );

	eI = VecIntAnd( i, mantissa );
	m = VecFloatOr( VecIntAsFloat(eI), one );

	/* Minimax polynomial fit of log2(x)/(x - 1), for x in range [1, 2[
	p = POLY4(m, 2.8882704548164776201f, -2.52074962577807006663f, 1.48116647521213171641f, -0.465725644288844778798f, 0.0596515482674574969533f); */
	p = VecFloatLoad1( 0.0596515482674574969533f );
	for( U32 degree = 0; degree < (LogPolyDegree-1); ++degree )
		p = VecFloatMadd( p, m, vCoefficients[degree] );

	/* This effectively increases the polynomial degree by one, but ensures that log2(1) == 0*/
	return VecFloatMadd( p, VecFloatSub(m,one), e );
#else
	return VecFloatScale( VecFloatLog(a), 1.0f / logf(2.0f) );
#endif
}

FINLINE_Z VecFloat4 VecFloatLog10 ( register const VecFloat4& a ) ///< result.xyzw = log10( a.xyzw )
{
	return VecFloatLoad4( log10f(VecFloatGetX(a)), log10f(VecFloatGetY(a)), log10f(VecFloatGetZ(a)), log10f(VecFloatGetW(a)) );
}

FINLINE_Z VecFloat4 VecFloatPowEst ( register const VecFloat4& a, register const VecFloat4& power ) ///< xyzw = pow( a.xyzw, power.xyzw ). This is an estimate function, can be quite inaccurate!
{
#if defined (_PC_SSE2 )
	register const VecFloat4 log2a = VecFloatLog2( a );
	return VecFloatExp2( VecFloatMul(log2a,power) );
#else
	Float x = powf( VecFloatGetX(a), VecFloatGetX(power) );
	Float y = powf( VecFloatGetY(a), VecFloatGetY(power) );
	Float z = powf( VecFloatGetZ(a), VecFloatGetZ(power) );
	Float w = powf( VecFloatGetW(a), VecFloatGetW(power) );
	return VecFloatLoad4( x, y, z, w );
#endif
}

FINLINE_Z VecFloat4 VecFloatPowEstSafe ( register const VecFloat4& a, register const VecFloat4& power ) ///< xyzw = (a.xyzw == 0 ? 0.0 : pow( a.xyzw, power.xyzw ))
{
	const VecFloat4 zero = VecFloatSplatZero();
	return VecFloatSelectEqual( a, zero, zero, VecFloatPowEst(a,power) );
}


// ---------------------- Swizzle operations

FINLINE_Z VecFloat4 VecFloatSplatX ( register const VecFloat4& a )  ///< xyzw = a.xxxx
{
#if defined (_PC_AVX )
	return _mm_permute_ps( a, 0 );
#elif defined (_PC_SSE )
	return _mm_shuffle_ps( a, a, 0 );
#else
	VecFloat4 res;
	res.x = a.x;
	res.y = a.x;
	res.z = a.x;
	res.w = a.x;
	return res;
#endif
}

FINLINE_Z VecFloat4 VecFloatSplatY ( register const VecFloat4& a )  ///< xyzw = a.yyyy
{
#if defined (_PC_AVX )
	return _mm_permute_ps( a, _MM_SHUFFLE(1,1,1,1) );
#elif defined (_PC_SSE )
	return _mm_shuffle_ps( a, a, _MM_SHUFFLE(1,1,1,1) );
#else
	VecFloat4 res;
	res.x = a.y;
	res.y = a.y;
	res.z = a.y;
	res.w = a.y;
	return res;
#endif
}

FINLINE_Z VecFloat4 VecFloatSplatZ ( register const VecFloat4& a )  ///< xyzw = a.zzzz
{
#if defined (_PC_AVX )
	return _mm_permute_ps( a, _MM_SHUFFLE(2,2,2,2) );
#elif defined (_PC_SSE )
	return _mm_shuffle_ps( a, a, _MM_SHUFFLE(2,2,2,2) );
#else
	VecFloat4 res;
	res.x = a.z;
	res.y = a.z;
	res.z = a.z;
	res.w = a.z;
	return res;
#endif
}

FINLINE_Z VecFloat4 VecFloatSplatW ( register const VecFloat4& a )  ///< xyzw = a.wwww
{
#if defined (_PC_AVX )
	return _mm_permute_ps( a, _MM_SHUFFLE(3,3,3,3) );
#elif defined (_PC_SSE )
	return _mm_shuffle_ps( a, a, _MM_SHUFFLE(3,3,3,3) );
#else
	VecFloat4 res;
	res.x = a.w;
	res.y = a.w;
	res.z = a.w;
	res.w = a.w;
	return res;
#endif
}

template <U32 ElementCount>
FINLINE_Z VecFloat4 VecFloatRotateLeft ( register const VecFloat4& a ) ///< xyzw = a.xyzw <<< ElementCount
{
#if defined( _PC_SSE )
	const S32 x = ElementCount & 3;
    const S32 y = (ElementCount+1) & 3;
    const S32 z = (ElementCount+2) & 3;
	const S32 w = (ElementCount+3) & 3;
	return _mm_shuffle_ps( a, a, _MM_SHUFFLE(w,z,y,x) );
#else
	const S32 x = ElementCount & 3;
    const S32 y = (ElementCount+1) & 3;
    const S32 z = (ElementCount+2) & 3;
	const S32 w = (ElementCount+3) & 3;
    return VecFloatLoad4( a.xyzw[x], a.xyzw[y], a.xyzw[z], a.xyzw[w] );
#endif
}


/////////////////////////// Don't use these fonctions directly !

template <VectorSwizzle x, VectorSwizzle y, VectorSwizzle z, VectorSwizzle w>
FINLINE_Z VecFloat4 _VecFloatSwizzle_ ( register const VecFloat4& a )   ///< Don't use this fonction directly !
{
#if defined (_PC_AVX )
	return _mm_permute_ps( a, _MM_SHUFFLE(w,z,y,x) );
#elif defined (_PC_SSE )
	return _mm_shuffle_ps( a, a, _MM_SHUFFLE(w,z,y,x) );
#else
	VecFloat4 res;
	res.x = a.xyzw[x];
	res.y = a.xyzw[y];
	res.z = a.xyzw[z];
	res.w = a.xyzw[w];
	return res;
#endif
}

///////////////////////////


template <VectorSwizzle SrcPos, VectorSwizzle DestPos> FINLINE_Z VecFloat4 VecFloatZeroInsert ( register const VecFloat4& a )   ///< Insert a value from a into a null vector. Say SrcPos=W, DestPos=Y: xyzw = { 0, a.w, 0, 0 }
{
#if defined( _PC_SSE4 )
	const U32 zMask = 15 - ( 1 << DestPos );
	const U32 mask = (SrcPos<<6) | (DestPos<<4) | zMask;
	return _mm_insert_ps( a, a, mask ); // a.w(11) -> r.y(01), only y pass (1101)
#elif defined (_PC_SSE )
	__m128 result = _mm_shuffle_ps( VecFloatSplatZero(), a, SrcPos<<6 ); // result = { 0, 0, a.x, a[SrcPos] }
	return _mm_shuffle_ps( result, result, 3<<(DestPos*2) );
#else
	VecFloat4 result = VecFloatSplatZero();
	result.xyzw[DestPos] = a.xyzw[SrcPos];
	return result;
#endif
}

FINLINE_Z VecFloat4 VecFloatSwizzleXXXY ( register const VecFloat4& a )   ///< xyzw = a.xxxy
{
    return _VecFloatSwizzle_ <VSWIZZLE_X, VSWIZZLE_X, VSWIZZLE_X, VSWIZZLE_Y> ( a );
}

FINLINE_Z VecFloat4 VecFloatSwizzleXXYY ( register const VecFloat4& a )   ///< xyzw = a.xxyy
{  
#if defined (_PC_SSE )
	return _mm_unpacklo_ps( a, a );
#else
    return _VecFloatSwizzle_ <VSWIZZLE_X, VSWIZZLE_X, VSWIZZLE_Y, VSWIZZLE_Y> ( a );
#endif
}

FINLINE_Z VecFloat4 VecFloatSwizzleXXZZ ( register const VecFloat4& a )   ///< xyzw = a.xxzz
{
#if defined( _PC_SSE3 )
	return _mm_moveldup_ps( a );
#else
    return _VecFloatSwizzle_ <VSWIZZLE_X, VSWIZZLE_X, VSWIZZLE_Z, VSWIZZLE_Z> ( a );
#endif
}

FINLINE_Z VecFloat4 VecFloatSwizzleXYZX ( register const VecFloat4& a )   ///< xyzw = a.xyzx
{
    return _VecFloatSwizzle_ <VSWIZZLE_X, VSWIZZLE_Y, VSWIZZLE_Z, VSWIZZLE_X> ( a );
}

FINLINE_Z VecFloat4 VecFloatSwizzleXZXZ ( register const VecFloat4& a )   ///< xyzw = a.xzxz
{
    return _VecFloatSwizzle_ <VSWIZZLE_X, VSWIZZLE_Z, VSWIZZLE_X, VSWIZZLE_Z> ( a );
}

FINLINE_Z VecFloat4 VecFloatSwizzleXWXX ( register const VecFloat4& a )   ///< xyzw = a.xwxx
{
    return _VecFloatSwizzle_ <VSWIZZLE_X, VSWIZZLE_W, VSWIZZLE_X, VSWIZZLE_X> ( a );
}

FINLINE_Z VecFloat4 VecFloatSwizzleYXXX ( register const VecFloat4& a )   ///< xyzw = a.yxxx
{
    return _VecFloatSwizzle_ <VSWIZZLE_Y, VSWIZZLE_X, VSWIZZLE_X, VSWIZZLE_X> ( a );
}

FINLINE_Z VecFloat4 VecFloatSwizzleYXWZ ( register const VecFloat4& a )   ///< xyzw = a.yxwz
{
    return _VecFloatSwizzle_ <VSWIZZLE_Y, VSWIZZLE_X, VSWIZZLE_W, VSWIZZLE_Z> ( a );
}

FINLINE_Z VecFloat4 VecFloatSwizzleYYWW ( register const VecFloat4& a )   ///< xyzw = a.yyww
{
#if defined( _PC_SSE3 )
	return _mm_movehdup_ps( a );
#else
    return _VecFloatSwizzle_ <VSWIZZLE_Y, VSWIZZLE_Y, VSWIZZLE_W, VSWIZZLE_W> ( a );
#endif
}

FINLINE_Z VecFloat4 VecFloatSwizzleYZXY ( register const VecFloat4& a )   ///< xyzw = a.yzxy
{
    return _VecFloatSwizzle_ <VSWIZZLE_Y, VSWIZZLE_Z, VSWIZZLE_X, VSWIZZLE_Y> ( a );
}

FINLINE_Z VecFloat4 VecFloatSwizzleYZXW ( register const VecFloat4& a )   ///< xyzw = a.yzxw
{
    return _VecFloatSwizzle_ <VSWIZZLE_Y, VSWIZZLE_Z, VSWIZZLE_X, VSWIZZLE_W> ( a );
}

FINLINE_Z VecFloat4 VecFloatSwizzleYZWX ( register const VecFloat4& a )   ///< xyzw = a.yzwx
{
    return _VecFloatSwizzle_ <VSWIZZLE_Y, VSWIZZLE_Z, VSWIZZLE_W, VSWIZZLE_X> ( a );
}

FINLINE_Z VecFloat4 VecFloatSwizzleYWXZ ( register const VecFloat4& a )   ///< xyzw = a.ywxz
{
    return _VecFloatSwizzle_ <VSWIZZLE_Y, VSWIZZLE_W, VSWIZZLE_X, VSWIZZLE_Z> ( a );
}

FINLINE_Z VecFloat4 VecFloatSwizzleYWYW ( register const VecFloat4& a )   ///< xyzw = a.ywyw
{
    return _VecFloatSwizzle_ <VSWIZZLE_Y, VSWIZZLE_W, VSWIZZLE_Y, VSWIZZLE_W> ( a );
}

FINLINE_Z VecFloat4 VecFloatSwizzleZXYX ( register const VecFloat4& a )   ///< xyzw = a.zxyx
{
    return _VecFloatSwizzle_ <VSWIZZLE_Z, VSWIZZLE_X, VSWIZZLE_Y, VSWIZZLE_X> ( a );
}

FINLINE_Z VecFloat4 VecFloatSwizzleZXYW ( register const VecFloat4& a )   ///< xyzw = a.zxyw
{
    return _VecFloatSwizzle_ <VSWIZZLE_Z, VSWIZZLE_X, VSWIZZLE_Y, VSWIZZLE_W> ( a );
}

FINLINE_Z VecFloat4 VecFloatSwizzleZYXX ( register const VecFloat4& a )   ///< xyzw = a.zyxx
{
    return _VecFloatSwizzle_ <VSWIZZLE_Z, VSWIZZLE_Y, VSWIZZLE_X, VSWIZZLE_X> ( a );
}

FINLINE_Z VecFloat4 VecFloatSwizzleZZYY ( register const VecFloat4& a )   ///< xyzw = a.zzyy
{
    return _VecFloatSwizzle_ <VSWIZZLE_Z, VSWIZZLE_Z, VSWIZZLE_Y, VSWIZZLE_Y> ( a );
}

FINLINE_Z VecFloat4 VecFloatSwizzleZWXX ( register const VecFloat4& a )   ///< xyzw = a.zwxx
{
    return _VecFloatSwizzle_ <VSWIZZLE_Z, VSWIZZLE_W, VSWIZZLE_X, VSWIZZLE_X> ( a );
}

FINLINE_Z VecFloat4 VecFloatSwizzleZWYZ ( register const VecFloat4& a )   ///< xyzw = a.zwyz
{
    return _VecFloatSwizzle_ <VSWIZZLE_Z, VSWIZZLE_W, VSWIZZLE_Y, VSWIZZLE_Z> ( a );
}

FINLINE_Z VecFloat4 VecFloatSwizzleZWZW ( register const VecFloat4& a )   ///< xyzw = a.zwzw
{
#if defined (_PC_SSE )
	return _mm_movehl_ps( a, a );
#else
    return _VecFloatSwizzle_ <VSWIZZLE_Z, VSWIZZLE_W, VSWIZZLE_Z, VSWIZZLE_W> ( a );
#endif
}

FINLINE_Z VecFloat4 VecFloatSwizzleZWWW ( register const VecFloat4& a )   ///< xyzw = a.zwww
{
    return _VecFloatSwizzle_ <VSWIZZLE_Z, VSWIZZLE_W, VSWIZZLE_W, VSWIZZLE_W> ( a );
}

FINLINE_Z VecFloat4 VecFloatSwizzleWZYX ( register const VecFloat4& a )   ///< xyzw = a.wzyx
{
    return _VecFloatSwizzle_ <VSWIZZLE_W, VSWIZZLE_Z, VSWIZZLE_Y, VSWIZZLE_X> ( a );
}

FINLINE_Z VecFloat4 VecFloatSwizzleWZWY ( register const VecFloat4& a )   ///< xyzw = a.wzwy
{
    return _VecFloatSwizzle_ <VSWIZZLE_W, VSWIZZLE_Z, VSWIZZLE_W, VSWIZZLE_Y> ( a );
}

FINLINE_Z VecFloat4 VecFloatSwizzleWXWX ( register const VecFloat4& a )   ///< xyzw = a.wxwx
{
    return _VecFloatSwizzle_ <VSWIZZLE_W, VSWIZZLE_X, VSWIZZLE_W, VSWIZZLE_X> ( a );
}

FINLINE_Z VecFloat4 VecFloatSwizzleWWWZ ( register const VecFloat4& a )   ///< xyzw = a.wwwz
{
    return _VecFloatSwizzle_ <VSWIZZLE_W, VSWIZZLE_W, VSWIZZLE_W, VSWIZZLE_Z> ( a );
}

FINLINE_Z VecFloat4 VecFloatPermuteX0X0X1X1 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.x, a.x, b.x, b.x }
{
#if defined (_PC_SSE )
	return _mm_shuffle_ps( a, b, 0 );
#else
	return VecFloatLoad4( a.x, a.x, b.x, b.x );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteX0Y0Z0X1 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.x, a.y, a.z, b.x }
{
#if defined (_PC_SSE4 )
	return _mm_insert_ps( a, b, 0x30 ); // b.x(00) -> r.w(11), all pass (0000)
#elif defined (_PC_SSE )
	VecFloat4 z0z0x1x1 = _mm_shuffle_ps( a, b, _MM_SHUFFLE(0,0,2,2) );
	return _mm_shuffle_ps( a, z0z0x1x1, _MM_SHUFFLE(2,0,1,0) );
#else
	return VecFloatLoad4( a.x, a.y, a.z, b.x );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteX0Y0Z0Y1 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.x, a.y, a.z, b.y }
{
#if defined (_PC_SSE4 )
	return _mm_insert_ps( a, b, 0x70 ); // b.y(01) -> r.w(11), all pass (0000)
#elif defined (_PC_SSE )
	VecFloat4 z0z0y1y1 = _mm_shuffle_ps( a, b, _MM_SHUFFLE(1,1,2,2) );
	return _mm_shuffle_ps( a, z0z0y1y1, _MM_SHUFFLE(2,0,1,0) );
#else
	return VecFloatLoad4( a.x, a.y, a.z, b.y );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteX0Y0Z0Z1 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.x, a.y, a.z, b.z }
{
#if defined (_PC_SSE4 )
	return _mm_insert_ps( a, b, 0xB0 ); // b.y(10) -> r.w(11), all pass (0000)
#elif defined (_PC_SSE )
	VecFloat4 z0z0z1z1 = _mm_shuffle_ps( a, b, _MM_SHUFFLE(2,2,2,2) );
	return _mm_shuffle_ps( a, z0z0z1z1, _MM_SHUFFLE(2,0,1,0) );
#else
	return VecFloatLoad4( a.x, a.y, a.z, b.z );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteX0Y0Z0W1 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.x, a.y, a.z, b.w }
{
#if defined (_PC_SSE4 )
	return _mm_insert_ps( a, b, 0xF0 ); // b.w(11) -> r.w(11), all pass (0000)
#elif defined (_PC_SSE )
	VecFloat4 z0z0w1w1 = _mm_shuffle_ps( a, b, _MM_SHUFFLE(3,3,2,2) );
	return _mm_shuffle_ps( a, z0z0w1w1, _MM_SHUFFLE(2,0,1,0) );
#else
	return VecFloatLoad4( a.x, a.y, a.z, b.w );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteX0Y0X1W0 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.x, a.y, b.x, a.w }
{
#if defined (_PC_SSE4 )
	return _mm_insert_ps( a, b, 0x20 ); // b.x(00) -> r.z(10), all pass (0000)
#elif defined (_PC_SSE )
	VecFloat4 w0w0x1x1 = _mm_shuffle_ps( a, b, _MM_SHUFFLE(0,0,3,3) );
	return _mm_shuffle_ps( a, w0w0x1x1, _MM_SHUFFLE(0,2,1,0) );
#else
	return VecFloatLoad4( a.x, a.y, b.x, a.w );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteX0Y0X1X1 ( register const VecFloat4& a, register const VecFloat4& b )   ///< xyzw = { a.x, a.y, b.x, b.x }
{
#if defined (_PC_SSE )
	return _mm_shuffle_ps( a, b, _MM_SHUFFLE(0,0,1,0) );
#else
	return VecFloatLoad4( a.x, a.y, b.x, b.x );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteX0Y0X1Y1 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.x, a.y, b.x, b.y }
{
#if defined( _PC_SSE )
	return _mm_movelh_ps( a, b );
#else
	return VecFloatLoad4( a.x, a.y, b.x, b.y );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteX0Y0Y1X1 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.x, a.y, b.y, b.x }
{
#if defined (_PC_SSE )
	return _mm_shuffle_ps( a, b, _MM_SHUFFLE(0,1,1,0) );
#else
	return VecFloatLoad4( a.x, a.y, b.y, b.x );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteX0Y0Z1W0 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.x, a.y, b.z, a.w }
{
#if defined (_PC_SSE4 )
	return _mm_insert_ps( a, b, 0xA0 ); // b.z(10) -> r.z(10), all pass (0000)
#elif defined (_PC_SSE )
	VecFloat4 z1z1w0w0 = _mm_shuffle_ps( b, a, _MM_SHUFFLE(3,3,2,2) );
	return _mm_shuffle_ps( a, z1z1w0w0, _MM_SHUFFLE(2,0,1,0) );
#else
	return VecFloatLoad4( a.x, a.y, b.z, a.w );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteX0Y0Z1Y1 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.x, a.y, b.z, b.y }
{
#if defined (_PC_SSE )
	return _mm_shuffle_ps( a, b, _MM_SHUFFLE(1,2,1,0) );
#else
	return VecFloatLoad4( a.x, a.y, b.z, b.y );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteX0Y0Z1Z1 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.x, a.y, b.z, b.z }
{
#if defined (_PC_SSE )
	return _mm_shuffle_ps( a, b, _MM_SHUFFLE(2,2,1,0) );
#else
	return VecFloatLoad4( a.x, a.y, b.z, b.z );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteX0Y0Z1W1 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.x, a.y, b.z, b.w }
{
#if defined (_PC_SSE )
	return _mm_shuffle_ps( a, b, _MM_SHUFFLE(3,2,1,0) );
#else
	return VecFloatLoad4( a.x, a.y, b.z, b.w );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteX0Y0W1Z0 ( register const VecFloat4& a, register const VecFloat4& b )   ///< xyzw = { a.x, a.y, b.w, a.z }
{
#if defined (_PC_SSE )
	VecFloat4 z0z1w0w1 = _mm_unpackhi_ps( a, b );
	return _mm_shuffle_ps( a, z0z1w0w1, _MM_SHUFFLE(0,3,1,0) );
#else
	return VecFloatLoad4( a.x, a.y, b.w, a.z );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteX0Z0X1Z1 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.x, a.z, b.x, b.z }
{
#if defined (_PC_SSE )
	return _mm_shuffle_ps( a, b, _MM_SHUFFLE(2,0,2,0) );
#else
	return VecFloatLoad4( a.x, a.z, b.x, b.z );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteX0W0X1W1 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.x, a.w, b.x, b.w }
{
#if defined (_PC_SSE )
	return _mm_shuffle_ps( a, b, _MM_SHUFFLE(3,0,3,0) );
#else
	return VecFloatLoad4( a.x, a.w, b.x, b.w );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteX0X1X0X1( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.x, b.x, a.x, b.x }
{
#if defined(_PC_SSE)
	VecFloat4 x0x1x0x1 = _mm_unpacklo_ps( a, b );
	return _mm_shuffle_ps( x0x1x0x1, x0x1x0x1, _MM_SHUFFLE(1,0,1,0) );
#else
	return VecFloatLoad4( a.x, b.x, a.x, b.x );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteX0X1Y0Y1 ( register const VecFloat4& a, register const VecFloat4& b )   ///< xyzw = { a.x, b.x, a.y, b.y }
{
#if defined (_PC_SSE )
	return _mm_unpacklo_ps( a, b );
#else
	VecFloat4 res;
	res.x = a.x;
	res.y = b.x;
	res.z = a.y;
	res.w = b.y;
	return res;
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteX0X1X1X0 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.x, b.x, b.x, a.x }
{
#if defined (_PC_SSE )
	VecFloat4 x0x1y0y1 = _mm_unpacklo_ps( a, b );
	return _mm_shuffle_ps( x0x1y0y1, x0x1y0y1, _MM_SHUFFLE(0,1,1,0) );
#else
	return VecFloatLoad4( a.x, b.x, b.x, a.x );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteX0X1Z0W0 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.x, b.x, a.z, a.w }
{
#if defined (_PC_SSE4 )
	return _mm_insert_ps( a, b, 0x10 ); // b.x(00) -> r.y(01), all pass (0000)
#elif defined (_PC_SSE )
	VecFloat4 x0x0x1x1 = _mm_shuffle_ps( a, b, 0 );
	return _mm_shuffle_ps( x0x0x1x1, a, _MM_SHUFFLE(3,2,2,0) );
#else
	return VecFloatLoad4( a.x, b.x, a.z, a.w );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteX0X1Y1W0( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.x, b.x, b.y, a.w }
{
#if defined(_PC_SSE)
	VecFloat4 x0w0x1y1 = _mm_shuffle_ps( a, b, _MM_SHUFFLE(1,0,3,0) );
	return _mm_shuffle_ps( x0w0x1y1, x0w0x1y1, _MM_SHUFFLE(1,3,2,0) );
#else
	return VecFloatLoad4( a.x, b.x, b.y, a.w );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteX0X1Y1Z1 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.x, b.x, b.y, b.z }
{
#if defined (_PC_SSE )
	VecFloat4 x0x0x1x1 = _mm_shuffle_ps( a, b, 0 );
	return _mm_shuffle_ps( x0x0x1x1, b, _MM_SHUFFLE(2,1,2,0) );
#else
	return VecFloatLoad4( a.x, b.x, b.y, b.z );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteX0X1Z1W0 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.x, b.x, b.z, a.w }
{
#if defined (_PC_SSE )
	VecFloat4 x0w0x1z1 = _mm_shuffle_ps( a, b, _MM_SHUFFLE(2,0,3,0) );
	return _mm_shuffle_ps( x0w0x1z1, x0w0x1z1, _MM_SHUFFLE(1,3,2,0) );
#else
	return VecFloatLoad4( a.x, b.x, b.z, a.w );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteX0Y1Z0W0 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.x, b.y, a.z, a.w }
{
#if defined (_PC_SSE4 )
	return _mm_insert_ps( a, b, 0x50 ); // b.y(01) -> r.y(01), all pass (0000)
#elif defined (_PC_SSE )
	VecFloat4 x0x0y1y1 = _mm_shuffle_ps( a, b, _MM_SHUFFLE(1,1,0,0) );
	return _mm_shuffle_ps( x0x0y1y1, a, _MM_SHUFFLE(3,2,2,0) );
#else
	return VecFloatLoad4( a.x, b.y, a.z, a.w );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteX0Y1Z0W1 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.x, b.y, a.z, b.w }
{
#if defined (_PC_SSE4 )
	return _mm_blend_ps( a, b, 10 );
#elif defined (_PC_SSE )
	const int mask = _MM_SHUFFLE(3,1,2,0);
	VecFloat4 x0z0y1w1 = _mm_shuffle_ps( a, b, mask );
	return _mm_shuffle_ps( x0z0y1w1, x0z0y1w1, mask );
#else
	return VecFloatLoad4( a.x, b.y, a.z, b.w );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteX0Y1Z1Y0 ( register const VecFloat4& a, register const VecFloat4& b )   ///< xyzw = { a.x, b.y, b.z, a.y }
{
#if defined (_PC_SSE )
	VecFloat4 x0y0y1z1 = _mm_shuffle_ps( a, b, _MM_SHUFFLE(2,1,1,0) );
	return _mm_shuffle_ps( x0y0y1z1, x0y0y1z1, _MM_SHUFFLE(1,3,2,0) );
#else
	return VecFloatLoad4( a.x, b.y, b.z, a.y );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteX0Y1Z1W0 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.x, b.y, b.z, a.w }
{
#if defined (_PC_SSE4 )
	return _mm_blend_ps( a, b, 6 );
#elif defined (_PC_SSE )
	VecFloat4 x0w0y1z1 = _mm_shuffle_ps( a, b, _MM_SHUFFLE(2,1,3,0) );
	return _mm_shuffle_ps( x0w0y1z1, x0w0y1z1, _MM_SHUFFLE(1,3,2,0) );
#else
	return VecFloatLoad4( a.x, b.y, b.z, a.w );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteX0Y1Z1W1 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.x, b.y, b.z, b.w }
{
#if defined (_PC_SSE4 )
	return _mm_insert_ps( b, a, 0x00 ); // a.x(00) -> b.x(00), all pass (0000)
#elif defined (_PC_SSE )
	VecFloat4 x0x0y1y1 = _mm_shuffle_ps( a, b, _MM_SHUFFLE(1,1,0,0) );
	return _mm_shuffle_ps( x0x0y1y1, b, _MM_SHUFFLE(3,2,2,0) );
#else
	return VecFloatLoad4( a.x, b.y, b.z, b.w );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteX0W1Y0Z0 ( register const VecFloat4& a, register const VecFloat4& b )   ///< xyzw = { a.x, b.w, a.y, a.z }
{
#if defined (_PC_SSE )
	VecFloat4 x0x0w1w1 = _mm_shuffle_ps( a, b, _MM_SHUFFLE(3,3,0,0) );
	return _mm_shuffle_ps( x0x0w1w1, a, _MM_SHUFFLE(2,1,2,0) );
#else
	return VecFloatLoad4( a.x, b.w, a.y, a.z );
#endif
}


FINLINE_Z VecFloat4 VecFloatPermuteY0X0Y1X1 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.y, a.x, b.y, b.x }
{
#if defined (_PC_SSE )
	return _mm_shuffle_ps( a, b, _MM_SHUFFLE(0,1,0,1) );
#else
	return VecFloatLoad4( a.y, a.x, b.y, b.x );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteY0Y0Y1Y1 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.y, a.y, b.y, b.y }
{
#if defined (_PC_SSE )
	return _mm_shuffle_ps( a, b, _MM_SHUFFLE(1,1,1,1) );
#else
	return VecFloatLoad4( a.y, a.y, b.y, b.y );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteY0W0Y1W1 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.y, a.w, b.y, b.w }
{
#if defined (_PC_SSE )
	return _mm_shuffle_ps( a, b, _MM_SHUFFLE(3,1,3,1) );
#else
	return VecFloatLoad4( a.y, a.w, b.y, b.w );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteY0W0W1W1 ( register const VecFloat4& a, register const VecFloat4& b )   ///< xyzw = { a.y, a.w, b.w, b.w }
{
#if defined (_PC_SSE )
	return _mm_shuffle_ps( a, b, _MM_SHUFFLE(3,3,3,1) );
#else
	return VecFloatLoad4( a.y, a.w, b.w, b.w );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteY0X1W1X0 ( register const VecFloat4& a, register const VecFloat4& b )   ///< xyzw = { a.y, b.x, b.w, a.x }
{
#if defined (_PC_SSE )
	VecFloat4 x0y0x1w1 = _mm_shuffle_ps( a, b, _MM_SHUFFLE(3,0,1,0) );
	return _mm_shuffle_ps( x0y0x1w1, x0y0x1w1, _MM_SHUFFLE(0,3,2,1) );
#else
	return VecFloatLoad4( a.y, b.x, b.w, a.x );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteY0Y1Y0Y0 ( register const VecFloat4& a, register const VecFloat4& b )   ///< xyzw = { a.y, b.y, a.y, a.y }
{
#if defined (_PC_SSE )
	VecFloat4 x0x1y0y1 = _mm_unpacklo_ps( a, b );
	return _mm_shuffle_ps( x0x1y0y1, x0x1y0y1, _MM_SHUFFLE(2,2,3,2) );
#else
	return VecFloatLoad4( a.y, b.y, a.y, a.y );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteY0Y1Y1Y0 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.y, b.y, b.y, a.y }
{
#if defined (_PC_SSE )
	VecFloat4 x0x1y0y1 = _mm_unpacklo_ps( a, b );
	return _mm_shuffle_ps( x0x1y0y1, x0x1y0y1, _MM_SHUFFLE(2,3,3,2) );
#else
	return VecFloatLoad4( a.y, b.y, b.y, a.y );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteY0Y1Z1W1 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.y, b.y, b.z, b.w }
{
#if defined (_PC_SSE4 )
	return _mm_insert_ps( b, a, 0x40 ); // a.y(01) -> r.x(00), all pass (0000)
#elif defined (_PC_SSE )
	return _mm_movehl_ps( b, _mm_unpacklo_ps(a,b) );
#else
	return VecFloatLoad4( a.y, b.y, b.z, b.w );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteX0Z1Z0W0 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.x, b.z, a.z, a.w }
{
#if defined (_PC_SSE4 )
	return _mm_insert_ps( a, b, 0x90 ); // b.z(10) -> r.y(01), all pass (0000)
#elif defined (_PC_SSE )
	VecFloat4 x0x0z1z1 = _mm_shuffle_ps( a, b, _MM_SHUFFLE(2,2,0,0) );
	return _mm_shuffle_ps( x0x0z1z1, a, _MM_SHUFFLE(3,2,2,0) );
#else
	return VecFloatLoad4( a.x, b.z, a.z, a.w );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteX0Z1W1W0( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.x, b.z, b.w, a.w }
{
#if defined(_PC_SSE)
	VecFloat4 x0w0z1w1 = _mm_shuffle_ps( a, b, _MM_SHUFFLE(3,2,3,0) );
	return _mm_shuffle_ps( x0w0z1w1, x0w0z1w1, _MM_SHUFFLE(1,3,2,0) );
#else
	return VecFloatLoad4( a.x, b.z, b.w, a.w );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteY0Z0W0X1 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.y, a.z, a.w, b.x }
{
#if defined (_PC_SSE )
	VecFloat4 w0w0x1x1 = _mm_shuffle_ps( a, b, _MM_SHUFFLE(0,0,3,3) );
	return _mm_shuffle_ps( a, w0w0x1x1, _MM_SHUFFLE(2,0,2,1) );
#else
	return VecFloatLoad4( a.y, a.z, a.w, b.x );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteY0Z0W0Z1 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.y, a.z, a.w, b.z }
{
#if defined (_PC_SSE )
	VecFloat4 w0w0z1z1 = _mm_shuffle_ps( a, b, _MM_SHUFFLE(2,2,3,3) );
	return _mm_shuffle_ps( a, w0w0z1z1, _MM_SHUFFLE(2,0,2,1) );
#else
	return VecFloatLoad4( a.y, a.z, a.w, b.z );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteY0Z0X1Y1 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.y, a.z, b.x, b.y }
{
#if defined (_PC_SSE )
	return _mm_shuffle_ps( a, b, _MM_SHUFFLE(1,0,2,1) );
#else
	return VecFloatLoad4( a.y, a.z, b.x, b.y );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteY0Z0Y1Z1 ( register const VecFloat4& a, register const VecFloat4& b )   ///< xyzw = { a.y, a.z, b.y, b.z }
{
#if defined (_PC_SSE )
	return _mm_shuffle_ps( a, b, _MM_SHUFFLE(2,1,2,1) );
#else
	return VecFloatLoad4( a.y, a.z, b.y, b.z );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteY0Z0Z1Y1 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.y, a.z, b.z, b.y }
{
#if defined (_PC_SSE )
	return _mm_shuffle_ps( a, b, _MM_SHUFFLE(1,2,2,1) );
#else
	return VecFloatLoad4( a.y, a.z, b.z, b.y );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteY0W0Y1Y1 ( register const VecFloat4& a, register const VecFloat4& b )   ///< xyzw = { a.y, a.w, b.y, b.y }
{
#if defined (_PC_SSE )
	return _mm_shuffle_ps( a, b, _MM_SHUFFLE(1,1,3,1) );
#else
	return VecFloatLoad4( a.y, a.w, b.y, b.y );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteZ0X0Y0X1 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.z, a.x, a.y, b.x }
{
#if defined (_PC_SSE )
	VecFloat4 x0x1y0y1 = _mm_unpacklo_ps( a, b );
	return _mm_shuffle_ps( a, x0x1y0y1, _MM_SHUFFLE(1,2,0,2) );
#else
	return VecFloatLoad4( a.z, a.x, a.y, b.x );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteZ0X0X1Y0 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.z, a.x, b.x, a.y }
{
#if defined (_PC_SSE )
	VecFloat4 x0x1y0y1 = _mm_unpacklo_ps( a, b );
	return _mm_shuffle_ps( a, x0x1y0y1, _MM_SHUFFLE(2,1,0,2) );
#else
	return VecFloatLoad4( a.z, a.x, b.x, a.y );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteZ0X0X1W1 ( register const VecFloat4& a, register const VecFloat4& b )   ///< xyzw = { a.z, a.x, b.x, b.w }
{
#if defined (_PC_SSE )
	return _mm_shuffle_ps( a, b, _MM_SHUFFLE(3,0,0,2) );
#else
	return VecFloatLoad4( a.z, a.x, b.x, b.w );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteZ0X0W1X1 ( register const VecFloat4& a, register const VecFloat4& b )   ///< xyzw = { a.z, a.x, b.w, b.x }
{
#if defined (_PC_SSE )
	return _mm_shuffle_ps( a, b, _MM_SHUFFLE(0,3,0,2) );
#else
	return VecFloatLoad4( a.z, a.x, b.w, b.x );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteZ0Y0Z1X1 ( register const VecFloat4& a, register const VecFloat4& b )   ///< xyzw = { a.z, a.y, b.z, b.x }
{
#if defined (_PC_SSE )
	return _mm_shuffle_ps( a, b, _MM_SHUFFLE(0,2,1,2) );
#else
	return VecFloatLoad4( a.z, a.y, b.z, b.x );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteZ0Y0Z1Y1 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.z, a.y, b.z, b.y }
{
#if defined (_PC_SSE )
	return _mm_shuffle_ps( a, b, _MM_SHUFFLE(1,2,1,2) );
#else
	return VecFloatLoad4( a.z, a.y, b.z, b.y );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteZ0Z0X1Z0 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.z, a.z, b.x, a.z }
{
#if defined (_PC_SSE )
	VecFloat4 x1x1z0z0 = _mm_shuffle_ps( b, a, _MM_SHUFFLE(2,2,0,0) );
	return _mm_shuffle_ps( a, x1x1z0z0, _MM_SHUFFLE(2,0,2,2) );
#else
	return VecFloatLoad4( a.z, a.z, b.x, a.z );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteZ0Z0Z1Z0 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.z, a.z, b.z, a.z }
{
#if defined (_PC_SSE )
	VecFloat4 z1z1z0z0 = _mm_shuffle_ps( b, a, _MM_SHUFFLE(2,2,2,2) );
	return _mm_shuffle_ps( a, z1z1z0z0, _MM_SHUFFLE(2,0,2,2) );
#else
	return VecFloatLoad4( a.z, a.z, b.z, a.z );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteZ0Z0Z1Z1 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.z, a.z, b.z, b.z }
{
#if defined (_PC_SSE )
	return _mm_shuffle_ps( a, b, _MM_SHUFFLE(2,2,2,2) );
#else
	return VecFloatLoad4( a.z, a.z, b.z, b.z );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteZ0W0X1Y1 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.z, a.w, b.x, b.y }
{
#if defined (_PC_SSE )
	return _mm_shuffle_ps( a, b, _MM_SHUFFLE(1,0,3,2) );
#else
	return VecFloatLoad4( a.z, a.w, b.x, b.y );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteZ0W0Y1Y1 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.z, a.w, b.y, b.y }
{
#if defined( _PC_SSE )
	return _mm_shuffle_ps( a, b, _MM_SHUFFLE(1,1,3,2) );
#else
	return VecFloatLoad4( a.z, a.w, b.y, b.y );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteZ0W0Z1W1 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.z, a.w, b.z, b.w }
{
#if defined (_PC_SSE )
	return _mm_movehl_ps( b, a );
#else
	return VecFloatLoad4( a.z, a.w, b.z, b.w );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteZ0X1X0Y0 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.z, b.x, a.x, a.y }
{
#if defined (_PC_SSE )
	VecFloat4 z0z0x1x1 = _mm_shuffle_ps( a, b, _MM_SHUFFLE(0,0,2,2) );
	return _mm_shuffle_ps( z0z0x1x1, a, _MM_SHUFFLE(1,0,2,0) );
#else
	return VecFloatLoad4( a.z, b.x, a.x, a.y );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteZ0X1Y1Z1 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.z, b.x, b.y, b.z }
{
#if defined (_PC_SSE )
	VecFloat4 z0z0x1x1 = _mm_shuffle_ps( a, b, _MM_SHUFFLE(0,0,2,2) );
	return _mm_shuffle_ps( z0z0x1x1, b, _MM_SHUFFLE(2,1,2,0) );
#else
	return VecFloatLoad4( a.z, b.x, b.y, b.z );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteZ0Y1X1Z0 ( register const VecFloat4& a, register const VecFloat4& b )   ///< xyzw = { a.z, b.y, b.x, a.z }
{
#if defined (_PC_SSE )
	VecFloat4 z0z0x1y1 = _mm_shuffle_ps( a, b, _MM_SHUFFLE(1,0,2,2) );
	return _mm_shuffle_ps( z0z0x1y1, z0z0x1y1, _MM_SHUFFLE(0,2,3,0) );
#else
	return VecFloatLoad4( a.z, b.y, b.x, a.z );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteZ0Y1Z1W0 ( register const VecFloat4& a, register const VecFloat4& b )   ///< xyzw = { a.z, b.y, b.z, a.w }
{
#if defined (_PC_SSE )
	VecFloat4 z0w0y1z1 = _mm_shuffle_ps( a, b, _MM_SHUFFLE(2,1,3,2) );
	return _mm_shuffle_ps( z0w0y1z1, z0w0y1z1, _MM_SHUFFLE(1,3,2,0) );
#else
	return VecFloatLoad4( a.z, b.y, b.z, a.w );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteZ0Y1Z1W1 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.z, b.y, b.z, b.w }
{
	return VecFloatPermuteZ1Y0Z0W0( b, a );
}

FINLINE_Z VecFloat4 VecFloatPermuteZ0Z1W0W1 ( register const VecFloat4& a, register const VecFloat4& b )   ///< xyzw = { a.z, b.z, a.w, b.w }
{
#if defined (_PC_SSE )
	return _mm_unpackhi_ps( a, b );
#else
	VecFloat4 res;
	res.x = a.z;
	res.y = b.z;
	res.z = a.w;
	res.w = b.w;
	return res;
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteZ0Z1Z1Z0 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.z, b.z, b.z, a.z }
{
#if defined (_PC_SSE )
	VecFloat4 z0z1w0w1 = _mm_unpackhi_ps( a, b );
	return _mm_shuffle_ps( z0z1w0w1, z0z1w0w1, _MM_SHUFFLE(0,1,1,0) );
#else
	return VecFloatLoad4( a.z, b.z, b.z, a.z );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteZ0W1Z1Z0 ( register const VecFloat4& a, register const VecFloat4& b )   ///< xyzw = { a.z, b.w, b.z, a.z }
{
#if defined (_PC_SSE )
	VecFloat4 z0z0z1w1 = _mm_shuffle_ps( a, b, _MM_SHUFFLE(3,2,2,2) );
	return _mm_shuffle_ps( z0z0z1w1, z0z0z1w1, _MM_SHUFFLE(0,2,3,0) );
#else
	return VecFloatLoad4( a.z, b.w, b.z, a.z );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteW0X0Y1Z1 ( register const VecFloat4& a, register const VecFloat4& b )   ///< xyzw = { a.w, a.x, b.y, b.z }
{
#if defined (_PC_SSE )
	return _mm_shuffle_ps( a, b, _MM_SHUFFLE(2,1,0,3) );
#else
	return VecFloatLoad4( a.w, a.x, b.y, b.z );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteW0Z0W1Z1( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.w, b.z, b.w, b.z }
{
#if defined(_PC_SSE)
	return _mm_shuffle_ps( a, b, _MM_SHUFFLE(2,3,2,3) );
#else
	return VecFloatLoad4( a.w, a.z, b.w, b.z );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteW0X1W1Z0 ( register const VecFloat4& a, register const VecFloat4& b )   ///< xyzw = { a.w, b.x, b.w, a.z }
{
#if defined (_PC_SSE )
	VecFloat4 z0w0x1w1 = _mm_shuffle_ps( a, b, _MM_SHUFFLE(3,0,3,2) );
	return _mm_shuffle_ps( z0w0x1w1, z0w0x1w1, _MM_SHUFFLE(0,3,2,1) );
#else
	return VecFloatLoad4( a.w, b.x, b.w, a.z );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteW0Z1Y0X1 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { a.w, b.z, a.y, b.x }
{
#if defined (_PC_SSE )
	VecFloat4 w0y0z1x1 = _mm_shuffle_ps( a, b, _MM_SHUFFLE(0,2,1,3) );
	return _mm_shuffle_ps( w0y0z1x1, w0y0z1x1, _MM_SHUFFLE(3,1,2,0) );
#else
	return VecFloatLoad4( a.w, b.z, a.y, b.x );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteX1X0X0X0 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { b.x, a.x, a.x, a.x }
{
#if defined (_PC_SSE )
	VecFloat4 x0x1y0y1 = _mm_unpacklo_ps( a, b );
	return _mm_shuffle_ps( x0x1y0y1, x0x1y0y1, _MM_SHUFFLE(0,0,0,1) );
#else
	return VecFloatLoad4( b.x, a.x, a.x, a.x );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteX1Y0Y1W0 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { b.x, a.y, b.y, a.w }
{
#if defined (_PC_SSE )
	VecFloat4 y0w0x1y1 = _mm_shuffle_ps( a, b, _MM_SHUFFLE(1,0,3,1) );
	return _mm_shuffle_ps( y0w0x1y1, y0w0x1y1, _MM_SHUFFLE(1,3,0,2) );
#else
	return VecFloatLoad4( b.x, a.y, b.y, a.w );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteX1Y0Z0W0 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { b.x, a.y, a.z, a.w }
{
	return VecFloatPermuteX0Y1Z1W1( b, a );
}

FINLINE_Z VecFloat4 VecFloatPermuteX1Y0Z0W1 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { b.x, a.y, a.z, a.w }
{
	return VecFloatPermuteX0Y1Z1W0( b, a );
}

FINLINE_Z VecFloat4 VecFloatPermuteX1Y0Z1W0 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { b.x, a.y, b.z, a.w }
{
	return VecFloatPermuteX0Y1Z0W1( b, a );
}

FINLINE_Z VecFloat4 VecFloatPermuteX1Y0Z1W1 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { b.x, a.y, b.z, b.w }
{
	return VecFloatPermuteX0Y1Z0W0( b, a );
}

FINLINE_Z VecFloat4 VecFloatPermuteX1Y1Z0W0 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { b.x, b.y, a.z, a.w }
{
	return VecFloatPermuteX0Y0Z1W1( b, a );
}

FINLINE_Z VecFloat4 VecFloatPermuteX1Y1Z0W1 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { b.x, b.y, a.z, b.w }
{
	return VecFloatPermuteX0Y0Z1W0( b, a );
}

FINLINE_Z VecFloat4 VecFloatPermuteX1Y1Z1W0 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { b.x, b.y, b.z, a.w }
{
	return VecFloatPermuteX0Y0Z0W1( b, a );
}

FINLINE_Z VecFloat4 VecFloatPermuteY1X0W0Z1 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { b.y, a.x, a.w, b.z }
{
#if defined (_PC_SSE )
	__m128 x0w0y1z1 = _mm_shuffle_ps( a, b, _MM_SHUFFLE(2,1,3,0) );
	return _mm_shuffle_ps( x0w0y1z1, x0w0y1z1, _MM_SHUFFLE(3,1,0,2) );
#else
	return VecFloatLoad4( b.y, a.x, a.w, b.z );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteY1Z1Z0W0 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { b.y, b.z, a.z, a.w }
{
#if defined (_PC_SSE )
	return _mm_shuffle_ps( b, a, _MM_SHUFFLE(3,2,2,1) );
#else
	return VecFloatLoad4( b.y, b.z, a.z, a.w );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteZ1Y0Z0W0 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { b.z, a.y, a.z, a.w }
{
#if defined (_PC_SSE4 )
	return _mm_insert_ps( a, b, 0x80 ); // b.z(10) -> r.x(00), all pass (0000)
#elif defined (_PC_SSE )
	__m128 z1z1y0y0 = _mm_shuffle_ps( b, a, _MM_SHUFFLE(1,1,2,2) );
	return _mm_shuffle_ps( z1z1y0y0, a, _MM_SHUFFLE(3,2,2,0) );
#else
	return VecFloatLoad4( b.z, a.y, a.z, a.w );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteZ1Y1Z0W0 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = { b.z, b.y, a.z, a.w }
{
#if defined (_PC_SSE )
	return _mm_shuffle_ps( b, a, _MM_SHUFFLE(3,2,1,2) );
#else
	return VecFloatLoad4( b.z, b.y, a.z, a.w );
#endif
}

FINLINE_Z VecFloat4 VecFloatPermuteW1X0Y0Z0 ( register const VecFloat4& a, register const VecFloat4& b )   ///< xyzw = { b.w, a.x, a.y, a.z }
{
#if defined (_PC_SSE )
	VecFloat4 x0x0w1w1 = _mm_shuffle_ps( a, b, _MM_SHUFFLE(3,3,0,0) );
	return _mm_shuffle_ps( x0x0w1w1, a, _MM_SHUFFLE(2,1,0,2) );
#else
	return VecFloatLoad4( b.w, a.x, a.y, a.z );
#endif
}

// ---------------------- Compare operations

FINLINE_Z U32 VecFloatAllZero ( register const VecFloat4& a ) ///< return a.x == 0 && a.y == 0 && a.z == 0 && a.w == 0 ? 1 : 0
{
#if defined (_PC_SSE4 )
	return VecIntAllZero( _mm_castps_si128(a) );
#else
	return VecFloatAllEqual( a, VecFloatSplatZero() );
#endif
}

FINLINE_Z U32 VecFloatAllEqual( register const VecFloat4& a, register const VecFloat4& b ) ///< Strict comparison. return a.xyzw == b.xyzw ? 1 : 0
{
#if defined (_PC_SSE )
	U32 result = _mm_movemask_ps( _mm_cmpeq_ps(a,b) );  // result � [0,15]
	return (result+1) >> 4;
#else
	return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
#endif
}

FINLINE_Z U32 VecFloatAllNotEqual( register const VecFloat4& a, register const VecFloat4& b ) ///< Strict comparison. return a.xyzw != b.xyzw ? 1 : 0
{
#if defined (_PC_SSE )
	U32 result = _mm_movemask_ps( _mm_cmpneq_ps(a,b) );  // result � [0,15]
	return (result+1) >> 4;
#else
	return a.x != b.x && a.y != b.y && a.z != b.z && a.w != b.w;
#endif
}

FINLINE_Z U32 VecFloatAllNearEqual( register const VecFloat4& a, register const VecFloat4& b, register const VecFloat4& epsilon ) ///< return a.xyzw ~= b.xyzw ? 1 : 0 with tolerance limits of |a-b| < epsilon
{
	return VecFloatAllLess( VecFloatAbs(VecFloatSub(a,b)), epsilon ); 
}

FINLINE_Z U32 VecFloatAllLess( register const VecFloat4& a, register const VecFloat4& b ) ///< return a.xyzw < b.xyzw ? 1 : 0
{
#if defined (_PC_SSE )
	U32 result = _mm_movemask_ps( _mm_cmplt_ps(a,b) );  // result � [0,15]
	return (result+1) >> 4;
#else
	return a.x < b.x && a.y < b.y && a.z < b.z && a.w < b.w;
#endif
}

FINLINE_Z U32 VecFloatAllLessOrEqual( register const VecFloat4& a, register const VecFloat4& b ) ///< return a.xyzw <= b.xyzw ? 1 : 0
{
#if defined (_PC_SSE )
	U32 result = _mm_movemask_ps( _mm_cmple_ps(a,b) );  // result � [0,15]
	return (result+1) >> 4;
#else
	return a.x <= b.x && a.y <= b.y && a.z <= b.z && a.w <= b.w;
#endif
}

FINLINE_Z U32 VecFloatAllGreater( register const VecFloat4& a, register const VecFloat4& b ) ///< return a.xyzw > b.xyzw ? 1 : 0
{
#if defined (_PC_SSE )
	U32 result = _mm_movemask_ps( _mm_cmpgt_ps(a,b) );  // result � [0,15]
	return (result+1) >> 4;
#else
	return a.x > b.x && a.y > b.y && a.z > b.z && a.w > b.w;
#endif
}

FINLINE_Z U32 VecFloatAllGreaterOrEqual( register const VecFloat4& a, register const VecFloat4& b ) ///< return a.xyzw >= b.xyzw ? 1 : 0
{
#if defined (_PC_SSE )
	U32 result = _mm_movemask_ps( _mm_cmpge_ps(a,b) );  // result � [0,15]
	return (result+1) >> 4;
#else
	return a.x >= b.x && a.y >= b.y && a.z >= b.z && a.w >= b.w;
#endif
}

FINLINE_Z U32 VecFloatAllInBounds( register const VecFloat4& a, register const VecFloat4& b ) ///< return a.xyzw <= b.xyzw && a.xyzw >= -b.xyzw ? 1 : 0
{
#if defined (_PC_SSE )
	register const __m128 negB = VecFloatNegate( b );
    register const __m128 t = _mm_and_ps( _mm_cmple_ps(a,b), _mm_cmpge_ps(a,negB) );
    return _mm_movemask_ps(t) == 0x0f;
#else
	return VecFloatAllLessOrEqual( a, b )
		&& VecFloatAllLessOrEqual( VecFloatNegate(b), a );
#endif
}

FINLINE_Z U32 VecFloatAll3Equal( register const VecFloat4& a, register const VecFloat4& b ) ///< Strict comparison. return a.xyz == b.xyz ? 1 : 0
{
#if defined (_PC_SSE )
	U32 result = _mm_movemask_ps( _mm_cmpeq_ps(a,b) );
	return ( result & 7 ) == 7 ? 1 : 0;
#else
	return a.x == b.x && a.y == b.y && a.z == b.z;
#endif
}

FINLINE_Z U32 VecFloatAll3LessOrEqual( register const VecFloat4& a, register const VecFloat4& b ) ///< return a.xyz <= b.xyz ? 1 : 0
{
#if defined (_PC_SSE )
	U32 result = _mm_movemask_ps( _mm_cmple_ps(a,b) );
	return ( result & 7 ) == 7 ? 1 : 0;
#else
	return a.x <= b.x && a.y <= b.y && a.z <= b.z;
#endif
}

FINLINE_Z U32 VecFloatAll3Greater( register const VecFloat4& a, register const VecFloat4& b ) ///< return a.xyz > b.xyz ? 1 : 0
{
#if defined (_PC_SSE )
	U32 result = _mm_movemask_ps( _mm_cmpgt_ps(a,b) );
	return ( result & 7 ) == 7 ? 1 : 0;
#else
	return a.x > b.x && a.y > b.y && a.z > b.z;
#endif
}

FINLINE_Z U32 VecFloatAll3GreaterOrEqual( register const VecFloat4& a, register const VecFloat4& b ) ///< return a.xyz >= b.xyz ? 1 : 0
{
#if defined (_PC_SSE )
	U32 result = _mm_movemask_ps( _mm_cmpge_ps(a,b) );
	return ( result & 7 ) == 7 ? 1 : 0;
#else
	return a.x >= b.x && a.y >= b.y && a.z >= b.z;
#endif
}

FINLINE_Z U32 VecFloatAll3InBounds( register const VecFloat4& a, register const VecFloat4& b ) ///< return a.xyz <= b.xyz && a.xyz >= -b.xyz ? 1 : 0
{
#if defined (_PC_SSE )
	register const __m128 negB = VecFloatNegate( b );
    register const __m128 t = _mm_and_ps( _mm_cmple_ps(a,b), _mm_cmpge_ps(a,negB) );
    return ( _mm_movemask_ps(t) & 0x7 ) == 0x7;
#else
	return VecFloatAll3LessOrEqual( a, b )
		&& VecFloatAll3LessOrEqual( VecFloatNegate(b), a );
#endif
}

FINLINE_Z U32 VecFloatAnyEqual ( register const VecFloat4& a, register const VecFloat4& b )   ///< ///< Strict comparison. return a.x == b.x || a.y == b.y || a.z == b.z || a.w == b.w ? 1 : 0
{
	return ! VecFloatAllNotEqual( a, b );
}

FINLINE_Z U32 VecFloatAnyNotEqual( register const VecFloat4& a, register const VecFloat4& b ) ///< Strict comparison. return a.xyzw != b.xyzw ? 1 : 0
{
#if defined (_PC_SSE )
	U32 result = _mm_movemask_ps( _mm_cmpeq_ps(a,b) );
	return 1 - ( (result+1) >> 4 );
#else
	return a.x != b.x || a.y != b.y || a.z != b.z || a.w != b.w;
#endif
}

FINLINE_Z U32 VecFloatAnyLess( register const VecFloat4& a, register const VecFloat4& b ) ///< return a.x < b.x || a.y < b.y || a.z < b.z || a.w < b.w ? 1 : 0
{
#if defined (_PC_SSE )
	return _mm_movemask_ps( _mm_cmplt_ps(a,b) ) > 0;
#else
	return a.x < b.x || a.y < b.y || a.z < b.z || a.w < b.w;
#endif
}

FINLINE_Z U32 VecFloatAnyGreater( register const VecFloat4& a, register const VecFloat4& b ) ///< return a.x > b.x || a.y > b.y || a.z > b.z || a.w > b.w ? 1 : 0
{
#if defined (_PC_SSE )
	return _mm_movemask_ps( _mm_cmpgt_ps(a,b) ) > 0;
#else
	return a.x > b.x || a.y > b.y || a.z > b.z || a.w > b.w;
#endif
}

FINLINE_Z U32 VecFloatPackSign( register const VecFloat4& v ) ///< res = sign(v.w)<<3 | sign(v.z)<<2 | sign(v.y)<<1 | sign(v.x)
{
#if defined(_PC_SSE )
	return _mm_movemask_ps( v );
#else
	U32 sx = VecFloatGetX(v) < 0.f ? 1 : 0;
	U32 sy = VecFloatGetY(v) < 0.f ? 2 : 0;
	U32 sz = VecFloatGetZ(v) < 0.f ? 4 : 0;
	U32 sw = VecFloatGetW(v) < 0.f ? 8 : 0;
	return sx | sy | sz | sw;
#endif
}

FINLINE_Z U32 VecFloat4x4IsEqual( register const VecFloat4x4& a, register const VecFloat4x4& b ) ///< Strict comparison. return a{0-3}.xyzw == b{0-3}.xyzw ? 1 : 0
{
#if defined (_PC_SSE )
	__m128 results[4];
	results[0] =_mm_cmpeq_ps( a[0], b[0] );
	results[1] =_mm_cmpeq_ps( a[1], b[1] );
	results[2] =_mm_cmpeq_ps( a[2], b[2] );
	results[3] =_mm_cmpeq_ps( a[3], b[3] );
	results[0] = _mm_and_ps( results[0], results[1] );
	results[1] = _mm_and_ps( results[2], results[3] );
	results[0] = _mm_and_ps( results[0], results[1] );
	U32 result = _mm_movemask_ps( results[0] );  // result � [0,15]
	return (result+1) >> 4;
#else
	return VecFloatAllEqual( VecFloat4x4GetRow0(a), VecFloat4x4GetRow0(b) )
		&& VecFloatAllEqual( VecFloat4x4GetRow1(a), VecFloat4x4GetRow1(b) )
		&& VecFloatAllEqual( VecFloat4x4GetRow2(a), VecFloat4x4GetRow2(b) )
		&& VecFloatAllEqual( VecFloat4x4GetRow3(a), VecFloat4x4GetRow3(b) );
#endif
}


// ---------------------- Rounding

FINLINE_Z VecFloat4 VecFloatSplitParts ( register const VecFloat4& a, register VecFloat4& intPart ) ///< xyzw = modf( a.xyzw, intPart.xyzw ) => breaks a into fractional and integral parts
{
	intPart = VecFloatFloor( a );
	return VecFloatSub( a, intPart );
}

FINLINE_Z VecFloat4 VecFloatMod ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = fmod( a.xyzw, b.xyzw ) => compute remainder of division of a by b
{
	VecFloat4 quotient = VecFloatTruncate( VecFloatDiv(a,b) );
	return VecFloatSub( a, VecFloatMul(quotient,b) );
}

FINLINE_Z VecFloat4 VecFloatFloor ( register const VecFloat4& a ) ///< xyzw = floor( a.xyzw )
{
#if defined (_PC_SSE4 )
	return _mm_floor_ps( a );
#elif defined (_PC_SSE2 )
	// For further optimization, we can also assume that _MM_ROUND_DOWN is always the default rounding mode
	const int previousRounding = _MM_GET_ROUNDING_MODE();
	_MM_SET_ROUNDING_MODE( _MM_ROUND_DOWN );
	__m128 result = _mm_cvtepi32_ps( _mm_cvtps_epi32(a) );
	_MM_SET_ROUNDING_MODE( previousRounding );
	return result;
#else
	return VecFloatLoad4( FLOORF(VecFloatGetX(a)), FLOORF(VecFloatGetY(a)), FLOORF(VecFloatGetZ(a)), FLOORF(VecFloatGetW(a)) );
#endif
}

FINLINE_Z VecFloat4 VecFloatCeil ( register const VecFloat4& a ) ///< xyzw = ceil( a.xyzw )
{
#if defined (_PC_SSE4 )
	return _mm_ceil_ps( a );
#elif defined (_PC_SSE2 )
	const int previousRounding = _MM_GET_ROUNDING_MODE();
	_MM_SET_ROUNDING_MODE( _MM_ROUND_UP );
	__m128 result = _mm_cvtepi32_ps( _mm_cvtps_epi32(a) );
	_MM_SET_ROUNDING_MODE( previousRounding );
	return result;
#else
    return VecFloatLoad4( CEILFF(VecFloatGetX(a)), CEILFF(VecFloatGetY(a)), CEILFF(VecFloatGetZ(a)), CEILFF(VecFloatGetW(a)) );
#endif
}

FINLINE_Z VecFloat4 VecFloatNearest ( register const VecFloat4& a ) ///< xyzw = rintf( a.xyzw )
{
#if defined (_PC_SSE4 )
	return _mm_round_ps( a, _MM_FROUND_TO_NEAREST_INT );
#elif defined (_PC_SSE2 )
	const int previousRounding = _MM_GET_ROUNDING_MODE();
	_MM_SET_ROUNDING_MODE( _MM_ROUND_NEAREST );
	__m128 result = _mm_cvtepi32_ps( _mm_cvtps_epi32(a) );
	_MM_SET_ROUNDING_MODE( previousRounding );
	return result;
#else
    return VecFloatLoad4( ROUNDF(VecFloatGetX(a)), ROUNDF(VecFloatGetY(a)), ROUNDF(VecFloatGetZ(a)), ROUNDF(VecFloatGetW(a)) );
#endif
}

FINLINE_Z VecFloat4 VecFloatTruncate ( register const VecFloat4& a ) ///< xyzw = trunc( a.xyzw )
{
#if defined (_PC_SSE4 )
	return _mm_round_ps( a, _MM_FROUND_TO_ZERO );
#elif defined (_PC_SSE2 )
	return _mm_cvtepi32_ps( _mm_cvttps_epi32(a) );
#else
	const VecInt4 v = VecIntLoad4( TRUNCINT(VecFloatGetX(a)), TRUNCINT(VecFloatGetY(a)), TRUNCINT(VecFloatGetZ(a)), TRUNCINT(VecFloatGetW(a)) );
    return VecIntToFloat( v );
#endif
}


// ---------------------- Common maths

FINLINE_Z VecFloat4 VecFloatModAngles ( register const VecFloat4& angles ) ///< result.xyzw = modulo2pi( angles.xyzw )
{
	// result.xyzw = angles.xyzw - 2π * round( angles.xyzw / 2π );
    VecFloat4 v = VecFloatNearest( VecFloatMul(angles,VectorConstantsPrivate::vReciprocalTwoPi) );
    return VecFloatNegMsub( VectorConstantsPrivate::vTwoPi, v, angles );
}

FINLINE_Z VecFloat4 VecFloatAngleBetweenNormals3 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = radianAngle( a.xyz, b.xyz ) with a and b already normalized
{
	return VecFloatACos( VecFloatSaturateExtended( VecFloatDot3(a,b) ) );
}

FINLINE_Z VecFloat4 VecFloatAngleBetweenNormals4 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = radianAngle( a.xyzw, b.xyzw ) with a and b already normalized
{
	return VecFloatACos( VecFloatSaturateExtended( VecFloatDot4(a,b) ) );
}

FINLINE_Z VecFloat4 VecFloatAngleBetweenVectors3 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = radianAngle( a.xyz, b.xyz ) with a and b potentially denormalized
{
    VecFloat4 a1 = VecFloatMul( VecFloatDot3(a,a), VecFloatDot3(b,b) );
    VecFloat4 cosAngle = VecFloatMul( VecFloatDot3(a,b), VecFloatRSqrt(a1) );
    return VecFloatACos( VecFloatSaturateExtended(cosAngle) );
}

FINLINE_Z VecFloat4 VecFloatAngleBetweenVectors4 ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = radianAngle( a.xyzw, b.xyzw )
{
	VecFloat4 a1 = VecFloatMul( VecFloatDot4(a,a), VecFloatDot4(b,b) );
    VecFloat4 cosAngle = VecFloatMul( VecFloatDot4(a,b), VecFloatRSqrt(a1) );
    return VecFloatACos( VecFloatSaturateExtended(cosAngle) );
}

FINLINE_Z VecFloat4 VecFloatLerp ( register const VecFloat4& a, register const VecFloat4& b, const Float scalar )  ///< xyzw = lerp ( a.xyzw, b.xyzw, scalar )
{
	return VecFloatLerp( a, b, VecFloatLoad1(scalar) );
}

FINLINE_Z VecFloat4 VecFloatLerp ( register const VecFloat4& a, register const VecFloat4& b, register const VecFloat4& c )  ///< xyzw = lerp ( a.xyzw, b.xyzw, c )
{
	return VecFloatMadd( VecFloatSub(b,a), c, a );
}

FINLINE_Z VecFloat4 VecFloatQuatSlerp( register const VecFloat4& a, register const VecFloat4& b, register const VecFloat4& t )
{
	const VecFloat4 q1 = a;
	const VecFloat4 vZero = VecFloatSplatZero();
	const VecFloat4 vOne = VecFloatSplatOne();

	VecFloat4 q2 = b, omega, omegaT, cosOmega, sinv, q1Sin1, q2Sin2, invSinOmega, result;

	if ( VecFloatAllEqual(t,vZero) )
		return a;
	else if ( VecFloatAllEqual(t,vOne) )
		return b;

	cosOmega = VecFloatDot4( q1, q2 );
	
	if( VecFloatAllLess(cosOmega,vZero) )
	{
		q2 = VecFloatNegate( q2 );
		cosOmega = VecFloatNegate( cosOmega );
	}

	if( VecFloatAllLess(cosOmega,VectorConstantsPrivate::vSlerpEpsilon) )
	{
		omega = VecFloatACos( cosOmega );
		omegaT = VecFloatMul( t, omega );
		invSinOmega = VecFloatSub( vOne, VecFloatMul(cosOmega,cosOmega) );
		invSinOmega = VecFloatRSqrt( invSinOmega ); // 1 / sin(omega);
		sinv = VecFloatPermuteX0X1Y0Y1( VecFloatSub(omega,omegaT), omegaT );
		sinv = VecFloatSin( sinv ); // sin((1-t)*omega), sin(t*omega)
		q1Sin1 = VecFloatMul( q1, VecFloatSplatX(sinv) );
		q2Sin2 = VecFloatMul( q2, VecFloatSplatY(sinv) );
		return VecFloatMul( VecFloatAdd(q1Sin1,q2Sin2), invSinOmega );
	}
	else // Case where vectors are almost colinear
	{
		result = VecFloatLerp( q1, q2, t );
		return VecFloatQuaternionCNormalize( result );
	}
}

FINLINE_Z VecFloat4 VecFloatAddComponents( register const VecFloat4& a ) ///< xyzw = a.x + a.y + a.z + a.w
{
#if defined (_PC_SSE3 )
	const __m128 result = _mm_hadd_ps( a, a ); // { a.x+a.y, a.z+a.w, a.x+a.y, a.z+a.w }
	return _mm_hadd_ps( result, result );
#elif defined (_PC_SSE )
	VecFloat4 result;
    result = VecFloatAdd( VecFloatSplatX(a), VecFloatSplatY(a) );
    result = VecFloatAdd( VecFloatSplatZ(a), result );
    return VecFloatAdd( VecFloatSplatW(a), result );
#else
	return VecFloatLoad1( a.x + a.y + a.z + a.w );
#endif
}

FINLINE_Z VecFloat4 VecFloatCross3 ( register const VecFloat4& a, register const VecFloat4& b )  ///< xyz = cross ( a.xyz, b.xyz ), w undetermined
{
#if defined (_PC_SSE )
	VecFloat4 aYZXW, bZXYW, aZXYW, bYZXW, result;
	aYZXW = VecFloatSwizzleYZXW( a );
	bZXYW = VecFloatSwizzleZXYW( b );
	aZXYW = VecFloatSwizzleZXYW( a );
	bYZXW = VecFloatSwizzleYZXW( b );
	result = VecFloatSub( VecFloatMul(aYZXW,bZXYW), VecFloatMul(aZXYW,bYZXW) );
	return result;
#else
	const Float aX( a.x ), aY( a.y ), aZ( a.z ), bX( b.x ), bY( b.y ), bZ( b.z );
	VecFloat4 result;
	result.x = aY*bZ - aZ*bY;
	result.y = aZ*bX - aX*bZ;
	result.z = aX*bY - aY*bX;
	#ifdef _DEBUG // pour eviter une exception processeur du fait des proprietes du projet 
	result.w = 12345.6789f;
	#endif
	return result;
#endif
}

FINLINE_Z void VecFloatCross3_SoA ( register VecFloat4* RESTRICT_Z results, register const VecFloat4* RESTRICT_Z a, register const VecFloat4* RESTRICT_Z b )  ///< Same behavior as VecFloatCross3 for 4 3D vectors, but source and destination vectors use a SoA layout : { x0x1x2x3, y0y1y2y3, z0z1z2z3 } and must be different.
{
	results[0] = VecFloatMsub( a[1], b[2], VecFloatMul(a[2],b[1]) );
	results[1] = VecFloatMsub( a[2], b[0], VecFloatMul(a[0],b[2]) );
	results[2] = VecFloatMsub( a[0], b[1], VecFloatMul(a[1],b[0]) );
}

FINLINE_Z VecFloat4 VecFloatScalarDot3 ( register const VecFloat4& a, register const VecFloat4& b )  ///< x = dot(a.xyz,b.xyz), yzw undefined
{
#if defined (_PC_SSE4 )
	return _mm_dp_ps( a, b, 0x7F ); // mask = 1111 1110
#elif defined (_PC_SSE )
	__m128 dot = _mm_mul_ps( a, b );
	return _mm_add_ss( dot, _mm_add_ss( _mm_shuffle_ps(dot,dot,1), _mm_shuffle_ps(dot,dot,2) ) );
#else
	return VecFloatLoad4( a.x*b.x + a.y*b.y + a.z*b.z, 0, 0, 0 );
#endif
}

FINLINE_Z VecFloat4 VecFloatDot2 ( register const VecFloat4& a, register const VecFloat4& b )  ///< xyzw = dot ( a.xy, b.xy )
{
#if defined (_PC_SSE4 )
	return _mm_dp_ps( a, b, 0x3F ); // mask = 1111 1100
#elif defined (_PC_SSE )
	__m128 dot = _mm_mul_ps( a, b );
	dot = _mm_add_ss( dot, _mm_shuffle_ps(dot,dot,1) );
	return _mm_shuffle_ps( dot, dot, 0 );
#else
	return VecFloatLoad1( a.x*b.x + a.y*b.y );
#endif
}

FINLINE_Z VecFloat4 VecFloatDot3 ( register const VecFloat4& a, register const VecFloat4& b )  ///< xyzw = dot ( a.xyz, b.xyz )
{
#if defined (_PC_SSE4 )
	return _mm_dp_ps( a, b, 0x7F ); // mask = 1111 1110
#else
	return VecFloatSplatX( VecFloatScalarDot3(a,b) );
#endif
}

FINLINE_Z VecFloat4 VecFloatDot4 ( register const VecFloat4& a, register const VecFloat4& b )  ///< xyzw = dot ( a.xyzw, b.xyzw )
{
#if defined (_PC_SSE4 )
	return _mm_dp_ps( a, b, 0xFF ); // mask = 1111 1111
#elif defined (_PC_SSE3 )
    __m128 dot = _mm_mul_ps( a, b );
    dot = _mm_hadd_ps( dot, dot );
    return _mm_hadd_ps( dot, dot );
#elif defined (_PC_SSE )
    VecFloat4 dot = _mm_mul_ps( a, b );
    dot = _mm_add_ps( dot, _mm_shuffle_ps( dot, dot, _MM_SHUFFLE(1,0,3,2) ) );
    return _mm_add_ps( dot, _mm_shuffle_ps( dot, dot, _MM_SHUFFLE(0,1,2,3) ) );
#else
    return VecFloatLoad1( a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w );
#endif
}

FINLINE_Z VecFloat4 VecFloatDot3_SoA ( register const VecFloat4* a, register const VecFloat4* b )  ///< Perform a dot product between 4 3D vectors that use a SoA layout : { x0x1x2x3, y0y1y2y3, z0z1z2z3 }.
{
	register VecFloat4 result;
	result = VecFloatMul(  a[2], b[2] );
	result = VecFloatMadd( a[1], b[1], result );
	return   VecFloatMadd( a[0], b[0], result );
}

FINLINE_Z VecFloat4 VecFloatDot4_SoA ( register const VecFloat4* a, register const VecFloat4* b )  ///< Perform a dot product between 4 4D vectors that use a SoA layout : { x0x1x2x3, y0y1y2y3, z0z1z2z3, w0w1w2w3 }.
{
	register VecFloat4 result;
	result = VecFloatMul(  a[3], b[3] );
	result = VecFloatMadd( a[2], b[2], result );
	result = VecFloatMadd( a[1], b[1], result );
	return   VecFloatMadd( a[0], b[0], result );
}

FINLINE_Z VecFloat4 VecFloatDot3x4 ( register const VecFloat4* a, register const VecFloat4& b )  ///< xyzw = { dot(a[0].xyz,b.xyz), dot(a[1].xyz,b.xyz), dot(a[2].xyz,b.xyz), dot(a[3].xyz,b.xyz) }
{
#if defined( _PC_SSE4 ) // Platforms with a dot product intrinsic
	register VecFloat4 results[4];
	for( U32 i = 0; i < 4; ++i )
		results[i] = VecFloatDot3( a[i], b );
	return VecFloatTransposeColumnsX( results );
#else
	register VecFloat4 a_SoA[4], b_SoA[3];
	VecFloatTranspose4SoA( a_SoA, a );
	b_SoA[0] = VecFloatSplatX( b );
	b_SoA[1] = VecFloatSplatY( b );
	b_SoA[2] = VecFloatSplatZ( b );
	return VecFloatDot3_SoA( a_SoA, b_SoA );
#endif
}

FINLINE_Z VecFloat4 VecFloatDot4x4 ( register const VecFloat4* a, register const VecFloat4* b )  ///< xyzw = { dot(a[0],b[0]), dot(a[1],b[1]), dot(a[2],b[2]), dot(a[3],b[3]) }
{
#if defined( _PC_SSE4 ) // Platforms with a dot product intrinsic
	register VecFloat4 results[4];
	for( U32 i = 0; i < 4; ++i )
		results[i] = VecFloatDot4( a[i], b[i] );
	return VecFloatTransposeColumnsX( results );
#else
	register VecFloat4 a_SoA[4], b_SoA[4];
	VecFloatTranspose4SoA( a_SoA, a );
	VecFloatTranspose4SoA( b_SoA, b );
	return VecFloatDot4_SoA( a_SoA, b_SoA );
#endif
}

FINLINE_Z VecFloat4 VecFloatLength3 ( register const VecFloat4& a )  ///< xyzw = length ( a.xyz )
{
	return VecFloatSqrt( VecFloatDot3(a,a) );
}

FINLINE_Z VecFloat4 VecFloatLength4 ( register const VecFloat4& a )  ///< xyzw = length ( a.xyzw )
{
	return VecFloatSqrt( VecFloatDot4(a,a) );
}

FINLINE_Z VecFloat4 VecFloatLength3x3 ( register const VecFloat4* v )  ///< xyzw = { length(v[0].xyz,v[0].xyz), length(v[1].xyz,v[1].xyz), length(v[2].xyz,v[2].xyz), undefined }
{
	return VecFloatSqrt( VecFloatSquaredLength3x3(v) );
}

FINLINE_Z VecFloat4 VecFloatLength3x4 ( register const VecFloat4* v )  ///< xyzw = { length(v[0].xyz,v[0].xyz), length(v[1].xyz,v[1].xyz), length(v[2].xyz,v[2].xyz), length(v[3].xyz,v[3].xyz) }
{
	return VecFloatSqrt( VecFloatSquaredLength3x4(v) );
}

FINLINE_Z VecFloat4 VecFloatLength3_SoA ( register const VecFloat4* source )  ///< Same behavior as VecFloatLength3 for 4 3D vectors, but with a different layout : { x0x1x2x3, y0y1y2y3, z0z1z2z3 }.
{
	return VecFloatSqrt( VecFloatDot3_SoA(source,source) );
}

FINLINE_Z VecFloat4 VecFloatLength4_SoA ( register const VecFloat4* source )  ///< Same behavior as VecFloatLength4 for 4 4D vectors, but with a different layout : { x0x1x2x3, y0y1y2y3, z0z1z2z3, w0w1w2w3 }.
{
	return VecFloatSqrt( VecFloatDot4_SoA(source,source) );
}

FINLINE_Z VecFloat4 VecFloatSquaredLength3 ( register const VecFloat4& a )  ///< xyzw = dot ( a.xyz, a.xyz )
{
	return VecFloatDot3( a, a );
}

FINLINE_Z VecFloat4 VecFloatSquaredLength4 ( register const VecFloat4& a )  ///< xyzw = dot ( a.xyzw, a.xyzw )
{
	return VecFloatDot4( a, a );
}

FINLINE_Z VecFloat4 VecFloatSquaredLength3x3 ( register const VecFloat4* v )  ///< xyzw = { dot(v[0].xyz,v[0].xyz), dot(v[1].xyz,v[1].xyz), dot(v[2].xyz,v[2].xyz), undefined }
{
	register VecFloat4 results[3];
#if defined( _PC_SSE4 ) // Platforms with a dot product intrinsic
	for( U32 i = 0; i < 3; ++i )
		results[i] = VecFloatDot3( v[i], v[i] );
	return VecFloatTransposeColumnsX3( results );
#else
	VecFloatTranspose3SoA( results, v );
	return VecFloatDot3_SoA( results, results );
#endif
}

FINLINE_Z VecFloat4 VecFloatSquaredLength3x4 ( register const VecFloat4* v )  ///< xyzw = { dot(v[0].xyz,v[0].xyz), dot(v[1].xyz,v[1].xyz), dot(v[2].xyz,v[2].xyz), dot(v[3].xyz,v[3].xyz) }
{
	register VecFloat4 results[4];
#if defined( _PC_SSE4 ) // Platforms with a dot product intrinsic
	for( U32 i = 0; i < 4; ++i )
		results[i] = VecFloatDot3( v[i], v[i] );
	return VecFloatTransposeColumnsX( results );
#else
	VecFloatTranspose4SoA( results, v );
	return VecFloatDot3_SoA( results, results );
#endif
}

FINLINE_Z VecFloat4 VecFloatRLength3 ( register const VecFloat4& a, U32 refinements )  ///< xyzw = 1 / length ( a.xyz )
{
	return VecFloatRSqrtAccurate( VecFloatDot3(a,a), refinements );
}

FINLINE_Z VecFloat4 VecFloatRLength4 ( register const VecFloat4& a, U32 refinements )  ///< xyzw = 1 / length ( a.xyzw )
{
	return VecFloatRSqrtAccurate( VecFloatDot4(a,a), refinements );
}

FINLINE_Z VecFloat4 VecFloatSquareDistance3 ( register const VecFloat4& a, register const VecFloat4& b )  ///< xyzw = distance² ( a.xyz, b.xyz )
{
	const VecFloat4 diff = VecFloatSub( a, b );
	return VecFloatDot3( diff, diff );
}

FINLINE_Z VecFloat4 VecFloatDistance3 ( register const VecFloat4& a, register const VecFloat4& b )  ///< xyzw = distance ( a.xyz, b.xyz )
{
	return VecFloatSqrt( VecFloatSquareDistance3(a,b) );
}

FINLINE_Z VecFloat4 VecFloatSquareDistance4 ( register const VecFloat4& a, register const VecFloat4& b )  ///< xyzw = distance² ( a.xyzw, b.xyzw )
{
	const VecFloat4 diff = VecFloatSub( a, b );
	return VecFloatDot4( diff, diff );
}

FINLINE_Z VecFloat4 VecFloatDistance4 ( register const VecFloat4& a, register const VecFloat4& b )  ///< xyzw = distance ( a.xyzw, b.xyzw )
{
	return VecFloatSqrt( VecFloatSquareDistance4(a,b) );
}

FINLINE_Z VecFloat4 VecFloatNormalize3 ( register const VecFloat4& a, U32 refinements )  ///< xyzw = 1 / length(a.xyz). Does not prevent from a division by 0
{
	return VecFloatMul( a, VecFloatRLength3(a,refinements) );
}

FINLINE_Z VecFloat4 VecFloatNormalize4 ( register const VecFloat4& a, U32 refinements )  ///< xyzw = 1 / length(a.xyzw). Does not prevent from a division by 0
{
    return VecFloatMul( a, VecFloatRLength4(a,refinements) );
}

FINLINE_Z VecFloat4 VecFloatCNormalize3 ( register const VecFloat4& a, U32 refinements )  ///< xyzw = 1 / length(a.xyz) with check: when the vector length is almost null, xyzw is set to 0
{
#if defined (_PC_SSE )
	const __m128 zero = _mm_xor_ps( a, a );
	__m128 dot = VecFloatDot3(a,a);
	__m128 res = _mm_mul_ps( a, VecFloatRSqrtAccurate(dot,refinements) );
	return _mm_sel_ps( zero, res, _mm_cmpgt_ps(dot,VectorConstantsPrivate::vEpsSquareF) ); // return dot < epsilon² ? zero : res
#else
	const Float	n = VecFloatGetX( VecFloatScalarDot3(a,a) );
	const Float scale = n > Float_Eps*Float_Eps ? InvSqrt(1.f,n) : 0.f;
	return VecFloatScale( a, scale );
#endif
}

FINLINE_Z VecFloat4 VecFloatCNormalize4 ( register const VecFloat4& a, U32 refinements )  ///< xyzw = 1 / length(a.xyzw) with check: when the vector length is almost null, xyzw is set to 0
{
#if defined (_PC_SSE )
	const __m128 zero = _mm_xor_ps( a, a );
	__m128 dot = VecFloatDot4( a, a );
	__m128 res = _mm_mul_ps( a, VecFloatRSqrtAccurate(dot,refinements) );
	return _mm_sel_ps( zero, res, _mm_cmpgt_ps(dot,VectorConstantsPrivate::vEpsSquareF) ); // return dot < epsilon² ? zero : res
#else
	const Float	n = VecFloatGetX( VecFloatDot4(a,a) );
	const Float scale = n > Float_Eps*Float_Eps ? InvSqrt(1.f,n) : 0.f;
	return VecFloatScale( a, scale );
#endif
}

FINLINE_Z void VecFloatNormalize3_SoA ( register VecFloat4* results, register const VecFloat4* source, U32 refinements )  ///< Same behavior as VecFloatNormalize3 for 4 3D vectors, but with a different layout : { x0x1x2x3, y0y1y2y3, z0z1z2z3 }.
{
	register const VecFloat4 invLength = VecFloatRSqrtAccurate( VecFloatDot3_SoA(source,source), refinements );
	results[0] = VecFloatMul( source[0], invLength );
	results[1] = VecFloatMul( source[1], invLength );
	results[2] = VecFloatMul( source[2], invLength );
}

FINLINE_Z void VecFloatNormalize4_SoA ( register VecFloat4* results, register const VecFloat4* source, U32 refinements )  ///< Same behavior as VecFloatNormalize4 for 4 4D vectors, but with a different layout : { x0x1x2x3, y0y1y2y3, z0z1z2z3, w0w1w2w3 }.
{
	register const VecFloat4 invLength = VecFloatRSqrtAccurate( VecFloatDot4_SoA(source,source), refinements );
	results[0] = VecFloatMul( source[0], invLength );
	results[1] = VecFloatMul( source[1], invLength );
	results[2] = VecFloatMul( source[2], invLength );
	results[3] = VecFloatMul( source[3], invLength );
}

FINLINE_Z VecFloat4 VecFloatStep ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = a.xyzw >= b.xyzw ? 1 : 0, component-wise
{
	return VecFloatSelectGreaterOrEqual( a, b, VecFloatSplatOne(), VecFloatSplatZero() ); 
} 

FINLINE_Z VecFloat4 VecFloatSmoothstep ( register const VecFloat4& min, register const VecFloat4& max, register const VecFloat4& x ) ///< xyzw = smoothstep ( min, max, x ) using Hermite interpolation
{
	register VecFloat4 a, b;
	a = VecFloatReciprocal( VecFloatSub(max,min) );
	a = VecFloatMul( VecFloatSub(x,min), a );
	a = VecFloatSaturate( a );
	b = VecFloatSub( VectorConstantsPrivate::vThreeF, VecFloatAdd(a,a) );
	return VecFloatMul( VecFloatMul(a,a), b );
} 

FINLINE_Z VecFloat4 VecFloatBezier( register const VecFloat4& p1, register const VecFloat4& p2, register const VecFloat4& p3, register const VecFloat4& p4, register const VecFloat4& x ) ///< xyzw = bezier_interpolation( p1, p2, p3, p4, x )
{
	register VecFloat4 result;
	register const VecFloat4 oppX = VecFloatOneComplement( x );
	register const VecFloat4 squareOppX = VecFloatMul( oppX, oppX );
	register const VecFloat4 squareX = VecFloatMul( x, x );
	register const VecFloat4 X3oppX = VecFloatMul( VecFloatMul(x,VectorConstantsPrivate::vThreeF), oppX );
	register const VecFloat4 a = VecFloatMul( squareOppX, oppX );
	register const VecFloat4 b = VecFloatMul( oppX, X3oppX );
	register const VecFloat4 c = VecFloatMul( x, X3oppX );
	register const VecFloat4 d = VecFloatMul( squareX, x );
	result = VecFloatMadd( c, p3, VecFloatMul(d,p4) );
	result = VecFloatMadd( b, p2, result );
	return VecFloatMadd( a, p1, result );
}

FINLINE_Z VecFloat4 VecFloatQuaternionDistance ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = quatDistance ( a, b )
{
	register const VecFloat4 aCos = VecFloatACos( VecFloatDot4(a,b) );
	return VecFloatAdd( aCos, aCos );
}

FINLINE_Z VecFloat4 VecFloatDeterminantM3x3 ( register const VecFloat3x3& mat3x3 ) ///< xyzw = determinant( mat3x3 )
{
    return VecFloatDot3( mat3x3[2], VecFloatCross3(mat3x3[0],mat3x3[1]) );
}

FINLINE_Z VecFloat4 VecFloatDeterminantM4x4 ( register const VecFloat4x4& mat4x4 ) ///< xyzw = determinant( mat4x4 )
{
	register VecFloat4 V0, V1, V2, V3, V4, V5;
    register VecFloat4 P0, P1, P2, R, S;
    static const VecFloat4 sign = VecFloatLoad4( 1.0f, -1.0f, 1.0f, -1.0f );
	register const VecFloat4 m0 = VecFloat4x4GetRow0( mat4x4 );
	register const VecFloat4 m1 = VecFloat4x4GetRow1( mat4x4 );
	register const VecFloat4 m2 = VecFloat4x4GetRow2( mat4x4 );
	register const VecFloat4 m3 = VecFloat4x4GetRow3( mat4x4 );

    V0 = VecFloatSwizzleYXXX( m2 );
    V1 = VecFloatSwizzleZZYY( m3 );
    V2 = VecFloatSwizzleYXXX( m2 );
    V3 = VecFloatSwizzleWWWZ( m3 );
    V4 = VecFloatSwizzleZZYY( m2 );
    V5 = VecFloatSwizzleWWWZ( m3 );

    P0 = VecFloatMul( V0, V1 );
    P1 = VecFloatMul( V2, V3 );
    P2 = VecFloatMul( V4, V5 );

    V0 = VecFloatSwizzleZZYY( m2 );
    V1 = VecFloatSwizzleYXXX( m3 );
    V2 = VecFloatSwizzleWWWZ( m2 );
    V3 = VecFloatSwizzleYXXX( m3 );
    V4 = VecFloatSwizzleWWWZ( m2 );
    V5 = VecFloatSwizzleZZYY( m3 );

    P0 = VecFloatNegMsub( V0, V1, P0 );
    P1 = VecFloatNegMsub( V2, V3, P1 );
    P2 = VecFloatNegMsub( V4, V5, P2 );

    V0 = VecFloatSwizzleWWWZ( m1 );
    V1 = VecFloatSwizzleZZYY( m1 );
    V2 = VecFloatSwizzleYXXX( m1 );

    S = VecFloatMul( m0, sign );
    R = VecFloatMul( V0, P0 );
    R = VecFloatNegMsub( V1, P1, R );
    R = VecFloatMadd( V2, P2, R );

    return VecFloatDot4( S, R );
}

FINLINE_Z VecFloat4 VecFloatReflect3 ( register const VecFloat4& incident, register const VecFloat4& normal ) ///< xyzw = incident.xyzw - 2 * dot(incident.xyz,normal.xyz) * normal.xyzw
{
	VecFloat4 dot = VecFloatDot3( incident, normal );
    return VecFloatNegMsub( VecFloatAdd(dot,dot), normal, incident );
}

FINLINE_Z VecFloat4 VecFloatReflect4 ( register const VecFloat4& incident, register const VecFloat4& normal ) ///< xyzw = incident.xyzw - 2 * dot(incident.xyzw,normal.xyzw) * normal.xyzw
{
	VecFloat4 dot = VecFloatDot4( incident, normal );
    return VecFloatNegMsub( VecFloatAdd(dot,dot), normal, incident );
}

FINLINE_Z VecFloat4 VecFloat3x3Transform ( register const VecFloat3x3& mat3x3, register const VecFloat4& v ) ///< xyz = mul( mat3x3, v.xyz ), w = undefined
{
	VecFloat4 res;
	res =  VecFloatMul(  mat3x3[2], VecFloatSplatZ(v) );
	res =  VecFloatMadd( mat3x3[1], VecFloatSplatY(v), res );
	return VecFloatMadd( mat3x3[0], VecFloatSplatX(v), res );
}

FINLINE_Z void VecFloat3x3Scale( register VecFloat3x3& result, register const VecFloat3x3& m0, const Float scalar ) ///< result = m0 * scalar
{
	register const VecFloat4 vScalar = VecFloatLoad1( scalar );
	result[0] = VecFloatMul( m0[0], vScalar );
    result[1] = VecFloatMul( m0[1], vScalar );
    result[2] = VecFloatMul( m0[2], vScalar );
}

FINLINE_Z void VecFloat3x3Multiply ( register VecFloat3x3& result, register const VecFloat3x3& m1, register const VecFloat3x3& m2 ) ///< result = mul( m1, m2 ). w is also altered
{
	VecFloat3x3 temp;
    VecFloat3x3MultiplyN( &temp, &m1, m2, 1 );
	VecFloat3x3Copy( result, temp );
}

FINLINE_Z void VecFloat3x3MultiplyN ( register VecFloat3x3* RESTRICT_Z results, register const VecFloat3x3* RESTRICT_Z m, register const VecFloat3x3& n, const U32 count ) ///< results[i] = mul( m[i], n ). w is also altered. Source and destination matrices must be different.
{
	register const VecFloat4 n0z = VecFloatSplatZ( n[0] );
	register const VecFloat4 n0y = VecFloatSplatY( n[0] );
	register const VecFloat4 n0x = VecFloatSplatX( n[0] );
	register const VecFloat4 n1z = VecFloatSplatZ( n[1] );
	register const VecFloat4 n1y = VecFloatSplatY( n[1] );
	register const VecFloat4 n1x = VecFloatSplatX( n[1] );
	register const VecFloat4 n2z = VecFloatSplatZ( n[2] );
	register const VecFloat4 n2y = VecFloatSplatY( n[2] );
	register const VecFloat4 n2x = VecFloatSplatX( n[2] );

	for( U32 i = 0; i < count; ++i )
	{
		const VecFloat3x3& m_M( m[i] );
		VecFloat3x3& res_M( results[i] );

		res_M[0] = VecFloatMul ( m_M[2], n0z );
		res_M[0] = VecFloatMadd( m_M[1], n0y, res_M[0] );
		res_M[0] = VecFloatMadd( m_M[0], n0x, res_M[0] );

		res_M[1] = VecFloatMul ( m_M[2], n1z );
		res_M[1] = VecFloatMadd( m_M[1], n1y, res_M[1] );
		res_M[1] = VecFloatMadd( m_M[0], n1x, res_M[1] );

		res_M[2] = VecFloatMul ( m_M[2], n2z );
		res_M[2] = VecFloatMadd( m_M[1], n2y, res_M[2] );
		res_M[2] = VecFloatMadd( m_M[0], n2x, res_M[2] );
	}
}

FINLINE_Z void VecFloat3x3Transpose ( register VecFloat3x3& transposed, register const VecFloat3x3& mat3x3 ) ///< result = transpose( mat3x3 ). w is also altered
{
    VecFloat4 temp0, temp1;
    temp0 = VecFloatPermuteX0X1Y0Y1 ( mat3x3[0], mat3x3[1] );     // { x0, y0, x1, y1 }
    temp1 = VecFloatPermuteZ0Z1W0W1 ( mat3x3[0], mat3x3[1] );     // { x2, y2, x3, y3 }
    transposed[0] = VecFloatPermuteX0Y0X1Y1( temp0, mat3x3[2] );  // { x0, y0, z0, # }
    transposed[1] = VecFloatPermuteZ0W0Y1Y1( temp0, mat3x3[2] );  // { x1, y1, z1, # }
    transposed[2] = VecFloatPermuteX0Y0Z1W1( temp1, mat3x3[2] );  // { x2, y2, z2, # }
}

FINLINE_Z void VecFloat3x3MultiplyTransposeN ( register VecFloat3x3* RESTRICT_Z results, register const VecFloat3x3* RESTRICT_Z m, register const VecFloat3x3& n, const U32 count ) ///< results[i] = mul( transpose(m[i]), transpose(n) ). w is also altered. Source and destination matrices must be different.
{
	register VecFloat3x3 transposedN;
	
	VecFloat3x3Transpose( transposedN, n );	

	for( U32 i = 0; i < count; ++i )
	{
		const VecFloat3x3& m_M( m[i] );
		VecFloat3x3& res_M( results[i] );
		
		res_M[0] =	VecFloatMul(  VecFloatSplatX(m_M[2]), transposedN[2] );
		res_M[0] =	VecFloatMadd( VecFloatSplatX(m_M[1]), transposedN[1], res_M[0] );
		res_M[0] =	VecFloatMadd( VecFloatSplatX(m_M[0]), transposedN[0], res_M[0] );

		res_M[1] =	VecFloatMul(  VecFloatSplatY(m_M[2]), transposedN[2] );
		res_M[1] =	VecFloatMadd( VecFloatSplatY(m_M[1]), transposedN[1], res_M[1] );
		res_M[1] =	VecFloatMadd( VecFloatSplatY(m_M[0]), transposedN[0], res_M[1] );
	
		res_M[2] =	VecFloatMul(  VecFloatSplatZ(m_M[2]), transposedN[2] );
		res_M[2] =	VecFloatMadd( VecFloatSplatZ(m_M[1]), transposedN[1], res_M[2] );
		res_M[2] =	VecFloatMadd( VecFloatSplatZ(m_M[0]), transposedN[0], res_M[2] );
	}
}

FINLINE_Z void VecFloat3x3MultiplyTranspose ( register VecFloat3x3& result, register const VecFloat3x3& m1, register const VecFloat3x3& m2 ) ///< result = mul( transpose(m1), transpose(m2) ). w is also altered
{
	VecFloat3x3 temp;
	VecFloat3x3MultiplyTransposeN( &temp, &m1, m2, 1 );
	VecFloat3x3Copy( result, temp );
}

FINLINE_Z void VecFloat3x3Inverse ( register VecFloat3x3& inverse, register const VecFloat3x3& mat3x3 ) ///< result = inverse( mat3x3 ). Check for null determinants.
{
    VecFloat4 det, z1z0z0z1, y2y2y1y1, y1y0y0y1, z2z2z1z1, x2x2x1x1, x1x0x0x1;
    det = VecFloatDot3( mat3x3[2], VecFloatCross3(mat3x3[0],mat3x3[1]) );
    if( VecFloatAllNotEqual( VecFloatSplatZero(), det ) )
	{
		/*	inverse[0].x =  (mat3x3[1].y * mat3x3[2].z - mat3x3[2].y * mat3x3[1].z);
			inverse[0].y = -(mat3x3[0].y * mat3x3[2].z - mat3x3[2].y * mat3x3[0].z);
			inverse[0].z =  (mat3x3[0].y * mat3x3[1].z - mat3x3[1].y * mat3x3[0].z);

			inverse[1].x = -(mat3x3[1].x * mat3x3[2].z - mat3x3[2].x * mat3x3[1].z);
			inverse[1].y =  (mat3x3[0].x * mat3x3[2].z - mat3x3[2].x * mat3x3[0].z);
			inverse[1].z = -(mat3x3[0].x * mat3x3[1].z - mat3x3[1].x * mat3x3[0].z);

			inverse[2].x =  (mat3x3[1].x * mat3x3[2].y - mat3x3[2].x * mat3x3[1].y);
			inverse[2].y = -(mat3x3[0].x * mat3x3[2].y - mat3x3[2].x * mat3x3[0].y);
			inverse[2].z =  (mat3x3[0].x * mat3x3[1].y - mat3x3[1].x * mat3x3[0].y); */

		static const VecFloat4 signMask = { 1, -1, 1, -1 };

		z1z0z0z1 = VecFloatPermuteZ0Z1Z1Z0( mat3x3[1], mat3x3[0] );
		y2y2y1y1 = VecFloatPermuteY0Y0Y1Y1( mat3x3[2], mat3x3[1] );
		y1y0y0y1 = VecFloatPermuteY0Y1Y1Y0( mat3x3[1], mat3x3[0] );
		z2z2z1z1 = VecFloatPermuteZ0Z0Z1Z1( mat3x3[2], mat3x3[1] );
		x2x2x1x1 = VecFloatPermuteX0X0X1X1( mat3x3[2], mat3x3[1] );
		x1x0x0x1 = VecFloatPermuteX0X1X1X0( mat3x3[1], mat3x3[0] );

		inverse[0] = VecFloatMul( y2y2y1y1, z1z0z0z1 );
		inverse[0] = VecFloatMsub( y1y0y0y1, z2z2z1z1, inverse[0] );
		inverse[0] = VecFloatMul( inverse[0], signMask );

		inverse[1] = VecFloatMul( x2x2x1x1, z1z0z0z1 );
		inverse[1] = VecFloatNegMsub( x1x0x0x1, z2z2z1z1, inverse[1] );
		inverse[1] = VecFloatMul( inverse[1], signMask );

		inverse[2] = VecFloatMul( x2x2x1x1, y1y0y0y1 );
		inverse[2] = VecFloatMsub( x1x0x0x1, y2y2y1y1, inverse[2] );
		inverse[2] = VecFloatMul( inverse[2], signMask );
    
		det = VecFloatReciprocalAccurate( det, 1 );
		inverse[0] = VecFloatMul( inverse[0], det );
		inverse[1] = VecFloatMul( inverse[1], det );
		inverse[2] = VecFloatMul( inverse[2], det );
	}
}

FINLINE_Z VecFloat4 VecFloat4x4TransformH3 ( register const VecFloat4x4& mat4x4, register const VecFloat4& v ) ///< Homogeneous transform: xyzw = mul( mat4x4, float4(v.xyz,1) ); xyz /= w;
{
	register const VecFloat4 vT = VecFloat4x4Transform3( mat4x4, v );
	register const VecFloat4 w = VecFloatSplatW( vT );
	register VecFloat4 result = VecFloatMul( vT, VecFloatReciprocal(w) );
	return VecFloatPermuteX0Y0Z0W1( result, w );
}

FINLINE_Z VecFloat4 VecFloat4x4Transform3 ( register const VecFloat4x4& mat4x4, register const VecFloat4& v ) ///< xyzw = mul( mat4x4, float4(v.xyz,1) )
{
	register VecFloat4 tmp0;
	tmp0 = VecFloatMadd( mat4x4[2], VecFloatSplatZ(v), mat4x4[3] );
    tmp0 = VecFloatMadd( mat4x4[1], VecFloatSplatY(v), tmp0 );
    return VecFloatMadd( mat4x4[0], VecFloatSplatX(v), tmp0 );
}

FINLINE_Z VecFloat4 VecFloat4x4Transform4 ( register const VecFloat4x4& mat4x4, register const VecFloat4& v ) ///< xyzw = mul( mat4x4, v.xyzw )
{
	register VecFloat4 tmp0;
	const VecFloat4 xxxx = VecFloatSplatX ( v );
	const VecFloat4 yyyy = VecFloatSplatY ( v );
	const VecFloat4 zzzz = VecFloatSplatZ ( v );
	const VecFloat4 wwww = VecFloatSplatW ( v );
    tmp0 = VecFloatMul(  mat4x4[0], xxxx );
    tmp0 = VecFloatMadd( mat4x4[1], yyyy, tmp0 );
    tmp0 = VecFloatMadd( mat4x4[2], zzzz, tmp0 );
    return VecFloatMadd( mat4x4[3], wwww, tmp0 );
}

FINLINE_Z void VecFloat4x4TransformH3_SoA ( register VecFloat4* RESTRICT_Z results, register const VecFloat4x4& mat4x4, register const VecFloat4* RESTRICT_Z v, const U32 groups ) ///< Same behavior as VecFloat4x4TransformH3 but vectors have an SoA layout. Source vectors must be in groups of 3 : { x0x1x2x3, y0y1y2y3, z0z1z2z3 }, destination vectors in groups of 4 : { x0x1x2x3, y0y1y2y3, z0z1z2z3, w0w1w2w3 }.
{
	VecFloat4x4Transform3_SoA( results, mat4x4, v, groups );
	for( U32 i = 0; i < groups; ++i, results += 4 )
	{
		VecFloat4 inverseW = VecFloatReciprocal( results[3] );
		results[0] = VecFloatMul( results[0], inverseW );
		results[1] = VecFloatMul( results[1], inverseW );
		results[2] = VecFloatMul( results[2], inverseW );
	}
}

FINLINE_Z void VecFloat4x4Transform3_SoA ( register VecFloat4* RESTRICT_Z results, register const VecFloat4x4& mat4x4, register const VecFloat4* RESTRICT_Z v, const U32 groups ) ///< Same behavior as VecFloat4x4Transform3 but vectors have an SoA layout. Source vectors must be in groups of 3 : { x0x1x2x3, y0y1y2y3, z0z1z2z3 }, destination vectors in groups of 4 : { x0x1x2x3, y0y1y2y3, z0z1z2z3, w0w1w2w3 }.
{
	register const VecFloat4 row0 = VecFloat4x4GetRow0( mat4x4 );
	register const VecFloat4 row1 = VecFloat4x4GetRow1( mat4x4 );
	register const VecFloat4 row2 = VecFloat4x4GetRow2( mat4x4 );
	register const VecFloat4 row3 = VecFloat4x4GetRow3( mat4x4 );
	register const VecFloat4 m00 = VecFloatSplatX( row0 );
	register const VecFloat4 m01 = VecFloatSplatY( row0 );
	register const VecFloat4 m02 = VecFloatSplatZ( row0 );
	register const VecFloat4 m03 = VecFloatSplatW( row0 );
	register const VecFloat4 m10 = VecFloatSplatX( row1 );
	register const VecFloat4 m11 = VecFloatSplatY( row1 );
	register const VecFloat4 m12 = VecFloatSplatZ( row1 );
	register const VecFloat4 m13 = VecFloatSplatW( row1 );
	register const VecFloat4 m20 = VecFloatSplatX( row2 );
	register const VecFloat4 m21 = VecFloatSplatY( row2 );
	register const VecFloat4 m22 = VecFloatSplatZ( row2 );
	register const VecFloat4 m23 = VecFloatSplatW( row2 );
	register const VecFloat4 m30 = VecFloatSplatX( row3 );
	register const VecFloat4 m31 = VecFloatSplatY( row3 );
	register const VecFloat4 m32 = VecFloatSplatZ( row3 );
	register const VecFloat4 m33 = VecFloatSplatW( row3 );

	for( U32 i = 0; i < groups; ++i, v += 3, results += 4 )
	{
		results[0] = VecFloatMadd( m20, v[2], m30 );
		results[1] = VecFloatMadd( m21, v[2], m31 );
		results[2] = VecFloatMadd( m22, v[2], m32 );
		results[3] = VecFloatMadd( m23, v[2], m33 );

		results[0] = VecFloatMadd( m10, v[1], results[0] );
		results[1] = VecFloatMadd( m11, v[1], results[1] );
		results[2] = VecFloatMadd( m12, v[1], results[2] );
		results[3] = VecFloatMadd( m13, v[1], results[3] );

		results[0] = VecFloatMadd( m00, v[0], results[0] );
		results[1] = VecFloatMadd( m01, v[0], results[1] );	
		results[2] = VecFloatMadd( m02, v[0], results[2] );
		results[3] = VecFloatMadd( m03, v[0], results[3] );
	}
}

FINLINE_Z void VecFloat4x4Transform4_SoA ( register VecFloat4* RESTRICT_Z results, register const VecFloat4x4& mat4x4, register const VecFloat4* RESTRICT_Z v, const U32 groups ) ///< Transform groups of 4 4D vectors by a 4x4 matrix. Vectors use a SoA layout : { x0x1x2x3, y0y1y2y3, z0z1z2z3, w0w1w2w3 }. Source and destination must be different.
{
	register const VecFloat4 row0 = VecFloat4x4GetRow0( mat4x4 );
	register const VecFloat4 row1 = VecFloat4x4GetRow1( mat4x4 );
	register const VecFloat4 row2 = VecFloat4x4GetRow2( mat4x4 );
	register const VecFloat4 row3 = VecFloat4x4GetRow3( mat4x4 );
	register const VecFloat4 m00 = VecFloatSplatX( row0 );
	register const VecFloat4 m01 = VecFloatSplatY( row0 );
	register const VecFloat4 m02 = VecFloatSplatZ( row0 );
	register const VecFloat4 m03 = VecFloatSplatW( row0 );
	register const VecFloat4 m10 = VecFloatSplatX( row1 );
	register const VecFloat4 m11 = VecFloatSplatY( row1 );
	register const VecFloat4 m12 = VecFloatSplatZ( row1 );
	register const VecFloat4 m13 = VecFloatSplatW( row1 );
	register const VecFloat4 m20 = VecFloatSplatX( row2 );
	register const VecFloat4 m21 = VecFloatSplatY( row2 );
	register const VecFloat4 m22 = VecFloatSplatZ( row2 );
	register const VecFloat4 m23 = VecFloatSplatW( row2 );
	register const VecFloat4 m30 = VecFloatSplatX( row3 );
	register const VecFloat4 m31 = VecFloatSplatY( row3 );
	register const VecFloat4 m32 = VecFloatSplatZ( row3 );
	register const VecFloat4 m33 = VecFloatSplatW( row3 );

	for( U32 i = 0; i < groups; ++i, v += 3, results += 4 )
	{
		results[0] = VecFloatMul( m30, v[3] );
		results[1] = VecFloatMul( m31, v[3] );
		results[2] = VecFloatMul( m32, v[3] );
		results[3] = VecFloatMul( m33, v[3] );

		results[0] = VecFloatMadd( m20, v[2], results[0] );
		results[1] = VecFloatMadd( m21, v[2], results[1] );
		results[2] = VecFloatMadd( m22, v[2], results[2] );
		results[3] = VecFloatMadd( m23, v[2], results[3] );

		results[0] = VecFloatMadd( m10, v[1], results[0] );
		results[1] = VecFloatMadd( m11, v[1], results[1] );
		results[2] = VecFloatMadd( m12, v[1], results[2] );
		results[3] = VecFloatMadd( m13, v[1], results[3] );

		results[0] = VecFloatMadd( m00, v[0], results[0] );
		results[1] = VecFloatMadd( m01, v[0], results[1] );
		results[2] = VecFloatMadd( m02, v[0], results[2] );
		results[3] = VecFloatMadd( m03, v[0], results[3] );
	}
}

FINLINE_Z void VecFloat4x4Scale( register VecFloat4x4& result, register const VecFloat4x4& m0, const Float scalar ) ///< result = m0 * scalar
{
	register const VecFloat4 vScalar = VecFloatLoad1( scalar );
	result[0] = VecFloatMul( m0[0], vScalar );
    result[1] = VecFloatMul( m0[1], vScalar );
    result[2] = VecFloatMul( m0[2], vScalar );
    result[3] = VecFloatMul( m0[3], vScalar );
}

FINLINE_Z void VecFloat4x4Add ( register VecFloat4x4& result, register const VecFloat4x4& m1, register const VecFloat4x4& m2 ) ///< result = m2 * m1 -> ex: VecFloat4x4Multiply(proj, worldToView)
{
	for( U32 i = 0; i < 4; ++i )
		result[i] = VecFloatAdd( m1[i], m2[i] );
}

FINLINE_Z void VecFloat4x4Multiply ( register VecFloat4x4& result, register const VecFloat4x4& m1, register const VecFloat4x4& m2 ) ///< result = m2 * m1 -> ex: VecFloat4x4Multiply(proj, worldToView)
{
	VecFloat4x4 temp;
	VecFloat4x4MultiplyN( &temp, &m1, m2, 1 );
	VecFloat4x4Copy( result, temp );
}

FINLINE_Z void VecFloat4x4MultiplyN ( register VecFloat4x4* RESTRICT_Z results, register const VecFloat4x4* RESTRICT_Z m, register const VecFloat4x4& n, const U32 count ) ///< results[i] = mul( n, m[i] ). Source and destination matrices must be different.
{
	register const VecFloat4 n0w = VecFloatSplatW( n[0] );
	register const VecFloat4 n0z = VecFloatSplatZ( n[0] );
	register const VecFloat4 n0y = VecFloatSplatY( n[0] );
	register const VecFloat4 n0x = VecFloatSplatX( n[0] );
	register const VecFloat4 n1w = VecFloatSplatW( n[1] );
	register const VecFloat4 n1z = VecFloatSplatZ( n[1] );
	register const VecFloat4 n1y = VecFloatSplatY( n[1] );
	register const VecFloat4 n1x = VecFloatSplatX( n[1] );
	register const VecFloat4 n2w = VecFloatSplatW( n[2] );
	register const VecFloat4 n2z = VecFloatSplatZ( n[2] );
	register const VecFloat4 n2y = VecFloatSplatY( n[2] );
	register const VecFloat4 n2x = VecFloatSplatX( n[2] );
	register const VecFloat4 n3w = VecFloatSplatW( n[3] );
	register const VecFloat4 n3z = VecFloatSplatZ( n[3] );
	register const VecFloat4 n3y = VecFloatSplatY( n[3] );
	register const VecFloat4 n3x = VecFloatSplatX( n[3] );

	for( U32 i = 0; i < count; ++i )
	{
		const VecFloat4x4& m_M( m[i] );
		VecFloat4x4& res_M( results[i] );

		res_M[0] = VecFloatMul(  m_M[3], n0w );
		res_M[0] = VecFloatMadd( m_M[2], n0z, res_M[0] );
		res_M[0] = VecFloatMadd( m_M[1], n0y, res_M[0] );
		res_M[0] = VecFloatMadd( m_M[0], n0x, res_M[0] );
										 
		res_M[1] = VecFloatMul(  m_M[3], n1w );
		res_M[1] = VecFloatMadd( m_M[2], n1z, res_M[1] );
		res_M[1] = VecFloatMadd( m_M[1], n1y, res_M[1] );
		res_M[1] = VecFloatMadd( m_M[0], n1x, res_M[1] );
										 
		res_M[2] = VecFloatMul(  m_M[3], n2w );
		res_M[2] = VecFloatMadd( m_M[2], n2z, res_M[2] );
		res_M[2] = VecFloatMadd( m_M[1], n2y, res_M[2] );
		res_M[2] = VecFloatMadd( m_M[0], n2x, res_M[2] );
										 
		res_M[3] = VecFloatMul(  m_M[3], n3w ); 
		res_M[3] = VecFloatMadd( m_M[2], n3z, res_M[3] );
		res_M[3] = VecFloatMadd( m_M[1], n3y, res_M[3] );
		res_M[3] = VecFloatMadd( m_M[0], n3x, res_M[3] );
	}												
}

FINLINE_Z void VecFloat4x4Transpose ( register VecFloat4x4& transposed, register const VecFloat4x4& mat4x4 ) ///< result = transpose( mat4x4 )
{
#if defined (_PC_SSE )
	VecFloat4 tmp0, tmp1, tmp2, tmp3;
	tmp0 = _mm_shuffle_ps( mat4x4[0], mat4x4[1], 0x44 );
    tmp2 = _mm_shuffle_ps( mat4x4[0], mat4x4[1], 0xEE );
    tmp1 = _mm_shuffle_ps( mat4x4[2], mat4x4[3], 0x44 );
    tmp3 = _mm_shuffle_ps( mat4x4[2], mat4x4[3], 0xEE );
    transposed[0] = _mm_shuffle_ps( tmp0, tmp1, 0x88 );
    transposed[1] = _mm_shuffle_ps( tmp0, tmp1, 0xDD );
    transposed[2] = _mm_shuffle_ps( tmp2, tmp3, 0x88 );
    transposed[3] = _mm_shuffle_ps( tmp2, tmp3, 0xDD );
#else
	VecFloat4x4 P;
    P[0] = VecFloatPermuteX0X1Y0Y1( mat4x4[0], mat4x4[2] );
    P[1] = VecFloatPermuteX0X1Y0Y1( mat4x4[1], mat4x4[3] );
    P[2] = VecFloatPermuteZ0Z1W0W1( mat4x4[0], mat4x4[2] );
    P[3] = VecFloatPermuteZ0Z1W0W1( mat4x4[1], mat4x4[3] );
    transposed[0] = VecFloatPermuteX0X1Y0Y1( P[0], P[1] );
    transposed[1] = VecFloatPermuteZ0Z1W0W1( P[0], P[1] );
    transposed[2] = VecFloatPermuteX0X1Y0Y1( P[2], P[3] );
    transposed[3] = VecFloatPermuteZ0Z1W0W1( P[2], P[3] );
#endif
}

FINLINE_Z void VecFloat4x4MultiplyTranspose ( register VecFloat4x4& result, register const VecFloat4x4& m1, register const VecFloat4x4& m2 ) ///< result = transpose( mul(m1,m2) )
{
	VecFloat4x4Multiply( result, m1, m2 );
    VecFloat4x4Transpose( result, result );
}

FINLINE_Z void VecFloat4x4Inverse ( register VecFloat4x4& result, register const VecFloat4x4& m0 ) ///< result = inverse( m0 ), source and destination must be different
{
	// Code adapted from XMMatrixInverse
	VecFloat4x4	MT;
	VecFloat4 C0, C1, C2, C3, C4, C5, C6, C7, V00, V10, V01, V11, V02, V12, V03, V13, D0, D1, D2, vTemp;

	VecFloat4x4Transpose( MT, m0 );

	const VecFloat4 MT0 = VecFloat4x4GetRow0( MT );
	const VecFloat4 MT1 = VecFloat4x4GetRow1( MT );
	const VecFloat4 MT2 = VecFloat4x4GetRow2( MT );
	const VecFloat4 MT3 = VecFloat4x4GetRow3( MT );

	V00 = VecFloatSwizzleXXYY( MT2 );
	V10 = VecFloatSwizzleZWZW( MT3 );
	V01 = VecFloatSwizzleXXYY( MT0 );
	V11 = VecFloatSwizzleZWZW( MT1 );
	V02 = VecFloatPermuteX0Z0X1Z1( MT2, MT0 );
	V12 = VecFloatPermuteY0W0Y1W1( MT3, MT1 );

	D0 = VecFloatMul( V00, V10 );
	D1 = VecFloatMul( V01, V11 );
	D2 = VecFloatMul( V02, V12 );

	V00 = VecFloatSwizzleZWZW( MT2 );
	V10 = VecFloatSwizzleXXYY( MT3 );
	V01 = VecFloatSwizzleZWZW( MT0 );
	V11 = VecFloatSwizzleXXYY( MT1 );
	V02 = VecFloatPermuteY0W0Y1W1( MT2, MT0 );
	V12 = VecFloatPermuteX0Z0X1Z1( MT3, MT1 );

	V00 = VecFloatMul( V00, V10 );
	V01 = VecFloatMul( V01, V11 );
	V02 = VecFloatMul( V02, V12 );
	D0 = VecFloatSub( D0, V00 );
	D1 = VecFloatSub( D1, V01 );
	D2 = VecFloatSub( D2, V02 );

	V11 = VecFloatPermuteY0W0Y1Y1( D0, D2 );
	V00 = VecFloatSwizzleYZXY( MT1 ); 
	V10 = VecFloatPermuteZ0X0W1X1( V11, D0 ); 
	V01 = VecFloatSwizzleZXYX( MT0 );
	V11 = VecFloatPermuteY0Z0Y1Z1( V11, D0 );

	V13 = VecFloatPermuteY0W0W1W1( D1, D2 ); 
	V02 = VecFloatSwizzleYZXY( MT3 );
	V12 = VecFloatPermuteZ0X0W1X1( V13, D1 );
	V03 = VecFloatSwizzleZXYX( MT2 );
	V13 = VecFloatPermuteY0Z0Y1Z1( V13, D1 );

	C0 = VecFloatMul( V00, V10 );
	C2 = VecFloatMul( V01, V11 );
	C4 = VecFloatMul( V02, V12 );
	C6 = VecFloatMul( V03, V13 );

	V11 = VecFloatPermuteX0Y0X1X1( D0, D2 );
	V00 = VecFloatSwizzleZWYZ( MT1 );
	V10 = VecFloatPermuteW0X0Y1Z1( D0, V11 );
	V01 = VecFloatSwizzleWZWY( MT0 );
	V11 = VecFloatPermuteZ0Y0Z1X1( D0, V11 );

	V13 = VecFloatPermuteX0Y0Z1Z1( D1, D2 );
	V02 = VecFloatSwizzleZWYZ( MT3 );
	V12 = VecFloatPermuteW0X0Y1Z1( D1, V13 );
	V03 = VecFloatSwizzleWZWY( MT2 );
	V13 = VecFloatPermuteZ0Y0Z1X1( D1, V13 );

	V00 = VecFloatMul( V00, V10 );
	V01 = VecFloatMul( V01, V11 );
	V02 = VecFloatMul( V02, V12 );
	V03 = VecFloatMul( V03, V13 );
	C0 = VecFloatSub( C0, V00 );
	C2 = VecFloatSub( C2, V01 );
	C4 = VecFloatSub( C4, V02 );
	C6 = VecFloatSub( C6, V03 );

	V00 = VecFloatSwizzleWXWX( MT1 );
	V10 = VecFloatPermuteZ0Y1X1Z0( D0, D2 );
	V01 = VecFloatSwizzleYWXZ( MT0 );
	V11 = VecFloatPermuteY0X1W1X0( D2, D0 );
	V02 = VecFloatSwizzleWXWX( MT3 );
	V12 = VecFloatPermuteZ0W1Z1Z0( D1, D2 );
	V03 = VecFloatSwizzleYWXZ( MT2 );
	V13 = VecFloatPermuteW0X1W1Z0( D2, D1 );

	V00 = VecFloatMul( V00, V10 );
	V01 = VecFloatMul( V01, V11 );
	V02 = VecFloatMul( V02, V12 );
	V03 = VecFloatMul( V03, V13 );
	C1 = VecFloatSub( C0, V00 );
	C0 = VecFloatAdd( C0, V00 );
	C3 = VecFloatAdd( C2, V01 );
	C2 = VecFloatSub( C2, V01 );
	C5 = VecFloatSub( C4, V02 );
	C4 = VecFloatAdd( C4, V02 );
	C7 = VecFloatAdd( C6, V03 );
	C6 = VecFloatSub( C6, V03 );

	C0 = VecFloatPermuteX0Y1Z0W1( C0, C1 );
	C2 = VecFloatPermuteX0Y1Z0W1( C2, C3 );
	C4 = VecFloatPermuteX0Y1Z0W1( C4, C5 );
	C6 = VecFloatPermuteX0Y1Z0W1( C6, C7 );

	// Get the determinate
	vTemp = VecFloatReciprocal( VecFloatDot4(C0,MT0) );

	result[0] = VecFloatMul( C0, vTemp );
	result[1] = VecFloatMul( C2, vTemp );
	result[2] = VecFloatMul( C4, vTemp );
	result[3] = VecFloatMul( C6, vTemp );
}

FINLINE_Z void VecFloatBuildOrthoBase3 ( const register VecFloat4& v, register VecFloat4& n1, register VecFloat4& n2 ) ///< Build a 2D orthogonal reference from a vector
{
	register const VecFloat4 posY = VecFloatAbs( VecFloatSplatY(v) );
	register const VecFloat4 negativeV = VecFloatNegate( v );
    register const VecFloat4 zero = VecFloatSplatZero();
	register const VecFloat4 z0w0 = VecFloatPermuteZ0Z1W0W1( v, zero );
	register const VecFloat4 fail = VecFloatPermuteX0Y0X1Y1( z0w0, negativeV );    // { z, 0, -x, -y }
	register const VecFloat4 success = VecFloatPermuteY0X0Y1X1( z0w0, negativeV ); // { 0, z, -y, -x }
	n1 = VecFloatSelectLess( VectorConstantsPrivate::v099F, posY, success, fail );
	n1 = VecFloatNormalize3( n1 );
	n2 = VecFloatNormalize3( VecFloatCross3(n1,v) );
}

FINLINE_Z void VecFloatOrthonormalize3 ( register const VecFloat4& n, register VecFloat4& t, register VecFloat4& b )  ///< Gram-Schmidt orthonormalization. n must be normalized, t and b are inout parameters
{
	register VecFloat4 NdotT, BdotN, BdotT;
    register VecFloat4 tmp0; 

    NdotT = VecFloatDot3( t, n );
    t = VecFloatNegMsub( n, NdotT, t );

    BdotN = VecFloatDot3( b, n );
    BdotT = VecFloatDot3( b, t );
    tmp0 = VecFloatAdd ( VecFloatMul(n,BdotN), VecFloatMul(t,BdotT) );
    b = VecFloatSub( b, tmp0 );

	t = VecFloatCNormalize3( t );
    b = VecFloatCNormalize3( b );
}

FINLINE_Z VecFloat4 VecFloatQuaternionGetAxisAngle ( register const VecFloat4& q ) ///< xyzw = float4( normalize(q.xyz), angle(q) ), with angle expressed in radians
{
	register const VecFloat4 w = VecFloatSaturateExtended( VecFloatSplatW(q) );
	register const VecFloat4 s = VecFloatSqrt( VecFloatOneComplement( VecFloatMul(w,w) ) );
	register const VecFloat4 axis = VecFloatCNormalize3( q );
	if( VecFloatAllGreater(s,VectorConstantsPrivate::vEpsF) )
	{
		register const VecFloat4 aCosW = VecFloatACos( w );
		return VecFloatPermuteX0Y0Z0W1( axis, VecFloatAdd(aCosW,aCosW) );
	}
	return VecFloatSetWZero( axis );
}

FINLINE_Z VecFloat4 VecFloatQuaternionRotate ( register const VecFloat4& v, register const VecFloat4& q ) ///< xyz = rotate( v.xyz, q ), w undefined
{
	VecFloat4 result, r, w, qZXYW, qYZXW, qWWWW;

	qZXYW = VecFloatSwizzleZXYW( q );
	qYZXW = VecFloatSwizzleYZXW( q );
	qWWWW = VecFloatSplatW( q );

	// r = q.www * v.xyz + q.yzx * v.zxy - q.zxy * v.yzx;
	// w = -dot( q.xyz, v.xyz );
	r = VecFloatMul( qZXYW, VecFloatSwizzleYZXW(v) );
	r = VecFloatMsub( qYZXW, VecFloatSwizzleZXYW(v), r );
	r = VecFloatMadd( qWWWW, v, r );
	w = VecFloatDot3( q, v );

	// result = -w.xyz * q.xyz + r.xyz * q.www - r.yzx * q.zxy + r.zxy * q.yzx;
	result = VecFloatMul( VecFloatSwizzleZXYW(r), qYZXW );
	result = VecFloatMadd( VecFloatSwizzleYZXW(r), VecFloatNegate(qZXYW), result );
	result = VecFloatMadd( r, qWWWW, result );
	return VecFloatMadd( w, q, result );
}

FINLINE_Z VecFloat4 VecFloatQuaternionBuildAngleAxis ( const Float angle, register const VecFloat4& normalizedAxis ) ///< Create a quaternion based on an angle (expressed in radians) and a 3D normalized axis
{
	register const VecFloat4 sinCos = VecFloatSinCos1( angle*0.5f );
	return VecFloatPermuteX0Y0Z0W1( VecFloatScaleX(normalizedAxis,sinCos), VecFloatSplatY(sinCos) );
}

FINLINE_Z void VecFloatQuaternionBuildAnglesAxes3 ( register VecFloat3x3& quaternions, register const VecFloat4& angles, register const VecFloat3x3& normalizedAxes ) ///< for( i=0; i<3; ++i ) result[i] = VecFloatQuaternionBuildAngleAxis( angles[i], normalizedAxes[i] );
{
	register VecFloat4 sinAngles, cosAngles;
	register VecFloat4 axis0 = VecFloat3x3GetRow0( normalizedAxes );
	register VecFloat4 axis1 = VecFloat3x3GetRow1( normalizedAxes );
	register VecFloat4 axis2 = VecFloat3x3GetRow2( normalizedAxes );

	VecFloatSinCos( sinAngles, cosAngles, VecFloatMul(angles,VectorConstantsPrivate::vHalfF) );
	axis0 = VecFloatScaleX( axis0, sinAngles );
	axis1 = VecFloatScaleY( axis1, sinAngles );
	axis2 = VecFloatScaleZ( axis2, sinAngles );

	VecFloat3x3SetRow0( quaternions, VecFloatPermuteX0Y0Z0X1(axis0,cosAngles) );
	VecFloat3x3SetRow1( quaternions, VecFloatPermuteX0Y0Z0Y1(axis1,cosAngles) );
	VecFloat3x3SetRow2( quaternions, VecFloatPermuteX0Y0Z0Z1(axis2,cosAngles) );
}

FINLINE_Z void VecFloatQuaternionBuildAnglesAxes4 ( register VecFloat4x4& quaternions, register const VecFloat4& angles, register const VecFloat4x4& normalizedAxes ) ///< for( i=0; i<4; ++i ) result[i] = VecFloatQuaternionBuildAngleAxis( angles[i], normalizedAxes[i] );
{
	register VecFloat4 sinAngles, cosAngles;
	register VecFloat4 axis0 = VecFloat4x4GetRow0( normalizedAxes );
	register VecFloat4 axis1 = VecFloat4x4GetRow1( normalizedAxes );
	register VecFloat4 axis2 = VecFloat4x4GetRow2( normalizedAxes );
	register VecFloat4 axis3 = VecFloat4x4GetRow3( normalizedAxes );

    VecFloatSinCos( sinAngles, cosAngles, VecFloatMul(angles,VectorConstantsPrivate::vHalfF) );
	axis0 = VecFloatScaleX( axis0, sinAngles );
	axis1 = VecFloatScaleY( axis1, sinAngles );
	axis2 = VecFloatScaleZ( axis2, sinAngles );
	axis3 = VecFloatScaleW( axis3, sinAngles );

	VecFloat4x4SetRow0( quaternions, VecFloatPermuteX0Y0Z0X1(axis0,cosAngles) );
	VecFloat4x4SetRow1( quaternions, VecFloatPermuteX0Y0Z0Y1(axis1,cosAngles) );
	VecFloat4x4SetRow2( quaternions, VecFloatPermuteX0Y0Z0Z1(axis2,cosAngles) );
	VecFloat4x4SetRow3( quaternions, VecFloatPermuteX0Y0Z0W1(axis3,cosAngles) );
}

FINLINE_Z void VecFloatQuaternionBuildAnglesAxes_SoA ( register VecFloat4* quaternions, register const VecFloat4& angles, register const VecFloat4* normalizedAxes ) ///< Same behavior as VecFloatQuaternionBuildAnglesAxes for 4 quaternions, but source and destination vectors have an SoA layout : { x0x1x2x3, y0y1y2y3, z0z1z2z3, w0w1w2w3 }.
{
	register VecFloat4 sinAngles;
    VecFloatSinCos( sinAngles, quaternions[3], VecFloatMul(angles,VectorConstantsPrivate::vHalfF) );
	quaternions[0] = VecFloatMul( normalizedAxes[0], sinAngles );
	quaternions[1] = VecFloatMul( normalizedAxes[1], sinAngles );
	quaternions[2] = VecFloatMul( normalizedAxes[2], sinAngles );
}

FINLINE_Z VecFloat4 VecFloatQuaternionCNormalize ( register const VecFloat4& q, const U32 refinements ) ///< xyzw = 1 / length(a.xyzw) with check: when the quaternion length is almost null, xyzw is set to identity
{
	register const VecFloat4 identity = VecFloatSetIdentity();
	register const VecFloat4 squaredLength = VecFloatDot4( q, q );
	register const VecFloat4 normalizedQ = VecFloatMul( q, VecFloatRSqrtAccurate(squaredLength,refinements) );
	return VecFloatSelectGreaterOrEqual( squaredLength, VectorConstantsPrivate::vEpsSquareF, normalizedQ, identity );
}

FINLINE_Z VecFloat4 VecFloatQuaternionConjugate ( register const VecFloat4& q )
{
    return VecFloatMul( q, VectorConstantsPrivate::vConjugateMask );
}

FINLINE_Z VecFloat4 VecFloatQuaternionMul ( register const VecFloat4& a, register const VecFloat4& b ) ///< xyzw = a * b, with a and b two quaternions
{
	VecFloat4 negativeB, aX, aY, aZ, aW, bWZYX, bZWXY, bYXWZ, res;
    negativeB = VecFloatNegate( b );
    aW = VecFloatSplatW( a );
    aX = VecFloatSplatX( a );
    aY = VecFloatSplatY( a );
    aZ = VecFloatSplatZ( a );
    bWZYX = VecFloatPermuteW0Z1Y0X1( b, negativeB );
    bZWXY = VecFloatPermuteZ0W0X1Y1( b, negativeB );
    bYXWZ = VecFloatPermuteY1X0W0Z1( b, negativeB );
    res = VecFloatMul( aW, b );
    res = VecFloatMadd( aX, bWZYX, res );
    res = VecFloatMadd( aY, bZWXY, res );
    res = VecFloatMadd( aZ, bYXWZ, res );
    return res;
}

FINLINE_Z void VecFloatQuaternionMul_SoA ( VecFloat4* RESTRICT_Z results, register const VecFloat4* RESTRICT_Z a, register const VecFloat4* RESTRICT_Z b ) ///< Same behavior as VecFloatQuaternionMul for 4 quaternions, but source and destination vectors have an SoA layout : { x0x1x2x3, y0y1y2y3, z0z1z2z3, w0w1w2w3 } and must be different.
{
    results[3] = VecFloatSub( VecFloatMul(a[3],b[3]), VecFloatDot3_SoA(a,b) );
    VecFloatCross3_SoA( results, a, b );
    for( U32 i = 0; i < 3; ++i )
    {
        results[i] = VecFloatMadd( b[3], a[i], results[i] );
        results[i] = VecFloatMadd( a[3], b[i], results[i] );
    }
}

FINLINE_Z VecFloat4 VecFloatQuaternionToPitchYawRoll( register const VecFloat4& q)
{
	VecFloat4 vQ = VecFloatNormalize4( q );
	Float x = VecFloatGetX( vQ );
	Float y = VecFloatGetY( vQ );
	Float z = VecFloatGetZ( vQ );
	Float w = VecFloatGetW( vQ );
	VecFloat4 result = VecFloatLoad4( 
						/*pitch*/(Float)atan2( 2 * ( y * z + w * x ), w*w - x*x - y*y + z*z ),
						/*yaw*/	 (Float)asin( 2 * ( w * y - x * z ) ),
						/*roll*/ (Float)atan2( 2. * ( x * y + w * z ), w*w + x*x - y*y - z*z ),
								 0.0f);

	return result;
};



///////////////////////// 32-bit integer vectors /////////////////////////


// ---------------------- Load operations

FINLINE_Z VecInt4 VecIntLoad1 ( const S32 _xyzw ) ///< xyzw = _xyzw
{
#if defined (_PC_SSE2 )
	return _mm_set1_epi32( _xyzw );
#else
	VecInt4 result;
	result.x = result.y = result.z = result.w = _xyzw;
	return result;
#endif
}

FINLINE_Z VecInt4 VecIntLoad4 ( const S32 x, const S32 y, const S32 z, const S32 w )  ///< xyzw = { x y z w }
{
#if defined (_PC_SSE2 )
	return _mm_set_epi32( w, z, y, x );
#else
	VecInt4 result;
	result.x = x;
	result.y = y;
	result.z = z;
	result.w = w;
	return result;
#endif
}

FINLINE_Z VecInt4 VecIntLoadAligned ( const void* alignedMemory ) ///< Load xyzw from a 16-byte aligned memory address
{
#if defined (_PC_SSE2 )
	return _mm_load_si128( reinterpret_cast<const __m128i*>(alignedMemory) );
#else
	VecInt4 result;
	memcpy( &result, alignedMemory, sizeof(result) );
	return result;
#endif
}

FINLINE_Z VecInt4 VecIntLoadUnaligned ( const void* unalignedMemory ) ///< Load xyzw from any memory address
{
#if defined ( _PC_SSE3 )
	return _mm_lddqu_si128( reinterpret_cast<const __m128i*>(unalignedMemory) );
#elif defined (_PC_SSE2 )
	return _mm_loadu_si128( reinterpret_cast<const __m128i*>(unalignedMemory) );
#else
	VecInt4 result;
	memcpy( &result, unalignedMemory, sizeof(result) );
	return result;
#endif
}


// ---------------------- Set operations

FINLINE_Z VecInt4 VecIntSplatZero () ///< xyzw = 0
{
#if defined (_PC_SSE2 )
	return _mm_setzero_si128();
#else
	return VecIntLoad1( 0 );
#endif
}

FINLINE_Z VecInt4 VecIntSplatOne () ///< xyzw = 1
{
#if defined (_PC_SSE2 )
	return _mm_set1_epi32( 1 );
#else
	return VecIntLoad1( 1 );
#endif
}


// ---------------------- Save operations

template <U8 index> FINLINE_Z S32 VecIntGet ( register const VecInt4& a ) ///< access a[index]
{
#if defined (_PC_SSE4 )
	return  _mm_extract_epi32( a, index );
#elif defined (_PC_SSE2 )
	return index == 0 ? _mm_cvtsi128_si32(a) : a.m128i_i32[index];
#else
	return a.xyzw[index];
#endif
}

FINLINE_Z S32 VecIntGetX ( register const VecInt4& a )  { return VecIntGet <0> ( a ); }
FINLINE_Z S32 VecIntGetY ( register const VecInt4& a )  { return VecIntGet <1> ( a ); }
FINLINE_Z S32 VecIntGetZ ( register const VecInt4& a )  { return VecIntGet <2> ( a ); }
FINLINE_Z S32 VecIntGetW ( register const VecInt4& a )  { return VecIntGet <3> ( a ); }

FINLINE_Z void VecIntStoreAligned ( void* dest, register const VecInt4& source ) ///< dest = source, with dest a 16-byte aligned memory adress
{
#if defined (_PC_SSE2 )
	_mm_store_si128( reinterpret_cast<__m128i*>(dest), source );
#else
	memcpy( dest, &source, sizeof(source) );
#endif
}

FINLINE_Z void VecIntStreamStoreAligned ( void* dest, register const VecInt4& source ) ///< dest = source, with dest a 16-byte aligned memory address. This store doesn't pollute the cache, it is slower when dest points to data already in cache, and faster otherwise
{
#if defined (_PC_SSE2 )
	_mm_stream_si128( reinterpret_cast<__m128i*>(dest), source );
#else
	VecIntStoreAligned( dest, source );
#endif
}

FINLINE_Z void VecIntStoreUnaligned ( void* dest, register const VecInt4& source ) ///< dest = source. No specific memory alignement is required for dest, but prefer an aligned store when possible.
{
#if defined (_PC_SSE2 )
	_mm_storeu_si128( reinterpret_cast<__m128i*>(dest), source );
#else
	memcpy( dest, &source, sizeof(source) );
#endif
}

// ---------------------- Select operations

FINLINE_Z VecSelMask VecIntCompareLT( register const VecInt4& a, register const VecInt4& b ) ///< xyzw = ( a.xyzw < b.xyzw ) ? # : 0
{
#if defined (_PC_SSE2 )
	return _mm_castsi128_ps( _mm_cmplt_epi32(a,b) );
#else
	return VecIntCompareGT( b, a );
#endif
}

FINLINE_Z VecSelMask VecIntCompareLE( register const VecInt4& a, register const VecInt4& b ) ///< xyzw = ( a.xyzw <= b.xyzw ) ? # : 0
{
#if defined (_PC_SSE2 )
	register const __m128i vLT = _mm_cmplt_epi32( a, b );
	register const __m128i vEQ = _mm_cmpeq_epi32( a, b );
	return _mm_castsi128_ps( _mm_or_si128(vLT,vEQ) );
#else
	VecSelMask result;
	for( U32 i = 0; i < 4; ++i )
		result.xyzw[i] = ( a.xyzw[i] <= b.xyzw[i] ) ? 0xFFFFFFFF : 0;
	return result;
#endif
}

FINLINE_Z VecSelMask VecIntCompareGT( register const VecInt4& a, register const VecInt4& b ) ///< xyzw = ( a.xyzw > b.xyzw ) ? # : 0
{
#if defined (_PC_SSE2 )
	return _mm_castsi128_ps( _mm_cmpgt_epi32(a,b) );
#else
	VecSelMask result;
	for( U32 i = 0; i < 4; ++i )
		result.xyzw[i] = ( a.xyzw[i] > b.xyzw[i] ) ? 0xFFFFFFFF : 0;
	return result;
#endif
}

FINLINE_Z VecSelMask VecIntCompareGE( register const VecInt4& a, register const VecInt4& b ) ///< xyzw = ( a.xyzw >= b.xyzw ) ? # : 0
{
#if defined (_PC_SSE2 )
	register const __m128i vGT = _mm_cmpgt_epi32( a, b );
	register const __m128i vEQ = _mm_cmpeq_epi32( a, b );
	return _mm_castsi128_ps( _mm_or_si128(vGT,vEQ) );
#else
	VecSelMask result;
	for( U32 i = 0; i < 4; ++i )
		result.xyzw[i] = ( a.xyzw[i] >= b.xyzw[i] ) ? 0xFFFFFFFF : 0;
	return result;
#endif
}

FINLINE_Z VecSelMask VecIntCompareEQ( register const VecInt4& a, register const VecInt4& b ) ///< xyzw = ( a.xyzw == b.xyzw ) ? # : 0
{
#if defined (_PC_SSE2 )
	return _mm_castsi128_ps( _mm_cmpeq_epi32(a,b) );
#else
	VecSelMask result;
	for( U32 i = 0; i < 4; ++i )
		result.xyzw[i] = ( a.xyzw[i] == b.xyzw[i] ) ? 0xFFFFFFFF : 0;
	return result;
#endif
}

FINLINE_Z VecInt4 VecIntSelectMask ( register const VecInt4& a, register const VecInt4& b, register const VecSelMask& mask ) ///< for i � [0,4] : res[i] = ( mask[i] == 0 ) ? a[i] : b[i] => Not a bitwise selection!
{
#if defined (_PC_SSE2 )
	return _mm_sel_epi8( a, b, _mm_castps_si128(mask) );
#else
	Int4 result, maskI;
	memcpy( &maskI, &mask, sizeof(maskI) );
	for( U32 i = 0; i < 4; ++i )
		result.xyzw[i] = ( maskI.xyzw[i] == 0 ) ? a.xyzw[i] : b.xyzw[i];
	return result;
#endif
}

FINLINE_Z VecInt4 VecIntSelectLess ( register const VecInt4& a, register const VecInt4& b, register const VecInt4& success, register const VecInt4& fail ) ///< xyzw = ( a.xyzw < b.xyzw ) ? success.xyzw : fail.xyzw
{
#if defined( _PC_SSE2 )
	return _mm_sel_epi8( fail, success, _mm_cmplt_epi32(a,b) );
#else
	VecInt4 result;
	for( U32 i = 0; i < 4; ++i )
		result.xyzw[i] = a.xyzw[i] < b.xyzw[i] ? success.xyzw[i] : fail.xyzw[i];
	return result;
#endif
}

FINLINE_Z VecInt4 VecIntSelectLessOrEqual ( register const VecInt4& a, register const VecInt4& b, register const VecInt4& success, register const VecInt4& fail ) ///< xyzw = ( a.xyzw <= b.xyzw ) ? success.xyzw : fail.xyzw
{
#if defined( _PC_SSE2 )
	register const __m128i vLT = _mm_cmplt_epi32( a, b );
	register const __m128i vEQ = _mm_cmpeq_epi32( a, b );
	return _mm_sel_epi8( fail, success, _mm_or_si128(vLT,vEQ) );
#else
	VecInt4 result;
	for( U32 i = 0; i < 4; ++i )
		result.xyzw[i] = a.xyzw[i] <= b.xyzw[i] ? success.xyzw[i] : fail.xyzw[i];
	return result;
#endif
}

FINLINE_Z VecInt4 VecIntSelectGreater ( register const VecInt4& a, register const VecInt4& b, register const VecInt4& success, register const VecInt4& fail ) ///< xyzw = ( a.xyzw > b.xyzw ) ? success.xyzw : fail.xyzw
{
#if defined( _PC_SSE2 )
	return _mm_sel_epi8( fail, success, _mm_cmpgt_epi32(a,b) );
#else
	VecInt4 result;
	for( U32 i = 0; i < 4; ++i )
		result.xyzw[i] = a.xyzw[i] > b.xyzw[i] ? success.xyzw[i] : fail.xyzw[i];
	return result;
#endif
}

FINLINE_Z VecInt4 VecIntSelectGreaterOrEqual ( register const VecInt4& a, register const VecInt4& b, register const VecInt4& success, register const VecInt4& fail ) ///< xyzw = ( a.xyzw >= b.xyzw ) ? success.xyzw : fail.xyzw
{
#if defined( _PC_SSE2 )
	register const __m128i vGT = _mm_cmpgt_epi32( a, b );
	register const __m128i vEQ = _mm_cmpeq_epi32( a, b );
	return _mm_sel_epi8( fail, success, _mm_or_si128(vGT,vEQ) );
#else
	VecInt4 result;
	for( U32 i = 0; i < 4; ++i )
		result.xyzw[i] = a.xyzw[i] >= b.xyzw[i] ? success.xyzw[i] : fail.xyzw[i];
	return result;
#endif
}

FINLINE_Z VecInt4 VecIntSelectEqual ( register const VecInt4& a, register const VecInt4& b, register const VecInt4& success, register const VecInt4& fail ) ///< xyzw = ( a.xyzw == b.xyzw ) ? success.xyzw : fail.xyzw
{
#if defined( _PC_SSE2 )
	return _mm_sel_epi8( fail, success, _mm_cmpeq_epi32(a,b) );
#else
	VecInt4 result;
	for( U32 i = 0; i < 4; ++i )
		result.xyzw[i] = a.xyzw[i] == b.xyzw[i] ? success.xyzw[i] : fail.xyzw[i];
	return result;
#endif
}


// ---------------------- Logical operations

FINLINE_Z VecInt4 VecIntAnd ( register const VecInt4& a, register const VecInt4& b ) ///< xyzw = a.xyzw & b.xyzw
{
#if defined (_PC_SSE2 )
	return _mm_and_si128( a, b );
#else
	VecInt4 result;
	result.x = a.x & b.x;
	result.y = a.y & b.y;
	result.z = a.z & b.z;
	result.w = a.w & b.w;
	return result;
#endif
}

FINLINE_Z VecInt4 VecIntAndNot ( register const VecInt4& a, register const VecInt4& b ) ///< xyzw = a.xyzw & ~b.xyzw
{
#if defined (_PC_SSE2 )
	return _mm_andnot_si128( b, a );
#else
	VecInt4 result;
	result.x = a.x & (~b.x);
	result.y = a.y & (~b.y);
	result.z = a.z & (~b.z);
	result.w = a.w & (~b.w);
	return result;
#endif
}

FINLINE_Z VecInt4 VecIntOr ( register const VecInt4& a, register const VecInt4& b ) ///< xyzw = a.xyzw | b.xyzw
{
#if defined (_PC_SSE2 )
	return _mm_or_si128( a, b );
#else
	VecInt4 result;
	result.x = a.x | b.x;
	result.y = a.y | b.y;
	result.z = a.z | b.z;
	result.w = a.w | b.w;
	return result;
#endif
}

FINLINE_Z VecInt4 VecIntXor ( register const VecInt4& a, register const VecInt4& b ) ///< xyzw = a.xyzw ^ b.xyzw
{
#if defined (_PC_SSE2 )
	return _mm_xor_si128( a, b );
#else
	VecInt4 result;
	result.x = a.x ^ b.x;
	result.y = a.y ^ b.y;
	result.z = a.z ^ b.z;
	result.w = a.w ^ b.w;
	return result;
#endif
}


// ---------------------- Arithmetic operations

FINLINE_Z VecInt4 VecIntNegate ( register const VecInt4& a ) ///< xyzw = -xyzw
{
#if defined (_PC_SSE2 )
	return VecIntSub( VecIntSplatZero(), a );
#else
	return VecIntLoad4( -a.x, -a.y, -a.z, -a.w );
#endif
}

FINLINE_Z VecInt4 VecIntAdd ( register const VecInt4& a, register const VecInt4& b ) ///< xyzw = a.xyzw + b.xyzw
{
#if defined (_PC_SSE2 )
	return _mm_add_epi32( a, b );
#else
	return VecIntLoad4( a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w );
#endif
}

FINLINE_Z VecInt4 VecIntSub ( register const VecInt4& a, register const VecInt4& b ) ///< xyzw = a.xyzw - b.xyzw
{
#if defined (_PC_SSE2 )
	return _mm_sub_epi32( a, b );
#else
	return VecIntLoad4( a.x-b.x, a.y-b.y, a.z-b.z, a.w-b.w );
#endif
}

FINLINE_Z VecInt4 VecIntMul( register const VecInt4& a, register const VecInt4& b ) ///< res.xyzw = a.xyzw * b.xyzw
{
#if defined ( _PC_SSE4 )
	return _mm_mullo_epi32( a, b );
#elif defined ( _PC_SSE2 )
	const int shuffleMask = _MM_SHUFFLE(0,0,2,0);
	const __m128i res0 = _mm_mul_epu32( a, b );
	const __m128i res1 = _mm_mul_epu32( _mm_srli_si128(a,4), _mm_srli_si128(b,4) );
	const __m128i res0XZXX = _mm_shuffle_epi32( res0, shuffleMask );
	const __m128i res1XZXX = _mm_shuffle_epi32( res1, shuffleMask );
	return _mm_unpacklo_epi32( res0XZXX, res1XZXX );
#else
	Int4 aI, bI, result;
	VecIntStoreAligned( &aI, a );
	VecIntStoreAligned( &bI, b );
	for( U32 i = 0; i < 4; ++i )
		result.xyzw[i] = aI.xyzw[i] * bI.xyzw[i];
	return VecIntLoadAligned( &result );
#endif
}

FINLINE_Z VecInt4 VecIntHAdd( register const VecInt4& a, register const VecInt4& b ) ///< xyzw = { a.x+a.y, a.z+a.w, b.x+b.y, b.z+b.w }
{
#if defined (_PC_SSE3 )
	return _mm_hadd_epi32( a, b );
#else
	return VecIntAdd( VecIntPermuteX0Z0X1Z1(a,b), VecIntPermuteY0W0Y1W1(a,b) );
#endif
}

FINLINE_Z VecInt4 VecIntHSub( register const VecInt4& a, register const VecInt4& b ) ///< xyzw = { a.x-a.y, a.z-a.w, b.x-b.y, b.z-b.w }
{
#if defined (_PC_SSE3 )
	return _mm_hsub_epi32( a, b );
#else
	return VecIntSub( VecIntPermuteX0Z0X1Z1(a,b), VecIntPermuteY0W0Y1W1(a,b) );
#endif
}

FINLINE_Z VecInt4 VecIntSign ( register const VecInt4& a )   ///< res.xyzw = a.xyzw == 0 ? 0 : ( a.xyzw > 0 ? 1 : -1 )
{
	return VecIntSign( a, VecIntSplatOne() );
}

FINLINE_Z VecInt4 VecIntSign ( register const VecInt4& a, register const VecInt4& b )   ///< res.xyzw = a.xyzw == 0 ? 0 : ( a.xyzw > 0 ? b.xyzw : -b.xyzw )
{
#if defined( _PC_SSE3 )
    return _mm_sign_epi32( b, a );
#else
	register const VecInt4 zero = VecIntSplatZero();
	register const VecInt4 signNotNull = VecIntSelectGreater( a, zero, b, VecIntNegate(b) );
    return VecIntSelectEqual( a, zero, zero, signNotNull );
#endif
}

FINLINE_Z VecInt4 VecIntAbs ( register const VecInt4& a )  ///< xyzw = abs ( a.xyzw )
{
#if defined (_PC_SSE3 )
	return _mm_abs_epi32( a );
#else
	return VecIntMax( a, VecIntNegate(a) );
#endif
}

FINLINE_Z VecInt4 VecIntMin ( register const VecInt4& a, register const VecInt4& b )  ///< xyzw = min ( a.xyzw, b.xyzw )
{
#if defined (_PC_SSE2 )
	VecInt4 mask = _mm_cmpgt_epi32( a, b );
	return _mm_or_si128( _mm_and_si128(mask,b), _mm_andnot_si128(mask,a) );
#elif defined (_PC_SSE4 )
	return _mm_min_epi32( a, b );
#else
	return VecIntLoad4( Min(a.x,b.x), Min(a.y,b.y), Min(a.z,b.z), Min(a.w,b.w) );
#endif
}

FINLINE_Z VecInt4 VecIntMinComponent ( register const VecInt4& a )  ///< xyzw = min ( min(a.x,a.y), min(a.z,a.w) )
{
#if defined (_PC_SSE2 )
	VecInt4 x = VecIntSplatX ( a );
	VecInt4 y = VecIntSplatY ( a );
	VecInt4 z = VecIntSplatZ ( a );
	VecInt4 w = VecIntSplatW ( a );
	return VecIntMin( VecIntMin(x,y), VecIntMin(z,w) ); 
#else
	VecInt4 result;
	result.x = result.y = result.z = result.w = Min( Min(a.x,a.y), Min(a.z,a.w) );
	return result;
#endif
}

FINLINE_Z VecInt4 VecIntMinComponent3 ( register const VecInt4& a )  ///< xyzw = min ( a.x, min(a.y,a.z) )
{
#if defined (_PC_SSE2 )
	VecInt4 x = VecIntSplatX ( a );
	VecInt4 y = VecIntSplatY ( a );
	VecInt4 z = VecIntSplatZ ( a );
	return VecIntMin( x, VecIntMin(y,z) ); 
#else
	VecInt4 result;
	result.x = result.y = result.z = result.w = Min( a.x, Min(a.y,a.z) );
	return result;
#endif
}

FINLINE_Z VecInt4 VecIntMax ( register const VecInt4& a, register const VecInt4& b )  ///< xyzw = max ( a.xyzw, b.xyzw )
{
#if defined (_PC_SSE2 )
	VecInt4 mask = _mm_cmpgt_epi32( a, b );
	return _mm_or_si128( _mm_andnot_si128(mask,b), _mm_and_si128(mask,a) );
#elif defined (_PC_SSE4 )
	return _mm_max_epi32( a, b );
#else
	return VecIntLoad4( Max(a.x,b.x), Max(a.y,b.y), Max(a.z,b.z), Max(a.w,b.w) );
#endif
}

FINLINE_Z VecInt4 VecIntMaxComponent ( register const VecInt4& a )  ///< xyzw = max ( max(a.x,a.y), max(a.z,a.w) )
{
#if defined (_PC_SSE2 )
	VecInt4 x = VecIntSplatX ( a );
	VecInt4 y = VecIntSplatY ( a );
	VecInt4 z = VecIntSplatZ ( a );
	VecInt4 w = VecIntSplatW ( a );
	return VecIntMax( VecIntMax(x,y), VecIntMax(z,w) ); 
#else
	VecInt4 result;
	result.x = result.y = result.z = result.w = Max( Max(a.x,a.y), Max(a.z,a.w) );
	return result;
#endif
}

FINLINE_Z VecInt4 VecIntMaxComponent3 ( register const VecInt4& a )  ///< xyzw = max ( a.x, max(a.y,a.z) )
{
#if defined (_PC_SSE2 )
	VecInt4 x = VecIntSplatX ( a );
	VecInt4 y = VecIntSplatY ( a );
	VecInt4 z = VecIntSplatZ ( a );
	return VecIntMax( x, VecIntMax(y,z) ); 
#else
	VecInt4 result;
	result.x = result.y = result.z = result.w = Max( a.x, Max(a.y,a.z) );
	return result;
#endif
}

FINLINE_Z VecInt4 VecIntClamp ( register const VecInt4& a, register const VecInt4& min, register const VecInt4& max )  ///< xyzw = clamp ( a.xyzw, min.xyzw, max.xyzw )
{
	return VecIntMin( max, VecIntMax(min,a) );
}

FINLINE_Z VecInt4 VecIntShiftLeft ( register const VecInt4& a, const U32 offset )  ///< xyzw = a.xyzw << offset
{
#if defined (_PC_SSE2 )
	return _mm_slli_epi32( a, offset );
#else
	return VecIntLoad4( a.x<<offset, a.y<<offset, a.z<<offset, a.w<<offset );
#endif
}

FINLINE_Z VecInt4 VecIntShiftLeftX ( register const VecInt4& a, register const VecInt4& offset )  ///< xyzw = a.xyzw << offset.xxxx
{
#if defined (_PC_SSE2 )
	register const __m128i offsetX000 = _mm_and_si128( offset, VecIntLoadAligned(&VectorConstantsPrivate::vShiftXMask) );
	return _mm_sll_epi32( a, offsetX000 );
#else
	return VecIntLoad4( a.x<<offset.x, a.y<<offset.x, a.z<<offset.x, a.w<<offset.x );
#endif
}

FINLINE_Z VecInt4 VecIntShiftLeft ( register const VecInt4& a, register const VecInt4& offsets )  ///< xyzw = a.xyzw << offsets.xyzw
{
#if defined (_PC_SSE2 )
	// Neither component-wise shift nor bitwise permutation on SSE platforms...the only way is to store and reload :/
	Int4 mA, mOffsets;
	VecIntStoreAligned( mA.xyzw, a );
	VecIntStoreAligned( mOffsets.xyzw, offsets );
	for( U32 i = 0; i < 4; ++i )
		mA.xyzw[i] <<= mOffsets.xyzw[i];
	return VecIntLoadAligned( mA.xyzw );
#else
	return VecIntLoad4( a.x<<offsets.x, a.y<<offsets.y, a.z<<offsets.z, a.w<<offsets.w );
#endif
}

FINLINE_Z VecInt4 VecIntShiftRight ( register const VecInt4& a, const U32 offset )  ///< xyzw = a.xyzw >> Offset
{
	/* We can't use the "shift right" intrinsic directly since it returns QNan for negative numbers, so:
	- create a sign mask
	- shift the absolute value
	- xor the shifted value to restore its sign.
	See VecIntShiftRightPositive which offers significantly better performance but only works with positive numbers.
	*/
#if defined (_PC_SSE2 )
	return _mm_srai_epi32( a, offset );
#else
	return VecIntLoad4( a.x>>offset, a.y>>offset, a.z>>offset, a.w>>offset );
#endif
}

FINLINE_Z VecInt4 VecIntShiftRightPositive ( register const VecInt4& a, const U32 offset )  ///< xyzw = a.xyzw >> Offset, with a.xyzw >= 0
{
#if defined (_PC_SSE2 )
	return _mm_srli_epi32( a, offset );
#else
	return VecIntLoad4( a.x>>offset, a.y>>offset, a.z>>offset, a.w>>offset );
#endif
}

FINLINE_Z VecInt4 VecIntShiftRightPositiveX ( register const VecInt4& a, register const VecInt4& offset )  ///< xyzw = a.xyzw >> offset.xxxx
{
#if defined (_PC_SSE2 )
	register const __m128i offsetX000 = _mm_and_si128( offset, VecIntLoadAligned(&VectorConstantsPrivate::vShiftXMask) );
	return _mm_srl_epi32( a, offsetX000 );
#else
	return VecIntLoad4( a.x>>offset.x, a.y>>offset.x, a.z>>offset.x, a.w>>offset.x );
#endif
}

FINLINE_Z VecInt4 VecIntShiftRightPositive ( register const VecInt4& a, register const VecInt4& offsets )  ///< xyzw = a.xyzw >> offsets.xyzw
{
#if defined (_PC_SSE2 )
	// Neither component-wise shift nor bitwise permutation on SSE platforms...the only way is to store and reload :/
	Int4 mA, mOffsets;
	VecIntStoreAligned( mA.xyzw, a );
	VecIntStoreAligned( mOffsets.xyzw, offsets );
	for( U32 i = 0; i < 4; ++i )
		mA.xyzw[i] >>= mOffsets.xyzw[i];
	return VecIntLoadAligned( mA.xyzw );
#else
	return VecIntLoad4( a.x>>offsets.x, a.y>>offsets.y, a.z>>offsets.z, a.w>>offsets.w );
#endif
}



// ---------------------- Swizzle operations


/////////////////////////// Don't use these fonctions directly !

template <U8 index> FINLINE_Z VecInt4 _VecIntSplat_ ( register const VecInt4& a )
{
#if defined (_PC_SSE2 )
	return _mm_shuffle_epi32( a, _MM_SHUFFLE(index,index,index,index) );
#else
	return VecIntLoad1( a.xyzw[index] );
#endif
}

///////////////////////////

FINLINE_Z VecInt4 VecIntSplatX ( register const VecInt4& a )  ///< xyzw = a.xxxx
{
	return _VecIntSplat_ <0> ( a );
}

FINLINE_Z VecInt4 VecIntSplatY ( register const VecInt4& a )  ///< xyzw = a.yyyy
{
	return _VecIntSplat_ <1> ( a );
}

FINLINE_Z VecInt4 VecIntSplatZ ( register const VecInt4& a )  ///< xyzw = a.zzzz
{
	return _VecIntSplat_ <2> ( a );
}

FINLINE_Z VecInt4 VecIntSplatW ( register const VecInt4& a )  ///< xyzw = a.wwww
{
	return _VecIntSplat_ <3> ( a );
}

template <U32 ElementCount> FINLINE_Z VecInt4 VecIntRotateLeft ( register const VecInt4& a ) ///< xyzw = a.xyzw <<< ElementCount
{
#if defined( _PC_SSE )
	const S32 x = ElementCount & 3;
    const S32 y = (ElementCount+1) & 3;
    const S32 z = (ElementCount+2) & 3;
	const S32 w = (ElementCount+3) & 3;
	return _mm_shuffle_epi32( a, _MM_SHUFFLE(w,z,y,x) );
#else
	const S32 x = ElementCount & 3;
    const S32 y = (ElementCount+1) & 3;
    const S32 z = (ElementCount+2) & 3;
	const S32 w = (ElementCount+3) & 3;
    return VecIntLoad4( a.xyzw[x], a.xyzw[y], a.xyzw[z], a.xyzw[w] );
#endif
}

template <VectorSwizzle SrcPos, VectorSwizzle DestPos> FINLINE_Z VecInt4 VecIntZeroInsert ( register const VecInt4& a )   ///< Insert a value from a into a null vector. Say SrcPos=Z, DestPos=X: xyzw = { a.z, 0, 0, 0 }
{
	register VecFloat4 result = VecFloatZeroInsert <SrcPos, DestPos> ( VecIntAsFloat(a) );
	return VecFloatAsInt( result );
}

template <VectorSwizzle x, VectorSwizzle y, VectorSwizzle z, VectorSwizzle w> FINLINE_Z VecInt4 VecIntSwizzle ( register const VecInt4& a )   ///< say template parameters are {X0,Z0,Y0,W0}, xyzw = a.xzyw
{
#if defined (_PC_SSE2 )
	return _mm_shuffle_epi32( a, _MM_SHUFFLE(w,z,y,x) );
#else
	return VecIntLoad4( a.xyzw[x], a.xyzw[y], a.xyzw[z], a.xyzw[w] );
#endif
}

FINLINE_Z VecInt4 VecIntSwizzleXXZZ ( register const VecInt4& a )   ///< xyzw = a.xxzz
{
#if defined( _PC_SSE3 )
	return _mm_castps_si128( _mm_moveldup_ps( _mm_castsi128_ps(a) ) );
#else
    return VecIntSwizzle <VSWIZZLE_X, VSWIZZLE_X, VSWIZZLE_Z, VSWIZZLE_Z> ( a );
#endif
}

FINLINE_Z VecInt4 VecIntSwizzleYYWW ( register const VecInt4& a )   ///< xyzw = a.yyww
{
#if defined( _PC_SSE3 )
	return _mm_castps_si128( _mm_movehdup_ps( _mm_castsi128_ps(a) ) );
#else
    return VecIntSwizzle <VSWIZZLE_Y, VSWIZZLE_Y, VSWIZZLE_W, VSWIZZLE_W> ( a );
#endif
}

FINLINE_Z VecInt4 VecIntPermuteX0Y0Z0W1 ( register const VecInt4& a, register const VecInt4& b ) ///< xyzw = { a.x, a.y, a.z, b.w }
{
#if defined (_PC_SSE4 )
	return _mm_blend_epi16( a, b, 0xC0 );
#elif defined (_PC_SSE2 )
	register const __m128 result = VecFloatPermuteX0Y0Z0W1( _mm_castsi128_ps(a), _mm_castsi128_ps(b) );
	return _mm_castps_si128( result );
#else
	return VecIntLoad4( a.x, a.y, a.z, b.w );
#endif
}

FINLINE_Z VecInt4 VecIntPermuteX0Y0Z1W0 ( register const VecInt4& a, register const VecInt4& b ) ///< xyzw = { a.x, a.y, b.z, a.w }
{
#if defined (_PC_SSE4 )
	return _mm_blend_epi16( a, b, 0x30 );
#elif defined (_PC_SSE2 )
	register const __m128 result = VecFloatPermuteX0Y0Z1W0( _mm_castsi128_ps(a), _mm_castsi128_ps(b) );
	return _mm_castps_si128( result );
#else
	return VecIntLoad4( a.x, a.y, b.z, a.w );
#endif
}

FINLINE_Z VecInt4 VecIntPermuteX0Z0X1Z1 ( register const VecInt4& a, register const VecInt4& b ) ///< xyzw = { a.x, a.z, b.x, b.z }
{
#if defined (_PC_SSE2 )
	register const __m128 result = VecFloatPermuteX0Z0X1Z1( _mm_castsi128_ps(a), _mm_castsi128_ps(b) );
	return _mm_castps_si128( result );
#else
	return VecIntLoad4( a.x, a.z, b.x, b.z );
#endif
}

FINLINE_Z VecInt4 VecIntPermuteY0Z0W0X1 ( register const VecInt4& a, register const VecInt4& b ) ///< xyzw = { a.y, a.z, a.w, b.x }
{
#if defined (_PC_SSE3 )
	return _mm_alignr_epi8( b, a, 4 );
#elif defined (_PC_SSE2 )
	register const __m128 result = VecFloatPermuteY0Z0W0X1( _mm_castsi128_ps(a), _mm_castsi128_ps(b) );
	return _mm_castps_si128( result );
#else
	return VecIntLoad4( a.y, a.z, a.w, b.x );
#endif
}

FINLINE_Z VecInt4 VecIntPermuteX0Y1Z0W0 ( register const VecInt4& a, register const VecInt4& b ) ///< xyzw = { a.x, b.y, a.z, a.w }
{
#if defined (_PC_SSE4 )
	return _mm_blend_epi16( a, b, 0x0C );
#elif defined (_PC_SSE2 )
	register const __m128 result = VecFloatPermuteX0Y1Z0W0( _mm_castsi128_ps(a), _mm_castsi128_ps(b) );
	return _mm_castps_si128( result );
#else
	return VecIntLoad4( a.x, b.y, a.z, a.w );
#endif
}

FINLINE_Z VecInt4 VecIntPermuteY0W0Y1W1 ( register const VecInt4& a, register const VecInt4& b ) ///< xyzw = { a.y, a.w, b.y, b.w }
{
#if defined (_PC_SSE2 )
	register const __m128 result = VecFloatPermuteY0W0Y1W1( _mm_castsi128_ps(a), _mm_castsi128_ps(b) );
	return _mm_castps_si128( result );
#else
	return VecIntLoad4( a.y, a.w, b.y, b.w );
#endif
}

FINLINE_Z VecInt4 VecIntPermuteZ0X0X1W1 ( register const VecInt4& a, register const VecInt4& b ) ///< xyzw = { a.z, a.x, b.x, b.w }
{
#if defined (_PC_SSE2 )
	register const __m128 result = VecFloatPermuteZ0X0X1W1( _mm_castsi128_ps(a), _mm_castsi128_ps(b) );
	return _mm_castps_si128( result );
#else
	return VecIntLoad4( a.z, a.x, b.x, b.w );
#endif
}

FINLINE_Z VecInt4 VecIntPermuteX1Y0Z0W0 ( register const VecInt4& a, register const VecInt4& b ) ///< xyzw = { b.x, a.y, a.z, a.w }
{
#if defined (_PC_SSE4 )
	return _mm_blend_epi16( a, b, 0x03 );
#elif defined (_PC_SSE2 )
	register const __m128 result = VecFloatPermuteX1Y0Z0W0( _mm_castsi128_ps(a), _mm_castsi128_ps(b) );
	return _mm_castps_si128( result );
#else
	return VecIntLoad4( b.x, a.y, a.z, a.w );
#endif
}


// ---------------------- Compare operations

FINLINE_Z U32 VecIntAllZero ( register const VecInt4& a ) ///< return !a.x && !a.y && !a.z && !a.w ? 1 : 0
{
#if defined (_PC_SSE4 )
	return _mm_testz_si128( a, VecIntLoadAligned(&VectorConstantsPrivate::vAbsMask) );
#else
	return VecIntAllEqual( a, VecIntSplatZero() );
#endif
}

FINLINE_Z U32 VecIntAllEqual( register const VecInt4& a, register const VecInt4& b ) ///< Strict comparison. return a.xyzw == b.xyzw ? 1 : 0
{
#if defined (_PC_SSE2 )
	U32 result = _mm_movemask_epi8( _mm_cmpeq_epi32(a,b) );  // bits 0 to 15 of result must all be set to 1
	return result == 0xffff;
#else
	return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
#endif
}

FINLINE_Z U32 VecIntAnyNotEqual( register const VecInt4& a, register const VecInt4& b ) ///< Strict comparison. return a.xyzw != b.xyzw ? 1 : 0
{
#if defined (_PC_SSE2 )
	U32 result = _mm_movemask_epi8( _mm_cmpeq_epi32(a,b) );  // at least one of the 15 first bits of result has to be 0
	return result != 0xffff;
#else
	return a.x != b.x || a.y != b.y || a.z != b.z || a.w != b.w;
#endif
}

FINLINE_Z U32 VecIntAllLess( register const VecInt4& a, register const VecInt4& b ) ///< return a.xyzw < b.xyzw ? 1 : 0
{
#if defined (_PC_SSE )
	U32 result = _mm_movemask_epi8( _mm_cmplt_epi32(a,b) );  // bits 0 to 15 of result must all be set to 1
	return result == 0xffff;
#else
	return a.x < b.x && a.y < b.y && a.z < b.z && a.w < b.w;
#endif
}

FINLINE_Z U32 VecIntAllLessOrEqual( register const VecInt4& a, register const VecInt4& b ) ///< return a.xyzw <= b.xyzw ? 1 : 0
{
#if defined (_PC_SSE )
	return VecIntAllGreaterOrEqual( b, a );
#else
	return a.x <= b.x && a.y <= b.y && a.z <= b.z && a.w <= b.w;
#endif
}

FINLINE_Z U32 VecIntAllGreater( register const VecInt4& a, register const VecInt4& b ) ///< return a.xyzw > b.xyzw ? 1 : 0
{
#if defined (_PC_SSE )
	U32 result = _mm_movemask_epi8( _mm_cmpgt_epi32(a,b) );  // bits 0 to 15 of result must all be set to 1
	return result == 0xffff;
#else
	return a.x > b.x && a.y > b.y && a.z > b.z && a.w > b.w;
#endif
}

FINLINE_Z U32 VecIntAllGreaterOrEqual( register const VecInt4& a, register const VecInt4& b ) ///< return a.xyzw >= b.xyzw ? 1 : 0
{
#if defined (_PC_SSE )
	const __m128i greaterOrEqual = _mm_or_si128( _mm_cmpgt_epi32(a,b), _mm_cmpeq_epi32(a,b) );
	U32 result = _mm_movemask_epi8( greaterOrEqual );  // bits 0 to 15 of result must all be set to 1
	return result == 0xffff;
#else
	return a.x >= b.x && a.y >= b.y && a.z >= b.z && a.w >= b.w;
#endif
}

FINLINE_Z U32 VecIntAll3Equal( register const VecInt4& a, register const VecInt4& b ) ///< Strict comparison. return a.xyz == b.xyz ? 1 : 0
{
#if defined (_PC_SSE2 )
	__m128i comparison = _mm_cmpeq_epi32( a, b );
	U32 result = _mm_movemask_ps( _mm_castsi128_ps(comparison) );
	return ( result & 7 ) == 7 ? 1 : 0;
#else
	return a.x == b.x && a.y == b.y && a.z == b.z;
#endif
}


// ---------------------- Common maths

FINLINE_Z VecInt4 VecIntAddComponents( register const VecInt4& a ) ///< xyzw = a.x + a.y + a.z + a.w
{
#if defined (_PC_SSE3 )
	const __m128i result = _mm_hadd_epi32( a, a ); // { a.x+a.y, a.z+a.w, a.x+a.y, a.z+a.w }
	return _mm_hadd_epi32( result, result );
#elif defined (_PC_SSE2 )
	VecInt4 result;
    result = VecIntAdd( VecIntSplatX(a), VecIntSplatY(a) );
    result = VecIntAdd( VecIntSplatZ(a), result );
    return VecIntAdd( VecIntSplatW(a), result );
#else
	return VecIntLoad1( a.x + a.y + a.z + a.w );
#endif
}

FINLINE_Z VecInt4 VecIntAverage ( register const VecInt4& a, register const VecInt4& b )  ///< xyzw = ( a.xyzw + b.xyzw + 1 ) / 2
{
	const VecInt4 sum = VecIntAdd( VecIntAdd(a,b), VecIntSplatOne() );
	return VecIntShiftRight ( sum, 1 );
}



////////////////////////// 32-bit unsigned integer vectors //////////////////////////


// ---------------------- Load operations

FINLINE_Z VecUInt4 VecUIntLoad1 ( const U32 _xyzw ) ///< xyzw = _xyzw
{
#if defined (_PC_SSE2 )
	__m128i result;
	U32* pResult = reinterpret_cast <U32*> ( &result );
	pResult[0] = pResult[1] =  pResult[2] = pResult[3] = _xyzw;
	return result;
#else
	VecUInt4 result;
	result.x = result.y = result.z = result.w = _xyzw;
	return result;
#endif
}

FINLINE_Z VecUInt4 VecUIntLoad4 ( const U32 x, const U32 y, const U32 z, const U32 w )  ///< xyzw = { x y z w }
{
#if defined (_PC_SSE2 )
	__m128i result;
	U32* pResult = reinterpret_cast <U32*> ( &result );
	pResult[0] = x;
	pResult[1] = y;
	pResult[2] = z;
	pResult[3] = w;
	return result;
#else
	VecUInt4 result;
	result.x = x;
	result.y = y;
	result.z = z;
	result.w = w;
	return result;
#endif
}

FINLINE_Z VecUInt4 VecUIntLoadAligned ( const void* alignedMemory ) ///< Load xyzw from a 16-byte aligned memory address
{
#if defined (_PC_SSE2 )
	return _mm_load_si128( reinterpret_cast<const __m128i*>(alignedMemory) );
#else
	VecUInt4 result;
	memcpy( &result, alignedMemory, sizeof(result) );
	return result;
#endif
}

FINLINE_Z VecUInt4 VecUIntLoadUnaligned ( const void* unalignedMemory ) ///< Load xyzw from any memory address
{
#if defined ( _PC_SSE3 )
	return _mm_lddqu_si128( reinterpret_cast<const __m128i*>(unalignedMemory) );
#elif defined (_PC_SSE2 )
	return _mm_loadu_si128( reinterpret_cast<const __m128i*>(unalignedMemory) );
#else
	VecUInt4 result;
	memcpy( &result, unalignedMemory, sizeof(result) );
	return result;
#endif
}


// ---------------------- Save operations

FINLINE_Z void VecUIntStoreAligned ( void* dest, register const VecUInt4& source ) ///< dest = source, with dest a 16-byte aligned memory adress
{
#if defined (_PC_SSE2 )
	_mm_store_si128( reinterpret_cast<__m128i*>(dest), source );
#else
	memcpy( dest, &source, sizeof(source) );
#endif
}

FINLINE_Z void VecUIntStreamStoreAligned ( void* dest, register const VecUInt4& source ) ///< dest = source, with dest a 16-byte aligned memory address. This store doesn't pollute the cache, it is slower when dest points to data already in cache, and faster otherwise
{
#if defined (_PC_SSE2 )
	_mm_stream_si128( reinterpret_cast<__m128i*>(dest), source );
#else
	VecUIntStoreAligned( dest, source );
#endif
}

FINLINE_Z void VecUIntStoreUnaligned ( void* dest, register const VecUInt4& source ) ///< dest = source. No specific memory alignement is required for dest, but prefer an aligned store when possible.
{
#if defined (_PC_SSE2 )
	_mm_storeu_si128( reinterpret_cast<__m128i*>(dest), source );
#else
	memcpy( dest, &source, sizeof(source) );
#endif
}


// ---------------------- Logical operations

FINLINE_Z VecUInt4 VecUIntAnd ( register const VecUInt4& a, register const VecUInt4& b ) ///< xyzw = a.xyzw & b.xyzw
{
#if defined (_PC_SSE2 )
	return _mm_and_si128( a, b );
#else
	VecUInt4 result;
	result.x = a.x & b.x;
	result.y = a.y & b.y;
	result.z = a.z & b.z;
	result.w = a.w & b.w;
	return result;
#endif
}

FINLINE_Z VecUInt4 VecUIntAndNot ( register const VecUInt4& a, register const VecUInt4& b ) ///< xyzw = a.xyzw & ~b.xyzw
{
#if defined (_PC_SSE2 )
	return _mm_andnot_si128( b, a );
#else
	VecUInt4 result;
	result.x = a.x & (~b.x);
	result.y = a.y & (~b.y);
	result.z = a.z & (~b.z);
	result.w = a.w & (~b.w);
	return result;
#endif
}

FINLINE_Z VecUInt4 VecUIntOr ( register const VecUInt4& a, register const VecUInt4& b ) ///< xyzw = a.xyzw | b.xyzw
{
#if defined (_PC_SSE2 )
	return _mm_or_si128( a, b );
#else
	VecUInt4 result;
	result.x = a.x | b.x;
	result.y = a.y | b.y;
	result.z = a.z | b.z;
	result.w = a.w | b.w;
	return result;
#endif
}

FINLINE_Z VecUInt4 VecUIntXor ( register const VecUInt4& a, register const VecUInt4& b ) ///< xyzw = a.xyzw ^ b.xyzw
{
#if defined (_PC_SSE2 )
	return _mm_xor_si128( a, b );
#else
	VecUInt4 result;
	result.x = a.x ^ b.x;
	result.y = a.y ^ b.y;
	result.z = a.z ^ b.z;
	result.w = a.w ^ b.w;
	return result;
#endif
}


// ---------------------- Arithmetic operations

FINLINE_Z VecUInt4 VecUIntAdd ( register const VecUInt4& a, register const VecUInt4& b ) ///< xyzw = a.xyzw + b.xyzw
{
#if defined (_PC_SSE2 )
	return _mm_add_epi32( a, b );
#else
	return VecUIntLoad4( a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w );
#endif
}

FINLINE_Z VecUInt4 VecUIntSub ( register const VecUInt4& a, register const VecUInt4& b ) ///< xyzw = a.xyzw - b.xyzw
{
#if defined (_PC_SSE2 )
	return _mm_sub_epi32( a, b );
#else
	return VecUIntLoad4( a.x-b.x, a.y-b.y, a.z-b.z, a.w-b.w );
#endif
}

FINLINE_Z VecUInt4 VecUIntMul( register const VecUInt4& a, register const VecUInt4& b ) ///< res.xyzw = a.xyzw * b.xyzw
{
#if defined ( _PC_SSE4 )
	return _mm_mullo_epi32( a, b );
#elif defined ( _PC_SSE2 )
	const int shuffleMask = _MM_SHUFFLE(0,0,2,0);
	const __m128i res0 = _mm_mul_epu32( a, b );
	const __m128i res1 = _mm_mul_epu32( _mm_srli_si128(a,4), _mm_srli_si128(b,4) );
	const __m128i res0XZXX = _mm_shuffle_epi32( res0, shuffleMask );
	const __m128i res1XZXX = _mm_shuffle_epi32( res1, shuffleMask );
	return _mm_unpacklo_epi32( res0XZXX, res1XZXX );
#else
	UInt4 aUI, bUI, result;
	VecUIntStoreAligned( &aUI, a );
	VecUIntStoreAligned( &bUI, b );
	for( U32 i = 0; i < 4; ++i )
		result.xyzw[i] = aUI.xyzw[i] * bUI.xyzw[i];
	return VecUIntLoadAligned( &result );
#endif
}

FINLINE_Z VecULongLong2 VecUIntMul64( register const VecUInt4& a, register const VecUInt4& b ) ///< res.xy = a.xz * b.xz
{
#if defined (_PC_SSE2 )
	return _mm_mul_epu32( a, b );
#else
	UInt4 aI, bI;
	ULongLong2 result;
	VecUIntStoreAligned( &aI, a );
	VecUIntStoreAligned( &bI, b );
	result.x = (U64)aI.xyzw[0] * (U64)bI.xyzw[0];
	result.y = (U64)aI.xyzw[2] * (U64)bI.xyzw[2];
	return result;
#endif
}

FINLINE_Z VecUInt4 VecUIntShiftLeft ( register const VecUInt4& a, const U32 offset )  ///< xyzw = a.xyzw << offset
{
#if defined (_PC_SSE2 )
	return _mm_slli_epi32( a, offset );
#else
	return VecUIntLoad4( a.x<<offset, a.y<<offset, a.z<<offset, a.w<<offset );
#endif
}

FINLINE_Z VecUInt4 VecUIntShiftRight ( register const VecUInt4& a, const U32 offset )  ///< xyzw = a.xyzw >> Offset
{
#if defined (_PC_SSE2 )
	return _mm_srli_epi32( a, offset );
#else
	return VecUIntLoad4( a.x>>offset, a.y>>offset, a.z>>offset, a.w>>offset );
#endif
}


////////////////////////// 16-bit signed integer vectors //////////////////////////


// ---------------------- Load operations

FINLINE_Z VecShort8 VecShortLoad1 ( const S16 a )  ///< Replicate a into the 8 components of the vector register
{
#if defined (_PC_SSE2 )
	return _mm_set1_epi16( a );
#else
	VecShort8 result;
	for( U32 i = 0; i < 8; ++i )
		result.s0_7[i] = a;
	return result;
#endif
}

FINLINE_Z VecShort8 VecShortLoad8 ( S16 a, S16 b, S16 c, S16 d, S16 e, S16 f, S16 g, S16 h )  ///< Load 8 S16 into a vector register
{
	const Short8 result = { a, b, c, d, e, f, g, h };
	return VecShortLoadAligned( &result );
}

FINLINE_Z VecShort8 VecShortLoadAligned ( const void* alignedMemory ) ///< Load 8 signed shorts from a 16-byte aligned memory address 
{
#if defined (_PC_SSE2 )
	return _mm_load_si128( (__m128i*)(alignedMemory) );
#else
	VecShort8 result;
	memcpy( &result, alignedMemory, sizeof(result) );
	return result;
#endif
}

FINLINE_Z VecShort8 VecShortLoadUnaligned ( const void* unalignedMemory ) ///< Load 8 signed shorts from any (CPU-cacheable) memory address
{
#if defined ( _PC_SSE3 )
	return _mm_lddqu_si128( reinterpret_cast<const __m128i*>(unalignedMemory) );
#elif defined (_PC_SSE2 )
	return _mm_loadu_si128( reinterpret_cast<const __m128i*>(unalignedMemory) );
#else
	VecShort8 result;
	memcpy( &result, unalignedMemory, sizeof(result) );
	return result;
#endif
}


// ---------------------- Set operations

FINLINE_Z VecShort8 VecShortSplatZero () ///< abcdefgh = 0
{
#if defined (_PC_SSE2 )
	return _mm_setzero_si128();
#else
	return VecShortLoad1( 0 );
#endif
}

FINLINE_Z VecShort8 VecShortSplatOne () ///< abcdefgh = 1
{
#if defined (_PC_SSE2 )
	return _mm_set1_epi16( 1 );
#else
	return VecShortLoad1( 1 );
#endif
}


// ---------------------- Save operations

template <U8 index> FINLINE_Z S16 VecShortGet ( register const VecShort8& a ) ///< access a[index]
{
	const S16* val = reinterpret_cast <const S16*> ( &a );
	return val[index];
}

template <U8 index> FINLINE_Z S32 VecShortGetInt ( register const VecShort8& a ) ///< return (S32)a[index]
{
#if defined (_PC_SSE2 )
	return _mm_extract_epi16( a, index );
#else
	const S16 res = VecShortGet <index> ( a );
	return static_cast <S32> ( res );
#endif
}

FINLINE_Z void VecShortStoreAligned ( void* dest, register const VecShort8& source ) ///< dest = source, with dest a 16-byte aligned memory address
{
#if defined (_PC_SSE2 )
	_mm_store_si128( reinterpret_cast<__m128i*>(dest), source );
#else
	memcpy( dest, &source, sizeof(source) );
#endif
}

FINLINE_Z void VecShortStreamStoreAligned ( void* dest, register const VecShort8& source ) ///< dest = source, with dest a 16-byte aligned memory address. This store doesn't pollute the cache, it is slower when dest points to data already in cache, and faster otherwise
{
#if defined (_PC_SSE2 )
	_mm_stream_si128( reinterpret_cast<__m128i*>(dest), source );
#else
	VecShortStoreAligned( dest, source );
#endif
}

FINLINE_Z void VecShortStoreUnaligned ( void* dest, register const VecShort8& source ) ///< dest = source. No specific memory alignement is required for dest, but prefer an aligned store when possible.
{
#if defined (_PC_SSE2 )
	_mm_storeu_si128( reinterpret_cast<__m128i*>(dest), source );
#else
	memcpy( dest, &source, sizeof(source) );
#endif
}


// ---------------------- Select operations

FINLINE_Z VecSelMask VecShortCompareGT ( register const VecShort8& a, register const VecShort8& b ) ///< for i � [0,7] : ( a[i] > b[i] ) ? # : 0
{
#if defined (_PC_SSE2 )
	return _mm_castsi128_ps(_mm_cmpgt_epi16( a, b ));
#else
	VecSelMask result;
	Short8& resultUS = reinterpret_cast <Short8&> ( result );
	for( U32 i = 0; i < 8; ++i )
		resultUS.s0_7[i] = ( a.s0_7[i] > b.s0_7[i] ) ? 0xFFFF : 0;
	return result;
#endif
}

FINLINE_Z VecShort8 VecShortSelectMask ( register const VecShort8& a, register const VecShort8& b, register const VecSelMask& mask ) ///< for i � [0,7] : res[i] = ( mask[i] == 0 ) ? a[i] : b[i]
{
	return VecShortSelectBitMask( a, b, (const VecShort8&)mask );
}

FINLINE_Z VecShort8 VecShortSelectBitMask ( register const VecShort8& a, register const VecShort8& b, register const VecShort8& bitMask ) ///< for bit i � [0,127] : res[i] = ( mask[i] == 0 ) ? a[i] : b[i]
{
#if defined (_PC_SSE2 )
	return _mm_sel_epi8( a, b, bitMask );
#else
	Short8 result;
	S16 maskValue;
	for( U32 i = 0; i < 8; ++i )
	{
		maskValue = bitMask.s0_7[i];
		result.s0_7[i] = (a.s0_7[i]&~maskValue) | (b.s0_7[i]&maskValue);
	}
	return result;
#endif
}

FINLINE_Z VecShort8 VecShortSelectGreater ( register const VecShort8& a, register const VecShort8& b, register const VecShort8& success, register const VecShort8& fail ) ///< for i � [0,7] : ( a[i] > b[i] ) ? success[i] : fail[i]
{
#if defined (_PC_SSE2 )
	return VecShortSelectMask( fail, success, VecShortCompareGT(a,b) );
#else
	Short8 result;
	for( U32 i = 0; i < 8; ++i )
		result.s0_7[i] = ( a.s0_7[i] > b.s0_7[i] ) ? success.s0_7[i] : fail.s0_7[i];
	return result;
#endif
}


// ---------------------- Logical operations

FINLINE_Z VecShort8 VecShortAnd ( register const VecShort8& a, register const VecShort8& b ) ///< Perform a bitwise and between registers a and b
{
#if defined (_PC_SSE2 )
	return _mm_and_si128( a, b );
#else
	VecShort8 result;
	for( U32 i = 0; i < 8; ++i )
		result.s0_7[i] = a.s0_7[i] & b.s0_7[i];
	return result;
#endif
}

FINLINE_Z VecShort8 VecShortOr ( register const VecShort8& a, register const VecShort8& b ) ///< Perform a bitwise or between registers a and b
{
#if defined (_PC_SSE2 )
	return _mm_or_si128( a, b );
#else
	VecShort8 result;
	for( U32 i = 0; i < 8; ++i )
		result.s0_7[i] = a.s0_7[i] | b.s0_7[i];
	return result;
#endif
}


// ---------------------- Arithmetic operations

FINLINE_Z VecShort8 VecShortAdd ( register const VecShort8& a, register const VecShort8& b ) ///< res.abcdefgh = a.abcdefgh + b.abcdefgh
{
#if defined( _PC_SSE2 )
	return _mm_add_epi16( a, b );
#else
	VecShort8 res;
	for( U32 i = 0; i < 8; ++i )
		res.s0_7[i] = a.s0_7[i] + b.s0_7[i];
	return res;
#endif
}

FINLINE_Z VecShort8 VecShortSub ( register const VecShort8& a, register const VecShort8& b ) ///< res.abcdefgh = a.abcdefgh - b.abcdefgh
{
#if defined( _PC_SSE2 )
	return _mm_sub_epi16( a, b );
#else
	VecShort8 res;
	for( U32 i = 0; i < 8; ++i )
		res.s0_7[i] = a.s0_7[i] - b.s0_7[i];
	return res;
#endif
}

FINLINE_Z VecShort8 VecShortMin ( register const VecShort8& a, register const VecShort8& b )  ///< abcdefgh = min ( a.abcdefgh, b.abcdefgh )
{
#if defined (_PC_SSE2 )
	return _mm_min_epi16( a, b );
#else
	VecShort8 result;
	for( U32 i = 0; i < 8; ++i )
		result.s0_7[i] = Min( a.s0_7[i], b.s0_7[i] );
	return result;
#endif
}

FINLINE_Z VecShort8 VecShortMax ( register const VecShort8& a, register const VecShort8& b )  ///< abcdefgh = max ( a.abcdefgh, b.abcdefgh )
{
#if defined (_PC_SSE2 )
	return _mm_max_epi16( a, b );
#else
	VecShort8 result;
	for( U32 i = 0; i < 8; ++i )
		result.s0_7[i] = Max( a.s0_7[i], b.s0_7[i] );
	return result;
#endif
}


////////////////////////// 16-bit unsigned integer vectors //////////////////////////


// ---------------------- Load operations

FINLINE_Z VecUShort8 VecUShortLoad1 ( const U16 a )  ///< Replicate a into the 8 components of the vector register
{
#if defined (_PC_SSE2 )
	return _mm_set1_epi16( a );
#else
	VecUShort8 result;
	for( U32 i = 0; i < 8; ++i )
		result.u0_7[i] = a;
	return result;
#endif
}

FINLINE_Z VecUShort8 VecUShortLoad8 ( U16 a, U16 b, U16 c, U16 d, U16 e, U16 f, U16 g, U16 h )  ///< Load 8 U16 into a vector register
{
	const UShort8 result = { a, b, c, d, e, f, g, h };
	return VecUShortLoadAligned( &result );
}

FINLINE_Z VecUShort8 VecUShortLoadAligned ( const void* alignedMemory ) ///< Load 8 unsigned shorts from a 16-byte aligned memory address 
{
#if defined (_PC_SSE2 )
	return _mm_load_si128( (__m128i*)(alignedMemory) );
#else
	VecUShort8 result;
	memcpy( &result, alignedMemory, sizeof(result) );
	return result;
#endif
}

FINLINE_Z VecUShort8 VecUShortLoadUnaligned ( const void* unalignedMemory ) ///< Load 8 unsigned shorts from any (CPU-cacheable) memory address
{
#if defined ( _PC_SSE3 )
	return _mm_lddqu_si128( reinterpret_cast<const __m128i*>(unalignedMemory) );
#elif defined (_PC_SSE2 )
	return _mm_loadu_si128( reinterpret_cast<const __m128i*>(unalignedMemory) );
#else
	VecUShort8 result;
	memcpy( &result, unalignedMemory, sizeof(result) );
	return result;
#endif
}


// ---------------------- Set operations

FINLINE_Z VecUShort8 VecUShortSplatZero () ///< abcdefgh = 0
{
#if defined (_PC_SSE2 )
	return _mm_setzero_si128();
#else
	return VecUShortLoad1( 0 );
#endif
}

FINLINE_Z VecUShort8 VecUShortSplatOne () ///< abcdefgh = 1
{
#if defined (_PC_SSE2 )
	return _mm_set1_epi16( 1 );
#else
	return VecUShortLoad1( 1 );
#endif
}


// ---------------------- Save operations

template <U8 index> FINLINE_Z U16 VecUShortGet ( register const VecUShort8& a ) ///< access a[index]
{
	const U16* val = reinterpret_cast <const U16*> ( &a );
	return val[index];
}

template <U8 index> FINLINE_Z S32 VecUShortGetInt ( register const VecUShort8& a ) ///< return (S32)a[index]
{
#if defined (_PC_SSE2 )
	return _mm_extract_epi16( a, index );
#else
	const U16 res = VecUShortGet <index> ( a );
	return static_cast <S32> ( res );
#endif
}

FINLINE_Z void VecUShortStoreAligned ( void* dest, register const VecUShort8& source ) ///< dest = source, with dest a 16-byte aligned memory address
{
#if defined (_PC_SSE2 )
	_mm_store_si128( reinterpret_cast<__m128i*>(dest), source );
#else
	memcpy( dest, &source, sizeof(source) );
#endif
}

FINLINE_Z void VecUShortStreamStoreAligned ( void* dest, register const VecUShort8& source ) ///< dest = source, with dest a 16-byte aligned memory address. This store doesn't pollute the cache, it is slower when dest points to data already in cache, and faster otherwise
{
#if defined (_PC_SSE2 )
	_mm_stream_si128( reinterpret_cast<__m128i*>(dest), source );
#else
	VecUShortStoreAligned( dest, source );
#endif
}

FINLINE_Z void VecUShortStoreUnaligned ( void* dest, register const VecUShort8& source ) ///< dest = source. No specific memory alignement is required for dest, but prefer an aligned store when possible.
{
#if defined (_PC_SSE2 )
	_mm_storeu_si128( reinterpret_cast<__m128i*>(dest), source );
#else
	memcpy( dest, &source, sizeof(source) );
#endif
}


// ---------------------- Select operations

FINLINE_Z VecSelMask VecUShortCompareGT ( register const VecUShort8& a, register const VecUShort8& b ) ///< for i � [0,7] : ( a[i] > b[i] ) ? # : 0
{
#if defined (_PC_SSE2 )
	const __m128i add = VecUShortLoadAligned( &VectorConstantsPrivate::vUShortCompareAdd );
	return  _mm_castsi128_ps( _mm_cmpgt_epi16( _mm_add_epi16(a,add), _mm_add_epi16(b,add) ) );
#else
	VecSelMask result;
	UShort8& resultUS = reinterpret_cast <UShort8&> ( result );
	for( U32 i = 0; i < 8; ++i )
		resultUS.u0_7[i] = ( a.u0_7[i] > b.u0_7[i] ) ? 0xFFFF : 0;
	return result;
#endif
}

FINLINE_Z VecSelMask VecUShortCompareEQ ( register const VecUShort8& a, register const VecUShort8& b ) ///< for i � [0,7] : ( a[i] == b[i] ) ? # : 0
{
#if defined (_PC_SSE2 )
	return  _mm_castsi128_ps( _mm_cmpeq_epi16( a, b ) );
#else
	VecSelMask result;
	UShort8& resultUS = reinterpret_cast <UShort8&> ( result );
	for( U32 i = 0; i < 8; ++i )
		resultUS.u0_7[i] = ( a.u0_7[i] == b.u0_7[i] ) ? 0xFFFF : 0;
	return result;
#endif
}

FINLINE_Z VecUShort8 VecUShortSelectMask ( register const VecUShort8& a, register const VecUShort8& b, register const VecSelMask& mask ) ///< for i � [0,7] : res[i] = ( mask[i] == 0 ) ? a[i] : b[i]
{
	return VecUShortSelectBitMask( a, b, (const VecUShort8&)mask );
}

FINLINE_Z VecUShort8 VecUShortSelectBitMask ( register const VecUShort8& a, register const VecUShort8& b, register const VecUShort8& bitMask ) ///< for bit i � [0,127] : res[i] = ( mask[i] == 0 ) ? a[i] : b[i]
{
#if defined (_PC_SSE2 )
	return _mm_sel_epi8( a, b, bitMask );
#else
	UShort8 result;
	U16 maskValue;
	for( U32 i = 0; i < 8; ++i )
	{
		maskValue = bitMask.u0_7[i];
		result.u0_7[i] = (a.u0_7[i]&~maskValue) | (b.u0_7[i]&maskValue);
	}
	return result;
#endif
}

FINLINE_Z VecUShort8 VecUShortSelectEqual ( register const VecUShort8& a, register const VecUShort8& b, register const VecUShort8& success, register const VecUShort8& fail ) ///< for i � [0,7] : ( a[i] == b[i] ) ? success[i] : fail[i]
{
#if defined (_PC_SSE2 )
	return VecUShortSelectMask( fail, success, VecUShortCompareEQ(a,b) );
#else
	UShort8 result;
	for( U32 i = 0; i < 8; ++i )
		result.u0_7[i] = ( a.u0_7[i] == b.u0_7[i] ) ? success.u0_7[i] : fail.u0_7[i];
	return result;
#endif
}

FINLINE_Z VecUShort8 VecUShortSelectGreater ( register const VecUShort8& a, register const VecUShort8& b, register const VecUShort8& success, register const VecUShort8& fail ) ///< for i � [0,7] : ( a[i] > b[i] ) ? success[i] : fail[i]
{
#if defined (_PC_SSE2 )
	return VecUShortSelectMask( fail, success, VecUShortCompareGT(a,b) );
#else
	UShort8 result;
	for( U32 i = 0; i < 8; ++i )
		result.u0_7[i] = ( a.u0_7[i] > b.u0_7[i] ) ? success.u0_7[i] : fail.u0_7[i];
	return result;
#endif
}



// ---------------------- Logical operations

FINLINE_Z VecUShort8 VecUShortAnd ( register const VecUShort8& a, register const VecUShort8& b ) ///< Perform a bitwise and between registers a and b
{
#if defined (_PC_SSE2 )
	return _mm_and_si128( a, b );
#else
	VecUShort8 result;
	for( U32 i = 0; i < 8; ++i )
		result.u0_7[i] = a.u0_7[i] & b.u0_7[i];
	return result;
#endif
}

FINLINE_Z VecUShort8 VecUShortOr ( register const VecUShort8& a, register const VecUShort8& b ) ///< Perform a bitwise or between registers a and b
{
#if defined (_PC_SSE2 )
	return _mm_or_si128( a, b );
#else
	VecUShort8 result;
	for( U32 i = 0; i < 8; ++i )
		result.u0_7[i] = a.u0_7[i] | b.u0_7[i];
	return result;
#endif
}


// ---------------------- Arithmetic operations

FINLINE_Z VecUShort8 VecUShortAdd ( register const VecUShort8& a, register const VecUShort8& b ) ///< res.abcdefgh = a.abcdefgh + b.abcdefgh
{
#if defined( _PC_SSE2 )
	return _mm_add_epi16( a, b );
#else
	VecUShort8 res;
	for( U32 i = 0; i < 8; ++i )
		res.u0_7[i] = a.u0_7[i] + b.u0_7[i];
	return res;
#endif
}

FINLINE_Z VecUShort8 VecUShortSub ( register const VecUShort8& a, register const VecUShort8& b ) ///< res.abcdefgh = a.abcdefgh - b.abcdefgh
{
#if defined( _PC_SSE2 )
	return _mm_sub_epi16( a, b );
#else
	VecUShort8 res;
	for( U32 i = 0; i < 8; ++i )
		res.u0_7[i] = a.u0_7[i] - b.u0_7[i];
	return res;
#endif
}

FINLINE_Z VecUShort8 VecUShortSubSat ( register const VecUShort8& a, register const VecUShort8& b ) ///< res.abcdefgh = saturate( a.abcdefgh - b.abcdefgh )
{
#if defined( _PC_SSE2 )
	return _mm_subs_epu16( a, b );
#else
	VecUShort8 res;
	for( U32 i = 0; i < 8; ++i )
	{
		S32 diff = a.u0_7[i] - b.u0_7[i];
		res.u0_7[i] = static_cast<U16>( Max <S32> ( diff, 0 ) );
	}
	return res;
#endif
}

FINLINE_Z VecUShort8 VecUShortMin ( register const VecUShort8& a, register const VecUShort8& b )  ///< abcdefgh = min ( a.abcdefgh, b.abcdefgh )
{
#if defined (_PC_SSE4 )
	return _mm_min_epu16( a, b );
#elif defined (_PC_SSE2 )
	return VecUShortSelectGreater( b, a, a, b ); 
#else
	VecUShort8 result;
	for( U32 i = 0; i < 8; ++i )
		result.u0_7[i] = Min( a.u0_7[i], b.u0_7[i] );
	return result;
#endif
}

FINLINE_Z VecUShort8 VecUShortMax ( register const VecUShort8& a, register const VecUShort8& b )  ///< abcdefgh = max ( a.abcdefgh, b.abcdefgh )
{
#if defined (_PC_SSE4 )
	return _mm_max_epu16( a, b );
#elif defined (_PC_SSE2 )
	return VecUShortSelectGreater( a, b, a, b ); 
#else
	VecUShort8 result;
	for( U32 i = 0; i < 8; ++i )
		result.u0_7[i] = Max( a.u0_7[i], b.u0_7[i] );
	return result;
#endif
}



// ---------------------- Swizzle operations

template <U16 ElementCount> FINLINE_Z VecUShort8 VecUShortRotateLeft ( register const VecUShort8& a ) ///< abcdefgh = a.abcdefgh <<< ElementCount
{
#if defined( _PC_SSE )
    __m128i left = _mm_slli_si128 ( a, ElementCount<<1 );
	__m128i right = _mm_srli_si128( a, (8-ElementCount)<<1 );
    return _mm_or_si128( left, right );
#else
	UShort8 result;
	for( U32 i = 0; i < 8; ++i )
		result.u0_7[i] = a.u0_7 [ (ElementCount+i) & 7 ];
    return VecUShortLoadAligned( result.u0_7 );
#endif
}

FINLINE_Z VecUShort8 VecUShortSwizzleABCDFFGH ( register const VecUShort8& a ) ///< res.abcdefgh = a.abcdffgh
{
#if defined( _PC_SSE2 )
	return _mm_shufflehi_epi16( a, _MM_SHUFFLE(3,2,1,1) );
#else
	return VecUShortLoad8( a.a, a.b, a.c, a.d, a.f, a.f, a.g, a.h );
#endif
}

FINLINE_Z VecUShort8 VecUShortSwizzleACEGACEG ( register const VecUShort8& a ) ///< res.abcdefgh = a.acegaceg
{
#if defined( _PC_SSE3 )
	return _mm_shuffle_epi8( a, VecUCharLoadAligned(&VectorConstantsPrivate::vSwizzleMaskACEGACEG) );
#elif defined( _PC_SSE2 )
	const __m128i vClearMask = _mm_set1_epi32( 0x00FF );
	const __m128i aI = _mm_and_si128( a, vClearMask );
	return _mm_packs_epi32( aI, aI );
#else
	return VecUShortLoad8( a.a, a.c, a.e, a.g, a.a, a.c, a.e, a.g );
#endif
}

FINLINE_Z VecUShort8 VecUShortSwizzleBDFHBDFH ( register const VecUShort8& a ) ///< res.abcdefgh = a.bdfhbdfh
{
#if defined( _PC_SSE2 )
	const __m128i aI = _mm_srli_epi32( a, 16 );
	return _mm_packs_epi32( aI, aI );
#else
	return VecUShortLoad8( a.b, a.d, a.f, a.h, a.b, a.d, a.f, a.h );
#endif
}

FINLINE_Z VecUShort8 VecUShortPermuteA0A1B0B1C0C1D0D1 ( const VecUShort8& a, const VecUShort8& b )  ///< Interleave the lower U16 in a with the lower U16 in b.
{
#if defined( _PC_SSE2 )   
    return _mm_unpacklo_epi16( a, b );
#else
	VecUShort8 result;
	for( U32 i = 0; i < 4; ++i )
	{
		result.u0_7[i*2+0] = a.u0_7[i];
		result.u0_7[i*2+1] = b.u0_7[i];
	}
	return result;
#endif 
}

FINLINE_Z VecUShort8 VecUShortPermuteE0E1F0F1G0G1H0H1 ( const VecUShort8& a, const VecUShort8& b )  ///< Interleave the higher U16 in a with the higher U16 in b.
{
#if defined( _PC_SSE2 )   
    return _mm_unpackhi_epi16( a, b );
#else
	VecUShort8 result;
	for( U32 i = 0; i < 4; ++i )
	{
		result.u0_7[i*2+0] = a.u0_7[i+4];
		result.u0_7[i*2+1] = b.u0_7[i+4];
	}
	return result;
#endif 
}



////////////////////////// 8-bit unsigned char vectors //////////////////////////


// ---------------------- Load operations

FINLINE_Z VecUChar16 VecUCharLoad1 ( const U8 a )  ///< Replicate a into the 16 components of the vector register
{
#if defined (_PC_SSE2 )
	return _mm_set1_epi8( a );
#else
	VecUChar16 result;
	memset( &result, a, sizeof(result) );
	return result;
#endif
}

FINLINE_Z VecUChar16 VecUCharLoad16 ( U8 u0, U8 u1, U8 u2, U8 u3, U8 u4, U8 u5, U8 u6, U8 u7, U8 u8, U8 u9, U8 u10, U8 u11, U8 u12, U8 u13, U8 u14, U8 u15 )  ///< Load 16 U8 into a vector register
{
#if defined (_PC_SSE2 )
	return _mm_set_epi8( u15, u14, u13, u12, u11, u10, u9, u8, u7, u6, u5, u4, u3, u2, u1, u0 );
#else
	UChar16 result;
	result.u0_15[0]  = u0;
	result.u0_15[1]  = u1;
	result.u0_15[2]  = u2;
	result.u0_15[3]  = u3;
	result.u0_15[4]  = u4;
	result.u0_15[5]  = u5;
	result.u0_15[6]  = u6;
	result.u0_15[7]  = u7;
	result.u0_15[8]  = u8;
	result.u0_15[9]  = u9;
	result.u0_15[10] = u10;
	result.u0_15[11] = u11;
	result.u0_15[12] = u12;
	result.u0_15[13] = u13;
	result.u0_15[14] = u14;
	result.u0_15[15] = u15;
	return VecUCharLoadAligned( &result );
#endif
}

FINLINE_Z VecUChar16 VecUCharLoadAligned ( const void* alignedMemory ) ///< Load 16 unsigned bytes from a 16-byte aligned memory address 
{
#if defined (_PC_SSE2 )
	return _mm_load_si128( (__m128i*)(alignedMemory) );
#else
	VecUChar16 result;
	memcpy( &result, alignedMemory, sizeof(result) );
	return result;
#endif
}

FINLINE_Z VecUChar16 VecUCharLoadUnaligned ( const void* unalignedMemory ) ///< Load 16 unsigned bytes from any (CPU-cacheable) memory address
{
#if defined ( _PC_SSE3 )
	return _mm_lddqu_si128( reinterpret_cast<const __m128i*>(unalignedMemory) );
#elif defined (_PC_SSE2 )
	return _mm_loadu_si128( reinterpret_cast<const __m128i*>(unalignedMemory) );
#else
	VecUChar16 result;
	memcpy( &result, unalignedMemory, sizeof(result) );
	return result;
#endif
}


// ---------------------- Set operations

FINLINE_Z VecUChar16 VecUCharSplatZero () ///< for( i ranging from 0 to 15 ) res[i] = 0
{
#if defined (_PC_SSE2 )
	return _mm_setzero_si128();
#else
	VecUChar16 result;
	memset( &result, 0, sizeof(result) );
	return result;
#endif
}

FINLINE_Z VecUChar16 VecUCharSplatOne () ///< for( i ranging from 0 to 15 ) res[i] = 1
{
#if defined (_PC_SSE2 )
	return _mm_set1_epi8( 1 );
#else
	VecUChar16 result;
	memset( &result, 1, sizeof(result) );
	return result;
#endif
}


// ---------------------- Save operations

template <U8 index> FINLINE_Z U8 VecUCharGet ( register const VecUChar16& a ) ///< return a[index]
{
	const U8* val = reinterpret_cast <const U8*> ( &a );
	return val[index];
}

template <U8 index> FINLINE_Z S32 VecUCharGetInt ( register const VecUChar16& a ) ///< return (S32)a[index]
{
#if defined (_PC_SSE4 )
	return _mm_extract_epi8( a, index );
#else
	const U8 res = VecUCharGet <index> ( a );
	return static_cast <S32> ( res );
#endif
}

FINLINE_Z void VecUCharStoreAligned ( void* dest, register const VecUChar16& source ) ///< dest = source, with dest a 16-byte aligned memory address
{
#if defined (_PC_SSE2 )
	_mm_store_si128( reinterpret_cast<__m128i*>(dest), source );
#else
	memcpy( dest, &source, sizeof(source) );
#endif
}

FINLINE_Z void VecUCharStreamStoreAligned ( void* dest, register const VecUChar16& source ) ///< dest = source, with dest a 16-byte aligned memory address. This store doesn't pollute the cache, it is slower when dest points to data already in cache, and faster otherwise
{
#if defined (_PC_SSE2 )
	_mm_stream_si128( reinterpret_cast<__m128i*>(dest), source );
#else
	VecUCharStoreAligned( dest, source );
#endif
}

FINLINE_Z void VecUCharStoreUnaligned ( void* dest, register const VecUChar16& source ) ///< dest = source. No specific memory alignement is required for dest, but prefer an aligned store when possible.
{
#if defined (_PC_SSE2 )
	_mm_storeu_si128( reinterpret_cast<__m128i*>(dest), source );
#else
	memcpy( dest, &source, sizeof(source) );
#endif
}

// ---------------------- Select operations

FINLINE_Z VecSelMask VecUCharCompareGT ( register const VecUChar16& a, register const VecUChar16& b ) ///< for i � [0,15] : ( a[i] > b[i] ) ? # : 0
{
#if defined (_PC_SSE2 )
	const __m128i add = VecUCharLoadAligned( &VectorConstantsPrivate::vUCharCompareAdd );
	return  _mm_castsi128_ps( _mm_cmpgt_epi8( _mm_add_epi8(a,add), _mm_add_epi8(b,add) ) );
#else
	VecSelMask result;
	UChar16& resultUC = reinterpret_cast <UChar16&> ( result );
	for( U32 i = 0; i < 16; ++i )
		resultUC.u0_15[i] = ( a.u0_15[i] > b.u0_15[i] ) ? 0xFF : 0;
	return result;
#endif
}

FINLINE_Z VecSelMask VecUCharCompareEQ ( register const VecUChar16& a, register const VecUChar16& b ) ///< for i � [0,15] : ( a[i] == b[i] ) ? # : 0
{
#if defined (_PC_SSE2 )
	return  _mm_castsi128_ps( _mm_cmpeq_epi8( a, b ) );
#else
	VecSelMask result;
	UChar16& resultUC = reinterpret_cast <UChar16&> ( result );
	for( U32 i = 0; i < 16; ++i )
		resultUC.u0_15[i] = ( a.u0_15[i] == b.u0_15[i] ) ? 0xFF : 0;
	return result;
#endif
}

FINLINE_Z U32 VecSelMaskPackBits( register const VecSelMask& v ) ///< res = isTrue(v.w)<<3 | isTrue(v.z)<<2 | isTrue(v.y)<<1 | isTrue(v.x)
{
#if defined(_PC_SSE )
	return _mm_movemask_ps( v );
#else
	const Int4& vI = reinterpret_cast <const Int4&> ( v );
	const U32 sx = vI.x != 0 ? 1 : 0;
	const U32 sy = vI.y != 0 ? 2 : 0;
	const U32 sz = vI.z != 0 ? 4 : 0;
	const U32 sw = vI.w != 0 ? 8 : 0;
	return sx | sy | sz | sw;
#endif
}

FINLINE_Z VecUChar16 VecUCharSelectMask ( register const VecUChar16& a, register const VecUChar16& b, register const VecSelMask& mask ) ///< for i � [0,15] : res[i] = ( mask[i] == 0 ) ? a[i] : b[i]
{
	return VecUCharSelectBitMask( a, b, (const VecUChar16&)mask );
}

FINLINE_Z VecUChar16 VecUCharSelectBitMask ( register const VecUChar16& a, register const VecUChar16& b, register const VecUChar16& bitMask ) ///< for bit i � [0,127] : res[i] = ( bitMask[i] == 0 ) ? a[i] : b[i]
{
#if defined (_PC_SSE2 )
	return _mm_sel_epi8( a, b, bitMask );
#else
	UChar16 result;
	U8 maskValue;
	for( U32 i = 0; i < 16; ++i )
	{
		maskValue = bitMask.u0_15[i];
		result.u0_15[i] = (a.u0_15[i]&~maskValue) | (b.u0_15[i]&maskValue);
	}
	return result;
#endif
}

FINLINE_Z VecUChar16 VecUCharSelectEqual ( register const VecUChar16& a, register const VecUChar16& b, register const VecUChar16& success, register const VecUChar16& fail ) ///< for i � [0,15] : ( a[i] == b[i] ) ? success[i] : fail[i]
{
#if defined (_PC_SSE2 )
	return VecUCharSelectMask( fail, success, VecUCharCompareEQ(a,b) );
#else
	UChar16 result;
	for( U32 i = 0; i < 16; ++i )
		result.u0_15[i] = ( a.u0_15[i] == b.u0_15[i] ) ? success.u0_15[i] : fail.u0_15[i];
	return result;
#endif
}

FINLINE_Z VecUChar16 VecUCharSelectGreater ( register const VecUChar16& a, register const VecUChar16& b, register const VecUChar16& success, register const VecUChar16& fail ) ///< for i � [0,15] : ( a[i] > b[i] ) ? success[i] : fail[i]
{
#if defined (_PC_SSE2 )
	return VecUCharSelectMask( fail, success, VecUCharCompareGT(a,b) );
#else
	UChar16 result;
	for( U32 i = 0; i < 16; ++i )
		result.u0_15[i] = ( a.u0_15[i] > b.u0_15[i] ) ? success.u0_15[i] : fail.u0_15[i];
	return result;
#endif
}



// ---------------------- Logical operations

FINLINE_Z VecUChar16 VecUCharAnd ( register const VecUChar16& a, register const VecUChar16& b ) ///< Perform a bitwise and between registers a and b
{
#if defined (_PC_SSE2 )
	return _mm_and_si128( a, b );
#else
	VecUChar16 result;
	for( U32 i = 0; i < 16; ++i )
		result.u0_15[i] = a.u0_15[i] & b.u0_15[i];
	return result;
#endif
}

FINLINE_Z VecUChar16 VecUCharOr ( register const VecUChar16& a, register const VecUChar16& b ) ///< Perform a bitwise or between registers a and b
{
#if defined (_PC_SSE2 )
	return _mm_or_si128( a, b );
#else
	VecUChar16 result;
	for( U32 i = 0; i < 16; ++i )
		result.u0_15[i] = a.u0_15[i] | b.u0_15[i];
	return result;
#endif
}


// ---------------------- Arithmetic operations

FINLINE_Z VecUChar16 VecUCharAdd ( register const VecUChar16& a, register const VecUChar16& b ) ///< res.abcdefghijklmnop = a.abcdefghijklmnop + b.abcdefghijklmnop
{
#if defined( _PC_SSE2 )
	return _mm_add_epi8( a, b );
#else
	VecUChar16 res;
	for( U32 i = 0; i < 16; ++i )
		res.u0_15[i] = a.u0_15[i] + b.u0_15[i];
	return res;
#endif
}

FINLINE_Z VecUChar16 VecUCharSub ( register const VecUChar16& a, register const VecUChar16& b ) ///< res.abcdefghijklmnop = a.abcdefghijklmnop - b.abcdefghijklmnop
{
#if defined( _PC_SSE2 )
	return _mm_sub_epi8( a, b );
#else
	VecUChar16 res;
	for( U32 i = 0; i < 16; ++i )
		res.u0_15[i] = a.u0_15[i] - b.u0_15[i];
	return res;
#endif
}

FINLINE_Z VecUChar16 VecUCharSubSat ( register const VecUChar16& a, register const VecUChar16& b ) ///< res.abcdefghijklmnop = saturate( a.abcdefghijklmnop - b.abcdefghijklmnop )
{
#if defined( _PC_SSE2 )
	return _mm_subs_epu8( a, b );
#else
	VecUChar16 res;
	for( U32 i = 0; i < 16; ++i )
	{
		S16 diff = a.u0_15[i] - b.u0_15[i];
		diff = Max <S16> ( diff, 0 );
		res.u0_15[i] = static_cast <U8> ( diff );
	}
	return res;
#endif
}

FINLINE_Z VecUChar16 VecUCharAbsDiff ( register const VecUChar16& a, register const VecUChar16& b ) ///< res.abcdefghijklmnop = abs( a.abcdefghijklmnop - b.abcdefghijklmnop )
{
#if defined( _PC_SSE2 )
	return VecUCharMax( VecUCharSubSat(a,b), VecUCharSubSat(b,a) );
#else
	VecUChar16 res;
	for( U32 i = 0; i < 16; ++i )
	{
		S16 diff = a.u0_15[i] - b.u0_15[i];
		res.u0_15[i] = static_cast <U8> ( Abs(diff) );
	}
	return res;
#endif
}

FINLINE_Z VecUChar16 VecUCharMin ( register const VecUChar16& a, register const VecUChar16& b )  ///< abcdefghijklmnop = min ( a.abcdefghijklmnop, b.abcdefghijklmnop )
{
#if defined (_PC_SSE2 )
	return _mm_min_epu8( a, b );
#else
	VecUChar16 result;
	for( U32 i = 0; i < 16; ++i )
		result.u0_15[i] = Min( a.u0_15[i], b.u0_15[i] );
	return result;
#endif
}

FINLINE_Z VecUChar16 VecUCharMax ( register const VecUChar16& a, register const VecUChar16& b )  ///< abcdefghijklmnop = max ( a.abcdefghijklmnop, b.abcdefghijklmnop )
{
#if defined (_PC_SSE2 )
	return _mm_max_epu8( a, b );
#else
	VecUChar16 result;
	for( U32 i = 0; i < 16; ++i )
		result.u0_15[i] = Max( a.u0_15[i], b.u0_15[i] );
	return result;
#endif
}

FINLINE_Z VecUShort8 VecUCharSad ( register const VecUChar16& a, register const VecUChar16& b ) ///< Sum of the absolute difference: res = { abs(a[0]-b[0])+abs(a[1]-b[1])+...+abs(a[7]-b[7]), undefined, undefined, undefined, abs(a[8]-b[8])+abs(a[9]-b[9])+...+abs(a[15]-b[15]), undefined, undefined, undefined }
{
#if defined( _PC_SSE2 )
	return _mm_sad_epu8( a, b );
#else
	VecUShort8 res;
	res.a = res.e = 0;
	for( U32 i = 0; i < 8; ++i )
	{
		S16 r0 = a.u0_15[i] - b.u0_15[i];
		S16 r1 = a.u0_15[8+i] - b.u0_15[8+i];
		res.a += Abs( r0 );
		res.e += Abs( r1 );
	}
	return res;
#endif
}

// ---------------------- Swizzle operations

FINLINE_Z VecUChar16 VecUCharSwizzleABCMDEFNGHIOJKLP ( register const VecUChar16& a )  ///< res = { a.abc, a.m, a.def, a.n, a.ghi, a.o, a.jkl, a.p }
{
#if defined( _PC_SSE3 )   
    return _mm_shuffle_epi8( a, VecUCharLoadAligned(&VectorConstantsPrivate::vSwizzleMaskABCMDEFNGHIOJKLP) );
#else
	VecUChar16 result;
	for( U32 i = 0; i < 4; ++i )
	{
		U32 offsetDest = i << 2;
		U32 offsetSrc = i * 3;
		result.u0_15[offsetDest+0] = a.u0_15[offsetSrc+0];
		result.u0_15[offsetDest+1] = a.u0_15[offsetSrc+1];
		result.u0_15[offsetDest+2] = a.u0_15[offsetSrc+2];
		result.u0_15[offsetDest+3] = a.u0_15[12+i];
	}
	return result;
#endif 
}

FINLINE_Z VecUChar16 VecUCharSwizzleACEGIKMOBDFHJLNP ( register const VecUChar16& a ) ///< res.abcdefghijklmnop = res.acegikmobdfhjlnp
{
#if defined( _PC_SSE3 )
	return _mm_shuffle_epi8( a, VecUCharLoadAligned(&VectorConstantsPrivate::vSwizzleMaskACEGIKMOBDFHJLNP) );
#elif defined( _PC_SSE2 )
	__m128i vACEGIKMO, vBDFHJLNP;
	VecUCharToUShortEvenOdd( vACEGIKMO, vBDFHJLNP, a );
	return _mm_packs_epi16( vACEGIKMO, vBDFHJLNP );
#else
	return VecUCharLoad16( a.u0,a.u2,a.u4,a.u6,a.u8,a.u10,a.u12,a.u14, a.u1,a.u3,a.u5,a.u7,a.u9,a.u11,a.u13,a.u15 );
#endif
}

FINLINE_Z VecUChar16 VecUCharSwizzleCBAMFEDNIHGOLKJP ( register const VecUChar16& a )  ///< res = { a.cba, a.m, a.fed, a.n, a.ihg, a.o, a.lkj, a.p }
{
#if defined( _PC_SSE3 )   
    return _mm_shuffle_epi8( a, VecUCharLoadAligned(&VectorConstantsPrivate::vSwizzleMaskCBAMFEDNIHGOLKJP) );
#else
	VecUChar16 result;
	for( U32 i = 0; i < 4; ++i )
	{
		U32 offsetDest = i << 2;
		U32 offsetSrc = i * 3;
		result.u0_15[offsetDest+0] = a.u0_15[offsetSrc+2];
		result.u0_15[offsetDest+1] = a.u0_15[offsetSrc+1];
		result.u0_15[offsetDest+2] = a.u0_15[offsetSrc+0];
		result.u0_15[offsetDest+3] = a.u0_15[12+i];
	}
	return result;
#endif 
}

FINLINE_Z VecUChar16 VecUCharPermuteLow ( const VecUChar16& a, const VecUChar16& b )  ///< Interleave the lower U8 in a with the lower U8 in b.
{
#if defined( _PC_SSE2 )   
    return _mm_unpacklo_epi8( a, b );
#else
	VecUChar16 result;
	for( U32 i = 0; i < 8; ++i )
	{
		result.u0_15[i*2+0] = a.u0_15[i];
		result.u0_15[i*2+1] = b.u0_15[i];
	}
	return result;
#endif 
}

FINLINE_Z VecUChar16 VecUCharPermuteHigh ( const VecUChar16& a, const VecUChar16& b )  ///< Interleave the higher U8 in a with the higher U8 in b.
{
#if defined( _PC_SSE2 )   
    return _mm_unpackhi_epi8( a, b );
#else
	VecUChar16 result;
	for( U32 i = 0; i < 8; ++i )
	{
		result.u0_15[i*2+0] = a.u0_15[i+8];
		result.u0_15[i*2+1] = b.u0_15[i+8];
	}
	return result;
#endif 
}




////////////////////////// Select mask vectors //////////////////////////


// ---------------------- Set operations

FINLINE_Z VecSelMask VecSelMaskSplatTrue () ///< xyzw = 0xFFFFFFFF
{
#if defined (_PC_SSE )
	const VecSelMask vFalse = _mm_setzero_ps();
	return _mm_cmpeq_ps( vFalse, vFalse );
#else
	VecSelMask result;
	memset( &result, 255, sizeof(result) );
	return result;
#endif
}

FINLINE_Z VecSelMask VecSelMaskSplatFalse () ///< xyzw = 0
{
#if defined (_PC_SSE )
	return _mm_setzero_ps();
#else
	VecSelMask result;
	memset( &result, 0, sizeof(result) );
	return result;
#endif
}



// ---------------------- Logical operations

FINLINE_Z VecSelMask VecSelMaskAnd( register const VecSelMask& a, register const VecSelMask& b ) ///< for bit i € [0,127] { res[i] = a[i] & b[i] }
{
#if defined (_PC_SSE )
	return VecFloatAnd( a, b );
#else
	Int4 result;
	for( U32 i = 0; i < 4; ++i )
		result.xyzw[i] = a.xyzw[i] & b.xyzw[i];
	return result;
#endif
}

FINLINE_Z VecSelMask VecSelMaskOr( register const VecSelMask& a, register const VecSelMask& b ) ///< for bit i € [0,127] { res[i] = a[i] | b[i] }
{
#if defined (_PC_SSE )
	return VecFloatOr( a, b );
#else
	Int4 result;
	for( U32 i = 0; i < 4; ++i )
		result.xyzw[i] = a.xyzw[i] | b.xyzw[i];
	return result;
#endif
}

FINLINE_Z VecSelMask VecSelMaskXor( register const VecSelMask& a, register const VecSelMask& b ) ///< for bit i € [0,127] { res[i] = a[i] ^ b[i] }
{
#if defined (_PC_SSE )
	return VecFloatXor( a, b );
#else
	Int4 result;
	for( U32 i = 0; i < 4; ++i )
		result.xyzw[i] = a.xyzw[i] ^ b.xyzw[i];
	return result;
#endif
}

FINLINE_Z VecSelMask VecSelMaskAndNot( register const VecSelMask& a, register const VecSelMask& b ) ///< for bit i € [0,127] { res[i] = a[i] & ~b[i] }
{
#if defined (_PC_SSE )
	return VecFloatAndNot( a, b );
#else
	Int4 result;
	for( U32 i = 0; i < 4; ++i )
		result.xyzw[i] = a.xyzw[i] & (~b.xyzw[i]);
	return result;
#endif
}

FINLINE_Z U32 VecSelMaskAllTrue( register const VecSelMask& a ) ///< return a.x != 0 && a.y != 0 && a.z != 0 && a.w != 0
{
#if defined (_PC_SSE )
	U32 result = _mm_movemask_ps( a );  // result � [0,15]
	return (result+1) >> 4;
#else
	return a.x != 0 && a.y != 0 && a.z != 0 && a.w != 0;
#endif
}

FINLINE_Z U32 VecSelMaskAll3True( register const VecSelMask& a ) ///< return a.x != 0 && a.y != 0 && a.z != 0
{
#if defined (_PC_SSE )
	U32 result = _mm_movemask_ps( a );
	return ( result & 7 ) == 7;
#else
	return a.x != 0 && a.y != 0 && a.z != 0;
#endif
}

FINLINE_Z U32 VecSelMaskAnyTrue( register const VecSelMask& a ) ///< return a.x != 0 || a.y != 0 || a.z != 0 || a.w != 0
{
#if defined (_PC_SSE )
	U32 result = _mm_movemask_ps( a );
	return result != 0;
#else
	return a.x != 0 || a.y != 0 || a.z != 0 || a.w != 0;
#endif
}

FINLINE_Z U32 VecSelMaskAllFalse( register const VecSelMask& a ) ///< return a.x == 0 && a.y == 0 && a.z == 0 && a.w == 0
{
#if defined (_PC_SSE )
	U32 result = _mm_movemask_ps( a );
	return ( result & 15 ) == 0;
#else
	return a.x == 0 && a.y == 0 && a.z == 0 && a.w == 0;
#endif
}

FINLINE_Z U32 VecSelMaskAll3False( register const VecSelMask& a ) ///< return a.x == 0 && a.y == 0 && a.z == 0
{
#if defined (_PC_SSE )
	U32 result = _mm_movemask_ps( a );
	return ( result & 7 ) == 0;
#else
	return a.x == 0 && a.y == 0 && a.z == 0;
#endif
}

FINLINE_Z U32 VecSelMaskAnyFalse( register const VecSelMask& a ) ///< return a.x == 0 || a.y == 0 || a.z == 0 || a.w == 0
{
#if defined (_PC_SSE )
	U32 result = _mm_movemask_ps( a );
	return result < 15;
#else
	return a.x == 0 || a.y == 0 || a.z == 0 || a.w == 0;
#endif
}

////////////////////////// Conversion operations //////////////////////////

FINLINE_Z VecInt4 VecFloatAsInt( register const VecFloat4& a ) ///< result = (VecInt4)a
{
#if defined (_PC_SSE2 )
	return _mm_castps_si128( a );
#else
	return reinterpret_cast <const VecInt4&> ( a );
#endif
}

FINLINE_Z VecFloat4 VecIntAsFloat( register const VecInt4& a ) ///< result = (VecFloat4)a
{
#if defined (_PC_SSE2 )
	return _mm_castsi128_ps( a );
#else
	return reinterpret_cast <const VecFloat4&> ( a );
#endif
}

FINLINE_Z VecUInt4 VecFloatAsUInt( register const VecFloat4& a ) ///< result = (VecUInt4)a
{
#if defined (_PC_SSE2 )
	return _mm_castps_si128( a );
#else
	return reinterpret_cast <const VecUInt4&> ( a );
#endif
}

FINLINE_Z VecFloat4 VecUIntAsFloat( register const VecUInt4& a ) ///< result = (VecFloat4)a
{
#if defined (_PC_SSE2 )
	return _mm_castsi128_ps( a );
#else
	return reinterpret_cast <const VecFloat4&> ( a );
#endif
}

FINLINE_Z VecUInt4 VecULongLongAsInt( register const VecULongLong2& a ) ///< result = (VecUInt4)a
{
#if defined (_PC_SSE2 )
	return a;
#else
	return VecUIntLoadAligned( &a );
#endif
}

FINLINE_Z VecULongLong2 VecUIntAsLongLong( register const VecUInt4& a ) ///< result = (VecULongLong2)a
{
#if defined (_PC_SSE2 )
	return a;
#else
	ULongLong2 result;
	VecUIntStoreAligned( &result, a );
	return result;
#endif
}

FINLINE_Z VecSelMask VecFloatAsSelMask( register const VecFloat4& a ) ///< result = (VecSelMask)a
{
#if defined (_PC_SSE2 )
	return a;
#else
	return reinterpret_cast <const VecSelMask&> ( a );
#endif
}

FINLINE_Z VecFloat4 VecSelMaskAsFloat( register const VecSelMask& a ) ///< result = (VecFloat4)a
{
#if defined (_PC_SSE2 )
	return a;
#else
	return reinterpret_cast <const VecFloat4&> ( a );
#endif
}

FINLINE_Z VecSelMask VecIntAsSelMask( register const VecInt4& a ) ///< result = (VecSelMask)a
{
#if defined (_PC_SSE2 )
	return _mm_castsi128_ps( a );
#else
	return reinterpret_cast <const VecSelMask&> ( a );
#endif
}

FINLINE_Z VecInt4 VecSelMaskAsInt( register const VecSelMask& a ) ///< result = (VecInt4)a
{
#if defined (_PC_SSE2 )
	return _mm_castps_si128( a );
#else
	return reinterpret_cast <const VecInt4&> ( a );
#endif
}

FINLINE_Z VecUInt4 VecIntAsUInt( register const VecInt4& a ) ///< result = (VecUInt4)a
{
#if defined (_PC_SSE2 )
	return a;
#else
	return reinterpret_cast <const VecUInt4&> ( a );
#endif
}

FINLINE_Z VecInt4 VecUIntAsInt( register const VecUInt4& a ) ///< result = (VecInt4)a
{
#if defined (_PC_SSE2 )
	return a;
#else
	return reinterpret_cast <const VecInt4&> ( a );
#endif
}

FINLINE_Z VecInt4 VecFloatToIntFloor ( register const VecFloat4& a ) ///< xyzw = (int4) floor( a.xyzw )
{
#if defined (_PC_SSE2 )
	// For further optimization, we can also assume that _MM_ROUND_DOWN is always the default rounding mode
	const int previousRounding = _MM_GET_ROUNDING_MODE();
	_MM_SET_ROUNDING_MODE( _MM_ROUND_DOWN );
	__m128i result = _mm_cvtps_epi32( a );
	_MM_SET_ROUNDING_MODE( previousRounding );
	return result;
#else
	return VecFloatToInt( VecFloatFloor(a) );
#endif
}

FINLINE_Z VecInt4 VecFloatToIntCeil ( register const VecFloat4& a ) ///< xyzw = (int4) ceil( a.xyzw )
{
#if defined (_PC_SSE2 )
	const int previousRounding = _MM_GET_ROUNDING_MODE();
	_MM_SET_ROUNDING_MODE( _MM_ROUND_UP );
	__m128i result = _mm_cvtps_epi32( a );
	_MM_SET_ROUNDING_MODE( previousRounding );
	return result;
#else
	return VecFloatToInt( VecFloatCeil(a) );
#endif
}

FINLINE_Z VecInt4 VecFloatToIntNearest ( register const VecFloat4& a ) ///< xyzw = (int4) rintf( a.xyzw )
{
#if defined (_PC_SSE2 )
	const int previousRounding = _MM_GET_ROUNDING_MODE();
	_MM_SET_ROUNDING_MODE( _MM_ROUND_NEAREST );
	__m128i result = _mm_cvtps_epi32( a );
	_MM_SET_ROUNDING_MODE( previousRounding );
	return result;
#else
	return VecFloatToInt( VecFloatNearest(a) );
#endif
}

FINLINE_Z VecInt4 VecFloatToIntNearestPositive ( register const VecFloat4& a ) ///< xyzw = (int4) rintf( a.xyzw ), assuming a.xyzw >= 0
{
#if defined (_PC_SSE2 )
	return VecFloatToInt( VecFloatAdd(a,VectorConstantsPrivate::vHalfF) );
#else
	return VecFloatToIntNearest( a );
#endif
}

FINLINE_Z VecInt4 VecFloatToInt ( register const VecFloat4& a ) ///< xyzw = (int4)a.xyzw
{
#if defined (_PC_SSE2 )
	return _mm_cvttps_epi32( a );
#else
	return VecIntLoad4( TRUNCINT(VecFloatGetX(a)), TRUNCINT(VecFloatGetY(a)), TRUNCINT(VecFloatGetZ(a)), TRUNCINT(VecFloatGetW(a)) );
#endif
}

FINLINE_Z VecFloat4 VecFloatFromShort4 ( const S16* s )   ///<  { (float)s[0], (float)s[1], (float)s[2], (float)s[3] }
{
	return VecFloatLoad4( static_cast<Float>(s[0]), static_cast<Float>(s[1]), static_cast<Float>(s[2]), static_cast<Float>(s[3]) );
}

template <U16 RightShift> FINLINE_Z VecFloat4 VecFloatFromUShort4 ( const U16* u )   ///<  for( i ranging from 0 to 3 ) results[i] = (float)(u[i]>>RightShift);
{
	return VecFloatLoad4( static_cast<Float>(u[0]>>RightShift), static_cast<Float>(u[1]>>RightShift), static_cast<Float>(u[2]>>RightShift), static_cast<Float>(u[3]>>RightShift) );
}

FINLINE_Z VecUShort8 VecFloatToUShortSat ( register const VecFloat4* v )   ///< for( i ranging from 0 to 7 ) res[i] = (U16)(v[i])
{
#if defined( _PC_SSE4 )
    return _mm_packus_epi32( _mm_cvttps_epi32(v[0]), _mm_cvttps_epi32(v[1]) );
#elif defined( _PC_SSE2 )
    __m128i vResults[2];
	// Convert from S32 to S16
	vResults[0] = _mm_cvttps_epi32( v[0] );
	vResults[1] = _mm_cvttps_epi32( v[1] );
	vResults[0] = _mm_packs_epi32( vResults[0], vResults[1] );
	// Convert from S16 to U16 and shift
	return _mm_max_epi16( vResults[0], _mm_setzero_si128() );
#else
	UShort8 res;
	const Float* source = reinterpret_cast <const Float*> ( v );
	for( U32 i = 0; i < 8; ++i )
		res.u0_7[i] = static_cast <U16> ( Max(source[i],0.f) );
	return VecUShortLoadAligned( res.u0_7 );
#endif 
}

template <U16 LeftShift> FINLINE_Z VecUShort8 VecFloatToUShortSat ( register const VecFloat4* v )   ///< for( i ranging from 0 to 7 ) res[i] = (U16)(v[i])<<LeftShift
{
#if defined( _PC_SSE2 )
	return _mm_slli_epi16( VecFloatToUShortSat(v), LeftShift );
#else
	UShort8 res;
	const Float* source = reinterpret_cast <const Float*> ( v );
	for( U32 i = 0; i < 8; ++i )
		res.u0_7[i] = static_cast <U16> ( Max(source[i],0.f) ) << LeftShift;
	return VecUShortLoadAligned( res.u0_7 );
#endif 
}

FINLINE_Z VecUChar16 VecFloatToUCharSat ( register const VecFloat4* v )   ///< for( i ranging from 0 to 15 ) res[i] = (U8)Clamp(v[i],0,255)
{
#if defined( _PC_SSE2 )
	__m128i vResults32[4], vResults16[2];
	// Convert from float to S16
	for( U32 i = 0; i < 4; ++i )
		vResults32[i] = _mm_cvttps_epi32( v[i] );
	vResults16[0] = _mm_packs_epi32( vResults32[0], vResults32[1] );
	vResults16[1] = _mm_packs_epi32( vResults32[2], vResults32[3] );
	// Convert from S16 to U8
	return _mm_packus_epi16( vResults16[0], vResults16[1] );
#else
	VecUShort8 res[2];
	res[0] = VecFloatToUShortSat ( v );
	res[1] = VecFloatToUShortSat ( v + 2 );
	return VecUShortToUCharSat( res );
#endif 
}

FINLINE_Z VecFloat4 VecIntToFloat ( register const VecInt4& a ) ///< xyzw = (float4)a.xyzw
{
#if defined (_PC_SSE2 )
	return _mm_cvtepi32_ps( a );
#else
	return VecFloatLoad4( (Float)VecIntGetX(a), (Float)VecIntGetY(a), (Float)VecIntGetZ(a), (Float)VecIntGetW(a) );
#endif
}

FINLINE_Z VecShort8 VecIntToShortSat ( register const VecInt4* v )   ///< for( i ranging from 0 to 7 ) res[i] = (S16)Clamp(v[i],-32768,32767)
{
#if defined (_PC_SSE2 )
	return _mm_packs_epi32( v[0], v[1] );
#else
	Short8 res;
	const S32* vI = reinterpret_cast <const S32*> ( v );
	for( U32 i = 0; i < 8; ++i )
	{
		S32 value = Clamp <S32> ( vI[i], -32768, 32767 );
		res.s0_7[i] = static_cast <S16> ( value );
	}
	return VecShortLoadAligned( &res );
#endif
}

FINLINE_Z VecUShort8 VecIntToUShortSat ( register const VecInt4* v )   ///< for( i ranging from 0 to 7 ) res[i] = (U16)Clamp(v[i],0,65535)
{
#if defined (_PC_SSE4 )
	return _mm_packus_epi32( v[0], v[1] );
#else
	UShort8 res;
	const S32* vI = reinterpret_cast <const S32*> ( v );
	for( U32 i = 0; i < 8; ++i )
	{
		S32 value = Clamp <S32> ( vI[i], 0, 65535 );
		res.u0_7[i] = static_cast <U16> ( value );
	}
	return VecUShortLoadAligned( &res );
#endif
}

FINLINE_Z VecUShort8 VecIntToUShortModulo ( register const VecInt4* v )   ///< for( i ranging from 0 to 7 ) res[i] = (U16)v[i]
{
#if defined (_PC_SSE2 )
	__m128i x = _mm_slli_epi32( v[0], 16 );
	__m128i y = _mm_slli_epi32( v[1], 16 );
	return _mm_packs_epi32( _mm_srai_epi32(x,16), _mm_srai_epi32(y,16) );
#else
	UShort8 res;
	const S32* vI = reinterpret_cast <const S32*> ( v );
	for( U32 i = 0; i < 8; ++i )
		res.u0_7[i] = static_cast <U16> ( vI[i] );
	return VecUShortLoadAligned( &res );
#endif
}

FINLINE_Z VecUChar16 VecIntToUCharSat ( register const VecInt4* v )   ///< for( i ranging from 0 to 15 ) res[i] = (U8)Clamp(v[i],0,255)
{
#if defined (_PC_SSE2 )
	return _mm_packus_epi16( _mm_packs_epi32(v[0],v[1]), _mm_packs_epi32(v[2],v[3]) );
#else
	register VecUShort8 vI[2];
	vI[0] = VecIntToUShortSat( v );
	vI[1] = VecIntToUShortSat( v + 2 );
	return VecUShortToUCharSat( vI );
#endif
}

FINLINE_Z VecUShort8 VecUIntToUShortSat ( register const VecUInt4* v )   ///< for( i ranging from 0 to 7 ) res[i] = (U16)Min(v[i],65535)
{
#if defined (_PC_SSE3 )
	static const Int4			vMaxUShort					= { 0xfff, 0xffff, 0xffff, 0xffff };
	const __m128i maxUShort = VecUIntLoadAligned( &vMaxUShort ); 
	const __m128i vIsGreater0 = _mm_cmpgt_epi32( v[0], maxUShort );
	const __m128i vIsGreater1 = _mm_cmpgt_epi32( v[1], maxUShort );
	__m128i a = _mm_or_si128( v[0], vIsGreater0 );
	__m128i b = _mm_or_si128( v[1], vIsGreater1 );
	a = _mm_shuffle_epi8( a, VecUCharLoadAligned(&VectorConstantsPrivate::vUInt2UshortMask0) );
	b = _mm_shuffle_epi8( b, VecUCharLoadAligned(&VectorConstantsPrivate::vUInt2UshortMask1) );
	return _mm_or_si128( a, b );
#else
	UShort8 res;
	const U32* vI = reinterpret_cast <const U32*> ( v );
	for( U32 i = 0; i < 8; ++i )
	{
		U32 value = Min <U32> ( vI[i], 65535 );
		res.u0_7[i] = static_cast <U16> ( value );
	}
	return VecUShortLoadAligned( &res );
#endif
}

FINLINE_Z VecUShort8 VecUIntToUShortModulo ( register const VecUInt4* v )   ///< for( i ranging from 0 to 7 ) res[i] = (U16)v[i]
{
#if defined (_PC_SSE3 )
	const __m128i a = _mm_shuffle_epi8( v[0], VecUCharLoadAligned(&VectorConstantsPrivate::vUInt2UshortMask0) );
	const __m128i b = _mm_shuffle_epi8( v[1], VecUCharLoadAligned(&VectorConstantsPrivate::vUInt2UshortMask1) );
	return _mm_or_si128( a, b );
#else
	UShort8 res;
	const U32* vI = reinterpret_cast <const U32*> ( v );
	for( U32 i = 0; i < 8; ++i )
		res.u0_7[i] = static_cast <U16> ( vI[i] );
	return VecUShortLoadAligned( &res );
#endif
}

FINLINE_Z void VecShortToFloat ( VecFloat4* results, const VecShort8& v )   ///< for( i ranging from 0 to 3 ) { results[0][i] = (float)v[i]; results[1][i] = (float)v[4+i]; }
{
#if defined( _PC_SSE4 )
	__m128i vLow = _mm_cvtepi16_epi32( v );
	__m128i vHigh = _mm_cvtepi16_epi32( _mm_shuffle_epi32( v, _MM_SHUFFLE(3,2,3,2) ) );
	results[0] = _mm_cvtepi32_ps( vLow );
	results[1] = _mm_cvtepi32_ps( vHigh );
#elif defined( _PC_SSE2 )
	__m128i signMask = _mm_cmplt_epi16( v, _mm_setzero_si128() );
	__m128i vHigh = _mm_unpackhi_epi16( v, signMask );
	__m128i vLow = _mm_unpacklo_epi16( v, signMask );
	results[0] = _mm_cvtepi32_ps( vLow );
	results[1] = _mm_cvtepi32_ps( vHigh );
#else
	Float res[8];
	const S16* source = reinterpret_cast <const S16*> ( &v );
	for( U32 i = 0; i < 8; ++i )
		res[i] = static_cast <Float> ( source[i] );
	results[0] = VecFloatLoadUnaligned( res );
	results[1] = VecFloatLoadUnaligned( res + 4 );
#endif 
}

template <U16 RightShift> FINLINE_Z void VecUShortToFloat ( VecFloat4* results, const VecUShort8& v )   ///< for( i ranging from 0 to 3 ) { results[0][i] = (float)(v[i]>>RightShift); results[1][i] = (float)(v[4+i]>>RightShift); }
{
#if defined( _PC_SSE4 )
	register const __m128i vShifted = _mm_srli_epi16( v, RightShift );
	__m128i vLow = _mm_cvtepu16_epi32( vShifted );
	__m128i vHigh = _mm_cvtepu16_epi32( _mm_shuffle_epi32( vShifted, _MM_SHUFFLE(3,2,3,2) ) );
	results[0] = _mm_cvtepi32_ps( vLow );
	results[1] = _mm_cvtepi32_ps( vHigh );
#elif defined( _PC_SSE2 )
	register const __m128i zero = _mm_setzero_si128();
    register const __m128i vShifted = _mm_srli_epi16( v, RightShift );
	__m128i vHigh = _mm_unpackhi_epi16( vShifted, zero );
	__m128i vLow = _mm_unpacklo_epi16( vShifted, zero );
	results[0] = _mm_cvtepi32_ps( vLow );
	results[1] = _mm_cvtepi32_ps( vHigh );
#else
	Float res[8];
	const U16* source = reinterpret_cast <const U16*> ( &v );
	for( U32 i = 0; i < 8; ++i )
		res[i] = static_cast <Float> ( source[i] >> RightShift );
	results[0] = VecFloatLoadUnaligned( res );
	results[1] = VecFloatLoadUnaligned( res + 4 );
#endif 
}

FINLINE_Z void VecUShortToInt ( VecInt4* results, const VecUShort8& a )  ///< Unpack 8 U16 to 8 S32 <=> let results be a 256-bit vector: for( i ranging from 0 to 7 ) { results[i] = (S32)a[i] }  
{
#if defined( _PC_SSE4 )
	results[0] = _mm_cvtepu16_epi32( a );
	results[1] = _mm_cvtepu16_epi32( _mm_shuffle_epi32(a,_MM_SHUFFLE(3,2,3,2)) );
#elif defined( _PC_SSE2 ) // Little endian platforms
	VecUShort8 zero = VecUShortSplatZero();
	results[0] = VecUShortPermuteA0A1B0B1C0C1D0D1( a, zero );
    results[1] = VecUShortPermuteE0E1F0F1G0G1H0H1( a, zero );
#else
	for( U32 i = 0; i < 4; ++i )
	{
		results[0].xyzw[i] = static_cast <S32> ( a.u0_7[i] );
		results[1].xyzw[i] = static_cast <S32> ( a.u0_7[i+4] );
	}
#endif 
}

VecUChar16 VecUShortToUCharSat ( register const VecUShort8* v )   ///< Pack the elements from v into 16 U8 and saturate <=> for( i ranging from 0 to 15 ) { res[i] = (U8)Min(v[i],255) }  
{
#if defined( _PC_SSE2 )   
    return _mm_packus_epi16( v[0], v[1] );
#else
	UChar16 res;
	const U16 maxU8 = 255;
	for( U32 i = 0; i < 8; ++i )
	{
		res.u0_15[i]   = static_cast <U8> ( Min(v[0].u0_7[i],maxU8) );
		res.u0_15[i+8] = static_cast <U8> ( Min(v[1].u0_7[i],maxU8) );
	}
	return res;
#endif 
}

FINLINE_Z void VecUCharToFloat ( VecFloat4* results, const VecUChar16& a )   ///< for( i ranging from 0 to 15 ) res[i] = (Float)a[i]
{
	register VecInt4 resultsI[4];
	VecUCharToInt( resultsI, a );
	for( U32 i = 0; i < 4; ++i )
		results[i] = VecIntToFloat( resultsI[i] );
}

FINLINE_Z void VecUCharToInt ( VecInt4* results, const VecUChar16& a )  ///< Unpack 16 U8 to 16 S32 <=> let results be a 512-bit vector: for( i ranging from 0 to 15 ) { results[i] = (S32)a[i] }  
{
#if defined( _PC_SSE4 )
	results[0] = _mm_cvtepu8_epi32( a );
	results[1] = _mm_cvtepu8_epi32( _mm_shuffle_epi32(a,1) );
	results[2] = _mm_cvtepu8_epi32( _mm_shuffle_epi32(a,2) );
	results[3] = _mm_cvtepu8_epi32( _mm_shuffle_epi32(a,3) );
#else
	VecUShort8 resU16[2];
	VecUCharToUShort( resU16, a );
    VecUShortToInt( results, resU16[0] );
	VecUShortToInt( results+2, resU16[1] );
#endif
}

FINLINE_Z void VecUCharToUShort ( VecUShort8* results, const VecUChar16& a )  ///< Unpack 16 U8 to 16 U16 <=> let results be a 256-bit vector: for( i ranging from 0 to 15 ) { results[i] = (U16)a[i] }  
{
#if defined( _PC_SSE4 )
	results[0] = _mm_cvtepu8_epi16( a );
	results[1] = _mm_cvtepu8_epi16( _mm_shuffle_epi32(a,_MM_SHUFFLE(3,2,3,2)) );
#elif defined( _PC_SSE2 ) // Little endian platforms
	VecUChar16 zero = VecUCharSplatZero();
	results[0] = VecUCharPermuteLow( a, zero );
    results[1] = VecUCharPermuteHigh( a, zero );
#else
	for( U32 i = 0; i < 8; ++i )
	{
		results[0].u0_7[i] = a.u0_15[i];
		results[1].u0_7[i] = a.u0_15[i+8];
	}
#endif 
}

FINLINE_Z void VecUCharToUShortEvenOdd ( register VecUShort8& even, register VecUShort8& odd, register const VecUChar16& a ) ///< even.abcdefgh = (U16)a.acegikmo; odd.abcdefgh = (U16)a.bdfhjlnp
{
#if defined( _PC_SSE3 )
	even = _mm_shuffle_epi8( a, VecUCharLoadAligned(&VectorConstantsPrivate::vEvenMask) );
	odd = _mm_shuffle_epi8( a, VecUCharLoadAligned(&VectorConstantsPrivate::vOddMask) );
#elif defined( _PC_SSE2 )
	const __m128i vClearMask = _mm_set1_epi16( 0x00FF );
	even = _mm_and_si128( vClearMask, a );
	odd = _mm_srli_epi16( _mm_andnot_si128(vClearMask,a), 8 );
#else
	for( U32 i = 0; i < 8; ++i )
	{
		even.u0_7[i] = a.u0_15[i*2];
		odd.u0_7[i] = a.u0_15[i*2+1];
	}
#endif
}

FINLINE_Z void VecFloatPack2ToS16NormedU ( S16* dest, register const VecFloat4& a ) ///< pack a.xy to 2 unaligned signed shorts, scaled from [-1,1] to [-32767,32767]
{
	dest[0] = static_cast <S16> ( VecFloatGetX(a) * 32767.f );
	dest[1] = static_cast <S16> ( VecFloatGetY(a) * 32767.f );
}

FINLINE_Z void VecFloatPack4ToS16NormedU ( S16* dest, register const VecFloat4& a ) ///< pack a.xyzw to 4 unaligned signed shorts, scaled from [-1,1] to [-32767,32767]
{
	dest[0] = static_cast <S16> ( VecFloatGetX(a) * 32767.f );
	dest[1] = static_cast <S16> ( VecFloatGetY(a) * 32767.f );
	dest[2] = static_cast <S16> ( VecFloatGetZ(a) * 32767.f );
	dest[3] = static_cast <S16> ( VecFloatGetW(a) * 32767.f );
}

FINLINE_Z void VecFloatPack4ToS16NormedA ( S16* dest, register const VecFloat4& a ) ///< pack a.xyzw to 4 16-byte aligned signed shorts, scaled from [-1,1] to [-32767,32767]
{
	return VecFloatPack4ToS16NormedU( dest, a );
}

FINLINE_Z VecFloat4 VecFloatUnpack2FromS16 ( const S16* source ) ///< unpack a.xy from 2 unaligned signed shorts, a.zw undetermined
{
	return VecFloatLoad4( static_cast<Float>(source[0]), static_cast<Float>(source[1]), 0.f, 0.f );
}

FINLINE_Z VecFloat4 VecFloatUnpack4FromS16 ( const S16* source ) ///< unpack a.xyzw from 4 unaligned signed shorts
{
	return VecFloatLoad4( static_cast<Float>(source[0]), static_cast<Float>(source[1]), static_cast<Float>(source[2]), static_cast<Float>(source[3]) );
}

FINLINE_Z U32 VecFloatPackSnorm8888( register const VecFloat4& normal ) ///< pack a 128-bit normalized vector to 4 platform-specific signed 8-bit integers
{
	U32 result;
#if defined( _PC_SSE2 ) // Little endian ARGB
	__m128 res = VecFloatMul( normal, VectorConstantsPrivate::v127F );
	__m128i iRes = _mm_cvttps_epi32( res );
	iRes = _mm_packs_epi32( iRes, iRes );
	iRes = _mm_packs_epi16( iRes, iRes );
	result = _mm_cvtsi128_si32( iRes );
#else
	const VecFloat4 vRes = VecFloatMul( normal, VectorConstantsPrivate::v127F );
    const VecInt4 iRes = VecFloatToIntNearest( vRes );
	const S8 x = static_cast<S8> ( VecIntGetX(iRes) );
	const S8 y = static_cast<S8> ( VecIntGetY(iRes) );
	const S8 z = static_cast<S8> ( VecIntGetZ(iRes) );
	const S8 w = static_cast<S8> ( VecIntGetW(iRes) );
	#if defined(_PC) || defined(_FAKE_XONE)// Little endian RGBA 
    result = (x&0xFF) | ( (y&0xFF) << 8 ) | ( (z&0xFF) << 16 ) | ( (w&0xFF) << 24 );
	#else // Default : little endian BGRA
    result = (z&0xFF) | ( (y&0xFF) << 8 ) | ( (x&0xFF) << 16 ) | ( (w&0xFF) << 24 );
	#endif
#endif
	return result;
}

FINLINE_Z VecFloat4 VecFloatUnpackSnorm8888( const U32 packedNormal ) ///< unpack 4 native signed 8-bit integers to a floating-point normalized vector
{
#if defined ( _PC_SSE2 ) // Little endian ARGB
	__m128i resultI = _mm_set1_epi32( packedNormal );
	__m128i signMask = _mm_cmplt_epi8( resultI, _mm_setzero_si128() );
	resultI = _mm_unpacklo_epi8( resultI, signMask );
	resultI = _mm_unpacklo_epi16( resultI, signMask );
	return _mm_mul_ps( _mm_cvtepi32_ps(resultI), VectorConstantsPrivate::vInv127F );
#else
	U8 R = ( packedNormal & 0xff );
	U8 G = ( packedNormal >> 8 ) & 0xff;
	U8 B = ( packedNormal >> 16 ) & 0xff;
	U8 A = ( packedNormal >> 24 ) & 0xff;
	S8 sX,sY,sZ,sW;

	#if defined(_PC) || defined(_FAKE_XONE) ///< Little endian RGBA
	sX = *reinterpret_cast<S8*>(&R);
    sY = *reinterpret_cast<S8*>(&G);
    sZ = *reinterpret_cast<S8*>(&B);
    sW = *reinterpret_cast<S8*>(&A);
	#else // Default : little endian BGRA
    sX = *reinterpret_cast<S8*>(&B);
    sY = *reinterpret_cast<S8*>(&G);
    sZ = *reinterpret_cast<S8*>(&R);
    sW = *reinterpret_cast<S8*>(&A);
	#endif

	VecInt4 vColor = VecIntLoad4(sX, sY, sZ, sW);
	return VecFloatScale( VecIntToFloat(vColor), 1.0f / 127.f );
#endif
}

FINLINE_Z U32 VecFloatPackColor8888 ( register const VecFloat4& color ) ///< pack a 128-bit rgba color vector to platform-specific 32-bit native color format, saturated
{
	U32 result;
#if defined( _PC_SSE2 ) // Little endian ARGB
	__m128 res = _mm_add_ps( _mm_mul_ps( color, VectorConstantsPrivate::v255F ), VectorConstantsPrivate::vHalfF );
	__m128i iRes = _mm_cvttps_epi32( res );
	iRes = _mm_packs_epi32( iRes, iRes );
	iRes = _mm_packus_epi16( iRes, iRes );
	result = _mm_cvtsi128_si32( iRes );
#else
	const VecFloat4 vRes = VecFloatScale( VecFloatSaturate(color), 255.0f );
    const VecInt4 iRes = VecFloatToIntNearest( vRes );
	#if defined(_PC) || defined(_FAKE_XONE)// Little endian RGBA 
    result = VecIntGetX(iRes) | ( VecIntGetY(iRes) << 8 ) | ( VecIntGetZ(iRes) << 16 ) | ( VecIntGetW(iRes) << 24 );
	#else // Default : little endian BGRA
    result = VecIntGetZ(iRes) | ( VecIntGetY(iRes) << 8 ) | ( VecIntGetX(iRes) << 16 ) | ( VecIntGetW(iRes) << 24 );
	#endif
#endif
	return result;
}

FINLINE_Z VecFloat4 VecFloatUnpackColor8888 ( const U32 color ) ///< unpack a native color word (8-bit per component) to a floating-point rgba vector
{
#if defined ( _PC_SSE2 ) // Little endian ARGB
	const __m128 inv255 = VectorConstantsPrivate::vInv255F;
	const __m128i zero = _mm_setzero_si128();
	__m128i resultI = _mm_set1_epi32( color );
	resultI = _mm_unpacklo_epi8( resultI, zero );
	resultI = _mm_unpacklo_epi16( resultI, zero );
	return _mm_mul_ps( _mm_cvtepi32_ps(resultI), inv255 );
#else
	VecInt4 vColor; 
	#if defined(_PC) || defined(_FAKE_XONE) ///< Little endian RGBA
	vColor.x = color & 0xff;
    vColor.y = ( color >> 8 ) & 0xff;
    vColor.z = ( color >> 16 ) & 0xff;
    vColor.w = ( color >> 24 ) & 0xff;
	#else // Default : little endian BGRA
    vColor.x = ( color >> 16 ) & 0xff;
    vColor.y = ( color >> 8 ) & 0xff;
    vColor.z = color & 0xff;
    vColor.w = ( color >> 24 ) & 0xff;
	#endif
	return VecFloatScale( VecIntToFloat(vColor), 1.0f / 255.f );
#endif
}

FINLINE_Z VecInt4 VecFloatPackColors8888 ( register const VecFloat4 colors[4] ) ///< pack 4 rgba vectors in the 4 words of a vector (platform-specific native color format, saturated)
{
	VecInt4 result;
#if defined( _PC_SSE2 ) // Little endian ARGB
	const __m128 v255F = VectorConstantsPrivate::v255F;
	const __m128 vHalfF = VectorConstantsPrivate::vHalfF;
	__m128 tF; __m128i t[4];
	for( U32 i = 0; i < 4; ++i )
	{
		tF = _mm_add_ps( _mm_mul_ps(colors[i],v255F), vHalfF );
		t[i] = _mm_cvtps_epi32( tF );
	}
	t[0] = _mm_packs_epi32( t[0], t[1] );
	t[2] = _mm_packs_epi32( t[2], t[3] );
	result = _mm_packus_epi16( t[0], t[2] );
#else
	result.x = VecFloatPackColor8888( colors[0] );
	result.y = VecFloatPackColor8888( colors[1] );
	result.z = VecFloatPackColor8888( colors[2] );
	result.w = VecFloatPackColor8888( colors[3] );
#endif
	return result;
}

FINLINE_Z void VecFloatUnpackColors8888 ( VecFloat4 unpackedColors[4], const VecInt4& packedColors ) ///< unpack 4 native color words (stored in a vector) to 4 floating-point rgba vectors
{
#if defined ( _PC_SSE2 ) // Little endian ARGB
	const __m128 inv255 = VectorConstantsPrivate::vInv255F;
	const __m128i zero = _mm_setzero_si128();
	const __m128i yyww = _mm_shuffle_epi32( packedColors, _MM_SHUFFLE(3,3,1,1) );
	__m128i resultX, resultY, resultZ, resultW;

	resultX = _mm_unpacklo_epi8( packedColors, zero );
	resultX = _mm_unpacklo_epi16( resultX, zero );
	unpackedColors[0] = _mm_mul_ps( _mm_cvtepi32_ps(resultX), inv255 );

	resultY = _mm_unpacklo_epi8( yyww, zero );
	resultY = _mm_unpacklo_epi16( resultY, zero );
	unpackedColors[1] = _mm_mul_ps( _mm_cvtepi32_ps(resultY), inv255 );

	resultZ = _mm_unpackhi_epi8( packedColors, zero );
	resultZ = _mm_unpacklo_epi16( resultZ, zero );
	unpackedColors[2] = _mm_mul_ps( _mm_cvtepi32_ps(resultZ), inv255 );

	resultW = _mm_unpackhi_epi8( yyww, zero );
	resultW = _mm_unpacklo_epi16( resultW, zero );
	unpackedColors[3] = _mm_mul_ps( _mm_cvtepi32_ps(resultW), inv255 );
#else
	POSTALIGNED128_Z S32 xyzw[4] ALIGNED128_Z;
	VecIntStoreAligned( xyzw, packedColors );
	unpackedColors[0] = VecFloatUnpackColor8888( xyzw[0] );
	unpackedColors[1] = VecFloatUnpackColor8888( xyzw[1] );
	unpackedColors[2] = VecFloatUnpackColor8888( xyzw[2] );
	unpackedColors[3] = VecFloatUnpackColor8888( xyzw[3] );
#endif
}

FINLINE_Z VecUChar16 VecUCharConvertR8G8B8ToR8G8B8A8 ( register const VecUChar16& a )  ///< res = { a.abc, 255, a.def, 255, a.ghi, 255, a.jkl, 255 }
{
#if defined(_PC_SSE2)
	const VecUChar16 vMask = VecUCharLoadAligned( &VectorConstantsPrivate::vUCharMask888 );
	return VecUCharSwizzleABCMDEFNGHIOJKLP( VecUCharMax(a,vMask) );
#else
	VecUChar16 result;
	for( U32 i = 0; i < 4; ++i )
	{
		U32 offsetDest = i << 2;
		U32 offsetSrc = i * 3;
		result.u0_15[offsetDest+0] = a.u0_15[offsetSrc+0];
		result.u0_15[offsetDest+1] = a.u0_15[offsetSrc+1];
		result.u0_15[offsetDest+2] = a.u0_15[offsetSrc+2];
		result.u0_15[offsetDest+3] = 255;
	}
	return result;
#endif 
}

FINLINE_Z VecUChar16 VecUCharConvertB8G8R8ToR8G8B8A8 ( register const VecUChar16& a )  ///< res = { a.cba, 255, a.fed, 255, a.ihg, 255, a.lkj, 255 }
{
#if defined(_PC_SSE2)
	const VecUChar16 vMask = VecUCharLoadAligned( &VectorConstantsPrivate::vUCharMask888 );
	return VecUCharSwizzleCBAMFEDNIHGOLKJP( VecUCharMax(a,vMask) );
#else
	VecUChar16 result;
	for( U32 i = 0; i < 4; ++i )
	{
		U32 offsetDest = i << 2;
		U32 offsetSrc = i * 3;
		result.u0_15[offsetDest+0] = a.u0_15[offsetSrc+2];
		result.u0_15[offsetDest+1] = a.u0_15[offsetSrc+1];
		result.u0_15[offsetDest+2] = a.u0_15[offsetSrc+0];
		result.u0_15[offsetDest+3] = 255;
	}
	return result;
#endif 
}

FINLINE_Z void VecUCharConvertR8G8B8A8ToR8G8B8 ( VecUChar16* destRGB, register const VecUChar16* srcRGBA )  ///< pack 12 RGBA32 color words (stored in 4 vectors) to 12 RGB24 color words (stored in 3 vectors)
{
#if defined(_PC_SSE4)
	VecFloat4 srcRGB[4];

	register const VecUChar16 vAlphaRemoverMask = VecUCharLoadAligned( &VectorConstantsPrivate::vRGBAToRGBMask );
	for( U32 i = 0; i < 4; ++i )
		srcRGB[i] = _mm_castsi128_ps( _mm_shuffle_epi8(srcRGBA[i],vAlphaRemoverMask) );

	destRGB[0] = _mm_castps_si128( VecFloatPermuteX0Y0Z0X1(srcRGB[0],srcRGB[1]) );
	destRGB[1] = _mm_castps_si128( VecFloatPermuteY0Z0X1Y1(srcRGB[1],srcRGB[2]) );
	destRGB[2] = _mm_castps_si128( VecFloatPermuteZ0X1Y1Z1(srcRGB[2],srcRGB[3]) );
#else
	const U8* src = reinterpret_cast <const U8*> ( srcRGBA );
	U8* dest = reinterpret_cast <U8*> ( destRGB );
	for( U32 i = 0; i < 12; ++i )
	{
		dest[i*3+0] = src[i*4+0];
		dest[i*3+1] = src[i*4+1];
		dest[i*3+2] = src[i*4+2];
	}
#endif 
}

FINLINE_Z void VecFloatPack16ToU8A ( U8* dest, register const VecFloat4* source ) ///< pack 16 floating-point values contained in 4 vectors to unsigned chars, 16-byte aligned. Source values must be in the range [0,1].
{
	dest[0]  = static_cast <U8> ( VecFloatGetX(source[0]) );
	dest[1]  = static_cast <U8> ( VecFloatGetY(source[0]) );
	dest[2]  = static_cast <U8> ( VecFloatGetZ(source[0]) );
	dest[3]  = static_cast <U8> ( VecFloatGetW(source[0]) );

	dest[4]  = static_cast <U8> ( VecFloatGetX(source[1]) );
	dest[5]  = static_cast <U8> ( VecFloatGetY(source[1]) );
	dest[6]  = static_cast <U8> ( VecFloatGetZ(source[1]) );
	dest[7]  = static_cast <U8> ( VecFloatGetW(source[1]) );

	dest[8]  = static_cast <U8> ( VecFloatGetX(source[2]) );
	dest[9]  = static_cast <U8> ( VecFloatGetY(source[2]) );
	dest[10] = static_cast <U8> ( VecFloatGetZ(source[2]) );
	dest[11] = static_cast <U8> ( VecFloatGetW(source[2]) );

	dest[12] = static_cast <U8> ( VecFloatGetX(source[3]) );
	dest[13] = static_cast <U8> ( VecFloatGetY(source[3]) );
	dest[14] = static_cast <U8> ( VecFloatGetZ(source[3]) );
	dest[15] = static_cast <U8> ( VecFloatGetW(source[3]) );
}

FINLINE_Z void VecIntPack4ToS16 ( S16* dest, register const VecInt4& a ) ///< pack a.xyzw to 4 unaligned signed shorts
{
#if defined( _PC_SSE2 )
	_mm_storel_epi64( reinterpret_cast<__m128i*>(dest), _mm_packs_epi32(a,a) );
#else
	dest[0] = static_cast <S16> ( VecIntGetX(a) );
	dest[1] = static_cast <S16> ( VecIntGetY(a) );
	dest[2] = static_cast <S16> ( VecIntGetZ(a) );
	dest[3] = static_cast <S16> ( VecIntGetW(a) );
#endif
}

FINLINE_Z VecFloat4 VecFloatTransposeColumnsX3( register const VecFloat4* columns ) ///< xyzw = { columns[0].x, columns[1].x, columns[2].x, undefined }
{
	return VecFloatPermuteX0Y0X1Y1( VecFloatPermuteX0X1Y0Y1(columns[0],columns[1]), columns[2] );
}

FINLINE_Z VecFloat4 VecFloatTransposeColumnsX( register const VecFloat4* columns ) ///< xyzw = { columns[0].x, columns[1].x, columns[2].x, columns[3].x }
{
	register const VecFloat4 a = VecFloatPermuteX0X1Y0Y1( columns[0], columns[1] );
	register const VecFloat4 b = VecFloatPermuteX0X1Y0Y1( columns[2], columns[3] );
	return VecFloatPermuteX0Y0X1Y1( a, b );
}

FINLINE_Z VecFloat4 VecFloatTransposeColumnsY( register const VecFloat4* columns ) ///< xyzw = { columns[0].y, columns[1].y, columns[2].y, columns[3].y }
{
	register const VecFloat4 a = VecFloatPermuteX0X1Y0Y1( columns[0], columns[1] );
	register const VecFloat4 b = VecFloatPermuteX0X1Y0Y1( columns[2], columns[3] );
	return VecFloatPermuteZ0W0Z1W1( a, b );
}

FINLINE_Z VecFloat4 VecFloatTransposeColumnsZ( register const VecFloat4* columns ) ///< xyzw = { columns[0].z, columns[1].z, columns[2].z, columns[3].z }
{
	register const VecFloat4 a = VecFloatPermuteZ0Z1W0W1( columns[0], columns[1] );
	register const VecFloat4 b = VecFloatPermuteZ0Z1W0W1( columns[2], columns[3] );
	return VecFloatPermuteX0Y0X1Y1( a, b );
}

FINLINE_Z VecFloat4 VecFloatTransposeColumnsW( register const VecFloat4* columns ) ///< xyzw = { columns[0].w, columns[1].w, columns[2].w, columns[3].w }
{
	register const VecFloat4 a = VecFloatPermuteZ0Z1W0W1( columns[0], columns[1] );
	register const VecFloat4 b = VecFloatPermuteZ0Z1W0W1( columns[2], columns[3] );
	return VecFloatPermuteZ0W0Z1W1( a, b );
}

FINLINE_Z void VecFloatTranspose3SoA ( register VecFloat4* dest, register const VecFloat4* source ) ///< transpose 3 vectors from SoA to AoS, and reciprocal
{
	VecFloat3x3Transpose( reinterpret_cast<VecFloat3x3&>(*dest), reinterpret_cast<const VecFloat3x3&>(*source) );
}

FINLINE_Z void VecFloatTranspose4SoA ( register VecFloat4* dest, register const VecFloat4* source ) ///< transpose 4 vectors from SoA to AoS, and reciprocal
{
	VecFloat4x4Transpose( reinterpret_cast<VecFloat4x4&>(*dest), reinterpret_cast<const VecFloat4x4&>(*source) );
}

FINLINE_Z void VecFloat4x4CopyM3x3 ( register VecFloat4x4& result, register const VecFloat3x3& m0 ) ///< result = m0, filling other components with default values
{
	result[3] = VecFloatSetIdentity();
	result[0] = VecFloatPermuteX0Y0Z0X1( m0[0], result[3] );
	result[1] = VecFloatPermuteX0Y0Z0X1( m0[1], result[3] );
	result[2] = VecFloatPermuteX0Y0Z0X1( m0[2], result[3] );
}

FINLINE_Z void VecFloatQuaternionTo3x3 ( register VecFloat3x3& matrix, register const VecFloat4& quat ) ///< Convert a quaternion to a 3x3 vector matrix 
{
	register VecFloat4 tmp0, tmp1, tmp2, tmp3, tmp4, tmp5;

    register const VecFloat4 xyzw2 = VecFloatAdd( quat, quat );
    register const VecFloat4 wwww = VecFloatSplatW( quat );
    register const VecFloat4 yzxw = VecFloatSwizzleYZXW( quat );
    register const VecFloat4 zxyw = VecFloatSwizzleZXYW( quat );
    register const VecFloat4 yzxw2 = VecFloatSwizzleYZXW( xyzw2 );
    register const VecFloat4 zxyw2 = VecFloatSwizzleZXYW( xyzw2 );
    tmp0 = VecFloatMul( yzxw2, wwww );
    tmp1 = VecFloatNegMsub( yzxw, yzxw2, VecFloatSplatOne() );
    tmp2 = VecFloatMul( yzxw, xyzw2 );
    tmp0 = VecFloatMadd( zxyw, xyzw2, tmp0 );
    tmp1 = VecFloatNegMsub( zxyw, zxyw2, tmp1 );
    tmp2 = VecFloatNegMsub( zxyw2, wwww, tmp2 );
    tmp3 = VecFloatPermuteX1Y0Z0W0( tmp0, tmp1 );
    tmp4 = VecFloatPermuteX1Y0Z0W0( tmp1, tmp2 );
    tmp5 = VecFloatPermuteX1Y0Z0W0( tmp2, tmp0 );
    matrix[0] = VecFloatPermuteX0Y0Z1W0( tmp3, tmp2 );
    matrix[1] = VecFloatPermuteX0Y0Z1W0( tmp4, tmp0 );
    matrix[2] = VecFloatPermuteX0Y0Z1W0( tmp5, tmp1 );
}

FINLINE_Z void VecFloatQuaternionTo4x4 ( Float* matrix, register const VecFloat4& quat ) ///< Convert a quaternion to a 4x4 storage matrix which address is 16-byte aligned
{
	POSTALIGNED128_Z Float _x2_xy_xz_xw[4] ALIGNED128_Z;
	POSTALIGNED128_Z Float _yx_y2_yz_yw[4] ALIGNED128_Z;
	POSTALIGNED128_Z Float _zx_zy_z2_zw[4] ALIGNED128_Z;

	register const VecFloat4 xyzw2 = VecFloatAdd( quat, quat );
	register const VecFloat4 x2_xy_xz_xw = VecFloatMul( quat, VecFloatSplatX(xyzw2) );
	register const VecFloat4 yx_y2_yz_yw = VecFloatMul( quat, VecFloatSplatY(xyzw2) );
	register const VecFloat4 zx_zy_z2_zw = VecFloatMul( quat, VecFloatSplatZ(xyzw2) );

	VecFloatStoreAligned( _x2_xy_xz_xw, x2_xy_xz_xw );
	VecFloatStoreAligned( _yx_y2_yz_yw, yx_y2_yz_yw );
	VecFloatStoreAligned( _zx_zy_z2_zw, zx_zy_z2_zw );

	matrix[0] = 1.f - _yx_y2_yz_yw[1] - _zx_zy_z2_zw[2];  // 1.f - y2 - z2
	matrix[1] = _yx_y2_yz_yw[0] + _zx_zy_z2_zw[3];        // xy + zw
	matrix[2] = _zx_zy_z2_zw[0] - _yx_y2_yz_yw[3];        // xz - yw
	matrix[3] = 0.f;

	matrix[4] = _x2_xy_xz_xw[1] - _zx_zy_z2_zw[3];        // xy - zw
	matrix[5] = 1.f - _x2_xy_xz_xw[0] - _zx_zy_z2_zw[2];  // 1.f - x2 - z2
	matrix[6] = _zx_zy_z2_zw[1] + _x2_xy_xz_xw[3];        // yz + xw
	matrix[7] = 0.f;

	matrix[8] = _x2_xy_xz_xw[2] + _yx_y2_yz_yw[3];        // xz + yw
	matrix[9] = _yx_y2_yz_yw[2] - _x2_xy_xz_xw[3];        // yz - xw
	matrix[10] = 1.f - _x2_xy_xz_xw[0] - _yx_y2_yz_yw[1]; // 1.f - x2 - y2
	matrix[11] = 0.f;

	matrix[12] = 0.f;
	matrix[13] = 0.f;
	matrix[14] = 0.f;
	matrix[15] = 1.f;
}

FINLINE_Z void VecFloatQuaternionTo4x4 ( register VecFloat4x4& matrix, register const VecFloat4& quat ) ///< Convert a quaternion to a 4x4 vector matrix 
{
	POSTALIGNED128_Z Float _x2_xy_xz_xw[4] ALIGNED128_Z;
	POSTALIGNED128_Z Float _yx_y2_yz_yw[4] ALIGNED128_Z;
	POSTALIGNED128_Z Float _zx_zy_z2_zw[4] ALIGNED128_Z;
	POSTALIGNED128_Z Float row[3][4] ALIGNED128_Z;

	register const VecFloat4 xyzw2 = VecFloatAdd( quat, quat );
	register const VecFloat4 x2_xy_xz_xw = VecFloatMul( quat, VecFloatSplatX(xyzw2) );
	register const VecFloat4 yx_y2_yz_yw = VecFloatMul( quat, VecFloatSplatY(xyzw2) );
	register const VecFloat4 zx_zy_z2_zw = VecFloatMul( quat, VecFloatSplatZ(xyzw2) );

	VecFloatStoreAligned( _x2_xy_xz_xw, x2_xy_xz_xw );
	VecFloatStoreAligned( _yx_y2_yz_yw, yx_y2_yz_yw );
	VecFloatStoreAligned( _zx_zy_z2_zw, zx_zy_z2_zw );

	row[0][0] = 1.f - _yx_y2_yz_yw[1] - _zx_zy_z2_zw[2]; // 1.f - y2 - z2
	row[0][1] = _yx_y2_yz_yw[0] + _zx_zy_z2_zw[3];       // xy + zw
	row[0][2] = _zx_zy_z2_zw[0] - _yx_y2_yz_yw[3];       // xz - yw
	row[0][3] = 0.f;

	row[1][0] = _x2_xy_xz_xw[1] - _zx_zy_z2_zw[3];       // xy - zw
	row[1][1] = 1.f - _x2_xy_xz_xw[0] - _zx_zy_z2_zw[2]; // 1.f - x2 - z2
	row[1][2] = _zx_zy_z2_zw[1] + _x2_xy_xz_xw[3];       // yz + xw
	row[1][3] = 0.f;

	row[2][0] = _x2_xy_xz_xw[2] + _yx_y2_yz_yw[3];       // xz + yw
	row[2][1] = _yx_y2_yz_yw[2] - _x2_xy_xz_xw[3];       // yz - xw
	row[2][2] = 1.f - _x2_xy_xz_xw[0] - _yx_y2_yz_yw[1]; // 1.f - x2 - y2
	row[2][3] = 0.f;

	matrix[0] = VecFloatLoadAligned( row[0] );
	matrix[1] = VecFloatLoadAligned( row[1] );
	matrix[2] = VecFloatLoadAligned( row[2] );
	matrix[3] = VecFloatSetIdentity();
}

FINLINE_Z VecFloat4 VecFloat3x3ToQuaternion( register const VecFloat3x3& m )
{
	Float4 mat[3], v;

	VecFloatStoreAligned( mat[0].xyzw, VecFloat3x3GetRow0(m) );
	VecFloatStoreAligned( mat[1].xyzw, VecFloat3x3GetRow1(m) );
	VecFloatStoreAligned( mat[2].xyzw, VecFloat3x3GetRow2(m) );

	Float root, trace = mat[0].x + mat[1].y + mat[2].z;

	if ( trace > 0.0f )
	{
        // |w| > 1/2, may as well choose w > 1/2
		root = Sqrt( trace + 1.f );	// 2w
		v.w  = 0.5f * root;
		root = 0.5f/root;		// 1/(4w)
		v.x  = (mat[1].z-mat[2].y)*root;
		v.y  = (mat[2].x-mat[0].z)*root;
		v.z  = (mat[0].y-mat[1].x)*root;
	}
	else
	{
		// |w| <= 1/2
		static int next[3] = { 1, 2, 0 };
		int i = 0;
		if ( mat[1].y > mat[0].x )
			i = 1;
		if ( mat[2].z > mat[i].xyzw[i] )
			i = 2;
		int j = next[i];
		int k = next[j];

		root = Sqrt( mat[i].xyzw[i] - mat[j].xyzw[j] - mat[k].xyzw[k] + 1.f );
		
		v.xyzw[i] = 0.5f * root;

		if (root != 0.0f)
			root = 0.5f/root;

		v.w = ( mat[j].xyzw[k] - mat[k].xyzw[j] ) * root;
		v.xyzw[j] = ( mat[i].xyzw[j] + mat[j].xyzw[i] ) * root;
		v.xyzw[k] = ( mat[i].xyzw[k] + mat[k].xyzw[i] ) * root; 
	}

	return VecFloatQuaternionCNormalize( VecFloatLoadAligned(v.xyzw) );
}

#endif //_VECTOR_LIB_Z_H
