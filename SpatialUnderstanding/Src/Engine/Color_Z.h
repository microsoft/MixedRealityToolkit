// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#ifndef _MATH_COLOR_Z_H
#define _MATH_COLOR_Z_H

#include <BeginDef_Z.h>


//----------------------------------------------------------------
// Color
//----------------------------------------------------------------

POSTALIGNED128_Z struct  Color
{
	union
	{
		struct
		{
			Float r ALIGNED128_Z;
			Float g,b,a;
		};
		VecFloat4	vec128;
	};

	inline operator VecFloat4& ()  { return vec128; }
	inline operator const VecFloat4& () const { return vec128; }

	// Init Method
	Color()	{}
	Color( const Float _rgba );
	Color( const Color& _c );
	Color( const VecFloat4& _c ) : vec128( _c )  { }
	explicit Color( const Vec3f& _v );
	explicit Color( const Vec4f& _v );
	Color( const Float _r, const Float _g, const Float _b, const Float _a = 1.f );
	Color( const U8 _r, const U8 _g, const U8 _b, const U8 _a=255);

	const Vec4f& xyzw() const;
	Vec4f& xyzw();

	const Vec3f& xyz() const;
	Vec3f& xyz();


    Float   GetGreyLevel() const;
	Color	FromRGBToHSV() const; // h goes in r, s in g, v in b, alpha in a. H is in [0;6] not [0;360] as usual!
	Color	FromHSVToRGB() const; // h must be in [0;6].

	Color	&Set( const Float _r, const Float _g, const Float _b, const Float _a = 1.f );
	Color   &operator = ( const Color &_c );
	Color	&Set(const Color &_v);	
	Color	operator*( const Float _f)	const;
	Color	operator-(const Color &_v) const;
	Color	operator+(const Color &_v) const;
	Color	operator*(const Color &_v) const;
	Color	&operator-=(const Color &_v);
	Color	&operator*=( const Float _f);
	Color	&operator+=(const Color &_v);
	Color	&operator*=(const Color &_v);
	Bool	operator==(const Color& v)	const;
	Bool	operator!=(const Color& v)	const;
};

typedef	Color	cA_Color;

inline Color operator*( const Float _f, const Color &_c)		{return  _c*_f;}

const EXTERN_Z Color COLOR_WHITE;
const EXTERN_Z Color COLOR_FULLWHITE;
const EXTERN_Z Color COLOR_BLACK;
const EXTERN_Z Color COLOR_GREY;
const EXTERN_Z Color COLOR_LIGHTGREY;
const EXTERN_Z Color COLOR_DARKGREY;
const EXTERN_Z Color COLOR_RED;
const EXTERN_Z Color COLOR_GREEN;
const EXTERN_Z Color COLOR_BLUE;
const EXTERN_Z Color COLOR_YELLOW;
const EXTERN_Z Color COLOR_CYAN;
const EXTERN_Z Color COLOR_MAGENTA;
const EXTERN_Z Color COLOR_ORANGE;
const EXTERN_Z Color COLOR_BROWN;
const EXTERN_Z Color COLOR_LIGHTBLUE;
const EXTERN_Z Color COLOR_LIGHTGREEN;
const EXTERN_Z Color COLOR_LIGHTRED;
const EXTERN_Z Color COLOR_DARKBLUE;
const EXTERN_Z Color COLOR_DARKGREEN;
const EXTERN_Z Color COLOR_DARKRED;
const EXTERN_Z Color COLOR_PINK;
const EXTERN_Z Color COLOR_PURPLE;
const EXTERN_Z Color COLOR_NULL;

const EXTERN_Z Char* SCOLOR_WHITE;
const EXTERN_Z Char* SCOLOR_FULLWHITE;
const EXTERN_Z Char* SCOLOR_BLACK;
const EXTERN_Z Char* SCOLOR_GREY;
const EXTERN_Z Char* SCOLOR_LIGHTGREY;
const EXTERN_Z Char* SCOLOR_DARKGREY;
const EXTERN_Z Char* SCOLOR_RED;
const EXTERN_Z Char* SCOLOR_GREEN;
const EXTERN_Z Char* SCOLOR_BLUE;
const EXTERN_Z Char* SCOLOR_YELLOW;
const EXTERN_Z Char* SCOLOR_CYAN;
const EXTERN_Z Char* SCOLOR_MAGENTA;
const EXTERN_Z Char* SCOLOR_ORANGE;
const EXTERN_Z Char* SCOLOR_BROWN;
const EXTERN_Z Char* SCOLOR_LIGHTBLUE;
const EXTERN_Z Char* SCOLOR_LIGHTGREEN;
const EXTERN_Z Char* SCOLOR_LIGHTRED;
const EXTERN_Z Char* SCOLOR_DARKBLUE;
const EXTERN_Z Char* SCOLOR_DARKGREEN;
const EXTERN_Z Char* SCOLOR_DARKRED;
const EXTERN_Z Char* SCOLOR_PINK;
const EXTERN_Z Char* SCOLOR_PURPLE;

#include <Color_Z.inl>

#endif //_MATH_COLOR_Z_H