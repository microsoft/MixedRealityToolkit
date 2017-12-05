////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#include "SkyBoxShared.hlsl"

// This geometry shader is a pass-through that leaves the geometry unmodified 
// and sets the render target array index.
[maxvertexcount(3)]
void main(triangle GSInput input[3], inout TriangleStream<PSInput> outStream)
{
    PSInput output;
    [unroll(3)]
    for (int i = 0; i < 3; ++i)
    {
        output = input[i];
        outStream.Append(output);
    }
}

