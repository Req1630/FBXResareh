#pragma once

#include "..\..\Global.h"
#include "..\FbxMeshData.h"

class CFbxModel;
class CFbxAnimationController;
class CCamera;
class CLight;

class CFbxRenderer
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
		DirectX::XMMATRIX mLightWVP;
		DirectX::XMFLOAT4 CameraPos;
		DirectX::XMFLOAT4 LightDir;
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
	CFbxRenderer();
	~CFbxRenderer();

	//-----------------------------------------.
	//				�쐬.
	//-----------------------------------------.
	HRESULT Create( ID3D11DeviceContext* pContext11 );

	//-----------------------------------------.
	//				�j��.
	//-----------------------------------------.
	void Destroy();

	//-----------------------------------------.
	//				�`��.
	//-----------------------------------------.
	void Render(
		CFbxModel& mdoel,
		CCamera& camera,
		CLight& light,
		CFbxAnimationController* pAc = nullptr );

private:
	// �A�j���[�V�����p�̍s��v�Z.
	void AnimMatrixCalculation( 
		CFbxModel& mdoel,
		const int& meahNo, 
		FBXMeshData& meshData, 
		CFbxAnimationController* pAc );

	//-----------------------------------------.
	//		�`��p�f�[�^�̍쐬�n.
	//-----------------------------------------.

	// �萔�o�b�t�@�̍쐬.
	HRESULT CreateBuffer();
	// �T���v���[�쐬.
	HRESULT CreateSampler();
	// �萔�o�b�t�@�쐬.
	HRESULT CreateConstantBuffer( const size_t& byte, ID3D11Buffer** buffer );

	// �V�F�[�_�[�̍쐬.
	HRESULT CreateShader();

private:
	/***************************************
	*			DirectX11.
	***************************************/
	ID3D11Device*			m_pDevice11;	// �f�o�C�X11.
	ID3D11DeviceContext*	m_pContext11;	// �R���e�L�X�g11.

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

	/***************************************
	*			�p�����[�^.
	***************************************/
	DirectX::XMFLOAT3 m_Position;	// ���W.
	DirectX::XMFLOAT3 m_Rotation;	// ��].
	DirectX::XMFLOAT3 m_Scale;		// �傫��.
};