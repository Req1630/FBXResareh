#include "FbxAnimationLoader.h"

CFbxAnimationLoader::CFbxAnimationLoader()
	: m_pFbxManager		( nullptr )
	, m_pFbxScene		( nullptr )
	, m_AnimDataList	()
	, m_Skeletons		()
{
}

CFbxAnimationLoader::~CFbxAnimationLoader()
{
}

//-----------------------------------------.
// 作成.
//-----------------------------------------.
HRESULT CFbxAnimationLoader::Create( const char* fileName )
{
	//------------------------------.
	// マネージャーの作成.
	//------------------------------.
	m_pFbxManager = FbxManager::Create();
	if( m_pFbxManager == nullptr ){
		ERROR_MESSAGE( "FbxManager Create Failure." );
		return E_FAIL;
	}

	//------------------------------.
	// インポーターの作成.
	//------------------------------.
	FbxImporter* pFbxImpoter = FbxImporter::Create( m_pFbxManager, "imp" );
	if( pFbxImpoter == nullptr ){
		ERROR_MESSAGE( "FbxImpoter Create Failure." );
		return E_FAIL;
	}

	//------------------------------.
	// FBXファイルの読み込み.
	//------------------------------.
	// ファイル名の設定.
	FbxString fbxFileName( fileName );
	if( pFbxImpoter->Initialize( fbxFileName.Buffer() ) == false ){
		ERROR_MESSAGE( "FbxFile Loading Failure." );
		return E_FAIL;
	}

	//------------------------------.
	// シーンオブジェクトの作成.
	//------------------------------.
	m_pFbxScene = FbxScene::Create( m_pFbxManager, "fbxScene" );
	if( m_pFbxScene == nullptr ){
		ERROR_MESSAGE( "FbxScene Create Failure." );
		return E_FAIL;
	}

	//------------------------------.
	// インポーターとシーンオブジェクトの関連付け.
	//------------------------------.
	if( pFbxImpoter->Import( m_pFbxScene ) == false ){
		SAFE_DESTROY( m_pFbxManager );
		SAFE_DESTROY( m_pFbxScene );
		SAFE_DESTROY( pFbxImpoter );
		ERROR_MESSAGE( "FbxScene Create Failure." );
		return E_FAIL;
	}

	bool convertReslut = false;
	FbxGeometryConverter geometryConverter( m_pFbxManager );
	// ポリゴンを三角形にする.
	// 多角形ポリゴンがあれば作りなおすので時間がかかる.
	convertReslut = geometryConverter.Triangulate( m_pFbxScene, true );
	geometryConverter.RemoveBadPolygonsFromMeshes( m_pFbxScene );
	convertReslut = geometryConverter.SplitMeshesPerMaterial( m_pFbxScene, true );

	//-----------------------------------.
	// FbxSkeletonの数を取得.
	//-----------------------------------.
	// FbxSkeletonを取得.
	int skeltonNum = m_pFbxScene->GetSrcObjectCount<FbxSkeleton>();
	for( int i = 0; i < skeltonNum; i++ ){
		m_Skeletons.emplace_back( m_pFbxScene->GetSrcObject<FbxSkeleton>(i) );
	}
	//-----------------------------------.
	// FbxMeshの数を取得.
	//-----------------------------------.
	// FbxMeshを取得.
	int meshNum = m_pFbxScene->GetSrcObjectCount<FbxMesh>();
	for( int i = 0; i < meshNum; i++ ){
		m_MeshClusterData.emplace_back();
		LoadSkin( m_pFbxScene->GetSrcObject<FbxMesh>(i), m_MeshClusterData.back() );
	}
	//-----------------------------------.
	//	アニメーションフレームの取得.
	//-----------------------------------.
	GetAnimationFrame( m_pFbxScene );

	// インポーターの解放.
	SAFE_DESTROY( pFbxImpoter );

	return S_OK;
}

//-----------------------------------------.
// 破壊.
//-----------------------------------------.
void CFbxAnimationLoader::Destroy()
{
	SAFE_DESTROY( m_pFbxScene );
	SAFE_DESTROY( m_pFbxManager );
}

//-----------------------------------------.
// アニメーションの読み込み.
//-----------------------------------------.
HRESULT CFbxAnimationLoader::LoadAnimationData(
	FbxScene*							pFbxScene,
	std::vector<FBXMeshClusterData>&	meshClusterData,
	std::vector<FbxSkeleton*>&			skeletons,
	std::vector<SAnimationData>*		outAnimDataList )
{
	m_MeshClusterData = meshClusterData;
	m_Skeletons = skeletons;
	GetAnimationFrame( pFbxScene );
	*outAnimDataList = m_AnimDataList;
	return S_OK;
}

//-----------------------------------------.
// スキン情報読み込み.
//-----------------------------------------.
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

		meshClusterData.ClusterKey[pNode->GetName()] = pNode;
		meshClusterData.ClusterName.emplace_back( pNode->GetName() );
	}
}

//-----------------------------------------.
// キーフレームの取得.
//-----------------------------------------.
void CFbxAnimationLoader::GetAnimationFrame( FbxScene* pScene )
{
	// フレームレートの取得.
	FbxTime::EMode timeMode = pScene->GetGlobalSettings().GetTimeMode();
	double framerate = FbxTime::GetFrameRate( timeMode );

	FbxNode* pNode = pScene->GetRootNode();
	// アニメーションの数を取得.
	int stackCount = pScene->GetSrcObjectCount<FbxAnimStack>();
	m_AnimDataList.resize( stackCount );

	// アニメーションの数分.
	for( int i = 0; i < stackCount; i++ ){
		SAnimationData& animData = m_AnimDataList[i];
		animData.FrameRate = framerate;	// フレームレートの設定.
		FbxTimeSpan timeSpan;
		// アニメーションデータ取得.
		FbxAnimStack* animStack = pScene->GetSrcObject<FbxAnimStack>(i);
		int numLayer = animStack->GetMemberCount<FbxAnimLayer>();
		pScene->SetCurrentAnimationStack( animStack );

		// アニメーションのいろんな時間を取得.
		pNode->GetAnimationInterval( timeSpan, animStack );

		// アニメーションの時間を取得.
		FbxTime time = timeSpan.GetStop() - timeSpan.GetStart();
		FbxLongLong ms = time.GetMilliSeconds();	// ミリ秒に変換.

		// 加算フレームの設定.
		FbxTime frameTime;
		frameTime.SetTime( 0, 0, 0, 1, 0, pScene->GetGlobalSettings().GetTimeMode() );
		animData.AnimSpeed = frameTime.GetSecondDouble();

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
}

//-----------------------------------------.
// アニメーションのフレーム行列を取得.
//-----------------------------------------.
void CFbxAnimationLoader::GetAnimationFrameMatrix( SAnimationData& animData, FbxTimeSpan& timeSpan, FbxNode* pNode )
{
	// アニメーションフレーム.
	int totalFrame = (int)std::ceil(animData.AnimationTime * animData.FrameRate)+1;
	// アニメーションのスタート時間.
	double start = timeSpan.GetStart().GetSecondDouble();

	// メッシュの数分のフレーム行列を取得.
	int meshNo = 0;	// メッシュ番号.
	for( auto& animSkelton : m_MeshClusterData ){
		animData.KeyFrameLinkMatrix.emplace_back();
		animData.KeyFrameLinkMatrix.back().resize( animSkelton.ClusterKey.size() );
		

		// スケルトンの数分ループ.
		for( auto& s : m_Skeletons ){
			FbxNode* skeletonNode = s->GetNode();
			// 追加したノードが見つからなければ終了.
			if( animSkelton.ClusterKey.find(skeletonNode->GetName()) == 
				animSkelton.ClusterKey.end() ) continue;
			int numOfBones = animSkelton.ClusterKey.size();
			if( numOfBones <= 0 ) continue;
			std::map<double, FbxMatrix> keyFrame;

			// フレームの数分.
			for( int i = 0; i < totalFrame; i++ ){
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

		// フレームリストを追加.
		if( animData.KeyFrameLinkMatrix.empty() == false ){
			animData.FrameList.emplace_back();
			for( auto& f : animData.KeyFrameLinkMatrix[meshNo][0] ){
				animData.FrameList.back().emplace_back( f.first );
			}
		}

		// フレームの数分.
		animData.KeyFrameMatrix.emplace_back();
		for( int i = 0; i < totalFrame; i++ ){
			double keyTime = start + (i * (1.0f/animData.FrameRate));
			FbxTime time;	// 指定時間の行列を取得するための時間.
			time.SetSecondDouble( keyTime );
			FbxAMatrix m = pNode->EvaluateGlobalTransform( time );
			animData.KeyFrameMatrix.back()[keyTime] = m;
		}
		meshNo++;
	}
}