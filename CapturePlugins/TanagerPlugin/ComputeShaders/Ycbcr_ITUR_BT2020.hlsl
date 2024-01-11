RWTexture2D<float4> inputTexture : register(u0); // 16bpc float 444 texture
RWTexture2D<float4> outputTexture : register(u1); // 16bpc float 444 texture

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float4x4 mat = float4x4(1.0, -0.000043128,  1.474587959, 0.0,
                            1.0, -0.164535603, -0.57133834,  0.0,
                            1.0,  1.881390696, -0.000115718, 0.0,
                            0.0,  0.0,          0.0,         1.0);
    
    // YCbCr values are read in CbYCr order, flip them to YCbCr.
    float4 color = inputTexture[DTid.xy].yxzw;
    color.yz -= 0.5f;
    outputTexture[DTid.xy] = mul(mat, color);
}
