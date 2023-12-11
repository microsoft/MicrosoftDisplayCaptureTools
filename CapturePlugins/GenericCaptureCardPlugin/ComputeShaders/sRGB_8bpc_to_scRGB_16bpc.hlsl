Texture2D<uint4> inputTexture : register(t0); // 16bpc uint 444 texture
RWTexture2D<float4> outputTexture : register(u1); // 16bpc uint 444 texture

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float4 color = ((float4) inputTexture[DTid.xy]) / 255.0f;
    
    color.x = color.x < 0.081 ? color.x / 4.5 : pow((color.x + 0.099) / 1.099, 1.0 / 0.45);
    color.y = color.y < 0.081 ? color.y / 4.5 : pow((color.y + 0.099) / 1.099, 1.0 / 0.45);
    color.z = color.z < 0.081 ? color.z / 4.5 : pow((color.z + 0.099) / 1.099, 1.0 / 0.45);
    
    outputTexture[DTid.xy]= color;
}