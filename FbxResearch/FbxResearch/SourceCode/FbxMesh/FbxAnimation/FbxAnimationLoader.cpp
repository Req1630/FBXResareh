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
// �쐬.
//-----------------------------------------.
HRESULT CFbxAnimationLoader::Create( const char* fileName )
{
	//------------------------------.
	// �}�l�[�W���[�̍쐬.
	//------------------------------.
	m_pFbxManager = FbxManager::Create();
	if( m_pFbxManager == nullptr ){
		ERROR_MESSAGE( "FbxManager Create Failure." );
		return E_FAIL;
	}

	//------------------------------.
	// �C���|�[�^�[�̍쐬.
	//------------------------------.
	FbxImporter* pFbxImpoter = FbxImporter::Create( m_pFbxManager, "imp" );
	if( pFbxImpoter == nullptr ){
		ERROR_MESSAGE( "FbxImpoter Create Failure." );
		return E_FAIL;
	}

	//------------------------------.
	// FBX�t�@�C���̓ǂݍ���.
	//------------------------------.
	// �t�@�C�����̐ݒ�.
	FbxString fbxFileName( fileName );
	if( pFbxImpoter->Initialize( fbxFileName.Buffer() ) == false ){
		ERROR_MESSAGE( "FbxFile Loading Failure." );
		return E_FAIL;
	}

	//------------------------------.
	// �V�[���I�u�W�F�N�g�̍쐬.
	//------------------------------.
	m_pFbxScene = FbxScene::Create( m_pFbxManager, "fbxScene" );
	if( m_pFbxScene == nullptr ){
		ERROR_MESSAGE( "FbxScene Create Failure." );
		return E_FAIL;
	}

	//------------------------------.
	// �C���|�[�^�[�ƃV�[���I�u�W�F�N�g�̊֘A�t��.
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
	// �|���S�����O�p�`�ɂ���.
	// ���p�`�|���S��������΍��Ȃ����̂Ŏ��Ԃ�������.
	convertReslut = geometryConverter.Triangulate( m_pFbxScene, true );
	geometryConverter.RemoveBadPolygonsFromMeshes( m_pFbxScene );
	convertReslut = geometryConverter.SplitMeshesPerMaterial( m_pFbxScene, true );

	//-----------------------------------.
	// FbxSkeleton�̐����擾.
	//-----------------------------------.
	// FbxSkeleton���擾.
	int skeltonNum = m_pFbxScene->GetSrcObjectCount<FbxSkeleton>();
	for( int i = 0; i < skeltonNum; i++ ){
		m_Skeletons.emplace_back( m_pFbxScene->GetSrcObject<FbxSkeleton>(i) );
	}
	//-----------------------------------.
	// FbxMesh�̐����擾.
	//-----------------------------------.
	// FbxMesh���擾.
	int meshNum = m_pFbxScene->GetSrcObjectCount<FbxMesh>();
	for( int i = 0; i < meshNum; i++ ){
		m_MeshClusterData.emplace_back();
		LoadSkin( m_pFbxScene->GetSrcObject<FbxMesh>(i), m_MeshClusterData.back() );
	}
	//-----------------------------------.
	//	�A�j���[�V�����t���[���̎擾.
	//-----------------------------------.
	GetAnimationFrame( m_pFbxScene );

	// �C���|�[�^�[�̉��.
	SAFE_DESTROY( pFbxImpoter );

	return S_OK;
}

//-----------------------------------------.
// �j��.
//-----------------------------------------.
void CFbxAnimationLoader::Destroy()
{
	SAFE_DESTROY( m_pFbxScene );
	SAFE_DESTROY( m_pFbxManager );
}

//-----------------------------------------.
// �A�j���[�V�����̓ǂݍ���.
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
// �X�L�����ǂݍ���.
//-----------------------------------------.
void CFbxAnimationLoader::LoadSkin( FbxMesh* pMesh, FBXMeshClusterData& meshClusterData )
{
	// �_�E���L���X�g���ăX�L�������擾.
	FbxSkin* pSkin = (FbxSkin*)pMesh->GetDeformer( 0, FbxDeformer::eSkin );
	if( pSkin == nullptr ) return;

	// �{�[���̐�.
	int boneCount = pSkin->GetClusterCount();
	for( int boneIndex = 0; boneIndex < boneCount; boneIndex++ ){
		// �{�[�����擾.
		FbxCluster* pCluster = pSkin->GetCluster( boneIndex );
		FbxNode* pNode = pCluster->GetLink();

		meshClusterData.ClusterKey[pNode->GetName()] = pNode;
		meshClusterData.ClusterName.emplace_back( pNode->GetName() );
	}
}

//-----------------------------------------.
// �L�[�t���[���̎擾.
//-----------------------------------------.
void CFbxAnimationLoader::GetAnimationFrame( FbxScene* pScene )
{
	// �t���[�����[�g�̎擾.
	FbxTime::EMode timeMode = pScene->GetGlobalSettings().GetTimeMode();
	double framerate = FbxTime::GetFrameRate( timeMode );

	FbxNode* pNode = pScene->GetRootNode();
	// �A�j���[�V�����̐����擾.
	int stackCount = pScene->GetSrcObjectCount<FbxAnimStack>();
	m_AnimDataList.resize( stackCount );

	// �A�j���[�V�����̐���.
	for( int i = 0; i < stackCount; i++ ){
		SAnimationData& animData = m_AnimDataList[i];
		animData.FrameRate = framerate;	// �t���[�����[�g�̐ݒ�.
		FbxTimeSpan timeSpan;
		// �A�j���[�V�����f�[�^�擾.
		FbxAnimStack* animStack = pScene->GetSrcObject<FbxAnimStack>(i);
		int numLayer = animStack->GetMemberCount<FbxAnimLayer>();
		pScene->SetCurrentAnimationStack( animStack );

		// �A�j���[�V�����̂����Ȏ��Ԃ��擾.
		pNode->GetAnimationInterval( timeSpan, animStack );

		// �A�j���[�V�����̎��Ԃ��擾.
		FbxTime time = timeSpan.GetStop() - timeSpan.GetStart();
		FbxLongLong ms = time.GetMilliSeconds();	// �~���b�ɕϊ�.

		// ���Z�t���[���̐ݒ�.
		FbxTime frameTime;
		frameTime.SetTime( 0, 0, 0, 1, 0, pScene->GetGlobalSettings().GetTimeMode() );
		animData.AnimSpeed = frameTime.GetSecondDouble();

		// �J�n���Ԃ��擾.
		animData.StartTime = timeSpan.GetStart().GetSecondDouble();
		// �I�����Ԃ��擾.
		animData.EndTime = timeSpan.GetStop().GetSecondDouble();
		// �A�j���[�V�����̑S�̎��Ԃ�float�ɂ��Ď擾.
		// ms / 1000.0f = AnimTime.
		// 1000 / 1000.0f = 1.0f : �~���b��1000�Ŋ���ƕb�ɂȂ�.
		animData.AnimationTime = (double)(ms)/1000.0;

		// �A�j���[�V�����̃t���[�����̍s����擾.
		GetAnimationFrameMatrix( animData, timeSpan, pNode );
	}
}

//-----------------------------------------.
// �A�j���[�V�����̃t���[���s����擾.
//-----------------------------------------.
void CFbxAnimationLoader::GetAnimationFrameMatrix( SAnimationData& animData, FbxTimeSpan& timeSpan, FbxNode* pNode )
{
	// �A�j���[�V�����t���[��.
	int totalFrame = (int)std::ceil(animData.AnimationTime * animData.FrameRate)+1;
	// �A�j���[�V�����̃X�^�[�g����.
	double start = timeSpan.GetStart().GetSecondDouble();

	// ���b�V���̐����̃t���[���s����擾.
	int meshNo = 0;	// ���b�V���ԍ�.
	for( auto& animSkelton : m_MeshClusterData ){
		animData.KeyFrameLinkMatrix.emplace_back();
		animData.KeyFrameLinkMatrix.back().resize( animSkelton.ClusterKey.size() );
		

		// �X�P���g���̐������[�v.
		for( auto& s : m_Skeletons ){
			FbxNode* skeletonNode = s->GetNode();
			// �ǉ������m�[�h��������Ȃ���ΏI��.
			if( animSkelton.ClusterKey.find(skeletonNode->GetName()) == 
				animSkelton.ClusterKey.end() ) continue;
			int numOfBones = animSkelton.ClusterKey.size();
			if( numOfBones <= 0 ) continue;
			std::map<double, FbxMatrix> keyFrame;

			// �t���[���̐���.
			for( int i = 0; i < totalFrame; i++ ){
				double keyTime = start + (i * (1.0f/animData.FrameRate));
				FbxTime time;
				time.SetSecondDouble( keyTime );
				// �t���[�����̍s����擾.
				FbxAMatrix m = skeletonNode->EvaluateGlobalTransform( time );
				keyFrame[keyTime] = m;
			}
			for( int i = 0; i < (int)animSkelton.ClusterName.size(); i++ ){
				if( animSkelton.ClusterName[i] != skeletonNode->GetName() ) continue;
				// �X�P���g���̖��O�ƃA�j���[�V�����Ŏd�l����{�[���̖��O����v���Ă����.
				// ��Őݒ肵���L�[�t���[����ǉ�.
				animData.KeyFrameLinkMatrix[meshNo][i] = keyFrame;
			}
		}

		// �t���[�����X�g��ǉ�.
		if( animData.KeyFrameLinkMatrix.empty() == false ){
			animData.FrameList.emplace_back();
			for( auto& f : animData.KeyFrameLinkMatrix[meshNo][0] ){
				animData.FrameList.back().emplace_back( f.first );
			}
		}

		// �t���[���̐���.
		animData.KeyFrameMatrix.emplace_back();
		for( int i = 0; i < totalFrame; i++ ){
			double keyTime = start + (i * (1.0f/animData.FrameRate));
			FbxTime time;	// �w�莞�Ԃ̍s����擾���邽�߂̎���.
			time.SetSecondDouble( keyTime );
			FbxAMatrix m = pNode->EvaluateGlobalTransform( time );
			animData.KeyFrameMatrix.back()[keyTime] = m;
		}
		meshNo++;
	}
}