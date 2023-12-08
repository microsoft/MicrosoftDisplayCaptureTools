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
    
    /*
    uint upperLittleEndian =
        ((inputColor.lower & 0x000000FF) << 24) |
        ((inputColor.lower & 0x0000FF00) << 8) |
        ((inputColor.lower & 0x00FF0000) >> 8) |
        ((inputColor.lower & 0xFF000000) >> 24);
    
    uint lowerLittleEndian =
        ((inputColor.upper & 0x000000FF) << 24) |
        ((inputColor.upper & 0x0000FF00) << 8) |
        ((inputColor.upper & 0x00FF0000) >> 8) |
        ((inputColor.upper & 0xFF000000) >> 24);
    */

    // Data comes in as a 64-bit chunk with 2 adjacent pixel values encoded into it. Scanned left to right.
    // Data is now in the format of:
    
    //  Pad  B_B / B_Cb B_G / B_Y  B_R / B_Cr  A_B / A_Cb A_G / A_Y  A_R / A_Cr 
    // |----|----------|----------|-------- --|----------|----------|----------|
    // |   lower                           |     upper                         |
    //
    //
    // Pad = padding nibble
    // A = first pixel (RGB or CrYCb)
    // B = second pixel (RGB or CrYCb)
    
    if (pixelIndex % 2 == 0)
    {
        // Pixel A
        color.xyz = uint3(
            (inputColor.upper & 0x000003FF),        // R or Cr
            (inputColor.upper & 0x000FFC00) >> 10,  // G or Y
            (inputColor.upper & 0x3FF00000) >> 20); // B or Cb
    }
    else
    {
        // Pixel B
        color.xyz = uint3(
            ((inputColor.lower & 0x000000FF) << 2) | ((inputColor.upper & 0xC0000000) >> 30), // R or Cr
            (inputColor.lower & 0x0003FF00) >> 8,   // G or Y
            (inputColor.lower & 0x0FFC0000) >> 18); // B or Cb
    }

    outputTexture[DTid.xy] = color;
}