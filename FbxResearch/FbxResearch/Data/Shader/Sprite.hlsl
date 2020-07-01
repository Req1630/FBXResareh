//ｸﾞﾛｰﾊﾞﾙ変数.
//ｺﾝｽﾀﾝﾄﾊﾞｯﾌｧ.
//ｱﾌﾟﾘ側と同じﾊﾞｯﾌｧｻｲｽﾞになっている必要がある.
cbuffer per_mesh : register(b0)	//ﾚｼﾞｽﾀ番号.
{
	matrix	g_mWVP;		//ﾜｰﾙﾄﾞ,ﾋﾞｭｰ,ﾌﾟﾛｼﾞｪｸｼｮﾝの変換合成行列.
	float4	g_Color;	//色(RGBA:xyzw).
};

//頂点ｼｪｰﾀﾞの出力ﾊﾟﾗﾒｰﾀ.
//頂点ｼｪｰﾀﾞで制御した複数の値をﾋﾟｸｾﾙｼｪｰﾀﾞ側に渡すために用意している.
struct VS_OUTPUT
{
	float4	Pos		: SV_Position;	//座標(SV_:System-Value Semantics).
};

//頂点ｼｪｰﾀﾞ.
//主にﾓﾃﾞﾙの頂点の表示位置を決定する.
VS_OUTPUT VS_Main( float4 Pos : POSITION )
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Pos = mul( Pos, g_mWVP );

	return output;
}

//ﾋﾟｸｾﾙｼｪｰﾀﾞ.
//画面上に評されるﾋﾟｸｾﾙ(ﾄﾞｯﾄ1つ分)の色を決定する.
float4 PS_Main( VS_OUTPUT input ) : SV_Target
{
    float4 color = g_Color;

	return color;
}