cbuffer CS_CONSTANT_BUFFER : register(b0)
{
    uint PeakForBitDepth;
    uint A_min;
    uint A_levels;
    uint B_min;
    uint B_levels;
    uint C_min;
    uint C_levels;
    uint pad;
};

RWTexture2D<uint4> inputTexture : register(u0); // 16bpc uint 444 texture
RWTexture2D<float4> outputTexture : register(u1); // 16bpc float 444 texture

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint3 input = inputTexture[DTid.xy];
    input.x = clamp(input.x, A_min, PeakForBitDepth);
    input.y = clamp(input.y, B_min, PeakForBitDepth);
    input.z = clamp(input.z, C_min, PeakForBitDepth);
    
    float4 output;
    output.x = clamp(round((float)(input.x - A_min) * (float) PeakForBitDepth / (float) A_levels), 0, PeakForBitDepth);
    output.y = clamp(round((float)(input.y - B_min) * (float) PeakForBitDepth / (float) B_levels), 0, PeakForBitDepth);
    output.z = clamp(round((float)(input.z - C_min) * (float) PeakForBitDepth / (float) C_levels), 0, PeakForBitDepth);
    output.w = PeakForBitDepth;
    
    outputTexture[DTid.xy] = output / PeakForBitDepth;
}