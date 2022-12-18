#pragma once
#include "FbxInclude/FbxInclude.h"

#include <vector>

//======================================.
//				頂点情報.
//======================================.
struct VERTEX
{
	DirectX::XMFLOAT3	Pos;		// 頂点座標.
	DirectX::XMFLOAT3	Normal;		// 法線.
	DirectX::XMFLOAT4	Color;		// 頂点カラー.
	DirectX::XMFLOAT2	UV;			// UV座標.
	DirectX::XMUINT4	BoneIndex;	// ボーン番号.
	DirectX::XMFLOAT4	BoneWeight;	// ボーンの重み.

	VERTEX()
		: Pos			( 0.0f, 0.0f, 0.0f )
		, Normal		( 0.0f, 0.0f, 0.0f )
		, Color			( 0.0f, 0.0f, 0.0f, 0.0f )
		, UV			( 0.0f, 0.0f )
		, BoneIndex		( 0, 0, 0, 0 )
		, BoneWeight	( 0.0f, 0.0f, 0.0f, 0.0f )
	{}
};
//======================================.
//			マテリアル情報.
//======================================.
struct MATERIAL
{
	DirectX::XMFLOAT4	Ambient;	// アンビエント.
	DirectX::XMFLOAT4	Diffuse;	// ディヒューズ.
	DirectX::XMFLOAT4	Specular;	// スペキュラ.
	std::string			Name;		// マテリアルの名前.
};
//======================================.
//			スキンデータ.
//======================================.
struct SkinData
{
	bool HasSkins;
	std::vector<FbxMatrix> InitBonePositions;	// 初期ボーン座標.
	std::vector<std::string> BoneName;			// ボーン名の取得.
};
//======================================.
//			メッシュデータ.
//======================================.
struct FBXMeshData
{
	MATERIAL			Material;			// マテリアル.
	std::vector<VERTEX>	Vertices;			// 頂点情報.
	std::vector<UINT>	Indices;			// 頂点インデックス.

	ID3D11Buffer*		pVertexBuffer;		// 頂点バッファリスト.
	ID3D11Buffer*		pIndexBuffer;		// インデックスバッファリスト.

	SkinData			Skin;				// スキン情報.

	int					PolygonVertexCount;	// ポリゴン頂点インデックス数.

	FBXMeshData()
		: Material				()
		, Vertices				()
		, Indices				()
		, pVertexBuffer			( nullptr )
		, pIndexBuffer			( nullptr )
		, PolygonVertexCount	( 0 )
	{}
	// 解放処理.
	void Release()
	{
		SAFE_RELEASE( pVertexBuffer );
		SAFE_RELEASE( pIndexBuffer );
	}
};