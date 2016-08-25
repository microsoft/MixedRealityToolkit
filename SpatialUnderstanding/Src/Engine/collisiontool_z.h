// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#ifndef _COLLISIONTOOL_Z_H
#define _COLLISIONTOOL_Z_H

#include <SystemObject_Z.h>
#include <BeginDef_Z.h>

// ----------------------------------------------------------------------------
// Report / Data align / sizeof()=x^2
// ----------------------------------------------------------------------------
struct CollisionReport_Z
{
public:
	Vec4f		Inter;
	Vec4f		Normal;

	Vec2f		UV;
	Float		Dist;			//Align

	CollisionReport_Z() { Dist = 1E10f;}
	inline	Bool IsIntersect()const { return	Dist<1E10f; }
};

Bool AABoxVsTriangle(const Vec3f &_Min, const Vec3f &_Max, const Vec3f &v1, const Vec3f &v2, const Vec3f &v3);
Bool SphereVsBox(const Sphere_Z &Sph, const Box_Z &Box);
Bool SegmentVsSphere(const Segment_Z &Seg, const Sphere_Z &Sph);
Bool LineVsPatch(const Segment_Z &Ray, Vec4f *Point, CollisionReport_Z &Report, S32 Lod);

#endif