// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.


#include <pch.h>
#include <CollisionTool_Z.h>

/**************************************************************************/

#define GGEMS_LERP( A, B, C) ((B)+(A)*((C)-(B)))
#define GGEMS_MIN3(a,b,c) ((((a)<(b))&&((a)<(c))) ? (a) : (((b)<(c)) ? (b) : (c)))
#define GGEMS_MAX3(a,b,c) ((((a)>(b))&&((a)>(c))) ? (a) : (((b)>(c)) ? (b) : (c)))
#define GGEMS_INSIDE 0
#define GGEMS_OUTSIDE 1

/* Which of the six face-plane(s) is point P outside of? */
FINLINE_Z S32 face_plane(const Vec3f &p)
{
	S32 outcode;

	outcode = 0;
	if (p.x >  .5f) outcode |= 0x01;
	if (p.x < -.5f) outcode |= 0x02;
	if (p.y >  .5f) outcode |= 0x04;
	if (p.y < -.5f) outcode |= 0x08;
	if (p.z >  .5f) outcode |= 0x10;
	if (p.z < -.5f) outcode |= 0x20;
	return(outcode);
}

/* Which of the twelve edge plane(s) is point P outside of? */
FINLINE_Z S32 bevel_2d(const Vec3f &p)
{
	S32 outcode;

	outcode = 0;
	if (p.x + p.y > 1.0f) outcode |= 0x001;
	if (p.x - p.y > 1.0f) outcode |= 0x002;
	if (-p.x + p.y > 1.0f) outcode |= 0x004;
	if (-p.x - p.y > 1.0f) outcode |= 0x008;
	if (p.x + p.z > 1.0f) outcode |= 0x010;
	if (p.x - p.z > 1.0f) outcode |= 0x020;
	if (-p.x + p.z > 1.0f) outcode |= 0x040;
	if (-p.x - p.z > 1.0f) outcode |= 0x080;
	if (p.y + p.z > 1.0f) outcode |= 0x100;
	if (p.y - p.z > 1.0f) outcode |= 0x200;
	if (-p.y + p.z > 1.0f) outcode |= 0x400;
	if (-p.y - p.z > 1.0f) outcode |= 0x800;
	return(outcode);
}

/* Which of the eight corner plane(s) is point P outside of? */

FINLINE_Z S32 bevel_3d(const Vec3f &p)
{
	S32 outcode;

	outcode = 0;
	if ((p.x + p.y + p.z) > 1.5f) outcode |= 0x01;
	if ((p.x + p.y - p.z) > 1.5f) outcode |= 0x02;
	if ((p.x - p.y + p.z) > 1.5f) outcode |= 0x04;
	if ((p.x - p.y - p.z) > 1.5f) outcode |= 0x08;
	if ((-p.x + p.y + p.z) > 1.5f) outcode |= 0x10;
	if ((-p.x + p.y - p.z) > 1.5f) outcode |= 0x20;
	if ((-p.x - p.y + p.z) > 1.5f) outcode |= 0x40;
	if ((-p.x - p.y - p.z) > 1.5f) outcode |= 0x80;
	return(outcode);
}

/* Test the point "alpha" of the way from P1 to P2 */
/* See if it is on a face of the cube              */
/* Consider only faces in "mask"                   */

FINLINE_Z S32 check_point(const Vec3f &p1, const Vec3f &p2, Float alpha, S32 mask)
{
	Vec3f plane_point;
	plane_point.x = GGEMS_LERP(alpha, p1.x, p2.x);
	plane_point.y = GGEMS_LERP(alpha, p1.y, p2.y);
	plane_point.z = GGEMS_LERP(alpha, p1.z, p2.z);
	return(face_plane(plane_point) & mask);
}

/* Compute intersection of P1 --> P2 line segment with face planes */
/* Then test intersection point to see if it is on cube face       */
/* Consider only face planes in "outcode_diff"                     */
/* Note: Zero bits in "outcode_diff" means face line is outside of */

FINLINE_Z S32 check_line(const Vec3f &p1, const Vec3f &p2, S32 outcode_diff)
{

	if ((0x01 & outcode_diff) != 0)
		if (check_point(p1, p2, (.5f - p1.x) / (p2.x - p1.x), 0x3e) == GGEMS_INSIDE) return(GGEMS_INSIDE);
	if ((0x02 & outcode_diff) != 0)
		if (check_point(p1, p2, (-.5f - p1.x) / (p2.x - p1.x), 0x3d) == GGEMS_INSIDE) return(GGEMS_INSIDE);
	if ((0x04 & outcode_diff) != 0)
		if (check_point(p1, p2, (.5f - p1.y) / (p2.y - p1.y), 0x3b) == GGEMS_INSIDE) return(GGEMS_INSIDE);
	if ((0x08 & outcode_diff) != 0)
		if (check_point(p1, p2, (-.5f - p1.y) / (p2.y - p1.y), 0x37) == GGEMS_INSIDE) return(GGEMS_INSIDE);
	if ((0x10 & outcode_diff) != 0)
		if (check_point(p1, p2, (.5f - p1.z) / (p2.z - p1.z), 0x2f) == GGEMS_INSIDE) return(GGEMS_INSIDE);
	if ((0x20 & outcode_diff) != 0)
		if (check_point(p1, p2, (-.5f - p1.z) / (p2.z - p1.z), 0x1f) == GGEMS_INSIDE) return(GGEMS_INSIDE);
	return(GGEMS_OUTSIDE);
}

/* Test if 3D point is inside 3D triangle */

S32 point_triangle_intersection(const Vec3f &p, const Vec3f &v1, const Vec3f &v2, const Vec3f &v3)
{
	Vec3f vect12, vect23, vect31, vect1h, vect2h, vect3h;
	Vec3f cross12_1p, cross23_2p, cross31_3p;

	/* First, a quick bounding-box test:                               */
	/* If P is outside triangle bbox, there cannot be an intersection. */

	if (p.x > GGEMS_MAX3(v1.x, v2.x, v3.x) + 1e4f) return(GGEMS_OUTSIDE);
	if (p.z > GGEMS_MAX3(v1.z, v2.z, v3.z) + 1e4f) return(GGEMS_OUTSIDE);
	if (p.y > GGEMS_MAX3(v1.y, v2.y, v3.y) + 1e4f) return(GGEMS_OUTSIDE);
	if (p.x < GGEMS_MIN3(v1.x, v2.x, v3.x) - 1e4f) return(GGEMS_OUTSIDE);
	if (p.y < GGEMS_MIN3(v1.y, v2.y, v3.y) - 1e4f) return(GGEMS_OUTSIDE);
	if (p.z < GGEMS_MIN3(v1.z, v2.z, v3.z) - 1e4f) return(GGEMS_OUTSIDE);

	/* For each triangle side, make a vector out of it by subtracting vertexes; */
	/* make another vector from one vertex to point P.                          */
	/* The crossproduct of these two vectors is orthogonal to both and the      */
	/* signs of its X,Y,Z components indicate whether P was to the inside or    */
	/* to the outside of this triangle side.                                    */

	vect12 = v1 - v2;
	vect1h = v1 - p;
	cross12_1p = vect12^vect1h;

	vect23 = v2 - v3;
	vect2h = v2 - p;
	cross23_2p = vect23^vect2h;

	Float Sign1 = cross12_1p*cross23_2p;
	if (Sign1 < 0.f)	// Same Side ?
		return GGEMS_OUTSIDE;

	vect31 = v3 - v1;
	vect3h = v3 - p;
	cross31_3p = vect31^vect3h;

	Float Sign2 = cross23_2p*cross31_3p;
	if (Sign2 < 0.f)	// Same Side ?
		return GGEMS_OUTSIDE;

	return GGEMS_INSIDE;
}

/**********************************************/
/* This is the main algorithm procedure.      */
/* Triangle t is compared with a unit cube,   */
/* centered on the origin.                    */
/* It returns INSIDE (0) or OUTSIDE(1) if t   */
/* intersects or does not intersect the cube. */
/**********************************************/

VecFloat4 g_vAxis[3] = { VEC4F_LEFT, VEC4F_UP, VEC4F_FRONT };

S32 Cube111VsTriangleVec4(const VecFloat4 &_v0, const VecFloat4 &_v1, const VecFloat4 &_v2)
{
	VecSelMask vLeft[3], vRight[3];

	vRight[0] = VecFloatCompareGT(_v0, VectorConstantsPrivate::vHalfF);
	vLeft[0] = VecFloatCompareLT(_v0, VectorConstantsPrivate::vNegHalfF);
	if (VecSelMaskAll3False(VecSelMaskOr(vRight[0], vLeft[0])))
		return GGEMS_INSIDE;

	vRight[1] = VecFloatCompareGT(_v1, VectorConstantsPrivate::vHalfF);
	vLeft[1] = VecFloatCompareLT(_v1, VectorConstantsPrivate::vNegHalfF);
	if (VecSelMaskAll3False(VecSelMaskOr(vRight[1], vLeft[1])))
		return GGEMS_INSIDE;

	vRight[2] = VecFloatCompareGT(_v2, VectorConstantsPrivate::vHalfF);
	vLeft[2] = VecFloatCompareLT(_v2, VectorConstantsPrivate::vNegHalfF);
	if (VecSelMaskAll3False(VecSelMaskOr(vRight[2], vLeft[2])))
		return GGEMS_INSIDE;

	// inside box
	if (!VecSelMaskAll3False(VecSelMaskAnd(vLeft[0], VecSelMaskAnd(vLeft[1], vLeft[2]))))
		return GGEMS_OUTSIDE;
	if (!VecSelMaskAll3False(VecSelMaskAnd(vRight[0], VecSelMaskAnd(vRight[1], vRight[2]))))
		return GGEMS_OUTSIDE;

	// 9 tests
	VecFloat4 e[3];
	VecFloat4 fe[3];
	e[0] = VecFloatSub(_v1, _v0);	fe[0] = VecFloatAbs(e[0]);
	e[1] = VecFloatSub(_v2, _v1);	fe[1] = VecFloatAbs(e[1]);
	e[2] = VecFloatSub(_v0, _v2);	fe[2] = VecFloatAbs(e[2]);

	VecFloat4 a0, a1, a, r1, r2, vTot, vR;

#define _MACRO_AXIS(_axis, _edge, _vec0, _vec1) \
	r1 = VecFloatDot4(a, _v##_vec0); \
		r2 = VecFloatDot4(a, _v##_vec1); \
		vTot = VecFloatSelectEqual(g_vAxis[_axis], VectorConstantsPrivate::vZeroF, fe[_edge], VectorConstantsPrivate::vZeroF); \
		vTot = VecFloatMul(vTot, VectorConstantsPrivate::vHalfF);\
		vR = VecFloatAddComponents(vTot);\
		if (VecFloatAll3Greater(VecFloatMin(r1, r2), vR)) \
			return GGEMS_OUTSIDE;\
			if (VecFloatAll3GreaterOrEqual(VecFloatNegate(vR), VecFloatMax(r1, r2)))\
				return GGEMS_OUTSIDE;

	VecFloat4 eNeg = VecFloatNegate(e[0]);
	a0 = _VecFloatSwizzle_<VSWIZZLE_W, VSWIZZLE_W, VSWIZZLE_Y, VSWIZZLE_W>(e[0]);
	a1 = _VecFloatSwizzle_<VSWIZZLE_W, VSWIZZLE_Z, VSWIZZLE_W, VSWIZZLE_W>(eNeg);
	a = VecFloatAdd(a0, a1);
	_MACRO_AXIS(0, 0, 1, 2);
	a0 = _VecFloatSwizzle_<VSWIZZLE_Z, VSWIZZLE_W, VSWIZZLE_W, VSWIZZLE_W>(e[0]);
	a1 = _VecFloatSwizzle_<VSWIZZLE_W, VSWIZZLE_W, VSWIZZLE_X, VSWIZZLE_W>(eNeg);
	a = VecFloatAdd(a0, a1);
	_MACRO_AXIS(1, 0, 1, 2);
	a0 = _VecFloatSwizzle_<VSWIZZLE_W, VSWIZZLE_X, VSWIZZLE_W, VSWIZZLE_W>(e[0]);
	a1 = _VecFloatSwizzle_<VSWIZZLE_Y, VSWIZZLE_W, VSWIZZLE_W, VSWIZZLE_W>(eNeg);
	a = VecFloatAdd(a0, a1);
	_MACRO_AXIS(2, 0, 1, 2);

	eNeg = VecFloatNegate(e[1]);
	a0 = _VecFloatSwizzle_<VSWIZZLE_W, VSWIZZLE_W, VSWIZZLE_Y, VSWIZZLE_W>(e[1]);
	a1 = _VecFloatSwizzle_<VSWIZZLE_W, VSWIZZLE_Z, VSWIZZLE_W, VSWIZZLE_W>(eNeg);
	a = VecFloatAdd(a0, a1);
	_MACRO_AXIS(0, 1, 0, 1);
	a0 = _VecFloatSwizzle_<VSWIZZLE_Z, VSWIZZLE_W, VSWIZZLE_W, VSWIZZLE_W>(e[1]);
	a1 = _VecFloatSwizzle_<VSWIZZLE_W, VSWIZZLE_W, VSWIZZLE_X, VSWIZZLE_W>(eNeg);
	a = VecFloatAdd(a0, a1);
	_MACRO_AXIS(1, 1, 0, 1);
	a0 = _VecFloatSwizzle_<VSWIZZLE_W, VSWIZZLE_X, VSWIZZLE_W, VSWIZZLE_W>(e[1]);
	a1 = _VecFloatSwizzle_<VSWIZZLE_Y, VSWIZZLE_W, VSWIZZLE_W, VSWIZZLE_W>(eNeg);
	a = VecFloatAdd(a0, a1);
	_MACRO_AXIS(2, 1, 0, 1);

	eNeg = VecFloatNegate(e[2]);
	a0 = _VecFloatSwizzle_<VSWIZZLE_W, VSWIZZLE_W, VSWIZZLE_Y, VSWIZZLE_W>(e[2]);
	a1 = _VecFloatSwizzle_<VSWIZZLE_W, VSWIZZLE_Z, VSWIZZLE_W, VSWIZZLE_W>(eNeg);
	a = VecFloatAdd(a0, a1);
	_MACRO_AXIS(0, 2, 0, 1);
	a0 = _VecFloatSwizzle_<VSWIZZLE_Z, VSWIZZLE_W, VSWIZZLE_W, VSWIZZLE_W>(e[2]);
	a1 = _VecFloatSwizzle_<VSWIZZLE_W, VSWIZZLE_W, VSWIZZLE_X, VSWIZZLE_W>(eNeg);
	a = VecFloatAdd(a0, a1);
	_MACRO_AXIS(1, 2, 0, 1);
	a0 = _VecFloatSwizzle_<VSWIZZLE_W, VSWIZZLE_X, VSWIZZLE_W, VSWIZZLE_W>(e[2]);
	a1 = _VecFloatSwizzle_<VSWIZZLE_Y, VSWIZZLE_W, VSWIZZLE_W, VSWIZZLE_W>(eNeg);
	a = VecFloatAdd(a0, a1);
	_MACRO_AXIS(2, 2, 0, 1);

	// 
	VecFloat4 vNormal = VecFloatCross3(e[0], e[1]);
	VecFloat4 vValue0 = VecFloatSub(VectorConstantsPrivate::vNegHalfF, _v0);
	VecFloat4 vValue1 = VecFloatSub(VectorConstantsPrivate::vHalfF, _v0);
	VecFloat4 vMinF = VecFloatSelectLessOrEqual(vNormal, VectorConstantsPrivate::vZeroF, vValue1, vValue0);
	if (VecFloatAll3Greater(VecFloatDot3(vNormal, vMinF), VectorConstantsPrivate::vZeroF))
		return GGEMS_OUTSIDE;

	VecFloat4 vMaxF = VecFloatSelectLessOrEqual(vNormal, VectorConstantsPrivate::vZeroF, vValue0, vValue1);
	if (VecFloatAll3GreaterOrEqual(VecFloatDot3(vNormal, vMaxF), VectorConstantsPrivate::vZeroF))
		return GGEMS_INSIDE;

	return GGEMS_OUTSIDE;
}

S32 Cube111VsTriangle(const Vec3f &v1, const Vec3f &v2, const Vec3f &v3)
{
	S32 v1_test, v2_test, v3_test;
	Float d;
	Vec3f vect12, vect13, norm;
	Vec3f hitpp, hitpn, hitnp, hitnn;

	/* First compare all three vertexes with all six face-planes */
	/* If any vertex is inside the cube, return immediately!     */

	if ((v1_test = face_plane(v1)) == GGEMS_INSIDE) return(GGEMS_INSIDE);
	if ((v2_test = face_plane(v2)) == GGEMS_INSIDE) return(GGEMS_INSIDE);
	if ((v3_test = face_plane(v3)) == GGEMS_INSIDE) return(GGEMS_INSIDE);

	/* If all three vertexes were outside of one or more face-planes, */
	/* return immediately with a trivial rejection!                   */

	if ((v1_test & v2_test & v3_test) != 0) return(GGEMS_OUTSIDE);

	/* Now do the same trivial rejection test for the 12 edge planes */
	v1_test |= bevel_2d(v1) << 8;
	v2_test |= bevel_2d(v2) << 8;
	v3_test |= bevel_2d(v3) << 8;
	if ((v1_test & v2_test & v3_test) != 0) return(GGEMS_OUTSIDE);

	/* Now do the same trivial rejection test for the 8 corner planes */
	v1_test |= bevel_3d(v1) << 24;
	v2_test |= bevel_3d(v2) << 24;
	v3_test |= bevel_3d(v3) << 24;
	if ((v1_test & v2_test & v3_test) != 0) return(GGEMS_OUTSIDE);

	/* If vertex 1 and 2, as a pair, cannot be trivially rejected */
	/* by the above tests, then see if the v1-->v2 triangle edge  */
	/* intersects the cube.  Do the same for v1-->v3 and v2-->v3. */
	/* Pass to the intersection algorithm the "OR" of the outcode */
	/* bits, so that only those cube faces which are spanned by   */
	/* each triangle edge need be tested.                         */

	if ((v1_test & v2_test) == 0)
		if (check_line(v1, v2, v1_test | v2_test) == GGEMS_INSIDE) return(GGEMS_INSIDE);
	if ((v1_test & v3_test) == 0)
		if (check_line(v1, v3, v1_test | v3_test) == GGEMS_INSIDE) return(GGEMS_INSIDE);
	if ((v2_test & v3_test) == 0)
		if (check_line(v2, v3, v2_test | v3_test) == GGEMS_INSIDE) return(GGEMS_INSIDE);
	/* By now, we know that the triangle is not off to any side,     */
	/* and that its sides do not penetrate the cube.  We must now    */
	/* test for the cube intersecting the interior of the triangle.  */
	/* We do this by looking for intersections between the cube      */
	/* diagonals and the triangle...first finding the intersection   */
	/* of the four diagonals with the plane of the triangle, and     */
	/* then if that intersection is inside the cube, pursuing        */
	/* whether the intersection point is inside the triangle itself. */

	/* To find plane of the triangle, first perform crossproduct on  */
	/* two triangle side vectors to compute the normal vector.       */

	vect12 = v1 - v2;
	vect13 = v1 - v3;
	norm = vect12 ^ vect13;

	/* The normal vector "norm" X,Y,Z components are the coefficients */
	/* of the triangles AX + BY + CZ + D = 0 plane equation.  If we   */
	/* solve the plane equation for X=Y=Z (a diagonal), we get        */
	/* -D/(A+B+C) as a metric of the distance from cube center to the */
	/* diagonal/plane intersection.  If this is between -0.5 and 0.5, */
	/* the intersection is inside the cube.  If so, we continue by    */
	/* doing a point/triangle intersection.                           */
	/* Do this for all four diagonals.                                */

	d = norm * v1;

	/* if one of the diagonals is parallel to the plane, the other will intersect the plane */

	Float denom = (norm.x + norm.y + norm.z);
	if (fabs(denom)>Float_Eps)
		/* skip parallel diagonals to the plane; division by 0 can occur */
	{
		hitpp.x = hitpp.y = hitpp.z = d / denom;
		if (fabs(hitpp.x) <= 0.5)
			if (point_triangle_intersection(hitpp, v1, v2, v3) == GGEMS_INSIDE) return(GGEMS_INSIDE);
	}
	denom = (norm.x + norm.y - norm.z);
	if (fabs(denom)>Float_Eps)
	{
		hitpn.z = -(hitpn.x = hitpn.y = d / denom);
		if (fabs(hitpn.x) <= 0.5)
			if (point_triangle_intersection(hitpn, v1, v2, v3) == GGEMS_INSIDE) return(GGEMS_INSIDE);
	}
	denom = (norm.x - norm.y + norm.z);
	if (fabs(denom)>Float_Eps)
	{
		hitnp.y = -(hitnp.x = hitnp.z = d / denom);
		if (fabs(hitnp.x) <= 0.5)
			if (point_triangle_intersection(hitnp, v1, v2, v3) == GGEMS_INSIDE) return(GGEMS_INSIDE);
	}
	denom = (norm.x - norm.y - norm.z);
	if (fabs(denom)>Float_Eps)
	{
		hitnn.y = hitnn.z = -(hitnn.x = d / denom);
		if (fabs(hitnn.x) <= 0.5)
			if (point_triangle_intersection(hitnn, v1, v2, v3) == GGEMS_INSIDE) return(GGEMS_INSIDE);
	}

	/* No edge touched the cube; no cube diagonal touched the triangle. */
	/* We're done...there was no intersection.                          */

	return(GGEMS_OUTSIDE);
}

/**************************************************************************/

S32	Recurse_NaiveCube111VsTriangle(S32 _Level, const Vec3f &Localv1, const Vec3f &Localv2, const Vec3f &Localv3, Float HalfSize)
{
	Float NegHalfSize = -HalfSize;
	// Subdivide.
	Vec3f	p12 = (Localv1 + Localv2) *0.5f;
	Vec3f	p23 = (Localv2 + Localv3) *0.5f;
	Vec3f	p31 = (Localv3 + Localv1) *0.5f;

	if ((p12.x > NegHalfSize) && (p12.x < HalfSize)
		&& (p12.y > NegHalfSize) && (p12.y < HalfSize)
		&& (p12.z > NegHalfSize) && (p12.z < HalfSize)
		)
		return GGEMS_INSIDE;

	if ((p23.x > NegHalfSize) && (p23.x < HalfSize)
		&& (p23.y > NegHalfSize) && (p23.y < HalfSize)
		&& (p23.z > NegHalfSize) && (p23.z < HalfSize)
		)
		return GGEMS_INSIDE;

	if ((p31.x > NegHalfSize) && (p31.x < HalfSize)
		&& (p31.y > NegHalfSize) && (p31.y < HalfSize)
		&& (p31.z > NegHalfSize) && (p31.z < HalfSize)
		)
		return GGEMS_INSIDE;

	_Level--;
	if (!_Level)
		return GGEMS_OUTSIDE;

	if (Recurse_NaiveCube111VsTriangle(_Level, Localv1, p12, p31, HalfSize) == GGEMS_INSIDE)
		return GGEMS_INSIDE;
	if (Recurse_NaiveCube111VsTriangle(_Level, Localv2, p23, p12, HalfSize) == GGEMS_INSIDE)
		return GGEMS_INSIDE;
	if (Recurse_NaiveCube111VsTriangle(_Level, Localv3, p31, p23, HalfSize) == GGEMS_INSIDE)
		return GGEMS_INSIDE;

	return Recurse_NaiveCube111VsTriangle(_Level, p12, p23, p31, HalfSize);
}

S32	NaiveCube111VsTriangle(const Vec3f &Localv1, const Vec3f &Localv2, const Vec3f &Localv3, S32 MaxIt, Float HalfSize)
{
	Float NegHalfSize = -HalfSize;

	if ((Localv1.x > NegHalfSize) && (Localv1.x < HalfSize)
		&& (Localv1.y > NegHalfSize) && (Localv1.y < HalfSize)
		&& (Localv1.z > NegHalfSize) && (Localv1.z < HalfSize)
		)
		return GGEMS_INSIDE;

	if ((Localv2.x > NegHalfSize) && (Localv2.x < HalfSize)
		&& (Localv2.y > NegHalfSize) && (Localv2.y < HalfSize)
		&& (Localv2.z > NegHalfSize) && (Localv2.z < HalfSize)
		)
		return GGEMS_INSIDE;

	if ((Localv3.x > NegHalfSize) && (Localv3.x < HalfSize)
		&& (Localv3.y > NegHalfSize) && (Localv3.y < HalfSize)
		&& (Localv3.z > NegHalfSize) && (Localv3.z < HalfSize)
		)
		return GGEMS_INSIDE;

	return Recurse_NaiveCube111VsTriangle(MaxIt, Localv1, Localv2, Localv3, HalfSize);
}

/**************************************************************************/

Bool AABoxVsTriangle(const Vec3f &_Min, const Vec3f &_Max, const Vec3f &v1, const Vec3f &v2, const Vec3f &v3)
{
	Vec3f	Center = (_Min + _Max) * 0.5f;
	Vec3f	Scale = _Max - _Min;
	Scale.x = 1.f / Scale.x;
	Scale.y = 1.f / Scale.y;
	Scale.z = 1.f / Scale.z;
	Vec3f Localv1 = (v1 - Center) & Scale;
	Vec3f Localv2 = (v2 - Center) & Scale;
	Vec3f Localv3 = (v3 - Center) & Scale;

	Bool Res1 = (Cube111VsTriangle(Localv1, Localv2, Localv3) == GGEMS_INSIDE);

	return Res1;
}

/**************************************************************************/

//---------------------------------------------------------------------
//						... Collision Line / Patch
//---------------------------------------------------------------------

Bool LineVsPatch(const Segment_Z &Ray, Vec4f *Point, CollisionReport_Z &Report, S32 Lod)
{
	Bool Ret = FALSE;

	Vec4f	*PointPtr0 = Point;
	Vec4f	*PointPtr1 = Point + Lod + 1;
	Vec4f	*PointPtr2 = Point + 1;
	Vec4f	*PointPtr3 = PointPtr1 + 1;

	Vec4f	*Normal = Point + (Lod + 1)*(Lod + 1);

	Vec4f	*PPtr0;
	Vec4f	*PPtr1;
	Vec4f	*PPtr2;

	for (int i = 0; i<Lod; i++)
	{
		for (int j = 0; j<Lod; j++)
		{
			//Bsphere from the 2 next Triangles
			Float	Proj = (Normal->x - Ray.Org.x)*Ray.Dir.x +
				(Normal->y - Ray.Org.y)*Ray.Dir.y +
				(Normal->z - Ray.Org.z)*Ray.Dir.z;

			Vec3f	ortho = Proj*Ray.Dir + Ray.Org;

			//Retreive Sphere Center
			ortho.x -= Normal->x;
			ortho.y -= Normal->y;
			ortho.z -= Normal->z;

			//Vs Radius
			if (1)//ortho*ortho<=Normal->w*Normal->w)
			{
				Normal++;	//Skip to reach triangles normals

							// Paire of Triangle Vs Seg
				Vec4f	t1 = Ray.Org - (*PointPtr0);
				for (int k = 0; k<2; k++, Normal++)
				{
					if (Normal->w)
					{
						Float Dp = *Normal*t1;
						if (Dp>0.f)
						{
							Float	Dot = Normal->xyz()*Ray.Dir;
							Float	Dist = (POS_Z(Dot)<Float_Eps) ? -1.f : -(Float)(Dp / Dot);
							//Ray Range and Previous Report Dist Have to Fit
							if (Dist >= 0.f && Dist <= Ray.Len && Dist<Report.Dist)
							{
								// New test for point in triangle with barycentric coordinates
								// Same result used for proper UV coordinates inside patch
								Float diffU = 0.0f, diffV = 0.0f, mulU = 1.0f, mulV = 1.0f;
								if (k)
								{
									PPtr0 = PointPtr2; PPtr1 = PointPtr0; PPtr2 = PointPtr3;
									diffU = 1.0f;
									diffV = 0.0f;
									mulV = -1.0f;
								}
								else
								{
									PPtr0 = PointPtr1; PPtr1 = PointPtr3; PPtr2 = PointPtr0;
									diffU = 0.0f;
									mulU = -1.0f;
									diffV = 1.0f;
								}

								Vec3f	Inter = Ray.Dir*Dist + Ray.Org;
								Vec4f	t1 = *PPtr1 - *PPtr0;
								Vec4f	t2 = *PPtr2 - *PPtr0;
								Vec4f	tI = Inter - *PPtr0;

								Float	d11 = Vec4_Dot(t1, t1);
								Float	d12 = Vec4_Dot(t1, t2);
								Float	d1I = Vec4_Dot(t1, tI);
								Float	d22 = Vec4_Dot(t2, t2);
								Float	d2I = Vec4_Dot(t2, tI);

								Float	invDenom = 1.0f / (d11*d22 - d12*d12);
								Float	u = (d22*d1I - d12*d2I)*invDenom;
								Float	v = (d11*d2I - d12*d1I)*invDenom;

								if (u > 0.0f && v > 0.0f && (u + v) < 1.0f)
								{
									Report.Normal = *Normal;
									Report.Inter = Inter;
									Report.Dist = Dist;
									Report.UV.x = ((diffU - mulU * u) + j) / (float)Lod;
									Report.UV.y = ((diffV - mulV * v) + i) / (float)Lod;
									Ret = TRUE;
								}
							}
						}
					}
				}
			}

			else

			Normal += 3;

			PointPtr0++;	PointPtr1++;	PointPtr2++;	PointPtr3++;
		}
		PointPtr0++;	PointPtr1++;	PointPtr2++;	PointPtr3++;
	}
	return Ret;
}

//---------------------------------------------------------------------
//							.. Vs Box
//---------------------------------------------------------------------

Bool SphereVsBox(const Sphere_Z &Sph, const Box_Z &Box)
{
	//Retreive Translation
	Vec3f	g(Sph.Center.x - Box.Mat.m.m[0][3],
		Sph.Center.y - Box.Mat.m.m[1][3],
		Sph.Center.z - Box.Mat.m.m[2][3]);

	Vec3f	p(Box.Mat.m.m[0][0] * g.x + Box.Mat.m.m[0][1] * g.y + Box.Mat.m.m[0][2] * g.z,
		Box.Mat.m.m[1][0] * g.x + Box.Mat.m.m[1][1] * g.y + Box.Mat.m.m[1][2] * g.z,
		Box.Mat.m.m[2][0] * g.x + Box.Mat.m.m[2][1] * g.y + Box.Mat.m.m[2][2] * g.z);

	Float	*c = (float *)&p;
	Float	*bscale = (float *)&Box.Scale.x;

	Float	id, rr;
	rr = Sph.Radius*Sph.Radius;
	id = 0.f;
	for (int i = 0; i<3; i++, c++, bscale++)
	{
		Float	s;
		if (*c<-*bscale)
		{
			s = (*c + *bscale); id += s*s;
		}
		else if (*c>*bscale)
		{
			s = (*c - *bscale); id += s*s;
		}
		if (id>rr)
			return FALSE;
	}
	return	TRUE;
}

//---------------------------------------------------------------------
//							.. Vs Sphere
//---------------------------------------------------------------------
Bool SegmentVsSphere(const Segment_Z &Seg, const Sphere_Z &Sphere)
{
	register VecFloat4 proj, ortho;

	register const VecFloat4 zero = VecFloatSplatZero();

	register const VecFloat4 segOrg = VecFloatLoadAligned(&Seg.Org.x);
	register const VecFloat4 segDir = VecFloatLoadAligned(&Seg.Dir.x);
	register const VecFloat4 segLen = VecFloatSplatW(segOrg);

	register const VecFloat4 sphereCenter = VecFloatLoadAligned(&Sphere.Center.x);
	register const VecFloat4 sphereRadius = VecFloatSplatW(sphereCenter);

	// proj = (Sphere.Center-Seg.Org) . Seg.Dir
	proj = VecFloatDot3(VecFloatSub(sphereCenter, segOrg), segDir);
	proj = VecFloatClamp(proj, zero, segLen);

	// ortho = proj*dir + Seg.Org - Sphere.Center
	ortho = VecFloatMadd(segDir, proj, segOrg);
	ortho = VecFloatSub(ortho, sphereCenter);

	// return |ortho|� <= Sphere.Radius�
	U32 result = VecFloatAllLessOrEqual(VecFloatDot3(ortho, ortho), VecFloatMul(sphereRadius, sphereRadius));

	return result != 0;
}
