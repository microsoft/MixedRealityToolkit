// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include <pch.h>
#include <PlaySpace\PlaySpace_Tools_W.h>
#include <PlaySpace\PlaySpace_Mesh_W.h>

/**************************************************************************/
S32		PlaySpaceAlign_W::NbCheckNormal = NbAllocNormal;

PlaySpaceAlign_W::PlaySpaceAlign_W()
{
	Flush();
}

void	PlaySpaceAlign_W::AddNormal(Vec3f	_Normal)
{
	_Normal.CHNormalize();
	TabNormal[CurNormal++] = _Normal;

	if (NbNormal < NbCheckNormal)
		NbNormal++;

	if (CurNormal >= NbCheckNormal)
		CurNormal = 0;
}

Bool	PlaySpaceAlign_W::IsOkForAlign(Vec3f	&_Normal)
{
	if (NbNormal < NbCheckNormal)
		return FALSE;

	// Sum.
	Vec3f	SumNormal = TabNormal[0];
	for (S32 i = 1; i<NbCheckNormal; i++)
		SumNormal += TabNormal[i];

	SumNormal.CHNormalize();

	// Max Angle.
	Float MinCosAngle = 2.f;
	for (S32 i = 0; i<NbCheckNormal; i++)
		MinCosAngle = Min(MinCosAngle, SumNormal*TabNormal[i]);

	if (MinCosAngle < Cos(DegToRad(3.f)))
		return FALSE;

	_Normal = SumNormal;
	return TRUE;
}

void PlaySpaceAlign_W::Flush()
{
	CurNormal = 0;
	NbNormal = 0;
}

/**************************************************************************/

void	PlaySpaceAlign_W::SetNbCheckNormal(S32 _Nb)
{
	EXCEPTIONC_Z(_Nb<NbAllocNormal,"PlaySpaceAlign_W : SetNbCheckNormal => Too many Normal needed");
	NbCheckNormal = _Nb;
}


/**************************************************************************/

#define	NEW_DETECT_WALL_H		24
#define	NEW_DETECT_WALL_LATERAL	64

class LS_WallAngleDetect
{
public:
	Vec2f	CurNormal;
	Vec2f	SumNormal;
	S32		NbValue;
	Float	Surface;
};


FINLINE_Z void AddNormalToWallAngleDetect(DynArray_Z<LS_WallAngleDetect, 1024, FALSE, FALSE> &_TabAngles, Vec3f &_Normal, Float _Surface = 1.f)
{
	// Test All Quadrant. (Si on amène tout dans un qudrant, ça fait nawak sur les bords... :()
	Vec2f Normal2D(_Normal);
	Normal2D.CNormalize();

	// Search All compatible angles.
	Float BetterDot = -2.f;
	S32 Size = _TabAngles.GetSize();
	Float MinDot = Cos(DegToRad(6.f));
	for (S32 i = 0; i<Size; i++)
	{
		Vec2f	CurNormal = _TabAngles[i].CurNormal;
		Float TheDot1 = CurNormal.x * Normal2D.x + CurNormal.y * Normal2D.y;
		Float TheDot2 = CurNormal.y * Normal2D.x - CurNormal.x * Normal2D.y;
		Float TheDot3 = -TheDot1;
		Float TheDot4 = -TheDot2;

		// Max Dot...
		Float MaxDot = TheDot1;
		Vec2f MaxNorm = Normal2D;
		if (MaxDot < TheDot2)
		{
			MaxDot = TheDot2;
			MaxNorm.x = -Normal2D.y;
			MaxNorm.y = Normal2D.x;
		}
		if (MaxDot < TheDot3)
		{
			MaxDot = TheDot3;
			MaxNorm = -Normal2D;
		}
		if (MaxDot < TheDot4)
		{
			MaxDot = TheDot4;
			MaxNorm.x = Normal2D.y;
			MaxNorm.y = -Normal2D.x;
		}

		if (MaxDot > MinDot)
		{
			BetterDot = Max(MaxDot, BetterDot);
			_TabAngles[i].SumNormal += MaxNorm * _Surface;
			_TabAngles[i].Surface += _Surface;

			_TabAngles[i].CurNormal = _TabAngles[i].SumNormal;
			_TabAngles[i].CurNormal *= 1.f/_TabAngles[i].Surface;	// Prepare for Normalize !!!
			_TabAngles[i].CurNormal.CNormalize();
			_TabAngles[i].NbValue++;
		}
	}

	if (BetterDot < 0.f)
	{
		S32 Num = _TabAngles.Add();
		_TabAngles[Num].SumNormal = Normal2D * _Surface;// *Float(_NbVal);
		_TabAngles[Num].CurNormal = Normal2D;
		_TabAngles[Num].NbValue = 1;
		_TabAngles[Num].Surface = _Surface;

	}
}

/**************************************************************************/

Float	PlaySpaceAlign_W::GetAlignAxis_Mesh(Playspace_Mesh &_Mesh, Float _YGround, Float _YCeiling, Vec3f &_ResultNormal, Bool _DrawIt)
{
	// Prepare.
	_Mesh.ComputePointsLinks();
	_Mesh.ComputeFacesToolNormal();
	_Mesh.ComputePointsToolNormal();

	// Force Max Delta Ground-Ceiling...
	_YGround += 0.1f;
	_YCeiling -= 0.1f;

	if ((_YCeiling - _YGround) > 3.f)
		_YCeiling = _YGround + 3.f;
	else if ((_YCeiling - _YGround) < 1.f)
		_YCeiling = _YGround + 2.f;

	Float DeltaH = _YCeiling - _YGround;

	// Angle Table.
	DynArray_Z<LS_WallAngleDetect, 1024, FALSE, FALSE> TabAngles;

	// Now Get Some infos.
	Segment_Z	SegView;
	Util_L::GetViewSegment(&SegView);

	Vec3f	EyePos = SegView.Org;
	Vec3f	FlatSegViewDir = SegView.Dir;
	FlatSegViewDir.CHNormalize();

	// Search Vertical surfaces.
	Float MinCos = Cos(DegToRad(10.f));
	Float OpenCos = Cos(DegToRad(90.f));

	S32 NbPoint = _Mesh.m_TabPoints.GetSize();
	for (S32 i = 0; i<NbPoint; i++)
	{
		// Compute normale.
		Playspace_Mesh::ToolPointNormal &NormInfos = _Mesh.m_TabPointsToolNormal[i];
		if (NormInfos.Error < MinCos)
			continue;
		if (NormInfos.Surface < 0.0001f)
			continue;

		Vec3f vNormal = NormInfos.Normal;
		Vec3f vPos = _Mesh.m_TabPoints[i];

		// Ground Ceiling
		if (vPos.y < _YGround)
			continue;
		if (vPos.y > _YCeiling)
			continue;
		if ((vNormal.y > 0.707f) || (vNormal.y < -0.707f))
			continue;

		// Good Angle ?
		Vec3f Dir = vPos - EyePos;
		Dir.CHNormalize();

		if ((Dir * FlatSegViewDir) < OpenCos)
			continue;

		// Got IT !

		if (_DrawIt)
		{
			Color TheCoolCol = COLOR_GREEN;
			Float ColorV = Atan2(vNormal.z, vNormal.x);
			ColorV += M_PI;	// Rotation de 180 pour garder continuité MAIS voir que du positif.
			TheCoolCol.r = ColorV * 6.f / M_2_PI;
			TheCoolCol.g = 1.f;	// S
			TheCoolCol.b = 1.f; // V
			TheCoolCol = TheCoolCol.FromHSVToRGB();
			TheCoolCol.a = 1.f;
			DRAW_DEBUG_LINE3D(vPos, vPos + vNormal*0.2f, TheCoolCol, 0.01f);
		}

		AddNormalToWallAngleDetect(TabAngles, vNormal, NormInfos.Surface);
	}
	if (!TabAngles.GetSize())
		return FALSE;

	// Get the best One.
	Float MaxSurface = TabAngles[0].Surface;
	S32 NumMax = 0;
	for (S32 i = 1; i < TabAngles.GetSize(); i++)
	{
		S32 Surface = TabAngles[i].Surface;
		if (Surface > MaxSurface)
		{
			NumMax = i;
			MaxSurface = Surface;
		}
	}

	_ResultNormal.x = TabAngles[NumMax].CurNormal.x;
	_ResultNormal.y = 0.f;
	_ResultNormal.z = TabAngles[NumMax].CurNormal.y;

	return MaxSurface;
}

/**************************************************************************/

Bool	PlaySpaceAlign_W::SearchStabilizedAlignAxis_Mesh(Playspace_Mesh &_Mesh, Float _YGround, Float _YCeiling, Vec3f	&_ResultAxis, Bool _ForceIt, Bool _DrawIt)
{
	Vec3f StabilizedNormal;
	Float MaxSurface = GetAlignAxis_Mesh(_Mesh, _YGround, _YCeiling, StabilizedNormal, _DrawIt);
	if (MaxSurface < 0.2f)
		return FALSE;

	AddNormal(StabilizedNormal);
	_ResultAxis = StabilizedNormal;

	//	MESSAGE_Z("ALIGN TEST (%d) %f => %f",TabAngles.GetSize(),MaxSurface,Atan2(StabilizedNormal.z, StabilizedNormal.x));

	if (_ForceIt)
	{
		// Force axis but don't Flush.
		_ResultAxis = StabilizedNormal;
	}
	if (IsOkForAlign(StabilizedNormal))
	{
		_ResultAxis = StabilizedNormal;
		Flush();
		return TRUE;
	}

	if (_DrawIt)
	{
		Segment_Z	SegView;
		Util_L::GetViewSegment(&SegView);
		Vec3f	FlatSegViewDir = SegView.Dir;
		FlatSegViewDir.CHNormalize();

		Vec3f DeltaDraw = VEC3F_DOWN * 1.5f + FlatSegViewDir * 1.5f;
		DRAW_DEBUG_LINE3D(SegView.Org + VEC3F_LEFT * 3.f + DeltaDraw, SegView.Org + VEC3F_RIGHT * 1.f + DeltaDraw, COLOR_GREEN, 0.01f);
		DRAW_DEBUG_LINE3D(SegView.Org + VEC3F_FRONT * 3.f + DeltaDraw, SegView.Org + VEC3F_BACK * 1.f + DeltaDraw, COLOR_RED, 0.01f);
		DeltaDraw += VEC3F_UP * 0.01f;
		DRAW_DEBUG_LINE3D(SegView.Org + StabilizedNormal * 3.f + DeltaDraw, SegView.Org - StabilizedNormal * 1.f + DeltaDraw, COLOR_BLUE, 0.01f);
	}
	return FALSE;
}

/**************************************************************************/


