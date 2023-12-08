RWTexture2D<float4> inputTexture : register(u0); // 16bpc float 444 texture
RWTexture2D<float4> outputTexture : register(u1); // 16bpc float 444 texture

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float4x4 mat = float4x4(1.0000,  0.0000,  1.5748, 0.0000,
                            1.0000, -0.1873, -0.4681, 0.0000,
                            1.0000,  1.8556,  0.0000, 0.0000,
                            0.0000,  0.0000,  0.0000, 1.0000);
    
    // YCbCr values are read in CbYCr order, flip them to YCbCr.
    float4 color = inputTexture[DTid.xy].yxzw;
    outputTexture[DTid.xy] = mul(mat, color);
}