#include <pch.h>
#include <Memory_Z.h>
void *Z_Alloc(size_t size, const Char *comment, const Char *file, S32 line, U32 align)
{
	return ::_aligned_malloc(size,align);
}
void *Z_Realloc(void *ptr, size_t size, const Char *comment, const Char *file, S32 line, U32 align)
{
	return ::_aligned_realloc(ptr, size, align); 
}
void Z_Free(void *ptr)
{
	return ::_aligned_free(ptr);
}
