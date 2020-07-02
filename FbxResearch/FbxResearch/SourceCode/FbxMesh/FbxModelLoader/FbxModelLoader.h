#pragma once

#include "..\..\Global.h"
#include "..\FbxInclude\FbxInclude.h"

#include <vector>
#include <unordered_map>
#include <set>

#include "..\FbxMeshData.h"
#include "..\FbxAnimation/FbxAnimationLoader.h"

class CFbxModel;	// Fbx���f���N���X.

class CFbxModelLoader
{
public:
	CFbxModelLoader();
	~CFbxModelLoader();

	//-----------------------------------------.
	//				�쐬.
	//-----------------------------------------.
	HRESULT Create( ID3D11DeviceContext* pContext11 );

	//-----------------------------------------.
	//				�j��.
	//-----------------------------------------.
	void Destroy();

	//-----------------------------------------.
	//			���f���̓ǂݍ���.
	//-----------------------------------------.
	HRESULT LoadModel( CFbxModel* pModelData, const char* fileName );
	
private:
	//-----------------------------------------.
	//			�}�e���A���n.
	//-----------------------------------------.

	// �}�e���A���擾.
	void GetMaterial( FbxMesh* pMesh, FBXMeshData& mesh, const char* fileName, std::unordered_map<std::string, ID3D11ShaderResourceView*>& textures );
	// �e�N�X�`���̓ǂݍ���.
	HRESULT LoadTexture( FbxFileTexture* texture, const char* fileName, const char* keyName, std::unordered_map<std::string, ID3D11ShaderResourceView*>& textures );

	//-----------------------------------------.
	//		���b�V���f�[�^�̓ǂݍ���.
	//-----------------------------------------.

	// ���b�V���̓ǂݍ���.
	void LoadMesh( FbxMesh* pMesh, FBXMeshData& meshData );
	// �C���f�b�N�X���ǂݍ���.
	void LoadIndices( FbxMesh* pMesh, FBXMeshData& meshData );
	// ���_���ǂݍ���.
	void LoadVertices( FbxMesh* pMesh, VERTEX& vertex, int ctrlPointIndex );
	// �@�����ǂݍ���.
	void LoadNormals( FbxMesh* pMesh, VERTEX& vertex, int ctrlPointIndex, int vertexCounter );
	// ���_�J���[�ǂݍ���.
	void LoadColors( FbxMesh* pMesh, VERTEX& vertex, int ctrlPointIndex, int vertexCounter );
	// UV���ǂݍ���.
	void LoadUV( FbxMesh* pMesh, VERTEX& vertex, int ctrlPointIndex, int texUVIndex, int uvLayer );
	// �X�L�����ǂݍ���.
	void LoadSkin( FbxMesh* pMesh, FBXMeshData& meshData, std::vector<std::vector<float>>& weights, std::vector<std::vector<int>>& bones );
	// �E�F�C�g�ƃ{�[���̐ݒ�.
	void SetBoneWeight( VERTEX& vertex, const std::vector<float>& weight, const std::vector<int>& bone );

	//-----------------------------------------.
	//		�`��p�f�[�^�̍쐬�n.
	//-----------------------------------------.

	// ���_�o�b�t�@�쐬.
	HRESULT CreateVertexBuffers( FBXMeshData& meshData );
	// �C���f�b�N�X�o�b�t�@�쐬.
	HRESULT CreateIndexBuffers( FBXMeshData& meshData );
private:
	/***************************************
	*			DirectX11.
	***************************************/
	ID3D11Device*			m_pDevice11;	// �f�o�C�X11.
	ID3D11DeviceContext*	m_pContext11;	// �R���e�L�X�g11.
	/***************************************
	*			FBX SDK.
	***************************************/
	FbxManager* m_pFbxManager;	// FBX�}�l�[�W���[.
	FbxScene*	m_pFbxScene;	// FBX�V�[���I�u�W�F�N�g.
	std::unique_ptr<CFbxAnimationLoader> m_pAnimLoader;	// �A�j���[�V�����ǂݍ��݃N���X.
	std::vector<FBXMeshClusterData>	m_MeshClusterData;	// ���b�V���̃N���X�^�[���.

	std::vector<FbxSkeleton*>	m_Skeletons;	// �X�P���g�����.
};