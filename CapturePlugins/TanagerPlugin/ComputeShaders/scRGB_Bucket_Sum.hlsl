Texture2D<float4> textureA : register(t0);
Texture2D<float4> textureB : register(t1);
RWStructuredBuffer<float> outputSums : register(u0);

#define GROUP_SIZE 8

groupshared float2 groupSum[GROUP_SIZE];

[numthreads(GROUP_SIZE, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint3 DGid : SV_GroupID, uint DGIndex : SV_GroupIndex)
{
    float3 colorA = textureA.Load(DTid.xyz).xyz;
    float3 colorB = textureB.Load(DTid.xyz).xyz;
    
    float3 diff = pow(colorA - colorB, 2);
    
    groupSum[DGIndex].x = diff.x + diff.y + diff.z;
    
    GroupMemoryBarrierWithGroupSync();
    
    // At this point the groupSum array is filled with the squared differences for each group member.
    // They now need to be summed for each group.
    bool otherBank = false;
    
    [unroll]
    for (uint i = 1; i < GROUP_SIZE; i <<= 1)
    {
        uint shift = i << 1;
        
        if (otherBank)
            groupSum[DGIndex].x = groupSum[DGIndex / shift].y + groupSum[DGIndex / shift + i].y;
        else
            groupSum[DGIndex].y = groupSum[DGIndex / shift].x + groupSum[DGIndex / shift + i].x;
        
        otherBank = !otherBank;
        
        GroupMemoryBarrierWithGroupSync();
    }
    
    // At this point every thread's groupSum[DGIndex].y value is the sum of the squared differences for the group.
    // Put that value in the output buffer
    if (DGIndex == 0)
        outputSums[DGid.x] = groupSum[DGIndex].y;
}
