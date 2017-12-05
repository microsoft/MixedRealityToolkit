////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
    float4 pos      : SV_POSITION;
    float2 texCoord : TEXCOORD0;
};

Texture2D<float4>  rgbChannel     : t0;
SamplerState       defaultSampler : s0;

float4 main(PixelShaderInput input) : SV_TARGET
{
    float4 color = rgbChannel.Sample(defaultSampler, input.texCoord);
    if (color.a == 0.0f) discard;
    return color;
}
