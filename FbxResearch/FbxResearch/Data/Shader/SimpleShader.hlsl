// テクスチャ.
Texture2D		g_Texture	: register(t0);
// サンプラ.
SamplerState	g_samLinear	: register(s0);

// 定数バッファ(CPU側からの値受け取り場).
cbuffer per_mesh : register( b0 )
{
	matrix	g_W;			// ワールド.
	matrix	g_WVP;			// ワールド、ビュー、プロジェクション.
	float4	g_IsAnimation;
};
cbuffer per_mesh : register( b1 )
{
	float4	g_vAmbient;		// アンビエント色(環境色).
	float4	g_vDiffuse;		// ディフーズ色(拡散反射色).
	float4	g_vSpecular;	// スペキュラ色(鏡面反射色).
};
cbuffer per_bone : register( b2 )
{
	matrix g_ConstBoneWorld[255];	// ボーン行列.
};

// スキン情報.
struct Skin
{
	float4 Pos;
	float4 Normal;
};
// 頂点シェーダーの入力パラメータ.
struct VS_INPUT
{
    float4	Pos			: POSITION; 
    float4	Normal		: NORMAL;   
    float4	Color		: COLOR;
	float2	UV			: TEXCOORD;
	uint4	Bones		: BONE_INDEX;
	float4	Weights		: BONE_WEIGHT;
};
// 頂点シェーダーの出力パラメータ.
struct VS_OUTPUT
{
    float4 Pos      : SV_POSITION;
    float4 Normal   : NORMAL;
    float4 Color    : COLOR;
	float2 UV		: TEXCOORD;
};

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
	
	if( g_IsAnimation.x == 1.0f ){
		Skin skin = SiknVert(input);
		output.Pos = mul(skin.Pos, g_WVP);
		output.Normal = normalize(mul(skin.Normal, g_W));
	} else {
		output.Pos = mul(input.Pos, g_WVP);
		output.Normal = normalize(mul(input.Normal, g_W));
	}
	output.Normal.w = 0.0f;
	
	float4 lightDir = { 0.4f, -0.5f, 0.0f, 0.0f };
	float4 viewDir = { 0.0f, 0.0f, 1.0f, 0.0f };
	lightDir = normalize(lightDir);
	viewDir = normalize(viewDir);
	float NL = saturate(dot(output.Normal, lightDir));
	
	float4 Reflect = normalize(2 * NL*output.Normal - lightDir);
	float4 Specular = pow(saturate(dot(Reflect, viewDir)), 4);
	
	output.Color = g_vDiffuse * NL + Specular * g_vSpecular;
	output.Color *= input.Color;
	output.UV = input.UV;
    return output;
}

struct PS_OUTPUT
{
	float4 Color	: SV_Target0;
	float4 Normal	: SV_Target1;
};

//--------------------------------.
// ピクセルシェーダ.
//--------------------------------.
PS_OUTPUT PS_Main(VS_OUTPUT input) : SV_Target0
{
	float4 color = { 1.0f, 0.0f, 0.0f, 1.0f };
	color = g_Texture.Sample(g_samLinear, input.UV) / 2.0f + input.Color / 2.0f;
	
	PS_OUTPUT output = (PS_OUTPUT)0;
	output.Color = color;
	output.Normal = input.Normal;
	
	return output;
}