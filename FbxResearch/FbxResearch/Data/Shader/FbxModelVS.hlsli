// 定数バッファ(CPU側からの値受け取り場).
cbuffer per_mesh : register( b0 )
{
	matrix	g_W;			// ワールド.
	matrix	g_WVP;			// ワールド、ビュー、プロジェクション.
	matrix	g_LightWVP;
	float4	g_CameraPos;
	float4	g_LightDir;
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
    float4	Pos			: POSITION;		// 座標.
    float4	Normal		: NORMAL;		// 法線.
    float4	Color		: COLOR;		// 頂点カラー.
	float2	UV			: TEXCOORD;		// UV座標.
	uint4	Bones		: BONE_INDEX;	// ボーンのインデックス.
	float4	Weights		: BONE_WEIGHT;	// ボーンのウェイト.
};

// 頂点シェーダーの出力パラメータ.
struct VS_OUTPUT
{
    float4 Pos      : SV_POSITION;
    float4 Normal   : NORMAL;
    float4 Color    : COLOR;
	float2 UV		: TEXCOORD0;
	float4 ZDepth	: TEXCOORD1;
	float4 LightDir	: TEXCOORD2;
	float4 EyeDir	: TEXCOORD3;
};