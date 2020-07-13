#ifndef FBX_MODEL_LOADER_H
#define FBX_MODEL_LOADER_H

#include "..\FbxInclude\FbxInclude.h"
#include "..\FbxMeshData.h"
#include "..\FbxAnimation\FbxAnimationLoader.h"

#include <vector>
#include <unordered_map>
#include <set>

class CFbxModel;	// Fbx���f���N���X.

/************************************************************************
	CFbxModel��ǂݍ��ނ��߂̃N���X.
-------------------------------------------------------------------------
*	���f���͑��p�|���S���ł��s���邪�A�ǂݍ��݂��x���Ȃ�̂ŁA
*		�O���t�B�b�J�[�ɗ���ŎO�p�|���S���ŏo�͂��Ă�����������ǂ�.
*	���f���ƃe�N�X�`���͓����K�w�ɓ���Ēu������.
***/
class CFbxModelLoader
{
public:
	CFbxModelLoader();
	~CFbxModelLoader();

	//-----------------------------------------.
	//		�쐬.
	//-----------------------------------------.
	HRESULT Create( ID3D11Device* pDevice );

	//-----------------------------------------.
	//		�j��.
	//-----------------------------------------.
	void Destroy();

	//-----------------------------------------.
	//		���f���̓ǂݍ���.
	//-----------------------------------------.
	HRESULT LoadModel( CFbxModel* pModelData, const char* fileName );
	
private:
	//-----------------------------------------.
	//		�}�e���A���̓ǂݍ���.
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
	void LoadSkin( FbxMesh* pMesh, SkinData& skinData, std::vector<std::vector<float>>& weights, std::vector<std::vector<int>>& bones );
	// �E�F�C�g�ƃ{�[���̐ݒ�.
	void SetBoneWeight( VERTEX& vertex, const std::vector<float>& weight, const std::vector<int>& bone );

	//-----------------------------------------.
	//		�`��p�f�[�^�̍쐬.
	//-----------------------------------------.

	// ���_�o�b�t�@�쐬.
	HRESULT CreateVertexBuffers( FBXMeshData& meshData );
	// �C���f�b�N�X�o�b�t�@�쐬.
	HRESULT CreateIndexBuffers( FBXMeshData& meshData );

	//-----------------------------------------.
	//		�{�[�����n.
	//-----------------------------------------.

	// �{�[�������X�g�̍쐬.
	void CreateBoneNameList( CFbxModel* pModelData, const char* fileName );
	// �{�[�����X�g���e�L�X�g�ŏ�������.
	void WritingBoneNameList( std::map<std::string, std::pair<int, int>>& boneList, const char* fileName );

private:
	/***************************************
	*			DirectX11.
	***************************************/
	ID3D11Device*			m_pDevice11;	// �f�o�C�X11.

	/***************************************
	*			FBX SDK.
	***************************************/
	FbxManager* m_pFbxManager;	// FBX�}�l�[�W���[.
	FbxScene*	m_pFbxScene;	// FBX�V�[���I�u�W�F�N�g.
	std::vector<FBXMeshClusterData>	m_MeshClusterData;	// ���b�V���̃N���X�^�[���.
	std::vector<FbxSkeleton*>		m_Skeletons;		// �X�P���g�����.
};

#endif	// #ifndef FBX_MODEL_LOADER_H.