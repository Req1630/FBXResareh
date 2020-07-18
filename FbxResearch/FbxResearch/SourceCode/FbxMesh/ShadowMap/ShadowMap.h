#pragma once

#include "..\FbxInclude\FbxInclude.h"

class CFbxModel;
class CFbxAnimationController;
class CCamera;
class CLight;

class CShadowMap
{
	const wchar_t* SHADOW_SHADER_NAME	= L"Data\\Shader\\ShadowMap.hlsl";

	/***************************************
	*			��	�\���́@��.
	***************************************/
	//======================================.
	//	�R���X�^���g�o�b�t�@(���b�V����).
	//======================================.
	struct CBUFFER_PER_MESH
	{
		DirectX::XMMATRIX mLightWVP;
	};
public:
	CShadowMap();
	~CShadowMap();


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
		CLight& light );

private:
	//-----------------------------------------.
	//		�`��p�f�[�^�̍쐬�n.
	//-----------------------------------------.

	// �萔�o�b�t�@�̍쐬.
	HRESULT CreateBuffer();
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
	ID3D11VertexShader*	m_pVertexShader;	// ���_�V�F�[�_�[.
	ID3D11PixelShader*	m_pPixelShader;		// �s�N�Z���V�F�[�_�[.

	/***************************************
	*			�`��p�f�[�^.
	***************************************/
	ID3D11InputLayout*	m_pVertexLayout;	// ���_���C�A�E�g.
	ID3D11Buffer*		m_pCBufferPerMesh;	// �R���X�^���g�o�b�t�@.
};