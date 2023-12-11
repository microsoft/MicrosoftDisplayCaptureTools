Texture2D<float4> textureA : register(t0);
Texture2D<float4> textureB : register(t1);
RWTexture2D<float> outputSums : register(u0);

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint width;
    uint height;
    textureA.GetDimensions(width, height);
    
    float3 colorA = textureA.Load(DTid.xyz).xyz;
    float3 colorB = textureB.Load(DTid.xyz).xyz;
    
    float3 diff = pow(colorA - colorB, 2);
    
    outputSums[DTid.xy] = diff.x + diff.y + diff.z;
}
