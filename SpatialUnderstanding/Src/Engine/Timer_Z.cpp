// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include <pch.h>
#include <Timer_Z.h>
#include <Std_Z.h>

//-----------------------------------------------------------------------------
// Name: DXUtil_Timer()
// Desc: Performs timer opertations. Use the following commands:
//          TIMER_RESET           - to reset the timer
//          TIMER_START           - to start the timer
//          TIMER_STOP            - to stop (or pause) the timer
//          TIMER_ADVANCE         - to advance the timer by 0.1 seconds
//          TIMER_GETABSOLUTETIME - to get the absolute system time
//          TIMER_GETAPPTIME      - to get the current time
//          TIMER_GETELAPSEDTIME  - to get the time that elapsed between 
//                                  TIMER_GETELAPSEDTIME calls
//-----------------------------------------------------------------------------
enum TIMER_COMMAND { TIMER_RESET, TIMER_START, TIMER_STOP, TIMER_ADVANCE,
					 TIMER_GETABSOLUTETIME, TIMER_GETAPPTIME, TIMER_GETELAPSEDTIME };
#define _WINSTORE

#ifndef _WINSTORE
	#define GETTIMESTAMP timeGetTime
#else
	#define GETTIMESTAMP System::DateTime::Now
#endif

// Important de laisser en 'Double' pour pr�cision temps serveurs PC
double DXUtil_Timer( TIMER_COMMAND command )
{
	static BOOL     m_bTimerInitialized = FALSE;
	static BOOL     m_bUsingQPF         = FALSE;
	static BOOL     m_bTimerStopped     = TRUE;
	static LONGLONG m_llQPFTicksPerSec  = 0;

	// Initialize the timer
	if( FALSE == m_bTimerInitialized )
	{
		m_bTimerInitialized = TRUE;

		// Use QueryPerformanceFrequency() to get frequency of timer.  If QPF is
		// not supported, we will timeGetTime() which returns milliseconds.
		LARGE_INTEGER qwTicksPerSec;
		m_bUsingQPF = QueryPerformanceFrequency( &qwTicksPerSec );
		if( m_bUsingQPF )
			m_llQPFTicksPerSec = qwTicksPerSec.QuadPart;
	}

#ifdef _WINSTORE
	EXCEPTION_Z(m_bUsingQPF); //QPF should be always available on windows store apps (no fallback implemented)
#else // _WINSTORE
	if( m_bUsingQPF )
#endif // _WINSTORE
	{
		static volatile LONGLONG m_llStopTime        = 0;
		static volatile LONGLONG m_llLastElapsedTime = 0;
		static volatile LONGLONG m_llBaseTime        = 0;
		double fTime;
		double fElapsedTime;
		LARGE_INTEGER qwTime;
		
		// Get either the current time or the stop time, depending
		// on whether we're stopped and what command was sent
		if( m_llStopTime != 0 && command != TIMER_START && command != TIMER_GETABSOLUTETIME)
			qwTime.QuadPart = m_llStopTime;
		else
			QueryPerformanceCounter( &qwTime );

		// Return the elapsed time
		if( command == TIMER_GETELAPSEDTIME )
		{
			fElapsedTime = (double) ( qwTime.QuadPart - m_llLastElapsedTime ) / (double) m_llQPFTicksPerSec;
			m_llLastElapsedTime = qwTime.QuadPart;
			return fElapsedTime;
		}
	
		// Return the current time
		if( command == TIMER_GETAPPTIME )
		{
			double fAppTime = (double) ( qwTime.QuadPart - m_llBaseTime ) / (double) m_llQPFTicksPerSec;
			return fAppTime;
		}
	
		// Reset the timer
		if( command == TIMER_RESET )
		{
			m_llBaseTime        = qwTime.QuadPart;
			m_llLastElapsedTime = qwTime.QuadPart;
			m_llStopTime        = 0;
			m_bTimerStopped     = FALSE;
			return 0.0f;
		}
	
		// Start the timer
		if( command == TIMER_START )
		{
			if( m_bTimerStopped )
				m_llBaseTime += qwTime.QuadPart - m_llStopTime;
			m_llStopTime = 0;
			m_llLastElapsedTime = qwTime.QuadPart;
			m_bTimerStopped = FALSE;
			return 0.0f;
		}
	
		// Stop the timer
		if( command == TIMER_STOP )
		{
			if( !m_bTimerStopped )
			{
				m_llStopTime = qwTime.QuadPart;
				m_llLastElapsedTime = qwTime.QuadPart;
				m_bTimerStopped = TRUE;
			}
			return 0.0f;
		}
	
		// Advance the timer by 1/10th second
		if( command == TIMER_ADVANCE )
		{
			m_llStopTime += m_llQPFTicksPerSec/10;
			return 0.0f;
		}

		if( command == TIMER_GETABSOLUTETIME )
		{
			fTime = qwTime.QuadPart / (double) m_llQPFTicksPerSec;
			return fTime;
		}

		return -1.0f; // Invalid command specified
	}
#ifndef _WINSTORE
	else
	{
		// Get the time using timeGetTime()
		static double m_fLastElapsedTime  = 0.0;
		static double m_fBaseTime         = 0.0;
		static double m_fStopTime         = 0.0;
		double fTime;
		double fElapsedTime;
		
		// Get either the current time or the stop time, depending
		// on whether we're stopped and what command was sent
		if( m_fStopTime != 0.0 && command != TIMER_START && command != TIMER_GETABSOLUTETIME)
			fTime = m_fStopTime;
		else
			fTime = GETTIMESTAMP() * 0.001;
	
		// Return the elapsed time
		if( command == TIMER_GETELAPSEDTIME )
		{   
			fElapsedTime = (double) (fTime - m_fLastElapsedTime);
			m_fLastElapsedTime = fTime;
			// Passage au bout du INT32?
			if (fElapsedTime>1000.f) fElapsedTime=0.01f;
			if (fElapsedTime<-1000.f) fElapsedTime=0.01f;
			return fElapsedTime;
		}
	
		// Return the current time
		if( command == TIMER_GETAPPTIME )
		{
			return (fTime - m_fBaseTime);
		}
	
		// Reset the timer
		if( command == TIMER_RESET )
		{
			m_fBaseTime         = fTime;
			m_fLastElapsedTime  = fTime;
			m_fStopTime         = 0;
			m_bTimerStopped     = FALSE;
			return 0.0f;
		}
	
		// Start the timer
		if( command == TIMER_START )
		{
			if( m_bTimerStopped )
				m_fBaseTime += fTime - m_fStopTime;
			m_fStopTime = 0.0f;
			m_fLastElapsedTime  = fTime;
			m_bTimerStopped = FALSE;
			return 0.0f;
		}
	
		// Stop the timer
		if( command == TIMER_STOP )
		{
			if( !m_bTimerStopped )
			{
				m_fStopTime = fTime;
				m_fLastElapsedTime  = fTime;
				m_bTimerStopped = TRUE;
			}
			return 0.0f;
		}
	
		// Advance the timer by 1/10th second
		if( command == TIMER_ADVANCE )
		{
			m_fStopTime += 0.1f;
			return 0.0f;
		}

		if( command == TIMER_GETABSOLUTETIME )
		{
			return fTime;
		}

		return -1.0f; // Invalid command specified
	}
#endif //_!WINSTORE
}

void CalibrateTimer(void)
{
	DXUtil_Timer(TIMER_START);
}

float GetTime(void)
{
	float fTime=(float)DXUtil_Timer(TIMER_GETAPPTIME);//TIMER_GETABSOLUTETIME); // Car sinon perte de pr�cision.
	return fTime;
}
Float GetAbsoluteTime(void)
{
	return (float)GetTime();
}
// Important de laisser en 'Double' pour pr�cision temps serveurs PC
Double GetDAbsoluteTime(void)
{
	return (Double)DXUtil_Timer(TIMER_GETAPPTIME);
}

U64 GetAbsoluteTickCount()
{
	return (U64)(GetDAbsoluteTime()*1000.f);
}
