//��۰��ٕϐ�.
//�ݽ����ޯ̧.
//���ؑ��Ɠ����ޯ̧���ނɂȂ��Ă���K�v������.
cbuffer per_mesh : register(b0)	//ڼ޽��ԍ�.
{
	matrix	g_mWVP;		//ܰ���,�ޭ�,��ۼު���݂̕ϊ������s��.
	float4	g_Color;	//�F(RGBA:xyzw).
};

//���_����ނ̏o�����Ұ�.
//���_����ނŐ��䂵�������̒l���߸�ټ���ޑ��ɓn�����߂ɗp�ӂ��Ă���.
struct VS_OUTPUT
{
	float4	Pos		: SV_Position;	//���W(SV_:System-Value Semantics).
};

//���_�����.
//������ق̒��_�̕\���ʒu�����肷��.
VS_OUTPUT VS_Main( float4 Pos : POSITION )
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Pos = mul( Pos, g_mWVP );

	return output;
}

//�߸�ټ����.
//��ʏ�ɕ]������߸��(�ޯ�1��)�̐F�����肷��.
float4 PS_Main( VS_OUTPUT input ) : SV_Target
{
    float4 color = g_Color;

	return color;
}