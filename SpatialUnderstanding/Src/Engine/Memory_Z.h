// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#ifndef _MEMORY_Z_H
#define _MEMORY_Z_H

#include <TypeInfo_Z.h>

#include <Types_Z.h>
#include <BeginDef_Z.h>

 void *Z_Alloc(size_t size,const Char *comment,const Char *file,S32 line,U32 align);
 void *Z_Realloc(void *ptr,size_t size,const Char *comment,const Char *file,S32 line,U32 align);
 void Z_Free(void *ptr);

#define Alloc_Z(size)							Z_Alloc( size, TYPEINFO_Z(this), __FILE__, __LINE__,_ALLOCDEFAULTALIGN)
#define AllocF_Z(size)							Z_Alloc( size, __FUNCTION__, __FILE__, __LINE__,_ALLOCDEFAULTALIGN)
#define Realloc_Z(ptr,size)						Z_Realloc( ptr, size, TYPEINFO_Z(this), __FILE__, __LINE__,_ALLOCDEFAULTALIGN)
#define Free_Z(ptr)								Z_Free( ptr )
#define AllocAlign_Z(size,align)				Z_Alloc( size, TYPEINFO_Z(this), __FILE__, __LINE__, align)
#define ReallocAlign_Z(ptr,size,align)			Z_Realloc( ptr, size, TYPEINFO_Z(this), __FILE__, __LINE__,align)

#define malloc	AllocF_Z
#define realloc	ReallocF_Z
#define	free	Free_Z

#ifndef __PLACEMENT_NEW_INLINE
	#define __PLACEMENT_NEW_INLINE
		inline void *  	operator new(size_t size,void *Ptr) { return Ptr; }
		inline void operator delete(void *ptr,void *unused) { free(ptr); }
#endif

// Allocator On Stack.

#endif //_MEMORY_Z_H
