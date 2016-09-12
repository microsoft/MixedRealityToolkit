// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#ifndef _TOPOLOGYANALYZER_H_
#define _TOPOLOGYANALYZER_H_

class PlaySpaceInfos_W;
class PlaySpace_SurfaceInfos;
class PlaySpace_Surfel;

#define		CELL_NO_DIST	-1

#define		TOPOLOGY_REQUEST_OUTSIDE	(1<<0)
#define		TOPOLOGY_REQUEST_WALL		(1<<1)
#define		TOPOLOGY_REQUEST_VOID		(1<<2)
#define		TOPOLOGY_REQUEST_VALID		(1<<3)
#define		TOPOLOGY_REQUEST_SPACE		(1<<4)
#define		TOPOLOGY_REQUEST_ONLYONE	(1<<5)

// Trying to harmonize & debug sitable pos: Too narrow (characters in walls => 0.5m)
#define	MAX_SR_SIT_WIDTH		0.4f
#define	MAX_SR_SITHIGH_WIDTH	0.4f
#define	MAX_SR_SITDESK_WIDTH	0.4f

#define	MAX_SR_SIT_DEPTH		0.4f
#define	MAX_SR_SITHIGH_DEPTH	0.3f
#define	MAX_SR_SITDESK_DEPTH	0.15f

// anick mail lundi 29 septembre 2014 15:29
#define MIN_SR_SIT_HEIGHT 0.30f//old value : 0.30f // wanted value : 0.35f
#define MAX_SR_SIT_HEIGHT 0.65f//old value : 0.65f// wanted value : 0.55f
#define MIN_SR_SITHIGH_HEIGHT 0.65f// old value : 0.65f // wanted value : 0.55f
#define MAX_SR_SITHIGH_HEIGHT 1.2f// old value : 1.2f //wanted value : 0.75f
#define MIN_SR_DESK_HEIGHT 0.7f// old value : 0.7f //wanted value :  0.75f
#define MAX_SR_DESK_HEIGHT 1.1f // old value : 1.1f// wanted value : 1.f

struct TopologyRayCastResult_W
{
	enum Type
	{
		SURFACE, WALL
	};

	Vec3f m_vPos;
	Vec3f m_vNormal;
	Type m_type;

	// Surface case
	S32 m_iSufaceIdx;
	S32 m_iCellIdx;

	// Wall case
	S32 m_iWallIdx;
	S32 m_iColumnIdx;
};

typedef DynArray_Z<PlaySpace_Surfel*, 16, FALSE, FALSE>	PlaySpace_SurfelPtr_DA;

class TopologyAnalyzer_W
{
public:

	struct CellInfo
	{
		U32	m_iDistFromBorder;
		U32	m_iDistFromVoid;
		U32 m_iDistFromFloor;
		U32	m_iDistFromWall;
		U32 m_iSpaceToUp;

		CellInfo()
		{
			m_iDistFromBorder = CELL_NO_DIST;
			m_iDistFromVoid = CELL_NO_DIST;
			m_iDistFromFloor = CELL_NO_DIST;
			m_iDistFromWall = CELL_NO_DIST;
			m_iSpaceToUp = 1000;
		}

		Bool IsValid() const
		{
			return m_iDistFromBorder != CELL_NO_DIST && m_iDistFromBorder != 0;
		}
	};

	struct Surface
	{
		S32		m_iZoneId;
		Float	m_fHeightFromGround;
		Float	m_fWorldHeight;
		Float	m_fMinSpace;
		Vec3f	m_vMinPos;
		Vec3f	m_vMaxPos;
		Vec2i	m_vMinPos2D;
		Vec2i	m_vMaxPos2D;
		Float	m_fArea;
		Bool	m_bIsGround;
		Bool	m_bIsCeiling;
		S32		m_iNbCell;
		DynArray_Z<CellInfo>	m_daCells;
		DynArray_Z<Surface*> m_daNeighbors;

		Surface()
		{
			m_iZoneId = -1;
			m_fWorldHeight = 0.f;
			m_fHeightFromGround = 0.f;
			m_fMinSpace = 1e7f;
			m_vMinPos = m_vMaxPos = VEC3F_NULL;
			m_vMinPos2D = m_vMaxPos2D = Vec2i(0,0);
			m_fArea = 0.f;
			m_bIsGround= FALSE;
			m_iNbCell = 0;
		}
	};
	DynArray_Z<Surface>		m_daSurfaces;
	DynArray_Z<DynArray_Z<const Surface*> > m_daSurfaceGroups;

	struct WallColumnInfo
	{
		Float	m_fMin;
		Float	m_fMax;
		S32		m_iSpaceBottomSize;
		S32		m_iSurfaceIndex;
		S32DA	m_daSpaceInFrontOf;
		Float	m_fPartHeight;
		Bool	m_bIsValid;
		PlaySpace_SurfelPtr_DA	m_daSurfels;

		WallColumnInfo()
		{
			m_fMin = -Float_Max;
			m_fMax = Float_Max;
			m_iSpaceBottomSize = 0;
			m_iSurfaceIndex = -1;
			m_bIsValid = TRUE;
			m_fPartHeight = 0.f;
		}
	};

	struct Wall
	{
		Vec3f	m_vNormal;
		Vec3f	m_vTangent;
		Vec3f	m_vUp;
		Vec3f	m_vBaseStart;
		Vec3f	m_vBaseEnd;
		Vec3f	m_vCentroid;
		Float	m_fNbSurfel;
		Float	m_fHeight;
		Float	m_fWidth;
		Float	m_fArea;

		Float	m_fDistWall_DeprecatedV3;
		DynArray_Z<WallColumnInfo>	m_daColumns;
		PlaySpace_SurfelPtr_DA		m_daSurfels;
		DynArray_Z<Vec3f>			m_daPoints;
		Bool	m_bIsVirtual;
		Bool	m_bIsExternal;
		S32		m_ZoneID;

		Bool IsXWall() const
		{
			return !IsZWall();
		}

		Bool IsZWall() const
		{
			return (fabsf(m_vNormal.x) > 0.707f);
		}

		Wall()
		{
			m_vTangent = m_vNormal = VEC3F_NULL;
			m_vBaseStart = m_vBaseEnd = VEC3F_NULL;
			m_vCentroid = VEC3F_NULL;
			m_fHeight = m_fWidth = 0.f;
			m_fDistWall_DeprecatedV3 = 0.f;
			m_bIsVirtual = FALSE;
			m_bIsExternal = TRUE;
			m_ZoneID = -1;
			m_fNbSurfel = 0.f;
			m_fArea = 0.f;
		}
	};
	DynArray_Z<Wall>		m_daWalls;

	Float		m_fSizeVoxel;
	Float		m_YGround;
	Float		m_YCeiling;
    Vec2i       m_vMinPos2D;
    Vec2i       m_vMaxPos2D;
    Vec3f       m_vMinPos;
	Vec3f		m_vPlaySpaceCenter;
	Vec3f		m_vRoomCenter;
	Vec3f		m_vPlayerCenter,m_vPlayerDirection;

	Bool		m_IsAnalysed;

public:
	TopologyAnalyzer_W();
	~TopologyAnalyzer_W();

	Bool	IsAnalysed() const {return m_IsAnalysed;}
	void	Analyze(const PlaySpaceInfos_W* _pPlayspaceInfo,Bool _UseJob);
	void	Reset();

	void	GetAllPosOnWall(Float _heightMin, Float _heightSizeMin, Float _widthMin, Float _fDepth, Vec3fDA& _outPos, Vec3fDA& _outNormal, Bool _bAllowVirtualWall = TRUE);
	void	GetAllPosOnWallNearFloor(Float _heightMin, Float _widthMin, Float _floorDist, Float _fDepth, Vec3fDA& _outPos, Vec3fDA& _outNormal) const;
    void	GetAllPosOnWallNearCeiling(Float _ceilingDist, Float _widthMin, Float _fDepth, Vec3fDA& _outPos, Vec3fDA& _outNormal, FloatDA& _outWidth) const;
    void	GetAllLargePosOnWall(Float _heightMin, Float _heightSizeMin, Float _widthMin, Float _fDepth, Vec3fDA& _outPos, Vec3fDA& _outNormal, FloatDA& _outWidth, Bool _bAllowVirtualWall = TRUE) const;
	const Wall*	GetLargestWall(Bool _bAllowVirtualWall = TRUE, Bool _bForceExternalWall =FALSE) const;

	void	GetAllPosOnFloor(Float _minSize, Vec3fDA& _outPos, Float _minHeight = 0.f) const;
    void    GetAllRectanglePosOnFloor(Float m_inWidth, Float _minLength, Vec3fDA& _outPos, Vec3fDA& _outLengthDirection) const;
    void	GetLargestPosOnFloor(Vec3fDA& _outPos) const;

	void	GetAllPosSittable(Float _minHeight, Float _maxHeight, Float _depth, Vec3fDA& _outPos, Vec3fDA& _outNormal) const;

    void    GetAllLargePosSittable(Float _minHeight, Float _maxHeight, Float _depth, Float _widthMin, Vec3fDA& _outPos, Vec3fDA& _outNormal) const;
	void    GetAllLargePosSittableOnSurface(const Surface& _surfaces, const Vec3fDA& _daPossibleNormals, Float _minHeight, Float _maxHeight, Float _depth, Float _widthMin, Vec3fDA& _outPos, Vec3fDA& _outNormal) const;

    void    GetAllCouch(Float _fSitHeightMin, Float _fSitHeightMax, Float _fBackHeightMin, Float _fBackHeightMax, Float _fSitAreaMin, Vec3fDA& _outPos, Vec3fDA& _outNormal, FloatDA& _outLength, FloatDA& _outWidth, Bool _displayDebug = FALSE) const;

    void    GetAllPosOnSurface(const Surface& _surface, Float _fminSize, Vec3fDA& _outPos) const;
    void    GetAllPosOnSurfaceInSphere(const Vec3f& _sphereCenter, Float _sphereRadius, Float _posRadius, Vec3fDA& _outPos) const;

	Float	GetYCeiling() const { return m_YCeiling; }
    Float   GetYGround() const {return m_YGround;}

	Bool	IsFullWall(const Wall& _wall) const;

	inline Bool	HasSurfaces()const {return m_daSurfaces.GetSize()>0;}

	Bool	RectangleIsInPlaySpace(const PlaySpaceInfos_W* _pPlaySpaceInfos, const Vec2f& _vPos, const Vec2f& _vSize, const Quat& _qRot) const;
    Float   GetMaxYInRectangle(const PlaySpaceInfos_W* _pPlayspaceInfos, const Vec2f& _vPos, const Vec2f& _vSize, const Quat& _rot, Float _fMinY, Float _fMaxY) const;
    
	Bool    HasWallNear(const Vec3f& _vPos, Float _radius, Float _wallHeightMin) const;
	Float	GetDistWallNear(const Vec3f& _vPos, Float _wallHeightMin, Bool _virtual = FALSE) const;
	Float	GetDistNearestWall(const Vec3f& _vPos, Float _wallHeightMin) const;
	Float	GetDistNearestBorderOnGround(const Vec3f& _vPos, S32* _iNbSameDistNeighbors = NULL) const;

    Bool    FindAlignedRectangles(const FloatDA& _widths, const FloatDA& _lengths, Vec3fDA& _outPos, Vec3f& _outDir) const;
    Bool    FindRectanglesSequence(const FloatDA& _widths, const FloatDA& _lengths, Vec3fDA& _outPos, Vec3fDA& _outDirs) const;

    enum DebugFlags : U64
    {
        TA_DISPLAY_SURFACE = 1,
        TA_DISPLAY_SURFACE_INDICE =				((U64)1 << 1),
        TA_DISPLAY_DIST_FROM_WALL =				((U64)1 << 2),
        TA_DISPLAY_DIST_FROM_BORDER =			((U64)1 << 3),
        TA_DISPLAY_DIST_FROM_FLOOR =			((U64)1 << 4),
        TA_DISPLAY_DIST_FROM_VOID =				((U64)1 << 5),
        TA_DISPLAY_FLOOR_DIST_FROM_WALL =		((U64)1 << 6),
        TA_DISPLAY_FLOOR_DIST_FROM_BORDER =		((U64)1 << 7),
        TA_DISPLAY_FLOOR_DIST_FROM_FLOOR =		((U64)1 << 8),
        TA_DISPLAY_FLOOR_DIST_FROM_VOID =		((U64)1 << 9),
        TA_DISPLAY_CEILING_DIST_FROM_WALL =		((U64)1 << 10),
        TA_DISPLAY_CEILING_DIST_FROM_BORDER =	((U64)1 << 11),
        TA_DISPLAY_CEILING_DIST_FROM_FLOOR =	((U64)1 << 12),
        TA_DISPLAY_CEILING_DIST_FROM_VOID =		((U64)1 << 13),
        TA_DISPLAY_WALL =						((U64)1 << 14),
        TA_DISPLAY_WALL_NORMAL =				((U64)1 << 15),
        TA_DISPLAY_WALL_COLUMNS =				((U64)1 << 16),
        TA_DISPLAY_SURFACE_GROUP =				((U64)1 << 17),
        TA_DISPLAY_COUCH =						((U64)1 << 18),
        TA_DISPLAY_COUCH_SURFACES =				((U64)1 << 19),
        TA_DISPLAY_TABLE =						((U64)1 << 20),
        TA_DISPLAY_SITTABLE =					((U64)1 << 21),
        TA_DISPLAY_SITTABLE_NORMAL =			((U64)1 << 22),
        TA_DISPLAY_LARGE_SITTABLE =				((U64)1 << 23),
        TA_DISPLAY_LARGE_SITTABLE_NORMAL =		((U64)1 << 24),
        TA_DISPLAY_POS_ON_FLOOR =				((U64)1 << 25),
        TA_DISPLAY_POS_IN_SPHERE =				((U64)1 << 26),
        TA_DISPLAY_ALIGNED_RECTANGLES =			((U64)1 << 27),
        TA_DISPLAY_RECTANGLES_SEQUENCE =		((U64)1 << 28),
		TA_DISPLAY_ROOMCENTER =					((U64)1 << 29),
		TA_DISPLAY_SPACE_TO_UP =				((U64)1 << 30),
		TA_DISPLAY_FLOOR_CELL_INDEX =			((U64)1 << 31),
		TA_DISPLAY_TOPOLOGY_RAY_CASE =			((U64)1 << 32)
    };

    static Bool GroupsIsAwayFromWalls(const DynArray_Z<const Surface*>& _group);

protected:
	void	SetupSurface(const PlaySpaceInfos_W* _pPlayspaceInfos);
	void	SetupNeighbors(const PlaySpaceInfos_W* _pPlayspaceInfos);
	void	SetupNeighborsV3(const PlaySpaceInfos_W* _pPlayspaceInfos);
	void    SetupSurfaceGroups(const PlaySpaceInfos_W* _pPlayspaceInfos);
	void	SetupWall(const PlaySpaceInfos_W* _pPlayspaceInfos);
	void	SetupCeiling(const PlaySpaceInfos_W* _pPlayspaceInfos);

	void	ComputeCellsInfo(const PlaySpaceInfos_W* _pPlayspaceInfos);
	Wall*	GetWall(const PlaySpaceInfos_W* _pPlayspaceInfos, const Vec2f& _pos, const Vec3f& _normal, Float _height, Bool _isVirtual, Bool _isExternal);

	S32		ComputeSpaceBottomSize(const PlaySpace_SurfaceInfos	* _pFrom, S32 _delta);
	WallColumnInfo&	AddWallColumn(const PlaySpaceInfos_W* _pPlayspaceInfos, const Vec2f& _2dPos, Float _height, Float _yMin, Float _yMax, const Vec3f& _normal, Bool _isVirtual, Bool _isExternal);

	// Scan V3

	U32		ComputeCellDistWall_V3(const PlaySpaceInfos_W* _pPlayspaceInfos, const Surface& _surface, const Vec2i& _curPos, const Vec2i& _delta);
	U32		ComputeCellDistVoid_V3(const PlaySpaceInfos_W* _pPlayspaceInfos, const Surface& _surface, const Vec2i& _curPos, const Vec2i& _delta);
	U32		ComputeCellDistFloor_V3(const PlaySpaceInfos_W* _pPlayspaceInfos, const Surface& _surface, const Vec2i& _curPos, const Vec2i& _delta);
	Bool	IsWall_V3(const PlaySpaceInfos_W* _pPlayspaceInfos, Float _currentHeight, const Vec2i& _pos) const;
	Bool	IsVoid_V3(const PlaySpaceInfos_W* _pPlayspaceInfos, Float _currentHeight, const Vec2i& _pos);
	Bool	IsFloor_V3(const PlaySpaceInfos_W* _pPlayspaceInfos, Float _currentHeight, const Vec2i& _pos);
	S32		ComputeSpaceBottomSize_V3(const PlaySpaceInfos_W* _pPlayspaceInfos, S32 x, S32 y, Float height, S32 dx, S32 dy);
	S32		ComputeSpaceInFrontOf_V3(const PlaySpaceInfos_W* _pPlayspaceInfos, S32 x, S32 y, Wall& wall, WallColumnInfo& column, S32 dx, S32 dy);

	//Analyze
	void	OneFrameAnalyze(const PlaySpaceInfos_W* _pPlayspaceInfo);

	static S32 AnalyzeTask_NumCount;
	static const PlaySpaceInfos_W* AnalyzeTask_PlayspaceInfo;
	static void	AnalyzeJobTask(const U16 taskIndex, const U16 tasksCount, void* userData);

	JobGroup_Z	m_AnalyzeJob;

public :
	// General utils
	Bool	RayCastOnFace(const Vec3f& _vPos, const Vec3f& _vDir, const SafeArray_Z<Vec3f, 4>& _saFace, const Vec3f& _vNormal, Vec3f& _outResult) const;
	Bool	TopologyRayCast(const Vec3f& _vPos, const Vec3f& _vDir, TopologyRayCastResult_W& _outResult) const;

	void	GetNearestPointOnFace(const Vec3f& _vRefPoint, const SafeArray_Z<Vec3f, 4>& _saFace, Vec3f& _vOutNearestPoint) const;
	Float	GetDistPointVsFace(const Vec3f& _vRefPoint, const SafeArray_Z<Vec3f, 4>& _saFace, Vec3f* _vOutNearestPoint = NULL) const;

	void	GetNearestPointOnSurface(const Vec3f& _vRefPoint, const Surface& _surface, Vec3f& _vOutNearestPoint) const;
	Float	GetDistPointVsSurface(const Vec3f& _vRefPoint, const Surface& _surface, Vec3f* _vOutNearestPoint = NULL) const;

	void	GetNearestPointOnWall(const Vec3f& _vRefPoint, const Wall& _wall, Vec3f& _vOutNearestPoint) const;
	Float	GetDistPointVsWall(const Vec3f& _vRefPoint, const Wall& _wall, Vec3f* _vOutNearestPoint = NULL) const;
	//

    Vec3f   GetCellPosition(const Surface& _surface, S32 _iCell) const;
    void    GetAllRectanglePos(Float _minWidth, Float _minLength, const Surface& _surface, Vec3fDA& _outPos, Vec3fDA& _outLengthDirection) const;
	const Surface*	GetSurfaceFromPos(const Vec3f& _pos) const;
	const Surface*	GetHigherSurfaceInBox(const Vec3f& _pos, const Vec3f& _size) const;
	S32		GetSurfaceIndexFromZoneId(S32 _iZoneId) const;

    void    GetGroupsAwayFromWalls(Float _surfaceMinHeight, Float _surfaceMaxHeight, S32DA& _outGroups) const;

	Bool	CellIsVisibleFromPlayspaceCenter(const PlaySpaceInfos_W* _pPlayspaceInfos,const Vec3f& _pos) const;
	
	static Bool				SpaceOnWallIsFree(S32 _firstColumn, S32 _lastColumn, Float _bottomY, Float _topY, S32 _iDepth, Bool _bAllowPartially, const Wall& _wall);
		   Bool				SpaceOnWallIsFreeFloat(Float _fLeftOffset, Float _fRightOffset, Float _fBottomY, Float _fTopY, S32 _iDepth, Bool _bAllowPartially, const Wall& _wall) const;

    static const CellInfo*  GetCellInSurface(const Vec3f& _globalPos, const Surface& _surface, Float _fSizeVoxel);
	static const CellInfo*  GetCellInSurface(const Vec2i& _globalPos, const Surface& _surface);
	
	static void		        AddAllNeighborsRec(DynArray_Z<const Surface*>& group, const Surface* _surface);
	static void		        GetCellNeighbors(const Surface& _surface, S32 _c, DynArray_Z<Vec2i>& _outNeighbors);
	static Vec3f	        GetDominantDirection(const Vec3fDA& _rectangle, Float& _outLength);
    static Float            GetSurfaceArea(const Surface& _surface, Float _fSizeVoxel);

	static Bool				CellIsOk(const Vec3f& _pos, const Surface& _surface,Float _fSizeVoxel, U32 _request, U32 _limit = 0);
	static Bool				LineIsOk(const Vec3f& _first, const Vec3f& _last, const Surface& _surface, Float _fSizeVoxel, U32 _request, U32 _limit = 0);
	static Bool				RectangleIsOk(const Vec3f& _topLeft, const Vec3f& _topRight, const Vec3f& _bottomLeft, const Vec3f& _bottomRight, const Surface& _surface, Float _fSizeVoxel, U32 _request, U32 _limit = 0);

		   Bool				RectangleIsOkOnFloor(const Vec3f& _topLeft, const Vec3f& _topRight, const Vec3f& _bottomLeft, const Vec3f& _bottomRight, U32 _request, U32 _limit = 0) const;
};

#endif