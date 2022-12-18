#pragma once

#include "..\FbxInclude\FbxInclude.h"
#include "FbxAnimationData.h"

#include <Windows.h>
#include <vector>
#include <unordered_map>
#include <set>

class CFbxAnimationController;

// ���b�V���̃N���X�^�[(�{�[��)�̏��.
struct FBXMeshClusterData
{
	std::vector<std::string> ClusterName;	// �����p�̃N���X��.
};

/***************************************************************************
	�A�j���[�V�����f�[�^��ǂݍ��ނ��߂̃N���X.
----------------------------------------------------------------------------
*	���f���͑��p�|���S���ł��s���邪�A�ǂݍ��݂��x���Ȃ�̂ŁA
*		�O���t�B�b�J�[�ɗ���ŎO�p�|���S���ŏo�͂��Ă�����������ǂ�.
*	�A�j���[�V�������f���̓��b�V����񂪂���.
*	�X�L����񂾂��̃A�j���[�V�������f���͓ǂݍ��߂Ȃ�.
*	LoadAnimationData �� CFbxModelLoader �Ŏg�p���Ă���̂ŁA
*		�O���ł͎g�p���Ȃ��Ǝv��.
----------------------------------------------------------------------------
*	@brief : �A�j���[�V�����f�[�^��ǂݍ��ފ֐�.
*	@proc LoadAnim( CFbxAnimationController* pAc, const char* fileName ).
*	@param pAc		: �ǂݍ��܂������A�j���[�V�����R���g���[���[.
*	@param fileName	: �ǂݍ��݂����A�j���[�V�������f���̃t�@�C���p�X.
*	@return HRESULT : �G���[���e�Ȃ�.
***/
class CFbxAnimationLoader
{
public:
	CFbxAnimationLoader();
	~CFbxAnimationLoader();

	//-----------------------------------------.
	//	�쐬.
	//-----------------------------------------.
	HRESULT Create();

	//-----------------------------------------.
	//	�j��.
	//-----------------------------------------.
	void Destroy();

	//-----------------------------------------.
	//	�A�j���[�V�����̓ǂݍ���.
	//-----------------------------------------.
	HRESULT LoadAnim( SAnimationDataList* outAnimDataList, const char* fileName );

	//-----------------------------------------.
	//	�A�j���[�V�����̓ǂݍ���.
	//-----------------------------------------.
	HRESULT LoadAnimationData(
		FbxScene*							pFbxScene,
		std::vector<FBXMeshClusterData>&	meshClusterData,
		std::vector<FbxSkeleton*>&			skeletons,
		SAnimationDataList*					outAnimDataList );

private:
	//-----------------------------------------.
	//	�A�j���[�V�����̓ǂݎ�݌n.
	//-----------------------------------------.

	// �X�L�����ǂݍ���.
	void LoadSkin( FbxMesh* pMesh, FBXMeshClusterData& clusterData );
	// �L�[�t���[���̎擾.
	void GetAnimationFrame( FbxScene* pScene );
	// �A�j���[�V�����̃t���[���s����擾.
	void GetAnimationFrameMatrix( SAnimationData& animData, FbxTimeSpan& timeSpan, FbxNode* pNode );

private:
	/***************************************
	*			FBX SDK.
	***************************************/
	FbxManager* m_pFbxManager;	// FBX�}�l�[�W���[.

	/***************************************
	*		�A�j���[�V�����f�[�^�n.
	***************************************/
	std::vector<FBXMeshClusterData>	m_MeshClusterData;	// ���b�V���f�[�^.
	std::vector<FbxSkeleton*>		m_Skeletons;		// �X�P���g�����X�g.
	SAnimationDataList				m_AnimDataList;		// �A�j���[�V�����f�[�^���X�g.
};