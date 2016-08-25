// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#include "targetver.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

// Warnings we don't want
#pragma warning (disable : 4800)
#pragma warning (disable : 4244)
#pragma warning (disable : 4305)
#pragma warning (disable : 4316)
#pragma warning (disable : 4018)
#pragma warning (disable : 4101)

// Defines (config)...
#define _PC
#define	_ALLOCDEFAULTALIGN	16
#define _USE_SURFACE_RECO

// allow custom warnings to mark code to later fix
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

//#define COMPILE_WARNING 1
#define WARNING( txt ) __FILE__"(" TOSTRING(__LINE__) ") : warning: " txt
#define Delete_Z delete
#define New_Z new

#include <Types_Z.h>
#include <Assert_Z.h>
#include <Memory_Z.h>
#include <String_Z.h>

#include <Math_Z.h>
#include <Vec2f_Z.h>
#include <Vec3f_Z.h>
#include <Vec4f_Z.h>
#include <Mat3x3_Z.h>
#include <Mat4x4_Z.h>
#include <ByteColor_Z.h>

#include <BnkDynArray_Z.h>
#include <HashTable_Z.h>

#include <HashTable_Z.h>
#include <TaskScheduler_Z.h>

#include <collisiontool_z.h>

// To BE Developed
#define PROFILER_SCOPED_CPU_MARKER_L1(...)
#define PROFILER_SCOPED_CPU_MARKER(...)
#define DRAW_DEBUG_FACE(...)
#define DRAW_DEBUG_STRING3D_DURATION(...)
#define DRAW_DEBUG_LINE3D(...)
#define DRAW_DEBUG_LINE3D_FAST(...)
#define DRAW_DEBUG_STRING3D_SIZE_COLOR(...)
#define DRAW_DEBUG_ARROW(...)
#define DRAW_DEBUG_ARC3D(...)
#define DRAW_DEBUG_SPHERE3D(...)
#define DRAW_DEBUG_SPHERE3D_FAST(...)
#define DRAW_DEBUG_SPHERE3D_IF(...)
#define DRAW_DEBUG_DISK(...)
#define DRAW_DEBUG_STRING3D_FAST(...)
#define DRAW_DEBUG_STRING3D_FAST_IF(...)
#define DRAW_DEBUG_CROSS_FAST(...)
#define DRAW_DEBUG_STRING2D(...)
#define MESSAGE_Z //printf
#define OUTPUT_Z //printf
#define MAX_SEAD_ZONE_NODE_COLLIDE_NB		1024

class Util_L
{
protected:
	static Vec3f	CameraPos;
	static Vec3f	CameraDir;
	static Vec3f	CameraDirUp;

public:
	static void ApplyTranfoToCamera(Quat &_Transfo);
	static void SetCurrentCamera(const Vec3f &_Pos, const  Vec3f &_Dir, const  Vec3f &_DirUp);
	static Bool	GetViewSegment(Segment_Z *_out_pSeg, Vec3f *_pUpView = NULL);
	static Bool	GetAllViewData(Vec3f& _vPos, Vec3f& _vFront, Vec3f& _vRight, Vec3f& _vUp);

	static Quat	GetOrientationQuat(const Vec3f &_nodePos, const Vec3f &_viewPos, const Vec3f &_baseUp, S32 _order[3], Float *_pDist);
	static Quat	GetOrientationQuat(const Vec3f &_front, const Vec3f &_baseUp, S32 _order[3]);

	static Mat4x4 Convert_XMAT4x4_to_Mat4x4(DirectX::XMFLOAT4X4 src)
	{
		Mat4x4 dst;
		dst.GetRow(0).Set(src._11, src._12, src._13, src._14);
		dst.GetRow(1).Set(src._21, src._22, src._23, src._24);
		dst.GetRow(2).Set(src._31, src._32, src._33, src._34);
		dst.GetRow(3).Set(src._41, src._42, src._43, src._44);
		return dst;
	}
	static void Convert_Vec3f_to_XMFLOAT3(const Vec3f& src, DirectX::XMFLOAT3& dst)
	{
		dst.x = src.x;
		dst.y = src.y;
		dst.z = src.z;
	}
	static void Transform_Vec3f_to_XMFLOAT3(const Vec3f& src, DirectX::XMFLOAT3& dst, Mat4x4& mat)
	{
		Vec3f tmp = VecFloat4x4Transform3(mat, Vec4f(src));
		dst.x = tmp.x;
		dst.y = tmp.y;
		dst.z = tmp.z;
	}
	static Vec3f Convert_XMFLOAT3_to_Vec3f(const DirectX::XMFLOAT3& src)
	{
		return Vec3f(src.x, src.y, src.z);
	}
	static Vec3f Transform_XMFLOAT3_to_Vec3f(const DirectX::XMFLOAT3& _src, Mat4x4& mat)
	{
		Vec3f src = Vec3f(_src.x, _src.y, _src.z);
		return VecFloat4x4Transform3(mat, Vec4f(src));
	}

	// Strings
	static Bool						GetAttributeAfterColon(const char *_txtFull, const char *_txtSearched, const char **_pOutParamText = NULL);
};


// This is specific to SR	=> It is a reference to a SR-Cube
enum FRigTrackingState_Z
{
	FRIG_TRACKING_FAILED = 0,	// FRig not initialized or no location located (even attached...)
	FRIG_TRACKING_OK = 1,	// OK
	FRIG_TRACKING_ONLY_ROTATION = 2,	// Anchor not located, stationary not located, attached located -> returning attached
	FRIG_TRACKING_ANCHOR_NOT_LOCATED = 3,	// Anchor not located, stationary located -> returning stationary
};

#ifdef _USE_SPATIALMESH_IDX_16BITS
typedef U16											SpatialMeshIdxFormat;
#else
typedef U32											SpatialMeshIdxFormat;
#endif

#define FRIG_SURFACE_HANDLE_NULL	0

U32		GetGlobalFrameCount();
void	IncGlobalFrameCount();

// Function for scene
inline void	ApplyAlignToAllScene(Quat &_Transfo) {
	Util_L::ApplyTranfoToCamera(_Transfo);
}

// File helper functions
template<S32 SIZE> void File_Write_String(FILE* f, String_Z<SIZE>& str)
{
	const char* data = (const char*)str.GetPtr();
	int size = str.StrLen();
	fwrite(data, 1, size, f);
}
