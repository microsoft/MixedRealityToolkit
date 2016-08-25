// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#ifndef _TYPES_Z_H
#define _TYPES_Z_H

typedef bool				Bool;


// -------------------------------------------------------------------------
// C++ Language extensions for virtual overriding checks at compilation time (exist as MS extensions and in C++11)
#if defined(_PC)
	#define OVERRIDE_Z	override
	#define FINAL_Z		sealed

	#pragma warning (disable:4481)	// using override or sealed produces this kind of warning:
									// "warning C4481: nonstandard extension used: override specifier 'sealed'"
#else
	#define OVERRIDE_Z
	#define FINAL_Z
#endif

// Usage:
//struct Base {
//	virtual void SomeFunc(int oldParam, int newParam);
//	virtual void OtherFunc() FINAL_Z;		// derived classes are not allowed to reimplement this
//};
//struct Derived : Base {
//	virtual void SomeFunc(int oldParam) OVERRIDE_Z;	// doesn't compile because it does not override Base::SomeFunc (signature is different)
//	virtual void OtherFunc();						// doesn't compile because we are not allowed to override Base::OtherFunc
//};

// -------------------------------------------------------------------------
// virtual vs inlinde directives

#if defined(_PC)
	#define VIRTUAL		inline
#else
	#define VIRTUAL		virtual
#endif
#ifdef	_EDITION_Z
	#undef	VIRTUAL
	#define VIRTUAL		virtual
#endif

// -------------------------------------------------------------------------
// Alignement directives

#if (defined(_FAKE) || defined(_PC))
	#define ALIGNED_Z(x)
	#define ALIGNED_CLASS_Z(x)			__declspec(align(x))
	#define POSTALIGNED_Z(x)			__declspec(align(x))
	#define PACKED_Z
#else
	#define ALIGNED_Z(x)
	#define ALIGNED_CLASS_Z(x)			
	#define	POSTALIGNED_Z(x)
	#define PACKED_Z
#endif

#define	ALIGNED1024_Z		ALIGNED_Z(128)
#define	ALIGNED512_Z		ALIGNED_Z(64)
#define	ALIGNED256_Z		ALIGNED_Z(32)
#define	ALIGNED128_Z		ALIGNED_Z(16)
#define	ALIGNED64_Z			ALIGNED_Z(8)
#define	POSTALIGNED128_Z	POSTALIGNED_Z(16)

#define DECLARE_ALIGNED_Z(__var,__align)	POSTALIGNED_Z(__align) __var ALIGNED_Z(__align)

// -------------------------------------------------------------------------

#if defined(_MSC_VER)
	#define	FINLINE_Z			__forceinline
	#define	INLINE_Z			inline
#else
	#if ( defined(__SNC__) || defined(__GCC__) || defined(__clang__) )
		#define	FINLINE_Z		__attribute__((always_inline)) inline
	#else
		#define	FINLINE_Z		inline
	#endif
	#if defined(__SPU__)
		#ifdef _DEBUG
			#define INLINE_Z
		#else
			#define INLINE_Z		__attribute__((always_inline)) inline 
		#endif
	#else
		#define	INLINE_Z			inline
	#endif
#endif

#define RESTRICT_Z


// Avoid Read Write Order modification (Compilo Optimisation)
// For Threadsafe code.

#if defined(_FAKE)
	#include "intrin.h"
	#define READWRITEBARRIER_Z() _ReadWriteBarrier()
#else
	#include "intrin.h"
	#define READWRITEBARRIER_Z() MemoryBarrier(); _ReadWriteBarrier()
#endif

#if defined(__SNC__) || defined(__GCC__) || defined(__clang__)
	#define UNUSED_ATTRIBUTE_Z	__attribute__((unused))
#else
	#define UNUSED_ATTRIBUTE_Z
#endif

#define UNUSED_Z(_unused)	((void)&_unused)

#if defined(_MSC_VER) || defined(__clang__)
	#define DEPRECATEDC_Z(_comment)	__declspec(deprecated(_comment))
	#define DEPRECATED_Z			__declspec(deprecated)
#else
	#define DEPRECATEDC_Z(_comment)
	#define DEPRECATED_Z
#endif

// Cache prefetching

template <typename T> inline void PrefetchRead( const T* data )
{
}

#define	PREFETCH_READ_Z(addr)	PrefetchRead(addr)

#if defined(_PC_SSE)
	#define PREFETCH_READ_OFFSET_Z(offset,addr)		_mm_prefetch( (char*)(addr) + offset, _MM_HINT_NTA )
	#define	PREFETCH_WRITE_OFFSET_Z(offset,addr)
#else
	#define PREFETCH_READ_OFFSET_Z(offset,addr)
	#define	PREFETCH_WRITE_OFFSET_Z(offset,addr)
#endif

// Types declaration
#ifdef _MSC_VER 
	typedef unsigned __int64	U64;
	typedef __int64				S64;
#else
	typedef unsigned long long	U64	ALIGNED64_Z;
	typedef signed long	long	S64	ALIGNED64_Z;
#endif

typedef unsigned long		U32;
typedef signed long			S32;
typedef unsigned short		U16;
typedef signed short		S16;

// __int128 emulation
struct U128;
struct U128
{
	U64	Low,High;
	void	operator=(float) { Low = High = 0; }
	void	operator=(U64 value) { Low = High = value; }
	void	operator|=(const U128 &Ref) { Low |= Ref.Low; High |= Ref.High; }
	void	operator&=(const U128 &Ref) { Low &= Ref.Low; High &= Ref.High; }
	Bool	operator==(const U128 &Ref)const { return (Low == Ref.Low) && (High == Ref.High); }
	Bool	operator!=(const U128 &Ref)const { return (Low != Ref.Low) || (High != Ref.High); }
	Bool	operator&(const U128 &Ref)const { return (Low&Ref.Low) || (High&Ref.High); }
	U128	operator~()const { U128 Not; Not.Low = ~Low; Not.High = ~High;return Not; }
};
struct S128;
struct S128
{
	S64	Low,High;
	void	operator=(float) { Low = High = 0; }
	void	operator=(S64 value) { Low = High = value; }
	void	operator|=(const S128 &Ref) { Low |= Ref.Low; High |= Ref.High; }
	void	operator&=(const S128 &Ref) { Low &= Ref.Low; High &= Ref.High; }
	Bool	operator==(const S128 &Ref)const { return (Low == Ref.Low) && (High == Ref.High); }
	Bool	operator!=(const S128 &Ref)const { return (Low != Ref.Low) || (High != Ref.High); }
	Bool	operator&(const S128 &Ref)const { return (Low&Ref.Low) || (High&Ref.High); }
	S128	operator~()const { S128 Not; Not.Low = ~Low; Not.High = ~High;return Not; }
};

#ifndef _MSC_VER
	#if defined(MIDL_PASS)
	typedef struct _LARGE_INTEGER {
	#else // MIDL_PASS
	typedef union _LARGE_INTEGER {
		struct {
			U32 LowPart;
			S32 HighPart;
		} DUMMYSTRUCTNAME;
		struct {
			U32 LowPart;
			S32 HighPart;
		} u;
	#endif //MIDL_PASS
		U64 QuadPart;
	} LARGE_INTEGER;
#endif

typedef unsigned char		U8;
typedef signed char			S8;
typedef char				Char;
//typedef half				Half;
typedef float				Float;

#if defined(_PC)
	typedef	double			Double;
#else
	typedef	float			Double;
#endif

#if defined(_M_X64)
	typedef U64				INT_PTR_Z;
	#define ATOI_Z			_atoi64
	#define ATOU64_Z		_strtoui64
#else
	typedef U32				INT_PTR_Z;
	#define ATOI_Z			atoi
	#define ATOU64_Z		_strtoui64
#endif

// --------- GUID -----------------------

struct GUID_Z
{
public:
	GUID_Z() { data.raw = (U64)0; }
	Bool	operator==(const GUID_Z &Ref)const { return data.raw == Ref.data.raw; }
	Bool	operator!=(const GUID_Z &Ref)const { return data.raw != Ref.data.raw; }
	void	Null() { data.raw = (U64)-1; }

	explicit GUID_Z(unsigned int _in) { Null(); data.guid.Data1 = _in; }

#if defined(__cplusplus_winrt)
	GUID_Z(const Platform::Guid& _in) { Platform::Guid* pPlatformGuid = reinterpret_cast<Platform::Guid*>(this); pPlatformGuid[0] = _in; }
	void operator=(const Platform::Guid& _in) { Platform::Guid* pPlatformGuid = reinterpret_cast<Platform::Guid*>(this); pPlatformGuid[0] = _in; }
	operator const Platform::Guid& () const { return *reinterpret_cast<const Platform::Guid*>(this); }
#endif
#if defined(GUID_DEFINED)
	GUID_Z(const GUID& _in) { GUID* pPlatformGuid = reinterpret_cast<GUID*>(this); pPlatformGuid[0] = _in; }
	void operator=(const GUID& _in) { GUID* pPlatformGuid = reinterpret_cast<GUID*>(this); pPlatformGuid[0] = _in; }
	operator const GUID& () const { return *reinterpret_cast<const GUID*>(this); }
#endif

	inline void Set(U32 Data1, U16 Data2, U16 Data3, Char Data4[8]);
	inline void Get(U32& Data1, U16& Data2, U16& Data3, Char Data4[8]);

private:
	union {
		U128 raw;
		struct {
			U32 Data1;
			U16 Data2;
			U16 Data3;
			Char Data4[8];
		} guid;
	} data;
};

void GUID_Z::Set(U32 Data1, U16 Data2, U16 Data3, Char Data4[8])
{
	data.guid.Data1 = Data1;
	data.guid.Data2 = Data2;
	data.guid.Data3 = Data3;
	data.guid.Data4[0] = Data4[0];
	data.guid.Data4[1] = Data4[1];
	data.guid.Data4[2] = Data4[2];
	data.guid.Data4[3] = Data4[3];
	data.guid.Data4[4] = Data4[4];
	data.guid.Data4[5] = Data4[5];
	data.guid.Data4[6] = Data4[6];
	data.guid.Data4[7] = Data4[7];
}
void GUID_Z::Get(U32& Data1, U16& Data2, U16& Data3, Char Data4[8])
{
	Data1 = data.guid.Data1;
	Data2 = data.guid.Data2;
	Data3 = data.guid.Data3;
	Data4[0] = data.guid.Data4[0];
	Data4[1] = data.guid.Data4[1];
	Data4[2] = data.guid.Data4[2];
	Data4[3] = data.guid.Data4[3];
	Data4[4] = data.guid.Data4[4];
	Data4[5] = data.guid.Data4[5];
	Data4[6] = data.guid.Data4[6];
	Data4[7] = data.guid.Data4[7];
}

// --------- Intrinsics definitions

POSTALIGNED128_Z struct Float4 // Fallback type for VecFloat4
{
	union
	{
		struct
		{
			Float x ALIGNED128_Z;
			Float y,z,w;
		};
		Float xyzw[4];
	};
};

POSTALIGNED128_Z struct ULongLong2 // Fallback type for VecUULongLong2
{
	U64 x ALIGNED128_Z;
	U64 y;
};

POSTALIGNED128_Z struct Int4 // Fallback type for VecInt4
{
	union
	{
		struct
		{
			S32 x ALIGNED128_Z;
			S32 y,z,w;
		};
		S32 xyzw[4];
	};
};

POSTALIGNED128_Z struct UInt4 // Fallback type for VecUInt4
{
	union
	{
		struct
		{
			U32 x ALIGNED128_Z;
			U32 y,z,w;
		};
		U32 xyzw[4];
	};
};

POSTALIGNED128_Z struct Short8 // Fallback type for VecShort8
{
	union
	{
		struct
		{
			S16 a ALIGNED128_Z;
			S16 b,c,d,e,f,g,h;
		};
		S16 s0_7[8];
	};
};

POSTALIGNED128_Z struct UShort8 // Fallback type for VecUShort8
{
	union
	{
		struct
		{
			U16 a ALIGNED128_Z;
			U16 b,c,d,e,f,g,h;
		};
		U16 u0_7[8];
	};
};

POSTALIGNED128_Z struct UChar16 // Fallback type for VecUChar16
{
	union
	{
		struct
		{
			U8 u0 ALIGNED128_Z;
			U8 u1,u2,u3,u4,u5,u6,u7,u8,u9,u10,u11,u12,u13,u14,u15;
		};
		U8 u0_15[16];
	};
};

#if defined( _PC )
	#ifdef _PC_SSE
	typedef __m128              VecFloat4;
	typedef __m128				VecBInt4;
	typedef __m128				VecSelMask;
	#else
	typedef Float4              VecFloat4;
	typedef U128				VecBInt4;
	typedef Int4				VecSelMask;
	#endif
	typedef VecFloat4			VecFloat3x3[3];
	typedef VecFloat4  			VecFloat4x4[4];
	#ifdef _PC_SSE2
	typedef __m128i				VecULongLong2;
	typedef __m128i				VecInt4;
	typedef __m128i				VecUInt4;
	typedef __m128i				VecShort8;
	typedef __m128i				VecUShort8;
	typedef __m128i				VecChar16;
	typedef __m128i				VecUChar16;
	#else
	typedef ULongLong2			VecULongLong2;
	typedef Int4				VecInt4;
	typedef UInt4				VecUInt4;
	typedef Short8				VecShort8;
	typedef UShort8				VecUShort8;
	typedef Char				VecChar16[16];
	typedef UChar16				VecUChar16;
	#endif
#else
	typedef Float4              VecFloat4;
	typedef VecFloat4			VecFloat3x3[3];
	typedef VecFloat4			VecFloat4x4[4];
	typedef ULongLong2			VecULongLong2;
	typedef Int4				VecInt4;
	typedef UInt4				VecUInt4;
	typedef U128				VecBInt4;
	typedef Short8				VecShort8;
	typedef UShort8				VecUShort8;
	typedef Char				VecChar16[16];
	typedef UChar16				VecUChar16;
	typedef Int4				VecSelMask;
#endif
	
const Float	Float_Max	= 1.e+37f;
const Float	Float_Min	= 1.e-37f;
const Float Float_Eps	= 1.e-6f;
const Float Float_Eps_2	= 1.e-12f;

// PI constants
const Float Pi_Mul_2    = 6.28318530717958647692f;
const Float Pi			= 3.14159265358979323846f;
const Float Pi_Div_2 	= 1.57079632679489661923f;
const Float Pi_Div_4 	= 0.78539816339744830961f;
const Float Pi_Div_8 	= 0.39269908169872415480f;
const Float Sqrt_2		= 1.41421356237f;
const Float Sqrt_3		= 1.73205080757f;

// PLEASE NOTE: the use of this constant is old, please use above
const Float A_PI		= 3.14159265358979323846f;


#undef FALSE
#define	FALSE			(Bool) (0==1)

#undef TRUE
#define	TRUE			(Bool) (0==0)

#undef NULL
#define NULL			0

#define		ARRAY_CHAR_MAX		256
#define		EXT_ARRAY_CHAR_MAX	256
#define		MAX_ERROR_CHAR		4096

#if !defined(MAX_PATH)
	#define MAX_PATH			260
#endif

#if !defined(_countof)
	/* _countof helper */
	#if !defined(__cplusplus)
		#define _countof(_Array) (sizeof(_Array) / sizeof(_Array[0]))
	#else
	extern "C++"
	{
		template <typename _CountofType, size_t _SizeOfArray>
		char (*__countof_helper(_CountofType (&_Array)[_SizeOfArray]))[_SizeOfArray];
		#define _countof(_Array) (sizeof(*__countof_helper(_Array)) + 0)
	}
	#endif
#endif

	// Enum declaration specification due to platforms compiler restrictions
#define ENUM_FORWARD_TYPE(enumType)	: enumType
#define ENUM_TYPE(enumType)			: enumType

//enum operators helper
#define	DECLARE_INLINE_ENUM_FLAG_OPERATORS_Z(_enumType) \
	ASSERT_AT_COMPILE_TIME_Z(sizeof(_enumType) == sizeof(U32)); \
	FINLINE_Z _enumType operator|(_enumType _a, _enumType _b){ return _enumType( ((U32)_a) | ((U32)_b) ); } \
	FINLINE_Z _enumType& operator|=(_enumType& _a, _enumType _b){	return (_enumType&)( ((U32&)_a) |= ((U32)_b) ); } \
	FINLINE_Z _enumType operator&(_enumType _a, _enumType _b){ return _enumType((U32)_a & (U32)_b); } \
	FINLINE_Z _enumType& operator&=(_enumType& _a, _enumType _b){ return (_enumType&)( ((U32&)_a) &= ((U32)_b) ); } \
	FINLINE_Z _enumType operator^(_enumType _a, _enumType _b){ return _enumType((U32)_a ^ (U32)_b); } \
	FINLINE_Z _enumType& operator^=(_enumType& _a, _enumType _b){ return (_enumType&)( ((U32&)_a) ^= ((U32)_b) ); } \
	FINLINE_Z _enumType operator~(_enumType _a){ return _enumType( ~((U32)_a) ); }\
	FINLINE_Z _enumType operator++(_enumType _a) { _a = (_enumType)(_a+1); return _a;} \
	FINLINE_Z _enumType operator++(_enumType &_a, int) { _enumType ret=_a; _a=(_enumType)(_a+1); return ret; } \
	FINLINE_Z _enumType& operator<<=(_enumType& _a,int n){ return (_enumType&)(((U32&)_a)<<=n); } \
	FINLINE_Z _enumType& operator>>=(_enumType& _a,int n){ return (_enumType&)(((U32&)_a)>>=n); }

#define	DECLARE_INLINE_ENUM_FLAG_OPERATORS_IN_CLASS_Z(_enumType) \
	ASSERT_AT_COMPILE_TIME_Z(sizeof(_enumType) == sizeof(U32)); \
	FINLINE_Z friend _enumType operator|(_enumType _a, _enumType _b){ return _enumType( ((U32)_a) | ((U32)_b) ); } \
	FINLINE_Z friend _enumType& operator|=(_enumType& _a, _enumType _b){	return (_enumType&)( ((U32&)_a) |= ((U32)_b) ); } \
	FINLINE_Z friend _enumType operator&(_enumType _a, _enumType _b){ return _enumType((U32)_a & (U32)_b); } \
	FINLINE_Z friend _enumType& operator&=(_enumType& _a, _enumType _b){ return (_enumType&)( ((U32&)_a) &= ((U32)_b) ); } \
	FINLINE_Z friend _enumType operator^(_enumType _a, _enumType _b){ return _enumType((U32)_a ^ (U32)_b); } \
	FINLINE_Z friend _enumType& operator^=(_enumType& _a, _enumType _b){ return (_enumType&)( ((U32&)_a) ^= ((U32)_b) ); } \
	FINLINE_Z friend _enumType operator~(_enumType _a){ return _enumType( ~((U32)_a) ); }\
	FINLINE_Z friend _enumType operator++(_enumType _a) { _a = (_enumType)(_a+1); return _a;} \
	FINLINE_Z friend _enumType operator++(_enumType &_a, int) { _enumType ret=_a; _a=(_enumType)(_a+1); return ret; } \
	FINLINE_Z friend _enumType& operator<<=(_enumType& _a,int n){ return (_enumType&)(((U32&)_a)<<=n); } \
	FINLINE_Z friend _enumType& operator>>=(_enumType& _a,int n){ return (_enumType&)(((U32&)_a)>>=n); }

// Compilation Hack
#ifdef _DONT_SUPPORT_FORWARD_DECLARATIONS
	#define		ASSERT_AT_ENUM_FLAG_64_Z(_enumType)			ASSERT_AT_COMPILE_TIME_Z(sizeof(U64) == sizeof(U64));
#else
	#define		ASSERT_AT_ENUM_FLAG_64_Z(_enumType)			ASSERT_AT_COMPILE_TIME_Z(sizeof(_enumType) == sizeof(U64));
#endif

#define	DECLARE_INLINE_ENUM_FLAG_OPERATORS_64_Z(_enumType) \
	ASSERT_AT_ENUM_FLAG_64_Z(_enumType); \
	FINLINE_Z _enumType operator|(_enumType _a, _enumType _b){ return _enumType( ((U64)_a) | ((U64)_b) ); } \
	FINLINE_Z _enumType& operator|=(_enumType& _a, _enumType _b){	return (_enumType&)( ((U64&)_a) |= ((U64)_b) ); } \
	FINLINE_Z _enumType operator&(_enumType _a, _enumType _b){ return _enumType((U64)_a & (U64)_b); } \
	FINLINE_Z _enumType& operator&=(_enumType& _a, _enumType _b){ return (_enumType&)( ((U64&)_a) &= ((U64)_b) ); } \
	FINLINE_Z _enumType operator^(_enumType _a, _enumType _b){ return _enumType((U64)_a ^ (U64)_b); } \
	FINLINE_Z _enumType& operator^=(_enumType& _a, _enumType _b){ return (_enumType&)( ((U64&)_a) ^= ((U64)_b) ); } \
	FINLINE_Z _enumType operator~(_enumType _a){ return _enumType( ~((U64)_a) ); }\
	FINLINE_Z _enumType operator++(_enumType _a) { _a = (_enumType)(_a+((U64)1)); return _a;} \
	FINLINE_Z _enumType operator++(_enumType &_a, int) { _enumType ret=_a; _a=(_enumType)(_a+((U64)1)); return ret; } \
	FINLINE_Z _enumType& operator<<=(_enumType& _a,int n){ return (_enumType&)(((U64&)_a)<<=n); } \
	FINLINE_Z _enumType& operator>>=(_enumType& _a,int n){ return (_enumType&)(((U64&)_a)>>=n); }

#define	DECLARE_INLINE_ENUM_FLAG_OPERATORS_IN_CLASS_64_Z(_enumType) \
	ASSERT_AT_ENUM_FLAG_64_Z(_enumType); \
	FINLINE_Z friend _enumType operator|(_enumType _a, _enumType _b){ return _enumType( ((U64)_a) | ((U64)_b) ); } \
	FINLINE_Z friend _enumType& operator|=(_enumType& _a, _enumType _b){	return (_enumType&)( ((U64&)_a) |= ((U64)_b) ); } \
	FINLINE_Z friend _enumType operator&(_enumType _a, _enumType _b){ return _enumType((U64)_a & (U64)_b); } \
	FINLINE_Z friend _enumType& operator&=(_enumType& _a, _enumType _b){ return (_enumType&)( ((U64&)_a) &= ((U64)_b) ); } \
	FINLINE_Z friend _enumType operator^(_enumType _a, _enumType _b){ return _enumType((U64)_a ^ (U64)_b); } \
	FINLINE_Z friend _enumType& operator^=(_enumType& _a, _enumType _b){ return (_enumType&)( ((U64&)_a) ^= ((U64)_b) ); } \
	FINLINE_Z friend _enumType operator~(_enumType _a){ return _enumType( ~((U64)_a) ); }\
	FINLINE_Z friend _enumType operator++(_enumType _a) { _a = (_enumType)(_a+((U64)1)); return _a;} \
	FINLINE_Z friend _enumType operator++(_enumType &_a, int) { _enumType ret=_a; _a=(_enumType)(_a+((U64)1)); return ret; } \
	FINLINE_Z friend _enumType& operator<<=(_enumType& _a,int n){ return (_enumType&)(((U64&)_a)<<=n); } \
	FINLINE_Z friend _enumType& operator>>=(_enumType& _a,int n){ return (_enumType&)(((U64&)_a)>>=n); }


#define PASS_VA_ARGS(...) __VA_ARGS__

#define _INTERNAL_BUILD_FUNC_PTR_TABLE_1(FN_NAME_START, ...)	\
	FN_NAME_START false, __VA_ARGS__,		\
	FN_NAME_START true,  __VA_ARGS__,

#define _INTERNAL_BUILD_FUNC_PTR_TABLE_2(FN_NAME_START, ...)	\
	_INTERNAL_BUILD_FUNC_PTR_TABLE_1( FN_NAME_START, PASS_VA_ARGS( false, __VA_ARGS__ ) )	\
	_INTERNAL_BUILD_FUNC_PTR_TABLE_1( FN_NAME_START, PASS_VA_ARGS( true, __VA_ARGS__ ) )

#define _INTERNAL_BUILD_FUNC_PTR_TABLE_3(FN_NAME_START, ...)	\
	_INTERNAL_BUILD_FUNC_PTR_TABLE_2( FN_NAME_START, PASS_VA_ARGS( false, __VA_ARGS__ ) )	\
	_INTERNAL_BUILD_FUNC_PTR_TABLE_2( FN_NAME_START, PASS_VA_ARGS( true, __VA_ARGS__ ) )

#define _INTERNAL_BUILD_FUNC_PTR_TABLE_4(FN_NAME_START, ...)	\
	_INTERNAL_BUILD_FUNC_PTR_TABLE_3( FN_NAME_START, PASS_VA_ARGS( false, __VA_ARGS__ ) )	\
	_INTERNAL_BUILD_FUNC_PTR_TABLE_3( FN_NAME_START, PASS_VA_ARGS( true, __VA_ARGS__ ) )

#define _INTERNAL_BUILD_FUNC_PTR_TABLE_5(FN_NAME_START, ...)	\
	_INTERNAL_BUILD_FUNC_PTR_TABLE_4( FN_NAME_START, PASS_VA_ARGS( false, __VA_ARGS__ ) )	\
	_INTERNAL_BUILD_FUNC_PTR_TABLE_4( FN_NAME_START, PASS_VA_ARGS( true, __VA_ARGS__ ) )

// -------------
#define BUILD_FUNC_PTR_TABLE_1(FN_NAME_START, ...)	\
	FN_NAME_START false __VA_ARGS__,	\
	FN_NAME_START true __VA_ARGS__,

#define BUILD_FUNC_PTR_TABLE_2(FN_NAME_START, ...)	\
	_INTERNAL_BUILD_FUNC_PTR_TABLE_1( FN_NAME_START, false __VA_ARGS__ )	\
	_INTERNAL_BUILD_FUNC_PTR_TABLE_1( FN_NAME_START, true __VA_ARGS__ )

#define BUILD_FUNC_PTR_TABLE_3(FN_NAME_START, ...)	\
	_INTERNAL_BUILD_FUNC_PTR_TABLE_2( FN_NAME_START, false __VA_ARGS__ )	\
	_INTERNAL_BUILD_FUNC_PTR_TABLE_2( FN_NAME_START, true __VA_ARGS__ )

#define BUILD_FUNC_PTR_TABLE_4(FN_NAME_START, ...)	\
	_INTERNAL_BUILD_FUNC_PTR_TABLE_3( FN_NAME_START, false __VA_ARGS__ )	\
	_INTERNAL_BUILD_FUNC_PTR_TABLE_3( FN_NAME_START, true __VA_ARGS__ )

#define BUILD_FUNC_PTR_TABLE_5(FN_NAME_START, ...)	\
	_INTERNAL_BUILD_FUNC_PTR_TABLE_4( FN_NAME_START, false __VA_ARGS__ )	\
	_INTERNAL_BUILD_FUNC_PTR_TABLE_4( FN_NAME_START, true __VA_ARGS__ )

#define BUILD_FUNC_PTR_TABLE_6(FN_NAME_START, ...)	\
	_INTERNAL_BUILD_FUNC_PTR_TABLE_5( FN_NAME_START, false __VA_ARGS__ )	\
	_INTERNAL_BUILD_FUNC_PTR_TABLE_5( FN_NAME_START, true __VA_ARGS__ )

#define ADD_TEST(TEST, i)	| (((int)((bool)TEST))<<i)
#define BUILD_FUNC_PTR_INDEX(...)	0 __VA_ARGS__

#endif //_TYPES_Z_H
