#include "inc_GenericShapeShader.hlsli"

float4 main(VSOutput In) : SV_TARGET
{
    return In.col;
}
