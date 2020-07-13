#ifndef FBX_MODEL_LOADER_H
#define FBX_MODEL_LOADER_H

#include "..\FbxInclude\FbxInclude.h"
#include "..\FbxMeshData.h"
#include "..\FbxAnimation\FbxAnimationLoader.h"

#include <vector>
#include <unordered_map>
#include <set>

class CFbxModel;	// Fbxモデルクラス.

/************************************************************************
	CFbxModelを読み込むためのクラス.
-------------------------------------------------------------------------
*	モデルは多角ポリゴンでも行けるが、読み込みが遅くなるので、
*		グラフィッカーに頼んで三角ポリゴンで出力してもらった方が良い.
*	モデルとテクスチャは同じ階層に入れて置くこと.
***/
class CFbxModelLoader
{
public:
	CFbxModelLoader();
	~CFbxModelLoader();

	//-----------------------------------------.
	//		作成.
	//-----------------------------------------.
	HRESULT Create( ID3D11Device* pDevice );

	//-----------------------------------------.
	//		破壊.
	//-----------------------------------------.
	void Destroy();

	//-----------------------------------------.
	//		モデルの読み込み.
	//-----------------------------------------.
	HRESULT LoadModel( CFbxModel* pModelData, const char* fileName );
	
private:
	//-----------------------------------------.
	//		マテリアルの読み込み.
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
	void LoadSkin( FbxMesh* pMesh, SkinData& skinData, std::vector<std::vector<float>>& weights, std::vector<std::vector<int>>& bones );
	// ウェイトとボーンの設定.
	void SetBoneWeight( VERTEX& vertex, const std::vector<float>& weight, const std::vector<int>& bone );

	//-----------------------------------------.
	//		描画用データの作成.
	//-----------------------------------------.

	// 頂点バッファ作成.
	HRESULT CreateVertexBuffers( FBXMeshData& meshData );
	// インデックスバッファ作成.
	HRESULT CreateIndexBuffers( FBXMeshData& meshData );

	//-----------------------------------------.
	//		ボーン名系.
	//-----------------------------------------.

	// ボーン名リストの作成.
	void CreateBoneNameList( CFbxModel* pModelData, const char* fileName );
	// ボーンリストをテキストで書き込み.
	void WritingBoneNameList( std::map<std::string, std::pair<int, int>>& boneList, const char* fileName );

private:
	/***************************************
	*			DirectX11.
	***************************************/
	ID3D11Device*			m_pDevice11;	// デバイス11.

	/***************************************
	*			FBX SDK.
	***************************************/
	FbxManager* m_pFbxManager;	// FBXマネージャー.
	FbxScene*	m_pFbxScene;	// FBXシーンオブジェクト.
	std::vector<FBXMeshClusterData>	m_MeshClusterData;	// メッシュのクラスター情報.
	std::vector<FbxSkeleton*>		m_Skeletons;		// スケルトン情報.
};

#endif	// #ifndef FBX_MODEL_LOADER_H.