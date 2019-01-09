////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#include "PbrShared.hlsl"

#define GSInputPbr VSOutputNoVprtPbr
#define GSOutputPbr PSInputPbr

// This geometry shader is a pass-through that leaves the geometry unmodified 
// and sets the render target array index.
[maxvertexcount(3)]
void main(triangle GSInputPbr input[3], inout TriangleStream<GSOutputPbr> outStream)
{
    GSOutputPbr output;
    [unroll(3)]
    for (int i = 0; i < 3; ++i)
    {
        output = input[i];
        outStream.Append(output);
    }
}

