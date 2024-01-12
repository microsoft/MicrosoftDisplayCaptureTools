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
    
    
    uint Cb = (inputColor.upper & 0x000003FC) >> 2;
    uint Y_A = (inputColor.upper & 0x000FF000) >> 12;
    uint Cr = (inputColor.lower & 0x000000FF) >> 0;
    uint Y_B = (inputColor.lower & 0x0003FC00) >> 10;
    
    if (pixelIndex % 2 == 0)
    {
        color = uint4(Cb, Y_A, Cr, 255);
    }
    else
    {
        color = uint4(Cb, Y_B, Cr, 255);
    }

    outputTexture[DTid.xy] = color;
}