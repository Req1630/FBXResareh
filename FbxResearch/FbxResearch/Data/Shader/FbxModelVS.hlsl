#include "FbxModelVS.hlsli"

//--------------------------------.
// 頂点シェーダ.
//--------------------------------.
VS_OUTPUT VS_Main( VS_INPUT input )
{
    VS_OUTPUT output = (VS_OUTPUT)0;
	
	output.Pos = mul(input.Pos, g_WVP);
	output.Normal.xyz = normalize(mul(input.Normal.xyz, (float3x3)g_W));
	output.Normal.w = 0.0f;

	
	output.Color = input.Color;
	output.UV = input.UV;
	
	float4 pos = mul(input.Pos, g_W);
	output.EyeDir = normalize( g_CameraPos - pos );
	
	output.ZDepth = mul(pos, g_LightWVP);
	output.ZDepth = output.ZDepth.z / output.ZDepth.w;
	
	output.LightDir = normalize( g_LightDir );
	
    return output;
}