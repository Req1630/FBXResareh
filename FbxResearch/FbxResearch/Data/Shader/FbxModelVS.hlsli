// �萔�o�b�t�@(CPU������̒l�󂯎���).
cbuffer per_mesh : register( b0 )
{
	matrix	g_W;			// ���[���h.
	matrix	g_WVP;			// ���[���h�A�r���[�A�v���W�F�N�V����.
	matrix	g_LightWVP;
	float4	g_CameraPos;
	float4	g_LightDir;
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
    float4	Pos			: POSITION;		// ���W.
    float4	Normal		: NORMAL;		// �@��.
    float4	Color		: COLOR;		// ���_�J���[.
	float2	UV			: TEXCOORD;		// UV���W.
	uint4	Bones		: BONE_INDEX;	// �{�[���̃C���f�b�N�X.
	float4	Weights		: BONE_WEIGHT;	// �{�[���̃E�F�C�g.
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