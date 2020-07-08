#pragma once

#include "..\FbxInclude\FbxInclude.h"
#include "FbxAnimationData.h"

#include <Windows.h>
#include <vector>
#include <unordered_map>
#include <set>

class CFbxAnimationController;

// メッシュのクラスター(ボーン)の情報.
struct FBXMeshClusterData
{
	std::vector<std::string>					ClusterName;
	std::unordered_map<std::string, FbxNode*>	ClusterKey;

	FBXMeshClusterData(){}
};

class CFbxAnimationLoader
{
public:
	CFbxAnimationLoader();
	~CFbxAnimationLoader();

	// 作成.
	HRESULT Create();
	// 破壊.
	void Destroy();

	// アニメーションの読み込み.
	HRESULT LoadAnim( CFbxAnimationController* pAc, const char* fileName );

	// アニメーションの読み込み.
	HRESULT LoadAnimationData(
		FbxScene*							pFbxScene,
		std::vector<FBXMeshClusterData>&	meshClusterData,
		std::vector<FbxSkeleton*>&			skeletons,
		std::vector<SAnimationData>*		outAnimDataList );

	// アニメーションデータリストの取得.
	std::vector<SAnimationData>& GetAnimDateList(){ return m_AnimDataList; }

private:
	// スキン情報読み込み.
	void LoadSkin( FbxMesh* pMesh, FBXMeshClusterData& clusterData );
	// キーフレームの取得.
	void GetAnimationFrame( FbxScene* pScene );
	// アニメーションのフレーム行列を取得.
	void GetAnimationFrameMatrix( SAnimationData& animData, FbxTimeSpan& timeSpan, FbxNode* pNode );


private:
	FbxManager* m_pFbxManager;	// FBXマネージャー.
	FbxScene*	m_pFbxScene;	// FBXシーンオブジェクト.

	std::vector<FBXMeshClusterData>	m_MeshClusterData;	// メッシュデータ.
	std::vector<FbxSkeleton*>		m_Skeletons;		// スケルトンリスト.
	std::vector<SAnimationData>		m_AnimDataList;		// アニメーションデータリスト.
};