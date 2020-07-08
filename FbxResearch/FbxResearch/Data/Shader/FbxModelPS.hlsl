#include "FbxModelVS.hlsli"

// �e�N�X�`��.
Texture2D		g_Texture	: register(t0);
// �T���v��.
SamplerState	g_samLinear	: register(s0);

struct PS_OUTPUT
{
	float4 Color	: SV_Target0;
	float4 Normal	: SV_Target1;
	float4 ZDepth	: SV_Target2;
};

//--------------------------------.
// �s�N�Z���V�F�[�_.
//--------------------------------.
PS_OUTPUT PS_Main( VS_OUTPUT input )
{
	// �����@�@.
    float4 ambient = g_vAmbient;

	// �g�U���ˌ� �A.
    float NL = saturate( dot( input.Normal, input.LightDir) );
    float4 diffuse = ( g_vDiffuse / 2 + g_Texture.Sample( g_samLinear, input.UV ) ) * NL;

	// ���ʔ��ˌ� �B.
    float4 reflect = normalize( 1 * NL * input.Normal - input.LightDir );
    float4 specular =
		pow( saturate( dot( reflect, input.EyeDir )), 4 ) * g_vSpecular;

	// �t�H�����f���ŏI�F�@�@�A�B�̍��v.
    float4 Color = ambient + diffuse + specular;
	
	PS_OUTPUT output = (PS_OUTPUT)0;
	output.Color = g_Texture.Sample( g_samLinear, input.UV );
//	output.Color = Color;
	output.Normal = input.Normal;
	output.ZDepth = input.ZDepth.z / input.ZDepth.w;
	
	return output;
}