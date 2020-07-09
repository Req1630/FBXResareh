#pragma once

#include "..\FbxInclude\FbxInclude.h"
#include "..\FbxMeshData.h"
#include "..\FbxAnimation\FbxAnimationData.h"

#include <unordered_map>

class CFbxAnimationController;

class CFbxModel
{
public:
	CFbxModel();
	~CFbxModel();

	//-----------------------------------------.
	//				Fbxモデル系.
	//-----------------------------------------.

	// メッシュデータの取得.
	std::vector<FBXMeshData>& GetMeshData();
	// メッシュデータのリサイズ.
	void ReSizeMeshData( const int& size );
	// テクスチャの取得.
	std::unordered_map<std::string, ID3D11ShaderResourceView*>& GetTextures();
	// アニメーションコントローラーの取得.
	CFbxAnimationController* GetPtrAC();
	// アニメーションデータの設定.
	void SetAnimationData( const SAnimationDataList& animDataList );

	//-----------------------------------------.
	//			パラメータ取得.
	//-----------------------------------------.

	// ワールド行列を取得.
	DirectX::XMMATRIX GetWorldMatrix();

	// アニメーションコントローラーの取得.
	CFbxAnimationController GetAnimationController();

	//-----------------------------------------.
	//			パラメータ設定.
	//-----------------------------------------.

	// 座標の設定.
	void SetPosition( const DirectX::XMFLOAT3& pos );
	// 回転の設定.
	void SetRotation( const DirectX::XMFLOAT3& rot );
	// 大きさの設定.
	void SetScale( const DirectX::XMFLOAT3& scale );
	void SetScale( const float& scale );

	// アニメーション速度の設定.
	void SetAnimSpeed( const double& speed );
	// アニメーションの変更.
	void ChangeAnimation( int& index );

private:
	/***************************************
	*			Fbxモデル系.
	***************************************/
	std::vector<FBXMeshData> m_MeshData;	// メッシュデータ.
	std::unordered_map<std::string, ID3D11ShaderResourceView*> m_Textures;	// テクスチャリスト.
	CFbxAnimationController* m_pAc;			// アニメーションコントローラー.

	/***************************************
	*			パラメータ.
	***************************************/
	DirectX::XMFLOAT3 m_Position;	// 座標.
	DirectX::XMFLOAT3 m_Rotation;	// 回転.
	DirectX::XMFLOAT3 m_Scale;		// 大きさ.
};