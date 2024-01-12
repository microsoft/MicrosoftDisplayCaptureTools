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
    uint pixelIndexPrevLine = (DTid.y - 1) * width + DTid.x;
    uint pixelIndexNextLine = (DTid.y + 1) * width + DTid.x;
    uint intraPixel = DTid.x % 4;
    
    InputBufferStruct current = InBuf[pixelIndex / 4];

    // Previous Line
    InputBufferStruct prevLine = current;
    if (DTid.y != (uint) 0)
    {
        prevLine = InBuf[pixelIndexPrevLine / 4];
    }
    
    // Next Line
    InputBufferStruct nextLine = current;
    if (DTid.y < height - 1)
    {
        nextLine = InBuf[pixelIndexNextLine / 4];
    }
    
    
    color.xyz = uint3(
        (DTid.y % 2 == 0) ?
            ((intraPixel < 2) ?
                (current.upper & 0x000003FF) >> 0 :
                ((current.lower & 0x000000FF) << 2) | ((current.upper & 0xC0000000) >> 30)) :
            ((intraPixel < 2) ?
                (prevLine.upper & 0x000003FF) >> 0 :
                ((prevLine.lower & 0x000000FF) << 2) | ((prevLine.upper & 0xC0000000) >> 30)), // Cb
 
        ( intraPixel == 0) ? ((current.upper & 0x000FFC00) >> 10) :
        ((intraPixel == 1) ? ((current.upper & 0x3FF00000) >> 20) :
        ((intraPixel == 2) ? ((current.lower & 0x0003FF00) >>  8) :
                              (current.lower & 0x0FFC0000) >> 18)), // Y
 
        (DTid.y % 2 == 0) ?
            ((intraPixel < 2) ?
                (nextLine.upper & 0x000003FF) >> 0 :
                ((nextLine.lower & 0x000000FF) << 2) | ((nextLine.upper & 0xC0000000) >> 30)) :
            ((intraPixel < 2) ?
                (current.upper & 0x000003FF) >> 0 :
                ((current.lower & 0x000000FF) << 2) | ((current.upper & 0xC0000000) >> 30))); // Cr

    outputTexture[DTid.xy] = color;
}