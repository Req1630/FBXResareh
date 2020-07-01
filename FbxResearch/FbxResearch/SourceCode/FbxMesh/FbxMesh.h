#ifndef FBX_MESH_H
#define FBX_MESH_H

#include "..\Global.h"
#include "FbxInclude\FbxInclude.h"

#include <vector>
#include <map>
#include <set>

#include "FbxMeshData.h"
#include "FbxAnimationData.h"
#include "FbxAnimation/FbxAnimationLoader.h"
#include "FbxAnimationController.h"

/***********************************
*	FBX���f���N���X.
**/
class CFbxMesh
{
	// �V�F�[�_�[��.
	const wchar_t* SHADER_NAME = L"Data\\Shader\\SimpleShader.hlsl";

	/***************************************
	*			��	�\���́@��.
	***************************************/
	//======================================.
	//	�R���X�^���g�o�b�t�@(���b�V����).
	//======================================.
	struct CBUFFER_PER_MESH
	{
		DirectX::XMMATRIX mW;	// ���[���h�s��.
		DirectX::XMMATRIX mWVP;	// World,View,Proj �̍����ϊ��s��.
		DirectX::XMFLOAT4 IsAnimation;
	};
	//======================================.
	// �R���X�^���g�o�b�t�@(�}�e���A����).
	//======================================.
	struct CBUFFER_PER_MATERIAL
	{
		DirectX::XMFLOAT4	Ambient;	// �A���r�G���g.
		DirectX::XMFLOAT4	Diffuse;	// �f�B�q���[�Y.
		DirectX::XMFLOAT4	Specular;	// �X�y�L����.
	};
	//======================================.
	// �R���X�^���g�o�b�t�@(�{�[����).
	//======================================.
	struct CBUFFER_PER_BONE
	{
		DirectX::XMMATRIX Bone[255];
		CBUFFER_PER_BONE()
		{
			// �s��̏�����.
			for( int i = 0; i < 255; i++ ){
				Bone[i] = DirectX::XMMatrixIdentity();
			}
		}
	};

public:
	CFbxMesh();
	~CFbxMesh();

	//-----------------------------------------.
	//				�쐬.
	//-----------------------------------------.
	HRESULT Create( ID3D11DeviceContext* pContext11, const char* fileName );

	//-----------------------------------------.
	//				�j��.
	//-----------------------------------------.
	void Destroy();

	//-----------------------------------------.
	//				�`��.
	//-----------------------------------------.
	void Render( const DirectX::XMMATRIX& view, 
				 const DirectX::XMMATRIX& proj,
				 CFbxAnimationController* pAc = nullptr );

	//-----------------------------------------.
	//			�p�����[�^�擾�n.
	//-----------------------------------------.

	// ���[���h�s����擾.
	DirectX::XMMATRIX GetWorldMatrix();

	// �A�j���[�V�����R���g���[���[�̎擾.
	CFbxAnimationController GetAnimationController()
	{
		if( m_pAc != nullptr ) return *m_pAc;
		return CFbxAnimationController();
	}

	//-----------------------------------------.
	//			�p�����[�^�ݒ�n.
	//-----------------------------------------.

	// ���W�̐ݒ�.
	void SetPosition( const DirectX::XMFLOAT3& pos ){ m_Position = pos; }
	// ��]�̐ݒ�.
	void SetRotation( const DirectX::XMFLOAT3& rot ){ m_Rotation = rot; }
	// �傫���̐ݒ�.
	void SetScale( const DirectX::XMFLOAT3& scale ){ m_Scale = scale; }
	void SetScale( const float& scale ){ m_Scale = { scale, scale, scale }; }

	// �A�j���[�V�������x�̐ݒ�.
	void SetAnimSpeed( const double& speed )
	{
		if( m_pAc == nullptr ) return;
		m_pAc->SetAnimSpeed( speed );
	}
	// �A�j���[�V�������x�̐ݒ�.
	void ChangeAnimation( int& index )
	{
		if( m_pAc == nullptr ) return;
		m_pAc->ChangeAnimation( index );
	}

private:
	//-----------------------------------------.
	//		�`��p�f�[�^�̍쐬�n.
	//-----------------------------------------.

	// �o�b�t�@�̍쐬.
	HRESULT CreateBuffer();
	// ���_�o�b�t�@�쐬.
	HRESULT CreateVertexBuffers( FBXMeshData& meshData );
	// �C���f�b�N�X�o�b�t�@�쐬.
	HRESULT CreateIndexBuffers( FBXMeshData& meshData );
	// �T���v���[�쐬.
	HRESULT CreateSampler();
	// �R���X�^���g�o�b�t�@�쐬.
	HRESULT CreateConstantBuffer( const size_t& byte, ID3D11Buffer** buffer );

	// �V�F�[�_�[�̍쐬.
	HRESULT CreateShader();

	//-----------------------------------------.
	//			�}�e���A���n.
	//-----------------------------------------.

	// �}�e���A���擾.
	void GetMaterial( FbxMesh* pMesh, FBXMeshData& mesh, const char* fileName );
	// �e�N�X�`���̓ǂݍ���.
	HRESULT LoadTexture( FbxFileTexture* texture, const char* fileName, const char* keyName );

	//-----------------------------------------.
	//		�A�j���[�V�����n.
	//-----------------------------------------.
	 
	// �A�j���[�V�����p�̍s��v�Z.
	void AnimMatrixCalculation( const int& meahNo, FBXMeshData& meshData, CFbxAnimationController* pAc );

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
	//			�m�[�h�̏���.
	//-----------------------------------------.

	// �m�[�h�����ǂ�ċN�֐�.
	void RecurrenceNode( FbxNode* pNode );
	// �m�[�h�^�C�v�ʎ擾�֐�.
	void GetDataByNodeType( FbxNode* pNode );

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

	/***************************************
	*			�V�F�[�_�[.
	***************************************/
	ID3D11VertexShader*	m_pVertexShader;		// ���_�V�F�[�_�[.
	ID3D11PixelShader*	m_pPixelShader;			// �s�N�Z���V�F�[�_�[.

	/***************************************
	*			�`��p�f�[�^.
	***************************************/
	ID3D11InputLayout*	m_pVertexLayout;		// ���_���C�A�E�g.
	ID3D11Buffer*		m_pCBufferPerMesh;		// �R���X�^���g�o�b�t�@.
	ID3D11Buffer*		m_pCBufferPerMaterial;	// �R���X�^���g�o�b�t�@.
	ID3D11Buffer*		m_pCBufferPerBone;		// �R���X�^���g�o�b�t�@.
	ID3D11SamplerState*	m_pSampleLinear;		// �T���v��:�e�N�X�`���Ɋe��t�B���^��������.
	std::vector<FBXMeshData>	m_MeshData;		// ���b�V���f�[�^.
	std::vector<FbxSkeleton*>	m_Skeletons;	// �X�P���g�����.
	std::map<std::string, ID3D11ShaderResourceView*> m_Textures;	// �e�N�X�`�����X�g.

	CFbxAnimationController* m_pAc;	// �A�j���[�V�����R���g���[���[.
	std::unique_ptr<CFbxAnimationLoader> m_pAnimLoader;
	std::vector<FBXMeshClusterData>	m_MeshClusterData;

	/***************************************
	*			�p�����[�^.
	***************************************/
	DirectX::XMFLOAT3 m_Position;	// ���W.
	DirectX::XMFLOAT3 m_Rotation;	// ��].
	DirectX::XMFLOAT3 m_Scale;		// �傫��.
};

#endif	// #ifndef FBX_MESH_H.