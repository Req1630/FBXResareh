#pragma once

//�x���ɂ��Ă̺��ޕ��͂𖳌��ɂ���.4005:�Ē�`.
#pragma warning(disable:4005)

#include "..\Global.h"

#include <vector>

/**************************************************
*	���ײĸ׽.
**/
class CSprite
{
public:
	//======================================
	//	�\����.
	//======================================
	//�ݽ����ޯ̧�̱��ؑ��̒�`.
	//������ޓ��̺ݽ����ޯ̧�ƈ�v���Ă���K�v����.
	struct C_BUFFER
	{
		ALIGN16 DirectX::XMMATRIX	mW;				// ���[���h�s��.
		ALIGN16 DirectX::XMMATRIX	mWVP;
		ALIGN16 DirectX::XMFLOAT4	vColor;			// �J���[(RGBA�̌^�ɍ��킹��).
		ALIGN16 DirectX::XMFLOAT2	vUV;			// UV���W.
		ALIGN16 DirectX::XMFLOAT2	vViewPort;		// UV���W.
	};
	//���_�̍\����.
	struct VERTEX
	{
		DirectX::XMFLOAT3 Pos;	//���_���W.
		DirectX::XMFLOAT2 Tex;
	};

public:
	CSprite();	//�ݽ�׸�.
	~CSprite();	//�޽�׸�.

	//������.
	HRESULT Init( ID3D11DeviceContext* pContext11 );

	//���.
	void Release();

	//����ލ쐬.
	HRESULT CreateShader();
	//���ٍ쐬.
	HRESULT CreateModel();
	// �T���v���̍쐬.
	HRESULT InitSample();

	//�����ݸޗp.
	void Render( ID3D11ShaderResourceView* pSRV );
	void Render( std::vector<ID3D11ShaderResourceView*> gbuffers );

	//���W����ݒ�.
	void SetPosition(const DirectX::XMFLOAT3& vPos) {	m_vPosition = vPos;	}
	//���Wx��ݒ�.
	void SetPositionX( float x ){ m_vPosition.x = x; }
	//���Wy��ݒ�.
	void SetPositionY( float y ){ m_vPosition.y = y; }
	//���Wz��ݒ�.
	void SetPositionZ( float z ){ m_vPosition.z = z; }

	//��]����ݒ�.
	void SetRotation(const DirectX::XMFLOAT3& vRot){	m_vRotation = vRot;	}
	//��]��Y��ݒ�(Yaw).
	void SetRotationY(float y){	m_vRotation.y = y;	}
	//��]��X��ݒ�(Pitch).
	void SetRotationX(float x){	m_vRotation.x = x;	}
	//��]��Z��ݒ�(Roll).
	void SetRotationZ(float z){	m_vRotation.z = z;	}

	//�g��k������ݒ�.
	void SetScale( const DirectX::XMFLOAT3& vScale ){ m_vScale = vScale; }

private:
	ID3D11Device*			m_pDevice11;
	ID3D11DeviceContext*	m_pContext11;

	ID3D11VertexShader*		m_pVertexShader;	//���_�����.
	ID3D11InputLayout*		m_pVertexLayout;	//���_ڲ���.
	ID3D11PixelShader*		m_pPixelShader;		//�߸�ټ����.
	ID3D11PixelShader*		m_pPixelShaderLast;		//�߸�ټ����.
	ID3D11Buffer*			m_pConstantBuffer;	//�ݽ����ޯ̧.
	ID3D11Buffer*			m_pVertexBuffer;	//���_�ޯ̧.
	ID3D11SamplerState*		m_pSampleLinear;		// �T���v��:�e�N�X�`���Ɋe��t�B���^��������.

	DirectX::XMFLOAT3		m_vPosition;	//���W.
	DirectX::XMFLOAT3		m_vRotation;	//��]
	DirectX::XMFLOAT3		m_vScale;		//�g�k.
};