RWTexture2D<float4> inputTexture : register(u0); // 16-bit float per channel texture
RWTexture2D<uint4> outputTextureRgba8 : register(u1); // RGBA8 texture
RWTexture2D<float4> outputTextureFp16 : register(u2); // 16-bit float per channel texture

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    // Copy the input buffer to the output, for 709 no translation is needed 
    float4 input = inputTexture[DTid.xy];
    outputTextureFp16[DTid.xy] = input;
    
    // Convert the input buffer to sRGB 8bpc
    float4 sRGB = input;
    
    sRGB.r = sRGB.r <= 0.0031308 ? sRGB.r * 12.92 : 1.055 * pow(sRGB.r, 1.0 / 2.4) - 0.055;
    sRGB.g = sRGB.g <= 0.0031308 ? sRGB.g * 12.92 : 1.055 * pow(sRGB.g, 1.0 / 2.4) - 0.055;
    sRGB.b = sRGB.b <= 0.0031308 ? sRGB.b * 12.92 : 1.055 * pow(sRGB.b, 1.0 / 2.4) - 0.055;

    outputTextureRgba8[DTid.xy] = (uint4) (sRGB * 255.0f);
}