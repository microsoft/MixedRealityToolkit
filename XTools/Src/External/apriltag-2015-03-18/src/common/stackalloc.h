// MICROSOFT PROJECT B CHANGES BEGIN
#ifndef _STACHALLOC_H
#define _STACHALLOC_H

#ifdef _MSC_VER
#include <malloc.h>
#define stackalloc(name, type, length) type* name = malloc((length) * sizeof(type))
#define stackfree(name) free(name)
#else
#define stackalloc(name, type, length) type name[length]
#define stackfree(name) ((void)0)
#endif

#endif
// MICROSOFT PROJECT B CHANGES END
