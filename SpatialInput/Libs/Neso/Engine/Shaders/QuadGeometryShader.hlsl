////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
// Per-vertex data from the vertex shader.
struct GeometryShaderInput
{
    float4 pos     : SV_POSITION;
    float2 tex     : TEXCOORD0;
    uint   instId  : TEXCOORD15;
};

// Per-vertex data passed to the rasterizer.
struct GeometryShaderOutput
{
    float4 pos     : SV_POSITION;
    float2 tex     : TEXCOORD0;
    uint   rtvId   : SV_RenderTargetArrayIndex;
};

// This geometry shader is a pass-through that leaves the geometry unmodified 
// and sets the render target array index.
[maxvertexcount(3)]
void main(triangle GeometryShaderInput input[3], inout TriangleStream<GeometryShaderOutput> outStream)
{
    GeometryShaderOutput output;
    [unroll(3)]
    for (int i = 0; i < 3; ++i)
    {
        output.pos = input[i].pos;
        output.tex = input[i].tex;
        output.rtvId = input[i].instId;
        outStream.Append(output);
    }
}
