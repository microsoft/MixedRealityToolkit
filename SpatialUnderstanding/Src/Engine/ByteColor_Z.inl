// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

inline ByteColor::ByteColor() { }

inline ByteColor::ByteColor( const Float _r, const Float _g, const Float _b, const Float _a )	 { Set(_r,_g,_b,_a); }

inline ByteColor::ByteColor( const U8 _r, const U8 _g, const U8 _b, const U8 _a)
{
	Set(_r,_g,_b,_a);
}

inline ByteColor::ByteColor( const Color &_c )
{
	Set(_c.xyzw() );
}

inline ByteColor::ByteColor( const U32 _rgbaColor )	 { Set(_rgbaColor); }

inline void ByteColor::Set(const Color &_c)
{
	//TODO Raf: ne plus clamper la valeur en [0,1], Color doit TOUJOURS y �tre
	Set( _c.xyzw() );
}

inline void ByteColor::Set(const Float _r, const Float _g, const Float _b, const Float _a)
{
	Set( Color(_r,_g,_b,_a) );
}

inline void ByteColor::Set( const U8 _r, const U8 _g, const U8 _b, const U8 _a)
{
	RGBA.BYTE.r = _r;
	RGBA.BYTE.g = _g;
	RGBA.BYTE.b = _b;
	RGBA.BYTE.a = _a;
}

inline	U32	 ByteColor::Get() const				{ return RGBA.rgbaColor; }

inline	void  ByteColor::Set ( const U32 c )			{ RGBA.rgbaColor=c; }

inline ByteColor::operator U32 () const	{ return RGBA.rgbaColor; }

inline ByteColor ByteColor::operator*( const Float _f ) const
{
	const Float invDivider ( _f / 255.f );
	return ByteColor( RGBA.BYTE.r*invDivider, RGBA.BYTE.g*invDivider, RGBA.BYTE.b*invDivider, RGBA.BYTE.a*invDivider );
}