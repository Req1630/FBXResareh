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
	std::vector<std::string>					ClusterName;
	std::unordered_map<std::string, FbxNode*>	ClusterKey;

	FBXMeshClusterData(){}
};

class CFbxAnimationLoader
{
public:
	CFbxAnimationLoader();
	~CFbxAnimationLoader();

	// �쐬.
	HRESULT Create();
	// �j��.
	void Destroy();

	// �A�j���[�V�����̓ǂݍ���.
	HRESULT LoadAnim( CFbxAnimationController* pAc, const char* fileName );

	// �A�j���[�V�����̓ǂݍ���.
	HRESULT LoadAnimationData(
		FbxScene*							pFbxScene,
		std::vector<FBXMeshClusterData>&	meshClusterData,
		std::vector<FbxSkeleton*>&			skeletons,
		std::vector<SAnimationData>*		outAnimDataList );

	// �A�j���[�V�����f�[�^���X�g�̎擾.
	std::vector<SAnimationData>& GetAnimDateList(){ return m_AnimDataList; }

private:
	// �X�L�����ǂݍ���.
	void LoadSkin( FbxMesh* pMesh, FBXMeshClusterData& clusterData );
	// �L�[�t���[���̎擾.
	void GetAnimationFrame( FbxScene* pScene );
	// �A�j���[�V�����̃t���[���s����擾.
	void GetAnimationFrameMatrix( SAnimationData& animData, FbxTimeSpan& timeSpan, FbxNode* pNode );


private:
	FbxManager* m_pFbxManager;	// FBX�}�l�[�W���[.
	FbxScene*	m_pFbxScene;	// FBX�V�[���I�u�W�F�N�g.

	std::vector<FBXMeshClusterData>	m_MeshClusterData;	// ���b�V���f�[�^.
	std::vector<FbxSkeleton*>		m_Skeletons;		// �X�P���g�����X�g.
	std::vector<SAnimationData>		m_AnimDataList;		// �A�j���[�V�����f�[�^���X�g.
};