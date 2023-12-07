RWTexture2D<float4> inputTexture : register(u0); // 16-bit float per channel texture
RWTexture2D<uint4> outputTextureRgba8 : register(u1); // RGBA8 texture
RWTexture2D<float4> outputTextureFp16 : register(u2); // 16-bit float per channel texture

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float4 input = inputTexture[DTid.xy];
    outputTextureRgba8[DTid.xy] = (uint4) (input * 255.0);
    outputTextureFp16[DTid.xy] = input;
}