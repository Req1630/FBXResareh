#include "FbxModelVS.hlsli"

// ボーン行列の計算.
Skin SiknVert( VS_INPUT input )
{
	Skin output = (Skin)0;
	matrix boneTransform = (matrix)0;
	
	boneTransform += mul( g_ConstBoneWorld[input.Bones.x], input.Weights.x );
	boneTransform += mul( g_ConstBoneWorld[input.Bones.y], input.Weights.y );
	boneTransform += mul( g_ConstBoneWorld[input.Bones.z], input.Weights.z );
	boneTransform += mul( g_ConstBoneWorld[input.Bones.w], input.Weights.w );
	
	output.Pos = mul( boneTransform, input.Pos );
	output.Normal = mul( boneTransform, input.Normal );
	output.Pos.x = -output.Pos.x;
	output.Normal.x = -output.Normal.x;
	
	return output;
};

//--------------------------------.
// 頂点シェーダ.
//--------------------------------.
VS_OUTPUT VS_Main( VS_INPUT input )
{
    VS_OUTPUT output = (VS_OUTPUT)0;
	
	Skin skin = SiknVert(input);
	output.Pos = mul(skin.Pos, g_WVP);
	output.Normal = normalize(mul(skin.Normal, g_W));
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