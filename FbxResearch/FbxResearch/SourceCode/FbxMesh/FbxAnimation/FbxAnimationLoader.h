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
	std::vector<std::string> ClusterName;	// 検索用のクラス名.
};

/***************************************************************************
	アニメーションデータを読み込むためのクラス.
----------------------------------------------------------------------------
*	モデルは多角ポリゴンでも行けるが、読み込みが遅くなるので、
*		グラフィッカーに頼んで三角ポリゴンで出力してもらった方が良い.
*	アニメーションモデルはメッシュ情報がいる.
*	スキン情報だけのアニメーションモデルは読み込めない.
*	LoadAnimationData は CFbxModelLoader で使用しているので、
*		外部では使用しないと思う.
----------------------------------------------------------------------------
*	@brief : アニメーションデータを読み込む関数.
*	@proc LoadAnim( CFbxAnimationController* pAc, const char* fileName ).
*	@param pAc		: 読み込ませたいアニメーションコントローラー.
*	@param fileName	: 読み込みたいアニメーションモデルのファイルパス.
*	@return HRESULT : エラー内容など.
***/
class CFbxAnimationLoader
{
public:
	CFbxAnimationLoader();
	~CFbxAnimationLoader();

	//-----------------------------------------.
	//	作成.
	//-----------------------------------------.
	HRESULT Create();

	//-----------------------------------------.
	//	破壊.
	//-----------------------------------------.
	void Destroy();

	//-----------------------------------------.
	//	アニメーションの読み込み.
	//-----------------------------------------.
	HRESULT LoadAnim( SAnimationDataList* outAnimDataList, const char* fileName );

	//-----------------------------------------.
	//	アニメーションの読み込み.
	//-----------------------------------------.
	HRESULT LoadAnimationData(
		FbxScene*							pFbxScene,
		std::vector<FBXMeshClusterData>&	meshClusterData,
		std::vector<FbxSkeleton*>&			skeletons,
		SAnimationDataList*					outAnimDataList );

private:
	//-----------------------------------------.
	//	アニメーションの読み取み系.
	//-----------------------------------------.

	// スキン情報読み込み.
	void LoadSkin( FbxMesh* pMesh, FBXMeshClusterData& clusterData );
	// キーフレームの取得.
	void GetAnimationFrame( FbxScene* pScene );
	// アニメーションのフレーム行列を取得.
	void GetAnimationFrameMatrix( SAnimationData& animData, FbxTimeSpan& timeSpan, FbxNode* pNode );

private:
	/***************************************
	*			FBX SDK.
	***************************************/
	FbxManager* m_pFbxManager;	// FBXマネージャー.

	/***************************************
	*		アニメーションデータ系.
	***************************************/
	std::vector<FBXMeshClusterData>	m_MeshClusterData;	// メッシュデータ.
	std::vector<FbxSkeleton*>		m_Skeletons;		// スケルトンリスト.
	SAnimationDataList				m_AnimDataList;		// アニメーションデータリスト.
};