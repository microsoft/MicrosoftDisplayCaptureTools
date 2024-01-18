RWTexture2D<float4> inputTexture : register(u0); // 16-bit float per channel texture
RWTexture2D<uint4> outputTextureRgba8 : register(u1); // RGBA8 texture
RWTexture2D<float4> outputTextureFp16 : register(u2); // 16-bit float per channel texture

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    // Copy the input buffer to the output, for SMPTE170M we use the following primaries:
    // R x = 0.630,  y = 0.340
    // G x = 0.310,  y = 0.595
    // B x = 0.155,  y = 0.070
    // W x = 0.3127, y = 0.3290
    float4x4 mat = float4x4( 0.939555291, 0.050172214, 0.010272495, 0.0,
                             0.017775713, 0.965792910, 0.016431377, 0.0,
                            -0.00162227, -0.004370698, 1.005992968, 0.0,
                             0.0,         0.0,         0.0,         1.0);
    
    float4 input = mul(mat, inputTexture[DTid.xy]);
    outputTextureFp16[DTid.xy] = input;
    
    // Convert the input buffer to sRGB 8bpc
    float4 sRGB = input;
    
    sRGB.r = sRGB.r <= 0.0031308 ? sRGB.r * 12.92 : 1.055 * pow(sRGB.r, 1.0 / 2.4) - 0.055;
    sRGB.g = sRGB.g <= 0.0031308 ? sRGB.g * 12.92 : 1.055 * pow(sRGB.g, 1.0 / 2.4) - 0.055;
    sRGB.b = sRGB.b <= 0.0031308 ? sRGB.b * 12.92 : 1.055 * pow(sRGB.b, 1.0 / 2.4) - 0.055;

    outputTextureRgba8[DTid.xy] = (uint4) (sRGB * 255.0f);
}