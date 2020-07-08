#include "FbxModelVS.hlsli"

//--------------------------------.
// ���_�V�F�[�_.
//--------------------------------.
VS_OUTPUT VS_Main( VS_INPUT input )
{
    VS_OUTPUT output = (VS_OUTPUT)0;
	
	output.Pos = mul(input.Pos, g_WVP);
	output.Normal = normalize(mul(input.Normal, g_W));
	output.Normal.w = 0.0f;
	
	output.Color = input.Color;
	output.UV = input.UV;
	
	float4 pos = mul(input.Pos, g_W);
	output.EyeDir = normalize( g_CameraPos - pos );
	
	pos = mul(pos, g_LightWVP);
	output.ZDepth = pos;
	
	output.LightDir = normalize( g_LightDir );
	
    return output;
}