////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
cbuffer SceneBuffer : register(b0)
{
    float4x4 ViewProjection[2]  : packoffset(c0);
    float3 EyePosition[2]       : packoffset(c8);
    float3 LightDirection       : packoffset(c10);
    float3 LightColor           : packoffset(c11);
    int NumSpecularMipLevels    : packoffset(c12.x);
};

struct PSInputBasePbr
{
    float4 PositionProj : SV_POSITION;
    float3 PositionWorld: POSITION1;
    float3x3 TBN        : TANGENT;
    float2 TexCoord0    : TEXCOORD0;
};

// Used as output for PbrVertexShaderVprt/PbrGeometryShaderNoVprt and input for PbrPixelShader
struct PSInputPbr : PSInputBasePbr
{
    uint ViewId : SV_RenderTargetArrayIndex;
};

// Used as output for PbrVertexShaderNoVprt and input for PbrGeometryShaderNoVprt.
struct VSOutputNoVprtPbr : PSInputBasePbr
{
    uint ViewId : TEXCOORD1;
};
