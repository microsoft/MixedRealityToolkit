// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#ifndef __PLAYSPACE_SURFEL_H__
#define __PLAYSPACE_SURFEL_H__

/**************************************************************************/
/* SURFELS.                                                               */
/**************************************************************************/

class PlaySpace_Surfel
{
public:
	typedef enum {
		SURFDIR_LEFT = 0x0,
		SURFDIR_RIGHT = 0x1,
		SURFDIR_UP = 0x2,
		SURFDIR_DOWN = 0x3,
		SURFDIR_FRONT = 0x4,
		SURFDIR_BACK = 0x5
	} SurfDir;
	typedef enum {
		SURFFLAG_VIRTUAL = (1 << 1),
		SURFFLAG_BASIN = (1 << 2),
		SURFFLAG_NOGAMEPLAY = (1 << 3),
		SURFFLAG_BADSURFEL = (1 << 4),
		SURFFLAG_DEBUG = (1 << 5),
		SURFFLAG_EXTERNAL = (1 << 6),
		SURFFLAG_ALL = 0xFF
	} Flags;

	Vec3f	Normal;
	Vec3f	Point;

	S16		x, y, z;
	S16		ZoneId;
	S8		iDir;
	U8		Quality;
	U8		Flags;

	PlaySpace_Surfel *pNext;

	void Init()
	{
		Quality = 0;
		iDir = 0;
		ZoneId = -1;
		Flags = 0;
	}

	static FINLINE_Z const Vec3f	&iDir2Normal(S32 _iDir)
	{
		static Vec3f TabV[] = { VEC3F_LEFT, VEC3F_RIGHT, VEC3F_UP, VEC3F_DOWN, VEC3F_FRONT, VEC3F_BACK };
		return TabV[_iDir];
	}
	static FINLINE_Z const Vec3i	&iDir2iNormal(S32 _iDir)
	{
		static Vec3i TabV[] = { Vec3i(1, 0, 0), Vec3i(-1, 0, 0), Vec3i(0, 1, 0), Vec3i(0, -1, 0), Vec3i(0, 0, 1), Vec3i(0, 0, -1) };
		return TabV[_iDir];
	}
	static FINLINE_Z S32			Normal2iDir(Vec3f _Normal)
	{
		Vec3i	iNormal(SURFDIR_LEFT, SURFDIR_UP, SURFDIR_FRONT);

		// normal Dir.
		if (_Normal.x < 0.f)
		{
			_Normal.x = -_Normal.x;
			iNormal.x++;
		}
		if (_Normal.y < 0.f)
		{
			_Normal.y = -_Normal.y;
			iNormal.y++;
		}
		if (_Normal.z < 0.f)
		{
			_Normal.z = -_Normal.z;
			iNormal.z++;
		}
		S32	NormalDir = -1;
		if (_Normal.x > _Normal.y)
		{
			if (_Normal.x > _Normal.z)
				NormalDir = iNormal.x;
			else
				NormalDir = iNormal.z;
		}
		else if (_Normal.y > _Normal.z)
		{
			NormalDir = iNormal.y;
		}
		else
		{
			NormalDir = iNormal.z;
		}

		return NormalDir;
	}

	FINLINE_Z void	SetBasin(Bool _IsSet) { if (_IsSet) Flags |= SURFFLAG_BASIN; else Flags &= (SURFFLAG_ALL ^ SURFFLAG_BASIN); }
	FINLINE_Z Bool	IsBasin() const { return ((Flags & SURFFLAG_BASIN) != 0); }
	FINLINE_Z void	SetNoGameplay(Bool _IsSet) { if (_IsSet) Flags |= SURFFLAG_NOGAMEPLAY; else Flags &= (SURFFLAG_ALL ^ SURFFLAG_NOGAMEPLAY); }
	FINLINE_Z Bool	NoGameplay() const { return ((Flags & SURFFLAG_NOGAMEPLAY) != 0); }

	FINLINE_Z void	SetDebug(Bool _IsSet) { if (_IsSet) Flags |= SURFFLAG_DEBUG; else Flags &= (SURFFLAG_ALL ^ SURFFLAG_DEBUG); }
	FINLINE_Z Bool	IsDebug() const { return ((Flags & SURFFLAG_DEBUG) != 0); }

	FINLINE_Z Bool	RayCast(const Vec3f &_Pos, const Vec3f &_Dir, Vec3f &_vResult, Float &_NewDist)	// NewDist is MaxDist too
	{
		// Angle.
		Float	CosAngle = Normal * _Dir;
		if (CosAngle > -0.01f)
			return FALSE;

		// Dist.
		Vec3f	vDelta = _Pos - Point;
		Float	Dp = vDelta * Normal;
		Float	DistSurf = -Dp / CosAngle;
		if (DistSurf > _NewDist)
			return FALSE;

		Vec3f PtInter = _Dir*DistSurf + _Pos;
		if ((PtInter - Point).GetNorm2() >= (0.1f*0.1f))
			return FALSE;
		_NewDist = DistSurf;
		_vResult = PtInter;
		return TRUE;
	}

	FINLINE_Z	Bool	CanGo(PlaySpace_Surfel *_pOther)
	{
		// orientation 
		Float DotNormal = Normal * _pOther->Normal;
		if (DotNormal < 0.9f)
			return FALSE;

		Vec3f	vDelta = _pOther->Point - Point;
		Float	proj1 = Abs(Normal * vDelta);
		Float	proj2 = Abs(_pOther->Normal * vDelta);
		Float MaxProj = Max(proj1, proj2);
		if (MaxProj >= 0.04f)
			return FALSE;

		return TRUE;
	}
};

#endif //__PLAYSPACE_SURFEL_H__
