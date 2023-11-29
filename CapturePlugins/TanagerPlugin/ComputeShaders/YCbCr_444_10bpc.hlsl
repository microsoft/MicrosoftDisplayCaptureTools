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
    1.0,  0,       1.5748,
    1.0, -0.1873, -0.4681,
    1.0,  1.8556,  0
};

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint width;
    uint height;
    outputTextureRgba8.GetDimensions(width, height);
    
    uint4 color = uint4(0, 0, 0, 1024);

    // Calculate the index in the structured buffer
    uint pixelIndex = DTid.y * width + DTid.x;
    InputBufferStruct inputColor = InBuf[pixelIndex / 2];
    if (pixelIndex % 2 == 0)
    {
        uint upper = inputColor.upper;
        color = uint4((upper & 0x000FFC00) >> 10, (upper & 0xFFC00000) >> 20, (upper & 0x000003FF) >> 0, 1024);
    }
    else
    {
        uint lower = inputColor.lower;
        color = uint4((lower & 0x000FFC00) >> 10, (lower & 0xFFC00000) >> 20, (lower & 0x000003FF) >> 0, 1024);
    }

    uint3 yuv = color.xyz;
    uint3 rgb = mul(YCbCrToRGB709, yuv);
    outputTextureRgba8[DTid.xy] = uint4(rgb, 1024) >> 2;
    outputTextureFp16[DTid.xy] = (float4) (color) / 1024.0;
}