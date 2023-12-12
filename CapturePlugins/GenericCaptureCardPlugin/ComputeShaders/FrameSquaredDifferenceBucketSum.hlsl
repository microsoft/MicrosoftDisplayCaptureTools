Texture2D<float4> captureTex : register(t0);
Texture2D<float4> targetTex : register(t1);
RWTexture2D<float> outputSums : register(u0);

SamplerState downsampler
{
    Filter = MIN_LINEAR_MAG_POINT_MIP_LINEAR; // trilinear filtering
    AddressU = Clamp;
    AddressV = Clamp;
    AddressW = Clamp;
    BorderColor = float4(0, 0, 0, 0);
};

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{    
    uint targetWidth;
    uint targetHeight;
    targetTex.GetDimensions(targetWidth, targetHeight);
    
    // It's possible that the capture texture is a different size than the target texture for the 
    // GenericCaptureCards. To account for this we compute the scale factor from the dimentions and 
    // then use a Sampler.
    float2 uv = (float2) DTid.xy / float2(targetWidth - 1, targetHeight - 1);
    float3 captureColor = captureTex.SampleLevel(downsampler, uv, 0).xyz;
    
    float3 targetColor = targetTex.Load(DTid.xyz).xyz;
    
    float3 diff = pow(captureColor - targetColor, 2);
    
    outputSums[DTid.xy] = diff.x + diff.y + diff.z;
}
