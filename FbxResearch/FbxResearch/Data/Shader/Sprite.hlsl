//��۰��ٕϐ�.
//ø����́Aڼ޽� t(n).
Texture2D g_Texture0 : register(t0);
Texture2D g_Texture1 : register(t1);
Texture2D g_Texture2 : register(t2);
//����ׂ́Aڼ޽� s(n).
SamplerState g_samLinear : register(s0);

//�ݽ����ޯ̧.
cbuffer global : register(b0)
{
    matrix g_mW			: packoffset(c0); // ܰ��ލs��.
	matrix g_mWVP		: packoffset(c4); // ܰ��ލs��.
	float4 g_Color		: packoffset(c8); // �J���[.
	float2 g_vUV		: packoffset(c9); // UV���W.
	float2 g_vViewPort	: packoffset(c10); // �r���[�|�[�g��.
};

//�\����.
struct VS_OUTPUT
{
    float4 Pos : SV_Position;
    float4 Color : COLOR;
    float2 Tex : TEXCOORD;
};

VS_OUTPUT VS_Main(
	float4 Pos : POSITION,
	float2 Tex : TEXCOORD)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    output.Pos = mul(Pos, g_mWVP);
    output.Tex = Tex;
	// UV���W�����炷.
    output.Tex.x += g_vUV.x;
    output.Tex.y += g_vUV.y;

    return output;
}


// ���_�V�F�[�_.
VS_OUTPUT VS_MainUI(
	float4 Pos : POSITION,
	float2 Tex : TEXCOORD)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    output.Pos = mul(Pos, g_mW);

	// �X�N���[�����W�ɍ��킹��v�Z,
	output.Pos.x = (output.Pos.x / g_vViewPort.x) * 2.0f - 1.0f;
	output.Pos.y = 1.0f - (output.Pos.y / g_vViewPort.y) * 2.0f;

    output.Tex = Tex;
	
    return output;
}

// �s�N�Z���V�F�[�_.
float4 PS_Main(VS_OUTPUT input) : SV_Target
{
    float4 color = g_Texture0.Sample(g_samLinear, input.Tex);
    color.a *= g_Color.a;
    return color;
}

// �s�N�Z���V�F�[�_.
float4 PS_MainLast(VS_OUTPUT input) : SV_Target
{
	float4 color = g_Texture0.Sample(g_samLinear, input.Tex);
	float4 normal = g_Texture1.Sample(g_samLinear, input.Tex);
	float4 zbuffer = g_Texture2.Sample(g_samLinear, input.Tex);
	
//	return color * normal * zbuffer;
	
	// �F�̔��].
//	return float4(
//		float3(1.0f, 1.0f, 1.0f) - color.rgb, color.a );
	
	// �F�̊K���𗎂Ƃ�.
//	return float4(
//		color.rgb - fmod(color.rgb, 0.25f), color.a );
	
	// �ڂ���.
//	float w, h, levels;
//	g_Texture0.GetDimensions( 0, w, h,levels );
	
//	const float s = 4.0f;
//	float dx = 1.0f / w;
//	float dy = 1.0f / h;
//	float4 ref = float4( 0.0f, 0.0f, 0.0f, 0.0f );
	
//	ref += g_Texture0.Sample( g_samLinear, input.Tex+float2(-s*dx, -s*dy));	// ����.
//	ref += g_Texture0.Sample( g_samLinear, input.Tex+float2( 0*dx, -s*dy));	// ��.
//	ref += g_Texture0.Sample( g_samLinear, input.Tex+float2( s*dx, -s*dy));	// �E��.
//	ref += g_Texture0.Sample( g_samLinear, input.Tex+float2(-s*dx,  0*dy));	// ��.
//	ref += g_Texture0.Sample( g_samLinear, input.Tex+float2( 0*dx,  0*dy));	// ����.
//	ref += g_Texture0.Sample( g_samLinear, input.Tex+float2( s*dx,  0*dy));	// �E.
//	ref += g_Texture0.Sample( g_samLinear, input.Tex+float2(-s*dx,  s*dy));	// ����.
//	ref += g_Texture0.Sample( g_samLinear, input.Tex+float2( 0*dx,  s*dy));	// ��.
//	ref += g_Texture0.Sample( g_samLinear, input.Tex+float2( s*dx,  s*dy));	// �E��.

//	return ref / 9.0f;
	
	// �G���{�X���H.
//	float w, h, levels;
//	g_Texture0.GetDimensions( 0, w, h,levels );
	
//	const float s = 2.0f;
//	float dx = 1.0f / w;
//	float dy = 1.0f / h;
//	float4 ref = float4(0.0f, 0.0f, 0.0f, 0.0f);
	
//	ref += g_Texture0.Sample(g_samLinear, input.Tex + float2( -s * dx, -s * dy)) * 2; // ����.
//	ref += g_Texture0.Sample(g_samLinear, input.Tex + float2(  0 * dx, -s * dy)) * 1; // ��.
//	ref += g_Texture0.Sample(g_samLinear, input.Tex + float2(  s * dx, -s * dy)) * 0; // �E��.
//	ref += g_Texture0.Sample(g_samLinear, input.Tex + float2( -s * dx,  0 * dy)) * 1; // ��.
//	ref += g_Texture0.Sample(g_samLinear, input.Tex + float2(  0 * dx,  0 * dy)) * 1; // ����.
//	ref += g_Texture0.Sample(g_samLinear, input.Tex + float2(  s * dx,  0 * dy)) * -1; // �E.
//	ref += g_Texture0.Sample(g_samLinear, input.Tex + float2( -s * dx,  s * dy)) * 0; // ����.
//	ref += g_Texture0.Sample(g_samLinear, input.Tex + float2(  0 * dx,  s * dy)) * -1; // ��.
//	ref += g_Texture0.Sample(g_samLinear, input.Tex + float2(  s * dx,  s * dy)) * -1; // �E��.

//	return ref;
	
	// �V���[�v�l�X.
//	float w, h, levels;
//	g_Texture0.GetDimensions( 0, w, h,levels );
	
//	const float s = 2.0f;
//	float dx = 1.0f / w;
//	float dy = 1.0f / h;
//	float4 ref = float4(0.0f, 0.0f, 0.0f, 0.0f);
	
//	ref += g_Texture0.Sample(g_samLinear, input.Tex + float2( -s * dx, -s * dy)) * 0; // ����.
//	ref += g_Texture0.Sample(g_samLinear, input.Tex + float2(  0 * dx, -s * dy)) * -1; // ��.
//	ref += g_Texture0.Sample(g_samLinear, input.Tex + float2(  s * dx, -s * dy)) * 0; // �E��.
//	ref += g_Texture0.Sample(g_samLinear, input.Tex + float2( -s * dx,  0 * dy)) * -1; // ��.
//	ref += g_Texture0.Sample(g_samLinear, input.Tex + float2(  0 * dx,  0 * dy)) * 5; // ����.
//	ref += g_Texture0.Sample(g_samLinear, input.Tex + float2(  s * dx,  0 * dy)) * -1; // �E.
//	ref += g_Texture0.Sample(g_samLinear, input.Tex + float2( -s * dx,  s * dy)) * 0; // ����.
//	ref += g_Texture0.Sample(g_samLinear, input.Tex + float2(  0 * dx,  s * dy)) * -1; // ��.
//	ref += g_Texture0.Sample(g_samLinear, input.Tex + float2(  s * dx,  s * dy)) * 0; // �E��.

//	return ref;
	
		// �V���[�v�l�X.
	float w, h, levels;
	g_Texture0.GetDimensions( 0, w, h,levels );
	
	const float s = 4.0f;
	float dx = 1.0f / w;
	float dy = 1.0f / h;
	float4 ref = float4(0.0f, 0.0f, 0.0f, 0.0f);
	
	ref += g_Texture1.Sample(g_samLinear, input.Tex + float2(  0 * dx, -s * dy)) * -1; // ��.
	ref += g_Texture1.Sample(g_samLinear, input.Tex + float2( -s * dx,  0 * dy)) * -1; // ��.
	ref += g_Texture1.Sample(g_samLinear, input.Tex + float2(  0 * dx,  0 * dy)) * 4; // ����.
	ref += g_Texture1.Sample(g_samLinear, input.Tex + float2(  s * dx,  0 * dy)) * -1; // �E.
	ref += g_Texture1.Sample(g_samLinear, input.Tex + float2(  0 * dx,  s * dy)) * -1; // ��.

	float Y = dot(ref.rgb, float3(0.5, 0.5, 0.5));
	Y = pow( 1.0f - Y, 10.0f );
	Y = step( 0.1f, Y );
	color *= Y;
	return color;

}