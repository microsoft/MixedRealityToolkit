// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#ifndef __PLAYSPACE_OLDSCAN_H__
#define __PLAYSPACE_OLDSCAN_H__



class PlaySpace_SurfaceInfos
{
public:
	typedef enum {
		PSI_HAVEINFOS = 0x1,
		PSI_GOODINFOS = 0x2,
		PSI_WALL = 0x4,
		PSI_NO_OCCLUSION = 0x8,
		PSI_WALL_FRONT = 0x10,
		PSI_WALL_BACK = 0x20,
		PSI_WALL_LEFT = 0x40,
		PSI_WALL_RIGHT = 0x80,
		PSI_BORDER = 0x100,
		PSI_NOT_A_BASIN = 0x200,
		PSI_EXTWALL = 0x400,

		PSI_TEMP = 0x1000,
	} Flags;

	class Infos
	{
	public:
		Float	SumCurHeight;
		Float	SumNewHeight;
		S16		NbCurHeight;
		S16		NbNewCurHeight;

		Float	Height;
		Float	hSnap;
		S32		hInt;
		S16		Flags;
		S16		ZoneId;

		void	AddAccumulatedHeight(Float _Quality,Float _Height);
	};
	Float	x,y;
	Infos	Level[2];

	FINLINE_Z void Reset()
	{
		x = 0.f;
		y = 0.f;
		for (S32 i=0 ; i<2 ; i++)
		{
			Level[i].SumCurHeight = 0.f;
			Level[i].SumNewHeight = 0.f;
			Level[i].NbCurHeight = 0;
			Level[i].NbNewCurHeight = 0;

			Level[i].Height = 0.f;
			Level[i].hSnap = 0.f;
			Level[i].hInt = 0;
			Level[i].Flags = 0;
			Level[i].ZoneId = -1;
		}
	}
	FINLINE_Z void ClearWallFlags()
	{
		Level[0].Flags &= 0xFFFF ^ (PSI_WALL | PSI_EXTWALL | PSI_WALL_FRONT | PSI_WALL_BACK | PSI_WALL_LEFT | PSI_WALL_RIGHT);
		Level[1].Flags &= 0xFFFF ^ (PSI_WALL | PSI_EXTWALL | PSI_WALL_FRONT | PSI_WALL_BACK | PSI_WALL_LEFT | PSI_WALL_RIGHT);
	}
	static FINLINE_Z void	SetAsWall(PlaySpace_SurfaceInfos *_pCell,S32 _Nb,S32 _Delta,S16 _Flags)
	{
		if (_Nb <= 0)
			return;
		while (_Nb)
		{
			_pCell->Level[0].Flags |= _Flags;
			_pCell->Level[1].Flags |= _Flags;
			_Nb--;
			_pCell+=_Delta;
		}
	}
};

#endif //__PLAYSPACE_OLDSCAN_H__
