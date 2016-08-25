// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#ifndef __PLAYSPACE_MESH_H__
#define __PLAYSPACE_MESH_H__

class PlaySpace_CubeSurface;

typedef	SafeArray_Z<U32, 4>	ColoredCellSA;

class  PlaySpaceMesh_Stats
{
public:
	S32		FrameStamp;

	Float	HorizSurface;
	Float	TotalSurface;
	Float	UpSurface;
	Float	DownSurface;
	Float	WallSurface;
	Float	VirtualCeilingSurface;
	Float	VirtualWallSurface;

	// 0 : floor
	// 1 : ceilling
	// 2 : wall x neg
	// 3 : wall x pos
	// 4 : wall z neg
	// 5 : wall z pos
	// 5 : platform
	FloatDA	Areas[7];

	// 0 : Nb cells with IsPaintMode == 1
	// 1 : Nb cells with IsSeenQuality == 1
	// 2 : Nb cells with IsSeenQuality == 2
	// 3 : Nb cells with IsSeenQuality == 3
	ColoredCellSA ColoredCells;

	void Reset()
	{
		FrameStamp = -1;

		HorizSurface = 0.f;
		TotalSurface = 0.f;
		UpSurface = 0.f;
		DownSurface = 0.f;
		WallSurface = 0.f;
		VirtualCeilingSurface = 0.f;
		VirtualWallSurface = 0.f;

		for(S32 dir=0; dir<_countof(Areas); dir++)
			Areas[dir].Empty();

		ColoredCells.Null();
	}

	void operator=(const PlaySpaceMesh_Stats& _other)
	{
		FrameStamp	 = _other.FrameStamp;
		HorizSurface = _other.HorizSurface;
		TotalSurface = _other.TotalSurface;
		UpSurface	 = _other.UpSurface;	
		DownSurface	 = _other.DownSurface;	
		WallSurface = _other.WallSurface;
		VirtualCeilingSurface = _other.VirtualCeilingSurface;
		VirtualWallSurface = _other.VirtualWallSurface;

		for(S32 dir=0; dir<_countof(Areas); dir++)
			Areas[dir] = _other.Areas[dir];

		ColoredCells = _other.ColoredCells;
	}
};

class Playspace_Mesh
{
protected:
	class RefPoint
	{
	public:
		S32			NumPoint;
		RefPoint	*pNext;
	};

	// Private Datas.
	Bool		m_IsInEditMode;
	Bool		m_HavePointLinks;
	Bool		m_HavePointsToolNormal;
	Bool		m_HaveFaceToolNormal;

	Playspace_Mesh::RefPoint	*m_pFreeTabPoint;
	BnkDynArray_Z<Playspace_Mesh::RefPoint, 8192, FALSE, FALSE,_ALLOCDEFAULTALIGN,1024>	m_TabRefPoint;
	DynArray_Z<Playspace_Mesh::RefPoint*, 8, FALSE, FALSE>								m_QuadTreePt;

	// Functions.
	S32			GetQuadTreePoint(Vec3f &Point);

	Playspace_Mesh::RefPoint	*NewRefPoint()
	{
		Playspace_Mesh::RefPoint *pResult = m_pFreeTabPoint;
		if (pResult)
			m_pFreeTabPoint = pResult->pNext;
		else
			pResult = &(m_TabRefPoint.Add());
		return pResult;
	}
	void		RemoveQuadTreePoint(S32 Num, Vec3f &Point);
	void		InsertQuadTreePoint(S32 Num, Vec3f &Point);
	void		RenumQuadTreePoint(S32 Src,S32 Dst, Vec3f &Point);
	S32			GetPointInZoneQuadTreePoint(Vec3f &_Min,Vec3f &_Max,S32DA &_Result);
	S32			NbFacesOnEdge(S32 _p0, S32 _p1);
	void		ColapseEdge(S32 _p0, S32 _p1, bool atMiddle = TRUE);

	void		RemoveFace(S32 _f0,Bool _RemoveVtxIfNeeded);
	void		RemoveVertex(S32 _p0);

	// ProceduralSphere
	void ProceduralSphere_InitGeneration(S32 depth, Bool _faceNormalOrientedOutside);
	void ProceduralSphere_Subdivide(Vec3f &v1, Vec3f & v2, Vec3f & v3, S32 depth, Bool _faceNormalOrientedOutside);

public:
	class FlagFace
	{
	public:
		U8		IsTri : 1, IsVirtual : 1, IsMarked : 1,IsExternal : 1, IsPaintMode : 2,IsSeenQuality : 2;
	};
	class Face
	{
	public:
		// IsPaintMode : 0 = NoMode | 1 = UnPaint | 2 = Paint
		U8		IsTri : 1, IsVirtual : 1, IsMarked : 1,IsExternal : 1, IsPaintMode : 2,IsSeenQuality : 2;
		S32		TabPoints[4];
	};
	class PointsLinks
	{
	protected:
		static const S32	NbStaticFaces = 8;
		S32		NbFaces;
		S32		NbReserve;
		S32		TabFacesS[NbStaticFaces];
		S32		*TabFacesD;
	public:
		PointsLinks()
		{
			NbReserve = 0;
			NbFaces = 0;
			TabFacesD = NULL;
		}
		PointsLinks(const PointsLinks&_other)
		{
			NbReserve = 0;
			TabFacesD = NULL;
			*this = _other;
		}
		~PointsLinks()
		{
			if (TabFacesD)
				Free_Z(TabFacesD);
			NbReserve = 0;
			NbFaces = 0;
		}
		FINLINE_Z PointsLinks&	operator = (const PointsLinks&_other);
		FINLINE_Z void	Init(S32 *_TabValue,S32 _nb);
		S32		CopyTo(S32 *_TabValue,S32 _nbMax);
		S32		MergeTo(S32 *_TabValue,S32 _nb,S32 _nbMax);

		FINLINE_Z void	Add(const PointsLinks&_other);
		FINLINE_Z void	SetSize(S32 _Size);
		FINLINE_Z S32	GetNbFaces(){return NbFaces;}
		FINLINE_Z S32	GetFace(S32 _Num)
		{
			if (_Num < NbStaticFaces)
				return TabFacesS[_Num];
			return TabFacesD[_Num-NbStaticFaces];
		}
		FINLINE_Z Bool	RemoveFace(S32 _num);
		FINLINE_Z void	AddFace(S32 _num);
	};
	class ToolPointNormal
	{
	public:
		Vec3f	Normal;
		Float	Error;
		Float	Surface;
	};
	class ToolFaceNormal
	{
	public:
		Vec3f	Normal;
		Vec3f	Center;
		Float	Surface;
	};

	struct vItem
	{
		S32		point;
		Float	crossEdge;
		Float	dot;
		Float	crossCenter;

		Vec3f	radius;
		Vec3f	edge;

		vItem*	prev;
		vItem*	next;
	};

	// Public Datas.
	HugeDynArray_Z<Vec3f, 4096, FALSE, FALSE>					m_TabPoints;
	HugeDynArray_Z<PointsLinks, 4096, TRUE, TRUE>				m_TabPointsLinks;	// Newx and Delete because of Mixed Array :( => Change that !!!
	HugeDynArray_Z<ToolPointNormal, 4096, FALSE, FALSE>			m_TabPointsToolNormal;
	HugeDynArray_Z<ToolFaceNormal, 4096, FALSE, FALSE>			m_TabFaceToolNormal;
	HugeDynArray_Z<Playspace_Mesh::Face, 4096, FALSE, FALSE>	m_TabQuad;			// Faces dans le sens de l'affichage => inverse collisions, inverse Microsoft.

	Playspace_Mesh();
	Playspace_Mesh(const Playspace_Mesh&_other);
	Playspace_Mesh& operator = (const Playspace_Mesh&_other);

	// Global.
	void	CopyFrom(const Playspace_Mesh&_other, Bool _keepMemory = FALSE, Bool _vertsAndIndicesOnly = FALSE);

	void	SetEditMode(Bool _IsSet,Bool _KeepPointNormal=FALSE,Bool _KeepFaceNormal=FALSE);
	Bool	IsInEditMode() { return m_IsInEditMode; }

	Bool	IsEmpty() { return (m_TabQuad.GetSize() == 0); }
	void	Swap(Playspace_Mesh &_Other);
	void	Flush(Bool _KeepMemory);

	Float	GetSnapDist();

	// Point Face.
	S32		AddPoint(Vec3f	&Point);
	void	MovePoint(S32 _Num, Vec3f &_Dst);

	void	AddSurface(Vec3f *_pPoints, S32 _NbPoints, Bool _IsVirtual,U8 _IsPaintMode = 1,U8 _IsSeenQuality = 3);
	void	AddQuad(Vec3f &_p1, Vec3f &_p2, Vec3f &_p3, Vec3f &_p4, Bool _IsVirtual);

	S32		NbSharedPoint(S32 _f1,S32 _f2);
	S32		GetCommonFacesOnPoints(S32 _p0, S32 _p1, S32 *TabResult,S32 _NbMax,S32 _IgnoreFaceId = -1);
	S32		GetNeighbourFaces(S32 face, S32* neighbours, Float* dotNormals, Float* areas, S32 _NbMax);
	Bool	IsPointMovable(S32 p, Vec3f& newPos);
	Bool	IsCollapsableEdgeOnTri(S32 p0, S32 p1, float cosAngleMin2, Bool checkExternal = FALSE);

	// Add or Fill.
	void	AddMesh(HU32DA &_TabTriIdx, HugeDynArray_Z<Vec3f> &_TabVtx, Bool IsClockWise);
	void	AddMesh(U32 *_pTabTriIdx, S32 _NbTriIdx, Vec3f *_pTabVtx, S32 _NbVtx, Bool IsClockWise, FlagFace *pFlagsFaces = NULL);
	void	AddMesh(Playspace_Mesh &_Mesh,Bool _OnlyMarkedFaces);
	void	RemoveMarkedFaces();

	void	ProceduralSphere(S32 _depth, float _radius, Vec3f _center, Bool _faceNormalOrientedOutside = TRUE);

	// Filter.
	void	PlanarFilter_AccurateNew();		// Points filter => pb de chanfrein.. mais plus efficace pour le moment
	void	RemoveDeadFaces(Float _DistMax);	// Re-snap point ans Remove dead faces.

	void	RemoveBadFaces();
	
	void	SimplifyTri(Bool _simplifyExternal = TRUE);
	void	SimplifyTri2(Float cosMaxErr, Bool _ignoreVirtual = TRUE, Bool _ignoreExternal = TRUE);

	void	CheckBadWinding();
	void	Triangularize(bool flipWindingOrder = false);
	void	FillHole();		// avait bossé sur une autre version.
	void	Inflate(Float _Dist);
	void	AddNoise(Float _Dist);

	void	RemoveIsolatedFaces(S32 _Size);
	void	ComputeExternalFaces(Float _yGround, Float _yCeilling);
	void	OldComputeExternalFaces(Float _yGround, Float _yCeilling);

	void	Subdivide(float maxLength);
	void	PerformConvexHull(Playspace_Mesh & _sphereMesh, Bool _bFlagNewFaceAsVirtual);

	void	ApplyQuat(Quat &_Transfo);

	// Mark Faces.
	void	MarkCone(Vec3f &_Pos,Vec3f &_Dir,Float _HalfAngleDeg,Float _DistMax,Bool _FaceInside,Bool _AtLeastOnePoint);

	// Tools.
	Float	ComputePointNormal(S32 _p0, Vec3f &_Normal, Float *_LocalArea = NULL);
	void	ComputePointsLinks(Bool _Force = FALSE);
	void	ComputePointsToolNormal(Bool _Force = FALSE);
	void	ComputeFacesToolNormal(Bool _Force = FALSE);
	void	ComputeFacesAndPointToolNormalForMarkedFaces();

	void	InvalidatePointsLinks(Bool _flush = FALSE) { m_HavePointLinks = FALSE; if( _flush) m_TabPointsLinks.Flush(); }
	void	InvalidatePointsToolNormal(Bool _flush = FALSE) { m_HavePointsToolNormal = FALSE; if( _flush) m_TabPointsToolNormal.Flush();}
	void	InvalidateFaceToolNormal(Bool _flush = FALSE) { m_HaveFaceToolNormal = FALSE; if( _flush) m_TabFaceToolNormal.Flush();}

	Bool	CheckValidity();

	void	ComputeStats(Float MinCos, Float YGround, Float YCeiling, Float ZHorizMax, const Vec3f& _minAvailable, const Vec3f& _maxAvailable, PlaySpaceMesh_Stats& _OutStats);

	// To be clean if necessary (Hole filter is prettu good).
	void	DetectFacesOnBoundaries();
	HugeDynArray_Z<Playspace_Mesh::Face, 1024, FALSE, FALSE>	m_TabEdgyQuad;
	void	DetectFacesOnEdgeClusters();
	void	DetectHolesBoundaries();
	void	DetectHole();
	void	DetectHole2();
	void	ExportMesh_OBJ(char* filename,Bool _ExportVirtual = TRUE);
	void	CheckDuplicateFaces();
	void	CheckNonManifoldFaces();
	DynArray_Z<HugeDynArray_Z<Playspace_Mesh::Face, 1024, FALSE, FALSE>>	m_TabHolesBoundaries;
	Bool	AreFacesConnected(Playspace_Mesh::Face F1, Playspace_Mesh::Face F2);

	S32		GetNextPoint(S32 center, S32 point);
	S32		GetPointNeighbours(S32 pointIdx, S32* neighbours, S32 maxNeighbours, Bool* pIsOnBorder = NULL);
	void	DecomposePolyedraToTriangles(S32* points, S32 nbPts, S32* output, S32& nbGenTri);		// JY: Buggé !!!
	Bool	DecomposePolyedra(vItem* points, S32 nbPts, Vec3f& normal, S32* output, S32& nbGenTri);
	bool	IsPolyConvex(S32 nbPoints, S32* points);

};

#endif //__PLAYSPACE_MESH_H__
