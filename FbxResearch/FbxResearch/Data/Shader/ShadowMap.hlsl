cbuffer per_mesh : register( b0 )
{
	matrix g_LightWVP;	// ライトのワールド、ビュー、プロジェクション.	
};

// 頂点シェーダーの入力パラメータ.
struct VS_INPUT
{
	float4 Pos		: POSITION;		// 座標.
};

struct VS_OUTPUT
{
	float4 Pos			: POSITION;
};

VS_OUTPUT VS_Main( VS_INPUT input )
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	
	output.Pos = mul( input.Pos, g_LightWVP );
	
	return output;
}

float4 PS_Main( VS_OUTPUT input ) : SV_Target0
{
	float z = input.Pos.z / input.Pos.w;
	float4 color = float4( z, z, z, 1.0f );
	return color;
}