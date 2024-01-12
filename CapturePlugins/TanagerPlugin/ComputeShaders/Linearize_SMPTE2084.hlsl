RWTexture2D<float4> inputTexture : register(u0); // 16bpc uint 444 texture
RWTexture2D<float4> outputTexture : register(u1); // 16bpc uint 444 texture

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float4 color = inputTexture[DTid.xy];
    
    color.xyz = pow(max(pow(color.xyz, (1 / 78.84375) - 0.8359375), 0) / 
        (18.8515625 - 18.6875 * pow(color.xyz, (1 / 78.84375))), 1 / 0.1593017578);
    
    outputTexture[DTid.xy]= color;
}