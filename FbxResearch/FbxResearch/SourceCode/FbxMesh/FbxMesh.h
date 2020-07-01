#ifndef FBX_MESH_H
#define FBX_MESH_H

#include "..\Global.h"
#include "FbxInclude\FbxInclude.h"

#include <vector>
#include <map>
#include <set>

#include "FbxMeshData.h"
#include "FbxAnimationData.h"
#include "FbxAnimation/FbxAnimationLoader.h"
#include "FbxAnimationController.h"

/***********************************
*	FBXモデルクラス.
**/
class CFbxMesh
{
	// シェーダー名.
	const wchar_t* SHADER_NAME = L"Data\\Shader\\SimpleShader.hlsl";

	/***************************************
	*			↓	構造体　↓.
	***************************************/
	//======================================.
	//	コンスタントバッファ(メッシュ毎).
	//======================================.
	struct CBUFFER_PER_MESH
	{
		DirectX::XMMATRIX mW;	// ワールド行列.
		DirectX::XMMATRIX mWVP;	// World,View,Proj の合成変換行列.
		DirectX::XMFLOAT4 IsAnimation;
	};
	//======================================.
	// コンスタントバッファ(マテリアル毎).
	//======================================.
	struct CBUFFER_PER_MATERIAL
	{
		DirectX::XMFLOAT4	Ambient;	// アンビエント.
		DirectX::XMFLOAT4	Diffuse;	// ディヒューズ.
		DirectX::XMFLOAT4	Specular;	// スペキュラ.
	};
	//======================================.
	// コンスタントバッファ(ボーン毎).
	//======================================.
	struct CBUFFER_PER_BONE
	{
		DirectX::XMMATRIX Bone[255];
		CBUFFER_PER_BONE()
		{
			// 行列の初期化.
			for( int i = 0; i < 255; i++ ){
				Bone[i] = DirectX::XMMatrixIdentity();
			}
		}
	};

public:
	CFbxMesh();
	~CFbxMesh();

	//-----------------------------------------.
	//				作成.
	//-----------------------------------------.
	HRESULT Create( ID3D11DeviceContext* pContext11, const char* fileName );

	//-----------------------------------------.
	//				破壊.
	//-----------------------------------------.
	void Destroy();

	//-----------------------------------------.
	//				描画.
	//-----------------------------------------.
	void Render( const DirectX::XMMATRIX& view, 
				 const DirectX::XMMATRIX& proj,
				 CFbxAnimationController* pAc = nullptr );

	//-----------------------------------------.
	//			パラメータ取得系.
	//-----------------------------------------.

	// ワールド行列を取得.
	DirectX::XMMATRIX GetWorldMatrix();

	// アニメーションコントローラーの取得.
	CFbxAnimationController GetAnimationController()
	{
		if( m_pAc != nullptr ) return *m_pAc;
		return CFbxAnimationController();
	}

	//-----------------------------------------.
	//			パラメータ設定系.
	//-----------------------------------------.

	// 座標の設定.
	void SetPosition( const DirectX::XMFLOAT3& pos ){ m_Position = pos; }
	// 回転の設定.
	void SetRotation( const DirectX::XMFLOAT3& rot ){ m_Rotation = rot; }
	// 大きさの設定.
	void SetScale( const DirectX::XMFLOAT3& scale ){ m_Scale = scale; }
	void SetScale( const float& scale ){ m_Scale = { scale, scale, scale }; }

	// アニメーション速度の設定.
	void SetAnimSpeed( const double& speed )
	{
		if( m_pAc == nullptr ) return;
		m_pAc->SetAnimSpeed( speed );
	}
	// アニメーション速度の設定.
	void ChangeAnimation( int& index )
	{
		if( m_pAc == nullptr ) return;
		m_pAc->ChangeAnimation( index );
	}

private:
	//-----------------------------------------.
	//		描画用データの作成系.
	//-----------------------------------------.

	// バッファの作成.
	HRESULT CreateBuffer();
	// 頂点バッファ作成.
	HRESULT CreateVertexBuffers( FBXMeshData& meshData );
	// インデックスバッファ作成.
	HRESULT CreateIndexBuffers( FBXMeshData& meshData );
	// サンプラー作成.
	HRESULT CreateSampler();
	// コンスタントバッファ作成.
	HRESULT CreateConstantBuffer( const size_t& byte, ID3D11Buffer** buffer );

	// シェーダーの作成.
	HRESULT CreateShader();

	//-----------------------------------------.
	//			マテリアル系.
	//-----------------------------------------.

	// マテリアル取得.
	void GetMaterial( FbxMesh* pMesh, FBXMeshData& mesh, const char* fileName );
	// テクスチャの読み込み.
	HRESULT LoadTexture( FbxFileTexture* texture, const char* fileName, const char* keyName );

	//-----------------------------------------.
	//		アニメーション系.
	//-----------------------------------------.
	 
	// アニメーション用の行列計算.
	void AnimMatrixCalculation( const int& meahNo, FBXMeshData& meshData, CFbxAnimationController* pAc );

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
	//			ノードの処理.
	//-----------------------------------------.

	// ノードをたどる再起関数.
	void RecurrenceNode( FbxNode* pNode );
	// ノードタイプ別取得関数.
	void GetDataByNodeType( FbxNode* pNode );

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

	/***************************************
	*			シェーダー.
	***************************************/
	ID3D11VertexShader*	m_pVertexShader;		// 頂点シェーダー.
	ID3D11PixelShader*	m_pPixelShader;			// ピクセルシェーダー.

	/***************************************
	*			描画用データ.
	***************************************/
	ID3D11InputLayout*	m_pVertexLayout;		// 頂点レイアウト.
	ID3D11Buffer*		m_pCBufferPerMesh;		// コンスタントバッファ.
	ID3D11Buffer*		m_pCBufferPerMaterial;	// コンスタントバッファ.
	ID3D11Buffer*		m_pCBufferPerBone;		// コンスタントバッファ.
	ID3D11SamplerState*	m_pSampleLinear;		// サンプラ:テクスチャに各種フィルタをかける.
	std::vector<FBXMeshData>	m_MeshData;		// メッシュデータ.
	std::vector<FbxSkeleton*>	m_Skeletons;	// スケルトン情報.
	std::map<std::string, ID3D11ShaderResourceView*> m_Textures;	// テクスチャリスト.

	CFbxAnimationController* m_pAc;	// アニメーションコントローラー.
	std::unique_ptr<CFbxAnimationLoader> m_pAnimLoader;
	std::vector<FBXMeshClusterData>	m_MeshClusterData;

	/***************************************
	*			パラメータ.
	***************************************/
	DirectX::XMFLOAT3 m_Position;	// 座標.
	DirectX::XMFLOAT3 m_Rotation;	// 回転.
	DirectX::XMFLOAT3 m_Scale;		// 大きさ.
};

#endif	// #ifndef FBX_MESH_H.