struct InputBufferStruct
{
    uint upper;
    uint lower;
};

StructuredBuffer<InputBufferStruct> InBuf : register(t0);
RWTexture2D<uint4> outputTextureRgba8 : register(u0); // RGBA8 texture
RWTexture2D<float4> outputTextureFp16 : register(u1); // 16-bit float per channel texture

float3x3 YCbCrToRGB709 =
{
    1.0, 0, 1.5748,
    1.0, -0.1873, -0.4681,
    1.0, 1.8556, 0
};

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint width;
    uint height;
    outputTextureRgba8.GetDimensions(width, height);
    
    uint4 color = uint4(0, 0, 0, 255);

    // Calculate the index in the structured buffer
    uint pixelIndex = DTid.y * width + DTid.x;
    InputBufferStruct inputColor = InBuf[pixelIndex / 2];
    if (pixelIndex % 2 == 0)
    {
        uint upper = inputColor.upper;
        color = uint4((upper & 0x000FF000) >> 12, (upper & 0x3FC00000) >> 22, (upper & 0x000003FC) >> 2, 255);
    }
    else
    {
        uint lower = inputColor.lower;
        color = uint4((lower & 0x0003FC00) >> 10, (lower & 0x0FF00000) >> 20, (lower & 0x000000FF) >> 0, 255);
    }

    uint3 yuv = color.xyz;
    float3 rgb = mul(YCbCrToRGB709, (float3)yuv);
    outputTextureRgba8[DTid.xy] = uint4(rgb, 255);
    outputTextureFp16[DTid.xy] = float4(rgb / 255.0, 1.0);
}