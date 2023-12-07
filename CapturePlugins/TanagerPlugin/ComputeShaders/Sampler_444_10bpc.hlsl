struct InputBufferStruct
{
    uint upper;
    uint lower;
};

StructuredBuffer<InputBufferStruct> InBuf : register(t0);
RWTexture2D<uint4> outputTextureRgba8 : register(u0); // RGBA8 texture
RWTexture2D<float4> outputTextureFp16 : register(u1); // 16-bit float per channel texture

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
        color = uint4((upper & 0x000003FF) >> 0, (upper & 0x000FFC00) >> 10, (upper & 0x3FF00000) >> 20, 1024);
    }
    else
    {
        uint lower = inputColor.lower;
        color = uint4((lower & 0x000003FF) >> 0, (lower & 0x000FFC00) >> 10, (lower & 0x3FF00000) >> 20, 1024);
    }

    outputTextureRgba8[DTid.xy] = color >> 2;
    outputTextureFp16[DTid.xy] = (float4) (color) / 1024.0;
}