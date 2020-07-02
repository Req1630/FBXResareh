#pragma once

#include "..\..\Global.h"
#include "..\FbxInclude\FbxInclude.h"

#include <vector>
#include <unordered_map>
#include <set>

#include "..\FbxMeshData.h"
#include "..\FbxAnimation/FbxAnimationLoader.h"

class CFbxModel;	// Fbxモデルクラス.

class CFbxModelLoader
{
public:
	CFbxModelLoader();
	~CFbxModelLoader();

	//-----------------------------------------.
	//				作成.
	//-----------------------------------------.
	HRESULT Create( ID3D11DeviceContext* pContext11 );

	//-----------------------------------------.
	//				破壊.
	//-----------------------------------------.
	void Destroy();

	//-----------------------------------------.
	//			モデルの読み込み.
	//-----------------------------------------.
	HRESULT LoadModel( CFbxModel* pModelData, const char* fileName );
	
private:
	//-----------------------------------------.
	//			マテリアル系.
	//-----------------------------------------.

	// マテリアル取得.
	void GetMaterial( FbxMesh* pMesh, FBXMeshData& mesh, const char* fileName, std::unordered_map<std::string, ID3D11ShaderResourceView*>& textures );
	// テクスチャの読み込み.
	HRESULT LoadTexture( FbxFileTexture* texture, const char* fileName, const char* keyName, std::unordered_map<std::string, ID3D11ShaderResourceView*>& textures );

	//-----------------------------------------.
	//		メッシュデータの読み込み.
	//-----------------------------------------.

	// メッシュの読み込み.
	void LoadMesh( FbxMesh* pMesh, FBXMeshData& meshData );
	// インデックス情報読み込み.
	void LoadIndices( FbxMesh* pMesh, FBXMeshData& meshData );
	// 頂点情報読み込み.
	void LoadVertices( FbxMesh* pMesh, VERTEX& vertex, int ctrlPointIndex );
	// 法線情報読み込み.
	void LoadNormals( FbxMesh* pMesh, VERTEX& vertex, int ctrlPointIndex, int vertexCounter );
	// 頂点カラー読み込み.
	void LoadColors( FbxMesh* pMesh, VERTEX& vertex, int ctrlPointIndex, int vertexCounter );
	// UV情報読み込み.
	void LoadUV( FbxMesh* pMesh, VERTEX& vertex, int ctrlPointIndex, int texUVIndex, int uvLayer );
	// スキン情報読み込み.
	void LoadSkin( FbxMesh* pMesh, FBXMeshData& meshData, std::vector<std::vector<float>>& weights, std::vector<std::vector<int>>& bones );
	// ウェイトとボーンの設定.
	void SetBoneWeight( VERTEX& vertex, const std::vector<float>& weight, const std::vector<int>& bone );

	//-----------------------------------------.
	//		描画用データの作成系.
	//-----------------------------------------.

	// 頂点バッファ作成.
	HRESULT CreateVertexBuffers( FBXMeshData& meshData );
	// インデックスバッファ作成.
	HRESULT CreateIndexBuffers( FBXMeshData& meshData );
private:
	/***************************************
	*			DirectX11.
	***************************************/
	ID3D11Device*			m_pDevice11;	// デバイス11.
	ID3D11DeviceContext*	m_pContext11;	// コンテキスト11.
	/***************************************
	*			FBX SDK.
	***************************************/
	FbxManager* m_pFbxManager;	// FBXマネージャー.
	FbxScene*	m_pFbxScene;	// FBXシーンオブジェクト.
	std::unique_ptr<CFbxAnimationLoader> m_pAnimLoader;	// アニメーション読み込みクラス.
	std::vector<FBXMeshClusterData>	m_MeshClusterData;	// メッシュのクラスター情報.

	std::vector<FbxSkeleton*>	m_Skeletons;	// スケルトン情報.
};