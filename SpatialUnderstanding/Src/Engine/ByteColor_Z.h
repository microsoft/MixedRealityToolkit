// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#ifndef _MATH_BYTE_COLOR_Z_H
#define _MATH_BYTE_COLOR_Z_H

#include <Types_Z.h>

#include <BeginDef_Z.h>

struct Color;
struct Vec4f;

//----------------------------------------------------------------
// ByteColor
//----------------------------------------------------------------

struct  ByteColor
{
private:
	union 
	{
		struct  {
				U8 r,g,b,a;
				} BYTE;
		U32 rgbaColor;
	}RGBA;
public:
	
	inline ByteColor();
	inline ByteColor( const Float _r, const Float _g, const Float _b, const Float _a=1.f );
	inline ByteColor( const U8 _r, const U8 _g, const U8 _b, const U8 _a=255);
	inline ByteColor( const Color &_c );
	inline ByteColor( const U32 _rgbaColor );

	inline void Set(const Vec4f &_c);
	inline void Get(Vec4f &_c);
	inline void Set(const Color &_c);

	inline void Set(const Float _r, const Float _g, const Float _b, const Float _a=1.f);
	inline void Set( const U8 _r, const U8 _g, const U8 _b, const U8 _a=255);
	// Get BGRA little endian
	inline	U32	 Get() const;
	inline	void  Set ( const U32 c );
	inline operator U32	() const;

	inline ByteColor operator*( const Float _f ) const;
	
	inline ByteColor& operator *= ( const Float _f);
};

#include <ByteColor_Z.inl>

typedef	ByteColor	cA_ByteColor;
typedef DynArray_Z<ByteColor,32,FALSE,FALSE>	ByteColorDA;

const EXTERN_Z ByteColor BCOLOR_WHITE;
const EXTERN_Z ByteColor BCOLOR_NULL;


#endif //_MATH_BYTE_COLOR_Z_H
