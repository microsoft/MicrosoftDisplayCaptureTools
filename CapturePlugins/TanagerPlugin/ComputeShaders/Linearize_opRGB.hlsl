RWTexture2D<float4> inputTexture : register(u0); // 16bpc uint 444 texture
RWTexture2D<float4> outputTexture : register(u1); // 16bpc uint 444 texture

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float4 color = inputTexture[DTid.xy];
    
    color.xyz = pow(color.xyz, 2.19921875);
    
    outputTexture[DTid.xy]= color;
}