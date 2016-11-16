//////////////////////////////////////////////////
// Copyright (C) Microsoft  All Right Reserved
//////////////////////////////////////////////////

// Mimic the FIXED macro in arrays_csharp.i, but pin the memory without using the "fixed" keyword, which requires marking the function as unsafe
%define CSHARP_ARRAYS_PINNED( CTYPE, CSTYPE )

%typemap(ctype)   CTYPE PINNED[] "CTYPE*"
%typemap(imtype)  CTYPE PINNED[] "global::System.IntPtr"
%typemap(cstype)  CTYPE PINNED[] "CSTYPE[]"
%typemap(csin,
            pre=      "    global::System.Runtime.InteropServices.GCHandle pinHandle_$csinput = global::System.Runtime.InteropServices.GCHandle.Alloc($csinput, global::System.Runtime.InteropServices.GCHandleType.Pinned); try {",
            terminator="    } finally { pinHandle_$csinput.Free(); }") 
                  CTYPE PINNED[] "(global::System.IntPtr)pinHandle_$csinput.AddrOfPinnedObject()"

%typemap(in)      CTYPE PINNED[] "$1 = $input;"
%typemap(freearg) CTYPE PINNED[] ""
%typemap(argout)  CTYPE PINNED[] ""


%enddef // CSHARP_ARRAYS_PINNED

// Map arrays of primitve types in C++ to their associated types in C# using pinning 
CSHARP_ARRAYS_PINNED(signed char, sbyte)
CSHARP_ARRAYS_PINNED(unsigned char, byte)
CSHARP_ARRAYS_PINNED(short, short)
CSHARP_ARRAYS_PINNED(unsigned short, ushort)
CSHARP_ARRAYS_PINNED(int, int)
CSHARP_ARRAYS_PINNED(unsigned int, uint)
CSHARP_ARRAYS_PINNED(long, int)
CSHARP_ARRAYS_PINNED(unsigned long, uint)
CSHARP_ARRAYS_PINNED(long long, long)
CSHARP_ARRAYS_PINNED(unsigned long long, ulong)
CSHARP_ARRAYS_PINNED(float, float)
CSHARP_ARRAYS_PINNED(double, double)
CSHARP_ARRAYS_PINNED(bool, bool)