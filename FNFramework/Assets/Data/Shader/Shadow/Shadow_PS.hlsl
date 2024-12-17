#include "Shadow.hlsli"

//------------------------------
// シェーダーリソース
//------------------------------
float4 main(VS_Output In) : SV_Target0
{
	float depth = In.pos.z / In.pos.w;

	return float4(depth, depth, depth, 1.0f);
}
