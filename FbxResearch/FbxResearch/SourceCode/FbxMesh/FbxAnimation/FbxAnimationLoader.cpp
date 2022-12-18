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
// �쐬.
////////////////////////////////////////////////////////////////.
HRESULT CFbxAnimationLoader::Create()
{
	//------------------------------.
	// �}�l�[�W���[�̍쐬.
	//------------------------------.
	m_pFbxManager = FbxManager::Create();
	if( m_pFbxManager == nullptr ){
		_ASSERT_EXPR( false, TEXT("FbxManager�쐬���s") );
		MessageBox( nullptr, TEXT("FbxManager�쐬���s"), TEXT("Warning"), MB_OK );
		return E_FAIL;
	}

	return S_OK;
}

////////////////////////////////////////////////////////////////.
// �j��.
////////////////////////////////////////////////////////////////.
void CFbxAnimationLoader::Destroy()
{
	SAFE_DESTROY( m_pFbxManager );
}

////////////////////////////////////////////////////////////////.
// �A�j���[�V�����̓ǂݍ���.
////////////////////////////////////////////////////////////////.
HRESULT CFbxAnimationLoader::LoadAnim( SAnimationDataList* outAnimDataList, const char* fileName )
{
	//------------------------------.
	// �C���|�[�^�[�̍쐬.
	//------------------------------.
	FbxImporter* pFbxImpoter = FbxImporter::Create( m_pFbxManager, "imp" );
	if( pFbxImpoter == nullptr ){
		_ASSERT_EXPR( false, TEXT("FbxImporter�쐬���s") );
		MessageBox( nullptr, TEXT("FbxImporter�쐬���s"), TEXT("Warning"), MB_OK );
		return E_FAIL;
	}

	//------------------------------.
	// FBX�t�@�C���̓ǂݍ���.
	//------------------------------.
	// �t�@�C�����̐ݒ�.
	FbxString fbxFileName( fileName );
	if( pFbxImpoter->Initialize( fbxFileName.Buffer() ) == false ){
		_ASSERT_EXPR( false, TEXT("Fbx�t�@�C���ǂݍ��ݎ��s") );
		MessageBox( nullptr, TEXT("Fbx�t�@�C���ǂݍ��ݎ��s"), TEXT("Warning"), MB_OK );
		return E_FAIL;
	}

	//------------------------------.
	// �C���|�[�^�[�ƃV�[���I�u�W�F�N�g�̊֘A�t��.
	//------------------------------.
	//------------------------------.
	// �V�[���I�u�W�F�N�g�̍쐬.
	//------------------------------.
	FbxScene*	pFbxScene = FbxScene::Create( m_pFbxManager, "fbxScene" );
	if( pFbxScene == nullptr ){
		_ASSERT_EXPR( false, TEXT("FbxScene�쐬���s") );
		MessageBox( nullptr, TEXT("FbxScene�쐬���s"), TEXT("Warning"), MB_OK );
		return E_FAIL;
	}
	if( pFbxImpoter->Import( pFbxScene ) == false ){
		SAFE_DESTROY( pFbxScene );
		SAFE_DESTROY( pFbxImpoter );
		_ASSERT_EXPR( false, TEXT("FbxImpoter��FbxScene�̊֘A�t�����s") );
		MessageBox( nullptr, TEXT("FbxImpoter��FbxScene�̊֘A�t�����s"), TEXT("Warning"), MB_OK );
		return E_FAIL;
	}

	//-----------------------------------.
	// FbxSkeleton�̐����擾.
	//-----------------------------------.
	// FbxSkeleton���擾.
	int skeltonNum = pFbxScene->GetSrcObjectCount<FbxSkeleton>();
	m_Skeletons.resize( skeltonNum );
	for( int i = 0; i < skeltonNum; i++ ){
		m_Skeletons[i] = pFbxScene->GetSrcObject<FbxSkeleton>(i);
		m_Skeletons[i]->GetNode()->GetAnimationEvaluator();
	}

	//-----------------------------------.
	// FbxMesh�̐����擾.
	//-----------------------------------.
	// FbxMesh���擾.
	int meshNum = pFbxScene->GetSrcObjectCount<FbxMesh>();	
	m_MeshClusterData.resize( meshNum );
	for( int i = 0; i < meshNum; i++ ){
		LoadSkin( pFbxScene->GetSrcObject<FbxMesh>(i), m_MeshClusterData[i] );
	}
	//-----------------------------------.
	//	�A�j���[�V�����t���[���̎擾.
	//-----------------------------------.
	GetAnimationFrame( pFbxScene );

	// �A�j���[�V�����f�[�^���󂶂�Ȃ���΃��X�g��ǉ�.
	if( m_AnimDataList.AnimList.empty() == false ){
		*outAnimDataList = m_AnimDataList;
	}

	// �]���ȃ������̍폜.
	m_Skeletons.clear();
	m_Skeletons.shrink_to_fit();
	m_MeshClusterData.clear();
	m_MeshClusterData.shrink_to_fit();

	SAFE_DESTROY( pFbxScene );
	SAFE_DESTROY( pFbxImpoter );

	return S_OK;
}

////////////////////////////////////////////////////////////////.
// �A�j���[�V�����̓ǂݍ���.
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

	// �]���ȃ������̍폜.
	m_MeshClusterData.clear();
	m_MeshClusterData.shrink_to_fit();
	m_Skeletons.clear();
	m_Skeletons.shrink_to_fit();

	return S_OK;
}

////////////////////////////////////////////////////////////////.
// �X�L�����ǂݍ���.
////////////////////////////////////////////////////////////////.
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
		// �{�[�����̎擾.
		meshClusterData.ClusterName.emplace_back( pNode->GetName() );
	}
	// �]���ȃ������̍폜.
	meshClusterData.ClusterName.shrink_to_fit();
}

////////////////////////////////////////////////////////////////.
// �L�[�t���[���̎擾.
////////////////////////////////////////////////////////////////.
void CFbxAnimationLoader::GetAnimationFrame( FbxScene* pScene )
{
	// �t���[�����[�g�̎擾.
	FbxTime::EMode timeMode = pScene->GetGlobalSettings().GetTimeMode();
	double framerate = FbxTime::GetFrameRate( timeMode );

	FbxNode* pNode = pScene->GetRootNode();
	// �A�j���[�V�����̐����擾.
	int stackCount = pScene->GetSrcObjectCount<FbxAnimStack>();
	m_AnimDataList.AnimList.clear();
	m_AnimDataList.AnimList.resize( stackCount );
	std::vector<SAnimationData>& animDataList = m_AnimDataList.AnimList;
	// �A�j���[�V�����̐���.
	for( int i = 0; i < stackCount; i++ ){
		SAnimationData& animData = animDataList[i];
		animData.FrameRate = framerate;	// �t���[�����[�g�̐ݒ�.
		FbxTimeSpan timeSpan;
		// �A�j���[�V�����f�[�^�擾.
		FbxAnimStack* animStack = pScene->GetSrcObject<FbxAnimStack>(i);
		int numLayer = animStack->GetMemberCount<FbxAnimLayer>();
		pScene->SetCurrentAnimationStack( animStack );

		// �A�j���[�V�����̂����Ȏ��Ԃ��擾.
		pNode->GetAnimationInterval( timeSpan, animStack );

		// �A�j���[�V���������擾.
		animData.Name = animStack->GetName();

		// �A�j���[�V�����̎��Ԃ��擾.
		FbxTime time = timeSpan.GetStop() - timeSpan.GetStart();
		FbxLongLong ms = time.GetMilliSeconds();	// �~���b�ɕϊ�.

		// ���Z�t���[���̐ݒ�.
		FbxTime frameTime;
		frameTime.SetTime( 0, 0, 0, 1, 0, timeMode );
		animData.AnimSpeed = frameTime.GetSecondDouble() / ( 60.0 / framerate );

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
	// �A�j���[�V�����f�[�^�����������m�F.
	for( size_t i = 0; i < animDataList.size(); i++ ){
		if( animDataList[i].EndTime < 0.0 ){
			// �A�j���[�V�����̏I�����Ԃ� 0 ��菭�Ȃ��ƁA
			// ���炩�ɂ������ȃf�[�^�Ȃ̂ŁA
			// ���̃A�j���[�V�����f�[�^���폜����.
			animDataList[i] = animDataList.back();
			animDataList.pop_back();
			i--;
		}
	}
}

////////////////////////////////////////////////////////////////.
// �A�j���[�V�����̃t���[���s����擾.
////////////////////////////////////////////////////////////////.
void CFbxAnimationLoader::GetAnimationFrameMatrix( SAnimationData& animData, FbxTimeSpan& timeSpan, FbxNode* pNode )
{
	// �A�j���[�V�����t���[��.
	int totalFrame = (int)std::ceil(animData.AnimationTime * animData.FrameRate);
	// �A�j���[�V�����̃X�^�[�g����.
	double start = timeSpan.GetStart().GetSecondDouble();

	// ���b�V���̐����̃t���[���s����擾.
	int meshNo = 0;	// ���b�V���ԍ�.
	for( auto& animSkelton : m_MeshClusterData ){
		animData.KeyFrameLinkMatrix.emplace_back();
		animData.KeyFrameLinkMatrix.back().resize( animSkelton.ClusterName.size() );
		
		
		// �X�P���g���̐������[�v.
		for( auto& s : m_Skeletons ){
			FbxNode* skeletonNode = s->GetNode();
			// �ǉ������m�[�h��������Ȃ���ΏI��.
			if( std::find( animSkelton.ClusterName.begin(), animSkelton.ClusterName.end(), 
				skeletonNode->GetName() ) == animSkelton.ClusterName.end() ) continue;

			if( animSkelton.ClusterName.size() <= 0 ) continue;

			std::map<double, FbxMatrix> keyFrame;

			// �t���[���̐���.
			for( int i = 0; i <= totalFrame; i++ ){
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
		// �]���ȃ������̍폜.
		animData.KeyFrameLinkMatrix.shrink_to_fit();

		// �t���[�����X�g��ǉ�.
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