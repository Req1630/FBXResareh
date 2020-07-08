// �e�N�X�`��.
Texture2D		g_Texture	: register(t0);
// �T���v��.
SamplerState	g_samLinear	: register(s0);

// �萔�o�b�t�@(CPU������̒l�󂯎���).
cbuffer per_mesh : register( b0 )
{
	matrix	g_W;			// ���[���h.
	matrix	g_WVP;			// ���[���h�A�r���[�A�v���W�F�N�V����.
	matrix	g_LightWVP;
	float4	g_CameraPos;
	float4	g_LightDir;
	float4	g_IsAnimation;
};
cbuffer per_mesh : register( b1 )
{
	float4	g_vAmbient;		// �A���r�G���g�F(���F).
	float4	g_vDiffuse;		// �f�B�t�[�Y�F(�g�U���ːF).
	float4	g_vSpecular;	// �X�y�L�����F(���ʔ��ːF).
};
cbuffer per_bone : register( b2 )
{
	matrix g_ConstBoneWorld[255];	// �{�[���s��.
};

// �X�L�����.
struct Skin
{
	float4 Pos;
	float4 Normal;
};

// ���_�V�F�[�_�[�̓��̓p�����[�^.
struct VS_INPUT
{
    float4	Pos			: POSITION; 
    float4	Normal		: NORMAL;   
    float4	Color		: COLOR;
	float2	UV			: TEXCOORD;
	uint4	Bones		: BONE_INDEX;
	float4	Weights		: BONE_WEIGHT;
};

// ���_�V�F�[�_�[�̏o�̓p�����[�^.
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
// ���_�V�F�[�_.
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
	
	output.Color = input.Color;
	output.UV = input.UV;
	
	float4 pos = mul(input.Pos, g_W);
	output.EyeDir = normalize( g_CameraPos - pos );
	
	pos = mul(pos, g_LightWVP);
	output.ZDepth = pos;
	
	output.LightDir = normalize( g_LightDir );
	
    return output;
}

struct PS_OUTPUT
{
	float4 Color	: SV_Target0;
	float4 Normal	: SV_Target1;
	float4 ZDepth	: SV_Target2;
};

//--------------------------------.
// �s�N�Z���V�F�[�_.
//--------------------------------.
PS_OUTPUT PS_Main(VS_OUTPUT input) : SV_Target0
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