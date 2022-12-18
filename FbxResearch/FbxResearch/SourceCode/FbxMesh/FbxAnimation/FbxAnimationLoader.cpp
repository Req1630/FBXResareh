#include "FbxAnimationLoader.h"
#include "FbxAnimationController.h"

#include <map>

CFbxAnimationLoader::CFbxAnimationLoader()
	: m_pFbxManager		( nullptr )
	, m_MeshClusterData	()
	, m_AnimDataList	()
	, m_Skeletons		()
{
}

CFbxAnimationLoader::~CFbxAnimationLoader()
{
}

////////////////////////////////////////////////////////////////.
// 作成.
////////////////////////////////////////////////////////////////.
HRESULT CFbxAnimationLoader::Create()
{
	//------------------------------.
	// マネージャーの作成.
	//------------------------------.
	m_pFbxManager = FbxManager::Create();
	if( m_pFbxManager == nullptr ){
		_ASSERT_EXPR( false, TEXT("FbxManager作成失敗") );
		MessageBox( nullptr, TEXT("FbxManager作成失敗"), TEXT("Warning"), MB_OK );
		return E_FAIL;
	}

	return S_OK;
}

////////////////////////////////////////////////////////////////.
// 破壊.
////////////////////////////////////////////////////////////////.
void CFbxAnimationLoader::Destroy()
{
	SAFE_DESTROY( m_pFbxManager );
}

////////////////////////////////////////////////////////////////.
// アニメーションの読み込み.
////////////////////////////////////////////////////////////////.
HRESULT CFbxAnimationLoader::LoadAnim( SAnimationDataList* outAnimDataList, const char* fileName )
{
	//------------------------------.
	// インポーターの作成.
	//------------------------------.
	FbxImporter* pFbxImpoter = FbxImporter::Create( m_pFbxManager, "imp" );
	if( pFbxImpoter == nullptr ){
		_ASSERT_EXPR( false, TEXT("FbxImporter作成失敗") );
		MessageBox( nullptr, TEXT("FbxImporter作成失敗"), TEXT("Warning"), MB_OK );
		return E_FAIL;
	}

	//------------------------------.
	// FBXファイルの読み込み.
	//------------------------------.
	// ファイル名の設定.
	FbxString fbxFileName( fileName );
	if( pFbxImpoter->Initialize( fbxFileName.Buffer() ) == false ){
		_ASSERT_EXPR( false, TEXT("Fbxファイル読み込み失敗") );
		MessageBox( nullptr, TEXT("Fbxファイル読み込み失敗"), TEXT("Warning"), MB_OK );
		return E_FAIL;
	}

	//------------------------------.
	// インポーターとシーンオブジェクトの関連付け.
	//------------------------------.
	//------------------------------.
	// シーンオブジェクトの作成.
	//------------------------------.
	FbxScene*	pFbxScene = FbxScene::Create( m_pFbxManager, "fbxScene" );
	if( pFbxScene == nullptr ){
		_ASSERT_EXPR( false, TEXT("FbxScene作成失敗") );
		MessageBox( nullptr, TEXT("FbxScene作成失敗"), TEXT("Warning"), MB_OK );
		return E_FAIL;
	}
	if( pFbxImpoter->Import( pFbxScene ) == false ){
		SAFE_DESTROY( pFbxScene );
		SAFE_DESTROY( pFbxImpoter );
		_ASSERT_EXPR( false, TEXT("FbxImpoterとFbxSceneの関連付け失敗") );
		MessageBox( nullptr, TEXT("FbxImpoterとFbxSceneの関連付け失敗"), TEXT("Warning"), MB_OK );
		return E_FAIL;
	}

	//-----------------------------------.
	// FbxSkeletonの数を取得.
	//-----------------------------------.
	// FbxSkeletonを取得.
	int skeltonNum = pFbxScene->GetSrcObjectCount<FbxSkeleton>();
	m_Skeletons.resize( skeltonNum );
	for( int i = 0; i < skeltonNum; i++ ){
		m_Skeletons[i] = pFbxScene->GetSrcObject<FbxSkeleton>(i);
		m_Skeletons[i]->GetNode()->GetAnimationEvaluator();
	}

	//-----------------------------------.
	// FbxMeshの数を取得.
	//-----------------------------------.
	// FbxMeshを取得.
	int meshNum = pFbxScene->GetSrcObjectCount<FbxMesh>();	
	m_MeshClusterData.resize( meshNum );
	for( int i = 0; i < meshNum; i++ ){
		LoadSkin( pFbxScene->GetSrcObject<FbxMesh>(i), m_MeshClusterData[i] );
	}
	//-----------------------------------.
	//	アニメーションフレームの取得.
	//-----------------------------------.
	GetAnimationFrame( pFbxScene );

	// アニメーションデータが空じゃなければリストを追加.
	if( m_AnimDataList.AnimList.empty() == false ){
		*outAnimDataList = m_AnimDataList;
	}

	// 余分なメモリの削除.
	m_Skeletons.clear();
	m_Skeletons.shrink_to_fit();
	m_MeshClusterData.clear();
	m_MeshClusterData.shrink_to_fit();

	SAFE_DESTROY( pFbxScene );
	SAFE_DESTROY( pFbxImpoter );

	return S_OK;
}

////////////////////////////////////////////////////////////////.
// アニメーションの読み込み.
////////////////////////////////////////////////////////////////.
HRESULT CFbxAnimationLoader::LoadAnimationData(
	FbxScene*							pFbxScene,
	std::vector<FBXMeshClusterData>&	meshClusterData,
	std::vector<FbxSkeleton*>&			skeletons,
	SAnimationDataList*					outAnimDataList )
{
	m_MeshClusterData = meshClusterData;
	m_Skeletons = skeletons;
	GetAnimationFrame( pFbxScene );
	*outAnimDataList = m_AnimDataList;

	// 余分なメモリの削除.
	m_MeshClusterData.clear();
	m_MeshClusterData.shrink_to_fit();
	m_Skeletons.clear();
	m_Skeletons.shrink_to_fit();

	return S_OK;
}

////////////////////////////////////////////////////////////////.
// スキン情報読み込み.
////////////////////////////////////////////////////////////////.
void CFbxAnimationLoader::LoadSkin( FbxMesh* pMesh, FBXMeshClusterData& meshClusterData )
{
	// ダウンキャストしてスキン情報を取得.
	FbxSkin* pSkin = (FbxSkin*)pMesh->GetDeformer( 0, FbxDeformer::eSkin );
	if( pSkin == nullptr ) return;
	
	// ボーンの数.
	int boneCount = pSkin->GetClusterCount();
	for( int boneIndex = 0; boneIndex < boneCount; boneIndex++ ){
		// ボーン情報取得.
		FbxCluster* pCluster = pSkin->GetCluster( boneIndex );
		FbxNode* pNode = pCluster->GetLink();
		// ボーン名の取得.
		meshClusterData.ClusterName.emplace_back( pNode->GetName() );
	}
	// 余分なメモリの削除.
	meshClusterData.ClusterName.shrink_to_fit();
}

////////////////////////////////////////////////////////////////.
// キーフレームの取得.
////////////////////////////////////////////////////////////////.
void CFbxAnimationLoader::GetAnimationFrame( FbxScene* pScene )
{
	// フレームレートの取得.
	FbxTime::EMode timeMode = pScene->GetGlobalSettings().GetTimeMode();
	double framerate = FbxTime::GetFrameRate( timeMode );

	FbxNode* pNode = pScene->GetRootNode();
	// アニメーションの数を取得.
	int stackCount = pScene->GetSrcObjectCount<FbxAnimStack>();
	m_AnimDataList.AnimList.clear();
	m_AnimDataList.AnimList.resize( stackCount );
	std::vector<SAnimationData>& animDataList = m_AnimDataList.AnimList;
	// アニメーションの数分.
	for( int i = 0; i < stackCount; i++ ){
		SAnimationData& animData = animDataList[i];
		animData.FrameRate = framerate;	// フレームレートの設定.
		FbxTimeSpan timeSpan;
		// アニメーションデータ取得.
		FbxAnimStack* animStack = pScene->GetSrcObject<FbxAnimStack>(i);
		int numLayer = animStack->GetMemberCount<FbxAnimLayer>();
		pScene->SetCurrentAnimationStack( animStack );

		// アニメーションのいろんな時間を取得.
		pNode->GetAnimationInterval( timeSpan, animStack );

		// アニメーション名を取得.
		animData.Name = animStack->GetName();

		// アニメーションの時間を取得.
		FbxTime time = timeSpan.GetStop() - timeSpan.GetStart();
		FbxLongLong ms = time.GetMilliSeconds();	// ミリ秒に変換.

		// 加算フレームの設定.
		FbxTime frameTime;
		frameTime.SetTime( 0, 0, 0, 1, 0, timeMode );
		animData.AnimSpeed = frameTime.GetSecondDouble() / ( 60.0 / framerate );

		// 開始時間を取得.
		animData.StartTime = timeSpan.GetStart().GetSecondDouble();
		// 終了時間を取得.
		animData.EndTime = timeSpan.GetStop().GetSecondDouble();
		// アニメーションの全体時間をfloatにして取得.
		// ms / 1000.0f = AnimTime.
		// 1000 / 1000.0f = 1.0f : ミリ秒を1000で割ると秒になる.
		animData.AnimationTime = (double)(ms)/1000.0;

		// アニメーションのフレーム時の行列を取得.
		GetAnimationFrameMatrix( animData, timeSpan, pNode );

	}
	// アニメーションデータが正しいか確認.
	for( size_t i = 0; i < animDataList.size(); i++ ){
		if( animDataList[i].EndTime < 0.0 ){
			// アニメーションの終了時間が 0 より少ないと、
			// 明らかにおかしなデータなので、
			// そのアニメーションデータを削除する.
			animDataList[i] = animDataList.back();
			animDataList.pop_back();
			i--;
		}
	}
}

////////////////////////////////////////////////////////////////.
// アニメーションのフレーム行列を取得.
////////////////////////////////////////////////////////////////.
void CFbxAnimationLoader::GetAnimationFrameMatrix( SAnimationData& animData, FbxTimeSpan& timeSpan, FbxNode* pNode )
{
	// アニメーションフレーム.
	int totalFrame = (int)std::ceil(animData.AnimationTime * animData.FrameRate);
	// アニメーションのスタート時間.
	double start = timeSpan.GetStart().GetSecondDouble();

	// メッシュの数分のフレーム行列を取得.
	int meshNo = 0;	// メッシュ番号.
	for( auto& animSkelton : m_MeshClusterData ){
		animData.KeyFrameLinkMatrix.emplace_back();
		animData.KeyFrameLinkMatrix.back().resize( animSkelton.ClusterName.size() );
		
		
		// スケルトンの数分ループ.
		for( auto& s : m_Skeletons ){
			FbxNode* skeletonNode = s->GetNode();
			// 追加したノードが見つからなければ終了.
			if( std::find( animSkelton.ClusterName.begin(), animSkelton.ClusterName.end(), 
				skeletonNode->GetName() ) == animSkelton.ClusterName.end() ) continue;

			if( animSkelton.ClusterName.size() <= 0 ) continue;

			std::map<double, FbxMatrix> keyFrame;

			// フレームの数分.
			for( int i = 0; i <= totalFrame; i++ ){
				double keyTime = start + (i * (1.0f/animData.FrameRate));
				FbxTime time;
				time.SetSecondDouble( keyTime );
				// フレーム時の行列を取得.
				FbxAMatrix m = skeletonNode->EvaluateGlobalTransform( time );
				keyFrame[keyTime] = m;
			}
			for( int i = 0; i < (int)animSkelton.ClusterName.size(); i++ ){
				if( animSkelton.ClusterName[i] != skeletonNode->GetName() ) continue;
				// スケルトンの名前とアニメーションで仕様するボーンの名前が一致していれば.
				// 上で設定したキーフレームを追加.
				animData.KeyFrameLinkMatrix[meshNo][i] = keyFrame;
			}
		}
		// 余分なメモリの削除.
		animData.KeyFrameLinkMatrix.shrink_to_fit();

		// フレームリストを追加.
		if( animData.KeyFrameLinkMatrix.empty() == false ){
			animData.KeyList.emplace_back();
			for( auto& f : animData.KeyFrameLinkMatrix[meshNo][0] ){
				animData.KeyList.back().emplace_back( f.first );
			}
			animData.KeyList.back().emplace_back( animData.EndTime );
		}
		animData.KeyList.shrink_to_fit();

		meshNo++;
	}
}