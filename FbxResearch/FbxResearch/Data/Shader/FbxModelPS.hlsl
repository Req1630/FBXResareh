#include "FbxModelVS.hlsli"

// テクスチャ.
Texture2D		g_Texture	: register(t0);
// サンプラ.
SamplerState	g_samLinear	: register(s0);

struct PS_OUTPUT
{
	float4 Color	: SV_Target0;
	float4 Normal	: SV_Target1;
	float4 ZDepth	: SV_Target2;
};

//--------------------------------.
// ピクセルシェーダ.
//--------------------------------.
PS_OUTPUT PS_Main( VS_OUTPUT input )
{
	// 環境光　①.
    float4 ambient = g_vAmbient;

	// 拡散反射光 ②.
    float NL = saturate( dot( input.Normal, input.LightDir) );
    float4 diffuse = ( g_vDiffuse / 2 + g_Texture.Sample( g_samLinear, input.UV ) ) * NL;

	// 鏡面反射光 ③.
    float4 reflect = normalize( 1 * NL * input.Normal - input.LightDir );
    float4 specular =
		pow( saturate( dot( reflect, input.EyeDir )), 4 ) * g_vSpecular;

	// フォンモデル最終色　①②③の合計.
    float4 Color = ambient + diffuse + specular;
	
	PS_OUTPUT output = (PS_OUTPUT)0;
	output.Color = g_Texture.Sample( g_samLinear, input.UV );
//	output.Color = Color;
	output.Normal = input.Normal;
	output.ZDepth = input.ZDepth.z / input.ZDepth.w;
	
	return output;
}