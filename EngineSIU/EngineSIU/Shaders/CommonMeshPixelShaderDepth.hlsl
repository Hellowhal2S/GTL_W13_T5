#include "ShaderRegisters.hlsl"

#define CAMERA_NEAR 0.1f
#define CAMERA_FAR  50000.0f

float4 mainPS(PS_INPUT_CommonMesh Input) : SV_Target
{
    float Depth = length(Input.WorldPosition - ViewWorldLocation);

    float Range = saturate((Depth - CAMERA_NEAR) / (CAMERA_FAR - CAMERA_NEAR));
    
    return float4(Range, Range, Range, 1.f);
}
