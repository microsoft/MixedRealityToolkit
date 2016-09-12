// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#ifndef _CHECKSUMCRC32_Z_H
#define _CHECKSUMCRC32_Z_H

#include <BeginDef_Z.h>

////////////////////////////////////////////////////////////
// CheckSumCRC32_Z: Simple CRC32 checksum
////////////////////////////////////////////////////////////

class 	CheckSumCRC32_Z
{
	static	const U32 crc_table[256];
public:
	virtual	U32	Calc(U32 crc,const void *buf,U32 len);
};

#endif
