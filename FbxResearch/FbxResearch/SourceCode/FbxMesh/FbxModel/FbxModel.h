#pragma once

#include "..\FbxInclude\FbxInclude.h"
#include "..\FbxMeshData.h"
#include "..\FbxAnimation\FbxAnimationData.h"

#include <unordered_map>

class CFbxAnimationController;

class CFbxModel
{
public:
	CFbxModel();
	~CFbxModel();

	//-----------------------------------------.
	//				Fbx���f���n.
	//-----------------------------------------.

	// ���b�V���f�[�^�̎擾.
	std::vector<FBXMeshData>& GetMeshData();
	// ���b�V���f�[�^�̃��T�C�Y.
	void ReSizeMeshData( const int& size );
	// �e�N�X�`���̎擾.
	std::unordered_map<std::string, ID3D11ShaderResourceView*>& GetTextures();
	// �A�j���[�V�����R���g���[���[�̎擾.
	CFbxAnimationController* GetPtrAC();
	// �A�j���[�V�����f�[�^�̐ݒ�.
	void SetAnimationData( const SAnimationDataList& animDataList );

	//-----------------------------------------.
	//			�p�����[�^�擾.
	//-----------------------------------------.

	// ���[���h�s����擾.
	DirectX::XMMATRIX GetWorldMatrix();

	// �A�j���[�V�����R���g���[���[�̎擾.
	CFbxAnimationController GetAnimationController();

	//-----------------------------------------.
	//			�p�����[�^�ݒ�.
	//-----------------------------------------.

	// ���W�̐ݒ�.
	void SetPosition( const DirectX::XMFLOAT3& pos );
	// ��]�̐ݒ�.
	void SetRotation( const DirectX::XMFLOAT3& rot );
	// �傫���̐ݒ�.
	void SetScale( const DirectX::XMFLOAT3& scale );
	void SetScale( const float& scale );

	// �A�j���[�V�������x�̐ݒ�.
	void SetAnimSpeed( const double& speed );
	// �A�j���[�V�����̕ύX.
	void ChangeAnimation( int& index );

private:
	/***************************************
	*			Fbx���f���n.
	***************************************/
	std::vector<FBXMeshData> m_MeshData;	// ���b�V���f�[�^.
	std::unordered_map<std::string, ID3D11ShaderResourceView*> m_Textures;	// �e�N�X�`�����X�g.
	CFbxAnimationController* m_pAc;			// �A�j���[�V�����R���g���[���[.

	/***************************************
	*			�p�����[�^.
	***************************************/
	DirectX::XMFLOAT3 m_Position;	// ���W.
	DirectX::XMFLOAT3 m_Rotation;	// ��].
	DirectX::XMFLOAT3 m_Scale;		// �傫��.
};