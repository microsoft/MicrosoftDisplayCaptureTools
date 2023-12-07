struct InputBufferStruct
{
    uint upper;
    uint lower;
};

StructuredBuffer<InputBufferStruct> InBuf : register(t0);
RWTexture2D<uint4> outputTexture : register(u0); // 16bpc uint 444 texture

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint width;
    uint height;
    outputTexture.GetDimensions(width, height);
    
    uint4 color = uint4(0, 0, 0, 255);

    // Calculate the index in the structured buffer
    uint pixelIndex = DTid.y * width + DTid.x;
    InputBufferStruct inputColor = InBuf[pixelIndex / 2];
    if (pixelIndex % 2 == 0)
    {
        uint upper = inputColor.upper;
        color = uint4((upper & 0x000003FC) >> 2, (upper & 0x000FF000) >> 12, (upper & 0x3FC00000) >> 22, 255);
    }
    else
    {
        uint lower = inputColor.lower;
        color = uint4((lower & 0x000000FF) >> 0, (lower & 0x0003FC00) >> 10, (lower & 0x0FF00000) >> 20, 255);
    }

    outputTexture[DTid.xy] = color;
}