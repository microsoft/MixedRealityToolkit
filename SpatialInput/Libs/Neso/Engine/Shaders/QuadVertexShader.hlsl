////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
cbuffer ModelConstantBuffer : register(b0)
{
    float4x4 model;
}

// A constant buffer that stores each set of view and projection matrices in column-major format.
cbuffer ViewProjectionConstantBuffer : register(b1)
{
    float4x4 viewProjection[2];
};

// Per-vertex data used as input to the vertex shader.
struct VertexShaderInput
{
    float3 pos      : POSITION0;
    float2 texCoord : TEXCOORD0;
    uint   instId   : SV_InstanceID;
};

// Per-vertex data passed to the geometry shader.
// Note that the render target array index will be set by the geometry shader
// using the value of viewId.
struct VertexShaderOutput
{
    float4 pos      : SV_POSITION;
    float2 texCoord : TEXCOORD0;
#ifdef USE_VPRT
    uint        viewId   : SV_RenderTargetArrayIndex; // SV_InstanceID % 2
#else
    uint        viewId   : TEXCOORD15;  // SV_InstanceID % 2
#endif
};

// Simple shader to do vertex processing on the GPU.
VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    const float4 inputPos = float4(input.pos, 1.0f);

    // Note which view this vertex has been sent to. Used for matrix lookup.
    // Taking the modulo of the instance ID allows geometry instancing to be used
    // along with stereo instanced drawing; in that case, two copies of each 
    // instance would be drawn, one for left and one for right.
    const uint viewportIndex = input.instId % 2;

    // Transform the vertex position into world space.
    const float4 worldPos = mul(inputPos, model);

    output.pos = mul(worldPos, viewProjection[viewportIndex]);

    output.texCoord = input.texCoord;

    // Set the instance ID. The pass-through geometry shader will set the
    // render target array index to whatever value is set here.
    output.viewId = viewportIndex;

    return output;
}
