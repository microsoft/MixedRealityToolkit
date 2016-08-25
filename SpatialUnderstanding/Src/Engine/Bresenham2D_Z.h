// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#ifndef _BRESENHAM2D_Z_H
#define _BRESENHAM2D_Z_H

class Bresenham2DInt_Z
{
private:
	Bool	IsNegDX;
	Bool	IsNegDY;
	Bool	dYBiggerdX;

	U32		NbPointOnStep;
	U32		NbStep;

	S32		CurX;
	S32		CurY;

	S32		DeltaX;
	S32		DeltaY;

	S32		DeltaStepX;
	S32		DeltaStepY;

	Vec2f	Start;
	Vec2f	End;
	Vec2f	Dir;
	Vec2f	Step;

	S32		PreviousStepX;

	Float	ProgressRatio;

public:
	FINLINE_Z void InitSead(Float _StartX,Float _StartY,Float _EndX,Float _EndY)
	{
		Start.x = _StartX;
		Start.y = _StartY;

		End.x = _EndX;
		End.y = _EndY;

		Dir = End-Start;

		// Transform to Simple case.
		IsNegDX = FALSE;
		if (Dir.x < 0.f)
		{
			IsNegDX = TRUE;
			// Delta neg... Invert.
			Start.x = -Start.x;
			End.x = -End.x;
			Dir.x = -Dir.x;
		}

		IsNegDY = FALSE;
		if (Dir.y < 0.f)
		{
			IsNegDY = TRUE;
			// Delta neg... Invert.
			Start.y = -Start.y;
			End.y = -End.y;
			Dir.y = -Dir.y;
		}

		dYBiggerdX = FALSE;
		if (Dir.y > Dir.x)
		{
			// Delta Y have to be the little one
			dYBiggerdX = TRUE;
			
			register Float val;
			val = Start.x;Start.x = Start.y;Start.y = val;
			val = End.x;End.x = End.y;End.y = val;
			val = Dir.x;Dir.x = Dir.y;Dir.y = val;
		}

		// The General Case Now !
		CurX = FLOORINT(Start.x);
		CurY = FLOORINT(Start.y);

		DeltaX = 1;
		DeltaY = 0;

		DeltaStepX = -1;		// For Undo the Last increase in GetNextPointSead 
		DeltaStepY = 1;

		// Compute Nb Step.
		NbStep = FLOORINT(End.y) - CurY;
		if (NbStep)
		{
			Float LimitY = FLOORF(Start.y + 1.f);	// DON'T REPLACE BY CEILFF !!! Not Same Result
			LimitY = Min<Float>(End.y,LimitY);

			Float TheProgress = LimitY - Start.y;

			ProgressRatio = Dir.x / Dir.y;
			Step.x = Start.x + ProgressRatio * TheProgress;
			Step.y = LimitY;
		}
		else
		{
			Step = End;
		}

		PreviousStepX = FLOORINT(Step.x);
		NbPointOnStep = PreviousStepX - CurX + 1;

		// Restore Real X,Y and DX DY.
		if (dYBiggerdX)
		{
			S32 val;
			val = DeltaX; DeltaX = DeltaY; DeltaY = val;
			val = DeltaStepX; DeltaStepX = DeltaStepY; DeltaStepY = val;
			val = CurX; CurX = CurY; CurY = val;
		}

		if (IsNegDX)
		{
			CurX = -CurX - 1;	// -1 : Because Point is Right or Left from value
			DeltaX = -DeltaX;
			DeltaStepX = -DeltaStepX;
		}

		if (IsNegDY)
		{
			CurY = -CurY - 1;	// -1 : Because Point is Up or Down from value
			DeltaY = -DeltaY;
			DeltaStepY = -DeltaStepY;
		}
	}

	FINLINE_Z Bool	GetNextPointSead(S32 &X,S32 &Y)
	{
		// Process Horiz / Vert Line.
		while (NbPointOnStep <= 0)
		{
			// Prepare new Horiz / Vert Line.
			if (!NbStep)
				return FALSE;

			// Next Line.
			CurX+=DeltaStepX; 
			CurY+=DeltaStepY;
			NbStep--;

			// Compute Size Step !
			Step.y += 1.f;
			Float LimitY = Min<Float>(End.y,Step.y);
			Float TheProgress = LimitY - Start.y;
			Step.x = Start.x + ProgressRatio * TheProgress;
			Step.y = LimitY;

			S32 CurStepX = FLOORINT(Step.x);
			NbPointOnStep = CurStepX - PreviousStepX + 1;
			PreviousStepX = CurStepX;
		}
			
		// Cur Pos.
		X = CurX;
		Y = CurY;

		// Next Step.
		NbPointOnStep--;
		CurX+=DeltaX; 
		CurY+=DeltaY;

		return TRUE;
	}

// Specific Graph Part.
private:
	S32		G_StopX;
	S32		G_CurX;
	S32		G_CurY;

	Bool	G_InvertXY;

	S32		G_ystep,G_error,G_deltay,G_deltax;

public:
	void	InitGraph(S32 StartX,S32 StartY,S32 EndX,S32 EndY);
	Bool	GetNextPointGraph(S32 &X,S32 &Y);


#ifdef _PC
	static int	MySaveTGA(const char *name,const unsigned char *data,int width,int height);
	static void	TestAlgo1(char	*FileName);
	static void	TestAlgo2(char	*FileName);
#endif
};

#endif