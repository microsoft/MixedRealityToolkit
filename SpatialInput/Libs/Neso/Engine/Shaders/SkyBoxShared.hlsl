cbuffer SceneBuffer : register(b0)
{
    float4x4 InverseViewProjection[2]  : packoffset(c0);
};

struct VSOutputBase
{
    float4 pos : SV_POSITION;
    float3 tex : TEXCOORD0;
};

struct PSInput : VSOutputBase
{
    uint ViewId : SV_RenderTargetArrayIndex;
};

struct GSInput : VSOutputBase
{
    uint ViewId : TEXCOORD15;
};