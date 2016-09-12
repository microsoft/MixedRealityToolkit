// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

inline Color::Color( const Float _r, const Float _g, const Float _b, const Float _a )
{
	vec128 = VecFloatLoad4( _r, _g, _b, _a );
}

inline Color::Color( const Float _rgba )
{
	vec128 = VecFloatLoad1( _rgba );
}

inline Color::Color( const Color& _c )
{
	vec128 = _c.vec128;
}

inline Color::Color( const U8 _r, const U8 _g, const U8 _b, const U8 _a )
{
	r = _r/255.f;
	g = _g/255.f;
	b = _b/255.f;
	a = _a/255.f;
}

inline Color& Color::Set( const Float _r, const Float _g, const Float _b, const Float _a )
{
	vec128 = VecFloatLoad4( _r, _g, _b, _a );
	return *this;
}

inline Color& Color::Set( const Color &_v )
{
	vec128 = _v.vec128;
	return *this;
}

inline Color Color::operator * ( const Float _f) const
{
    return VecFloatScale( vec128, _f );
}

inline Color& Color::operator *= ( const Float _f )
{
    vec128 = VecFloatScale( vec128, _f );
    return *this;
}

inline Color& Color::operator = ( const Color& _c )
{
	vec128 = _c.vec128;
	return *this;
}

inline Color Color::operator + ( const Color& _v ) const
{
	return VecFloatAdd( vec128, _v.vec128 );
}

inline Color Color::operator-( const Color& _v ) const
{
	return VecFloatSub( vec128, _v.vec128 );
}

inline Color Color::operator*( const Color& _v ) const
{
	return VecFloatMul( vec128, _v.vec128 );
}

inline Color& Color::operator -= ( const Color &_v )
{
	vec128 = VecFloatSub( vec128, _v.vec128 );
	return *this;
}

inline Color& Color::operator += ( const Color &_v )
{
	vec128 = VecFloatAdd( vec128, _v.vec128 );
	return *this;
}

inline Color& Color::operator *= ( const Color &_v )
{
	vec128 = VecFloatMul( vec128, _v.vec128 );
	return *this;
}

inline Bool	Color::operator == ( const Color& v ) const
{
	return VecFloatAllNearEqual( vec128, v.vec128, VEC4F_EPSILON.vec128 ) != 0;
}

inline Bool	Color::operator!= ( const Color& v ) const
{
	return !( *this == v );
}

inline void MinColor(const Color &c1,const Color &c2, Color &result)
{
	result.vec128 = VecFloatMin( c1.vec128, c2.vec128 );
}

inline void MaxColor(const Color &c1,const Color &c2, Color &result)
{
	result.vec128 = VecFloatMax( c1.vec128, c2.vec128 );
}

inline Float  Color::GetGreyLevel() const
{
   return 0.299f*r + 0.587f*g + 0.114f*b;
}

inline Color Color::FromRGBToHSV() const
{
	Color col;
	float max = Max(Max(r, g), b);
	float min = Min(Min(r, g), b);

	col.b = max;
	col.g = (max > 0.f) ? ((max - min) / max) : 0.f;
	if (col.g == 0.f)
		col.r = 0.f;
	else
	{
		float delta = max - min;
		if (r == max)
			col.r = (g - b) / delta;
		else if (g == max)
			col.r = 2.f + (b - r) / delta;
		else if (b == max)
			col.r = 4.f + (r - g) / delta;
		if (col.r < 0.f)
			col.r += 6.f;
	}
	col.a = a;
	return col;
}

inline Color Color::FromHSVToRGB() const
{
	Color col;
	float h = r;
	if (g == 0.f)
		col.Set(b, b, b, a);
	else
	{
		float f, p, q, t;
		int i;
		if (h == 6.f)
			h = 0.f;
		i = int(floorf(h));
		f = h - float(i);
		p = b * (1.f - g);
		q = b * (1.f - (g * f));
		t = b * (1.f - (g * (1.f - f)));
		switch(i)
		{
		case 0: col.Set(b, t, p, a); break;
		case 1: col.Set(q, b, p, a); break;
		case 2: col.Set(p, b, t, a); break;
		case 3: col.Set(p, q, b, a); break;
		case 4: col.Set(t, p, b, a); break;
		case 5: col.Set(b, p, q, a); break;
		}
	}
	return col;
}