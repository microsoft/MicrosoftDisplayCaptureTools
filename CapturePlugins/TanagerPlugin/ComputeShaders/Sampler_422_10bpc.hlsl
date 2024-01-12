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
    uint4 color = uint4(0, 0, 0, 1023);

    uint width;
    uint height;
    outputTexture.GetDimensions(width, height);

    // Calculate the index in the structured buffer
    uint pixelIndex = DTid.y * width + DTid.x;
    InputBufferStruct inputColor = InBuf[pixelIndex / 2];
    
    uint Cb =  (inputColor.upper & 0x000003FF);
    uint Cr = ((inputColor.lower & 0x000000FF) << 2) | ((inputColor.upper & 0xC0000000) >> 30);
    uint Y_A = (inputColor.upper & 0x000FFC00) >> 10;
    uint Y_B = (inputColor.lower & 0x0003FF00) >> 8;
    
    if (pixelIndex % 2 == 0)
    {
        color.xyz = uint3(Cb, Y_A, Cr);
    }
    else
    {
        color.xyz = uint3(Cb, Y_B, Cr);
    }

    outputTexture[DTid.xy] = color;
}