#include "SkyBoxShared.hlsl"

TextureCube<float3> EnvironmentMap : register(t0);
SamplerState BasicSampler : register(s0);

float4 main(PSInput input) : SV_TARGET
{
    return float4(EnvironmentMap.Sample(BasicSampler, input.tex).rgb, 1.0);
}