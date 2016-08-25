// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "pch.h"

Vec3f	Util_L::CameraPos;
Vec3f	Util_L::CameraDir;
Vec3f	Util_L::CameraDirUp;

void Util_L::ApplyTranfoToCamera(Quat &_Transfo)
{
	CameraPos = _Transfo * CameraPos;
	CameraDir = _Transfo * CameraDir;
	CameraDirUp = _Transfo * CameraDirUp;
}

void Util_L::SetCurrentCamera(const Vec3f &_Pos, const Vec3f &_Dir, const Vec3f &_DirUp)
{
	CameraPos = _Pos;
	CameraDir = _Dir;
	CameraDirUp = _DirUp;
}

Bool Util_L::GetViewSegment(Segment_Z *_out_pSeg, Vec3f *_pUpView)
{
	_out_pSeg->Org = CameraPos;
	_out_pSeg->Dir = CameraDir;
	_out_pSeg->Len = 50.f;	if (_pUpView)
	*_pUpView = CameraDirUp;
	return TRUE;	
}

Bool Util_L::GetAllViewData(Vec3f& _vPos, Vec3f& _vFront, Vec3f& _vRight, Vec3f& _vUp)
{
	_vPos = CameraPos;
	_vFront = CameraDir;
	_vUp = CameraDirUp;
	_vRight = _vFront^_vUp;		// Notice that we are left handed in Engine ... Right is to -1. :(
	return TRUE;
}

Quat Util_L::GetOrientationQuat(const Vec3f &_nodePos, const Vec3f &_viewPos, const Vec3f &_baseUp, S32 _order[3], Float *_pDist)
{
	Vec3f	vFrontDir = _viewPos - _nodePos;
	if (_pDist)
	{
		(*_pDist) = vFrontDir.GetNorm();
	}
	vFrontDir.CNormalize();	
	return GetOrientationQuat(vFrontDir, _baseUp, _order);
}

Quat	Util_L::GetOrientationQuat(const Vec3f &_front, const Vec3f &_baseUp, S32 _order[3])
{
	// orientation / sol
	Mat4x4	rot;
	Quat	qRot;
	Vec3f	vRight = _front ^  _baseUp;
	vRight.CNormalize();
	Vec3f	vUp = vRight ^ _front;
	vUp.CNormalize();	
	Vec3f	v[3] = { _front, vUp, vRight };	
	for (S32 i = 0;i < 3;i++)
		rot.GetRow(i) = Sign(_order[i]) * v[Abs(_order[i]) - 1];
	qRot = Quat(rot);
	qRot.Normalize();
	return qRot;
}

Bool Util_L::GetAttributeAfterColon(const char *_txtFull, const char *_txtSearched, const char **_pOutParamText)
{
	extern U8 g_ChartoLower[256];
	const char *pKeptTxtForException = _txtFull;
	while (TRUE)
	{
		if (*_txtFull == ':')
		{
			if (*_txtSearched != 0) //Check we have check the whole _txtSearched 
			{
				return FALSE;
			}
			CANCEL_EXCEPTIONC_Z(_txtFull[1], "Missing Param after : %s", pKeptTxtForException);
			if (_pOutParamText)
				*_pOutParamText = _txtFull + 1;
			return TRUE;
		}		
		//if (g_ChartoLower[*_txtFull] != g_ChartoLower[*_txtSearched])
		if ((*_txtFull & 0xDF) != (*_txtSearched & 0xDF)) // CONEX -> faster, may cause pb with [ and { for instance
		{
			return FALSE;
		}
		else
		{
			if (*_txtFull == 0)
			{
				if (_pOutParamText)
					*_pOutParamText = NULL;
				return TRUE;
			}
		}
		_txtFull++;
		_txtSearched++;
	}
}

static U32 globalFrameCount = 0;
U32 GetGlobalFrameCount()
{
	return globalFrameCount;
}

void IncGlobalFrameCount()
{
	++globalFrameCount;
}
