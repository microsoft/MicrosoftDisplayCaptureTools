RWTexture2D<float4> inputTexture : register(u0); // 16bpc float 444 texture
RWTexture2D<float4> outputTexture : register(u1); // 16bpc float 444 texture

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float4x4 mat = float4x4(1.0000,  0.00000,  1.40199, 0.0000,
                            1.0000, -0.34411, -0.71410, 0.0000,
                            1.0000,  1.77198, -0.00013, 0.0000,
                            0.0000,  0.00000,  0.00000, 1.0000);
    
    // YCbCr values are read in CbYCr order, flip them to YCbCr.
    float4 color = inputTexture[DTid.xy].yxzw;
    color.yz -= 0.5f;
    outputTexture[DTid.xy] = mul(mat, color);
}