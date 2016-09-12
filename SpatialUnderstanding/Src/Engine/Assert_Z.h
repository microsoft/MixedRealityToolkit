// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#ifndef __ASSERT_Z_H__
#define __ASSERT_Z_H__

#include <BeginDef_Z.h>
#include <stdio.h>

//External Links
inline  Bool ExceptionBool_Z(Bool bCancelable,Bool bExt,const Char *exp, const Char *file, S32 line,const Char *_Message,...)
{
	if (bExt)
		return FALSE;

	va_list Marker;
	va_start(Marker, _Message);
	printf(_Message, Marker);
	va_end(Marker);
	
#if defined(_WIN32) && !defined(_SUBMISSION) && defined(_M_IX86)
	__asm int 3;
#endif

	return FALSE;

}
inline  void BreakPoint_Z()
{
#if defined(_WIN32) && !defined(_SUBMISSION) && defined(_M_IX86)
	__asm int 3;
#endif
}

#ifdef _EDITION_Z
	char EXTERN_Z LogStr[256];
#endif

#if !defined(__analysis_assume)
#   define __analysis_assume(x) ((void) 0)
#endif

#ifdef _SUBMISSION
	#define EXCEPTIONC_Z(exp,...)
	#define EXCEPTION_Z(exp)
	#define CANCEL_EXCEPTIONC_Z(exp,...)
	#define BREAKPOINT_Z()
#else
	//Exception, Ellipse, Expression, Direct
	#define	EXCEPTIONC_Z(exp,...)			do { Bool bnExp = (exp); __analysis_assume(bnExp); if(!bnExp) ExceptionBool_Z(FALSE,FALSE,#exp,__FILE__,__LINE__,__VA_ARGS__); } while (0)
    #define	EXCEPTION_Z(exp)				do { Bool bnExp = (exp); __analysis_assume(bnExp); if(!bnExp) ExceptionBool_Z(FALSE,FALSE,#exp,__FILE__,__LINE__,"");          } while (0)
	#define	CANCEL_EXCEPTIONC_Z(exp,...)	do { Bool bnExp = (exp); __analysis_assume(bnExp); if(!bnExp) ExceptionBool_Z(TRUE, FALSE,#exp,__FILE__,__LINE__,__VA_ARGS__); } while (0)
	#define BREAKPOINT_Z()					BreakPoint_Z()
#endif

#define CONCAT_(a,b) a##b
#define CONCAT(a,b) CONCAT_(a,b)
template<Bool>      class COMPILER_ASSERT;
template<>          class COMPILER_ASSERT<TRUE> { enum { dummy }; };
template<int size>  class assert_test{};

#define ASSERT_AT_COMPILE_TIME_Z(test) typedef assert_test<sizeof(COMPILER_ASSERT<(test)>)> CONCAT(dummy,__LINE__)
#define	ASSERTC_Z(exp,...)			do { Bool bnExp = (exp); __analysis_assume(bnExp); if(!bnExp) ExceptionBool_Z(FALSE,FALSE,#exp,__FILE__,__LINE__,__VA_ARGS__); } while (0)
#define	ASSERT_Z(exp)				do { Bool bnExp = (exp); __analysis_assume(bnExp); if(!bnExp) ExceptionBool_Z(FALSE,FALSE,#exp,__FILE__,__LINE__,"");          } while (0)

#endif //__ASSERT_Z_H__
