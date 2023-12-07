RWTexture2D<float4> inputTexture : register(u0); // 16bpc float 444 texture
RWTexture2D<float4> outputTexture : register(u1); // 16bpc float 444 texture

float3x3 mat = float3x3(1.0000,  0.0000,  1.5748,
                        1.0000, -0.1873, -0.4681,
                        1.0000,  1.8556,  0.0000);

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float4 color = inputTexture[DTid.xy];
    float3 result = mul(color.xyz, mat);
    outputTexture[DTid.xy] = float4(round(result), color.w);
}