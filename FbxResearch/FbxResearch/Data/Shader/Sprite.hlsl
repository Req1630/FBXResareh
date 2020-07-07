//ｸﾞﾛｰﾊﾞﾙ変数.
//ﾃｸｽﾁｬは、ﾚｼﾞｽﾀ t(n).
Texture2D g_Texture0 : register(t0);
Texture2D g_Texture1 : register(t1);
Texture2D g_Texture2 : register(t2);
//ｻﾝﾌﾟﾗは、ﾚｼﾞｽﾀ s(n).
SamplerState g_samLinear : register(s0);

//ｺﾝｽﾀﾝﾄﾊﾞｯﾌｧ.
cbuffer global : register(b0)
{
    matrix g_mW			: packoffset(c0); // ﾜｰﾙﾄﾞ行列.
	matrix g_mWVP		: packoffset(c4); // ﾜｰﾙﾄﾞ行列.
	float4 g_Color		: packoffset(c8); // カラー.
	float2 g_vUV		: packoffset(c9); // UV座標.
	float2 g_vViewPort	: packoffset(c10); // ビューポート幅.
};

//構造体.
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
	// UV座標をずらす.
    output.Tex.x += g_vUV.x;
    output.Tex.y += g_vUV.y;

    return output;
}


// 頂点シェーダ.
VS_OUTPUT VS_MainUI(
	float4 Pos : POSITION,
	float2 Tex : TEXCOORD)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    output.Pos = mul(Pos, g_mW);

	// スクリーン座標に合わせる計算,
	output.Pos.x = (output.Pos.x / g_vViewPort.x) * 2.0f - 1.0f;
	output.Pos.y = 1.0f - (output.Pos.y / g_vViewPort.y) * 2.0f;

    output.Tex = Tex;
	
    return output;
}

// ピクセルシェーダ.
float4 PS_Main(VS_OUTPUT input) : SV_Target
{
    float4 color = g_Texture0.Sample(g_samLinear, input.Tex);
    color.a *= g_Color.a;
    return color;
}

// ピクセルシェーダ.
float4 PS_MainLast(VS_OUTPUT input) : SV_Target
{
	float4 color = g_Texture0.Sample(g_samLinear, input.Tex);
	float4 normal = g_Texture1.Sample(g_samLinear, input.Tex);
	float4 zbuffer = g_Texture2.Sample(g_samLinear, input.Tex);
	
//	return color * normal * zbuffer;
	
	// 色の反転.
//	return float4(
//		float3(1.0f, 1.0f, 1.0f) - color.rgb, color.a );
	
	// 色の階調を落とす.
//	return float4(
//		color.rgb - fmod(color.rgb, 0.25f), color.a );
	
	// ぼかし.
//	float w, h, levels;
//	g_Texture0.GetDimensions( 0, w, h,levels );
	
//	const float s = 4.0f;
//	float dx = 1.0f / w;
//	float dy = 1.0f / h;
//	float4 ref = float4( 0.0f, 0.0f, 0.0f, 0.0f );
	
//	ref += g_Texture0.Sample( g_samLinear, input.Tex+float2(-s*dx, -s*dy));	// 左上.
//	ref += g_Texture0.Sample( g_samLinear, input.Tex+float2( 0*dx, -s*dy));	// 上.
//	ref += g_Texture0.Sample( g_samLinear, input.Tex+float2( s*dx, -s*dy));	// 右上.
//	ref += g_Texture0.Sample( g_samLinear, input.Tex+float2(-s*dx,  0*dy));	// 左.
//	ref += g_Texture0.Sample( g_samLinear, input.Tex+float2( 0*dx,  0*dy));	// 自分.
//	ref += g_Texture0.Sample( g_samLinear, input.Tex+float2( s*dx,  0*dy));	// 右.
//	ref += g_Texture0.Sample( g_samLinear, input.Tex+float2(-s*dx,  s*dy));	// 左下.
//	ref += g_Texture0.Sample( g_samLinear, input.Tex+float2( 0*dx,  s*dy));	// 下.
//	ref += g_Texture0.Sample( g_samLinear, input.Tex+float2( s*dx,  s*dy));	// 右下.

//	return ref / 9.0f;
	
	// エンボス加工.
//	float w, h, levels;
//	g_Texture0.GetDimensions( 0, w, h,levels );
	
//	const float s = 2.0f;
//	float dx = 1.0f / w;
//	float dy = 1.0f / h;
//	float4 ref = float4(0.0f, 0.0f, 0.0f, 0.0f);
	
//	ref += g_Texture0.Sample(g_samLinear, input.Tex + float2( -s * dx, -s * dy)) * 2; // 左上.
//	ref += g_Texture0.Sample(g_samLinear, input.Tex + float2(  0 * dx, -s * dy)) * 1; // 上.
//	ref += g_Texture0.Sample(g_samLinear, input.Tex + float2(  s * dx, -s * dy)) * 0; // 右上.
//	ref += g_Texture0.Sample(g_samLinear, input.Tex + float2( -s * dx,  0 * dy)) * 1; // 左.
//	ref += g_Texture0.Sample(g_samLinear, input.Tex + float2(  0 * dx,  0 * dy)) * 1; // 自分.
//	ref += g_Texture0.Sample(g_samLinear, input.Tex + float2(  s * dx,  0 * dy)) * -1; // 右.
//	ref += g_Texture0.Sample(g_samLinear, input.Tex + float2( -s * dx,  s * dy)) * 0; // 左下.
//	ref += g_Texture0.Sample(g_samLinear, input.Tex + float2(  0 * dx,  s * dy)) * -1; // 下.
//	ref += g_Texture0.Sample(g_samLinear, input.Tex + float2(  s * dx,  s * dy)) * -1; // 右下.

//	return ref;
	
	// シャープネス.
//	float w, h, levels;
//	g_Texture0.GetDimensions( 0, w, h,levels );
	
//	const float s = 2.0f;
//	float dx = 1.0f / w;
//	float dy = 1.0f / h;
//	float4 ref = float4(0.0f, 0.0f, 0.0f, 0.0f);
	
//	ref += g_Texture0.Sample(g_samLinear, input.Tex + float2( -s * dx, -s * dy)) * 0; // 左上.
//	ref += g_Texture0.Sample(g_samLinear, input.Tex + float2(  0 * dx, -s * dy)) * -1; // 上.
//	ref += g_Texture0.Sample(g_samLinear, input.Tex + float2(  s * dx, -s * dy)) * 0; // 右上.
//	ref += g_Texture0.Sample(g_samLinear, input.Tex + float2( -s * dx,  0 * dy)) * -1; // 左.
//	ref += g_Texture0.Sample(g_samLinear, input.Tex + float2(  0 * dx,  0 * dy)) * 5; // 自分.
//	ref += g_Texture0.Sample(g_samLinear, input.Tex + float2(  s * dx,  0 * dy)) * -1; // 右.
//	ref += g_Texture0.Sample(g_samLinear, input.Tex + float2( -s * dx,  s * dy)) * 0; // 左下.
//	ref += g_Texture0.Sample(g_samLinear, input.Tex + float2(  0 * dx,  s * dy)) * -1; // 下.
//	ref += g_Texture0.Sample(g_samLinear, input.Tex + float2(  s * dx,  s * dy)) * 0; // 右下.

//	return ref;
	
		// シャープネス.
	float w, h, levels;
	g_Texture0.GetDimensions( 0, w, h,levels );
	
	const float s = 4.0f;
	float dx = 1.0f / w;
	float dy = 1.0f / h;
	float4 ref = float4(0.0f, 0.0f, 0.0f, 0.0f);
	
	ref += g_Texture1.Sample(g_samLinear, input.Tex + float2(  0 * dx, -s * dy)) * -1; // 上.
	ref += g_Texture1.Sample(g_samLinear, input.Tex + float2( -s * dx,  0 * dy)) * -1; // 左.
	ref += g_Texture1.Sample(g_samLinear, input.Tex + float2(  0 * dx,  0 * dy)) * 4; // 自分.
	ref += g_Texture1.Sample(g_samLinear, input.Tex + float2(  s * dx,  0 * dy)) * -1; // 右.
	ref += g_Texture1.Sample(g_samLinear, input.Tex + float2(  0 * dx,  s * dy)) * -1; // 下.

	float Y = dot(ref.rgb, float3(0.5, 0.5, 0.5));
	Y = pow( 1.0f - Y, 10.0f );
	Y = step( 0.1f, Y );
	color *= Y;
	return color;

}