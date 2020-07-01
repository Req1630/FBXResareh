#include "CSprite.h"
#include "..\Direct11\D3DX11.h"

//�����̧�ٖ�(�ިڸ�؂��܂�).
const wchar_t* SHADER_NAME = L"Data\\Shader\\Sprite.hlsl";

//�ݽ�׸�.
CSprite::CSprite()
	: m_pDevice11		( nullptr )
	, m_pContext11		( nullptr )
	, m_pVertexShader	( nullptr )
	, m_pVertexLayout	( nullptr )
	, m_pPixelShader	( nullptr )
	, m_pConstantBuffer	( nullptr )
	, m_pVertexBuffer	( nullptr )
	, m_vPosition		()
	, m_vRotation		()
	, m_vScale			( 1.0f, 0.0f, 0.0f )
{
}

//�޽�׸�.
CSprite::~CSprite()
{
	//����������Ăяo��.
	Release();

	//�ʂ̂Ƃ���ŊǗ����Ă���̂ł����ł�nullptr�����ď�����.
	m_pContext11 = nullptr;
	m_pDevice11 = nullptr;
}

//������.
//	ID3D11Device* pDevice11 �O���ō쐬���Ď����Ă���B
//	ID3D11DeviceContext* pContext11 �O���ō쐬���Ď����Ă���B
HRESULT CSprite::Init()
{
	m_pDevice11 = CDirectX11::GetDevice();		//���Ԃ͕ʂ̂Ƃ���ɂ���.���Ƃ����L���Ă���.
	m_pContext11 = CDirectX11::GetContext();	//���Ԃ͕ʂ̂Ƃ���ɂ���.���Ƃ����L���Ă���.

	//����ލ쐬.
	if( FAILED( CreateShader() ))
	{
		return E_FAIL;
	}
	//����غ�ݍ쐬.
	if( FAILED( CreateModel() ))
	{
		return E_FAIL;
	}

	return S_OK;
}

//���.
void CSprite::Release()
{
	SAFE_RELEASE( m_pVertexBuffer );
	SAFE_RELEASE( m_pConstantBuffer );
	SAFE_RELEASE( m_pPixelShader );
	SAFE_RELEASE( m_pVertexLayout );
	SAFE_RELEASE( m_pVertexShader );

	m_pContext11 = nullptr;
	m_pDevice11 = nullptr;
}

//===========================================================
//	HLSĻ�ق�ǂݍ��ݼ���ނ��쐬����.
//	HLSL: High Level Shading Language �̗�.
//===========================================================
HRESULT CSprite::CreateShader()
{
	ID3DBlob* pCompiledShader = nullptr;
	ID3DBlob* pErrors = nullptr;
	UINT uCompileFlag = 0;
#ifdef _DEBUG
	uCompileFlag =
		D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
#endif//#ifdef _DEBUG

	//HLSL�����ްï������ނ�����ނ��쐬.
	if (FAILED(
		D3DCompileFromFile(
			SHADER_NAME,	//�����̧�ٖ�(HLSĻ��).
			nullptr, 
			nullptr, 
			"VS_Main",
			"vs_5_0", 
			uCompileFlag,
			0, 
			&pCompiledShader,
			nullptr )))
	{
		_ASSERT_EXPR( false, "hlsl�ǂݍ��ݎ��s" );
		return E_FAIL;
	}
	SAFE_RELEASE(pErrors);

	//��L�ō쐬��������ނ���u�ްï������ށv���쐬.
	if (FAILED(
		m_pDevice11->CreateVertexShader(
			pCompiledShader->GetBufferPointer(),
			pCompiledShader->GetBufferSize(),
			nullptr,
			&m_pVertexShader)))	//(out)�ްï�������.
	{
		_ASSERT_EXPR(false, "�ްï������ލ쐬���s");
		return E_FAIL;
	}

	//���_���߯�ڲ��Ă��`.
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{
			"POSITION",						//�ʒu.
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,	//DXGI��̫�ϯ�(32bit float�^*3).
			0,
			0,								//�ް��̊J�n�ʒu.
			D3D11_INPUT_PER_VERTEX_DATA, 0
		},
		{
			"TEXCOORD",						//ø����ʒu.
			0,
			DXGI_FORMAT_R32G32_FLOAT,		//DXGI��̫�ϯ�(32bit float�^*2).
			0,
			12,								//�ް��̊J�n�ʒu.
			D3D11_INPUT_PER_VERTEX_DATA, 0
		}
	};
	//���_���߯�ڲ��Ă̔z��v�f�����Z�o.
	UINT numElements = sizeof(layout) / sizeof(layout[0]);

	//���_���߯�ڲ��Ă��쐬.
	if (FAILED(
		m_pDevice11->CreateInputLayout(
			layout,
			numElements,
			pCompiledShader->GetBufferPointer(),
			pCompiledShader->GetBufferSize(),
			&m_pVertexLayout)))	//(out)���_���߯�ڲ���.
	{
		_ASSERT_EXPR( false, "���_���߯�ڲ��č쐬���s" );
		return E_FAIL;
	}
	SAFE_RELEASE(pCompiledShader);

	//HLSL�����߸�ټ���ނ�����ނ��쐬.
	if (FAILED(
		D3DCompileFromFile(
			SHADER_NAME,	//�����̧�ٖ�(HLSĻ��).
			nullptr, 
			nullptr, 
			"PS_Main",
			"ps_5_0", 
			uCompileFlag,
			0, 
			&pCompiledShader,
			nullptr )))
	{
		_ASSERT_EXPR( false, "hlsl�ǂݍ��ݎ��s" );
		return E_FAIL;
	}
	SAFE_RELEASE(pErrors);

	//��L�ō쐬��������ނ���u�߸�ټ���ށv���쐬.
	if (FAILED(
		m_pDevice11->CreatePixelShader(
			pCompiledShader->GetBufferPointer(),
			pCompiledShader->GetBufferSize(),
			nullptr,
			&m_pPixelShader)))	//(out)�߸�ټ����.
	{
		_ASSERT_EXPR( false, "�߸�ټ���ލ쐬���s" );
		return E_FAIL;
	}
	SAFE_RELEASE(pCompiledShader);

	//�ݽ���(�萔)�ޯ̧�쐬.
	//����ނɓ���̐��l�𑗂��ޯ̧.
	//�����ł͕ϊ��s��n���p.
	//����ނ� World, View, Projection �s���n��.
	D3D11_BUFFER_DESC cb;
	cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;	//�ݽ����ޯ̧���w��.
	cb.ByteWidth = sizeof(SHADER_CONSTANT_BUFFER);//�ݽ����ޯ̧�̻���.
	cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	//�������݂ű���.
	cb.MiscFlags = 0;	//���̑����׸�(���g�p).
	cb.StructureByteStride = 0;	//�\���̻̂���(���g�p).
	cb.Usage = D3D11_USAGE_DYNAMIC;	//�g�p���@:���ڏ�������.

	//�ݽ����ޯ̧�̍쐬.
	if (FAILED(
		m_pDevice11->CreateBuffer(
			&cb,
			nullptr,
			&m_pConstantBuffer)))
	{
		_ASSERT_EXPR( false, "�ݽ����ޯ̧�쐬���s" );
		return E_FAIL;
	}


	return S_OK;
}


//���ٍ쐬.
HRESULT CSprite::CreateModel()
{
	//�����(�l�p�`)�̒��_���쐬.
	VERTEX vertices[]=
	{
		//���_���W(x,y,z)				 
		DirectX::XMFLOAT3(-1.0f,-1.0f, 0.0f ),
		DirectX::XMFLOAT3(-1.0f, 1.0f, 0.0f ),
		DirectX::XMFLOAT3( 1.0f,-1.0f, 0.0f ),
		DirectX::XMFLOAT3( 1.0f, 1.0f, 0.0f ),
	};
	//�ő�v�f�����Z�o����.
	UINT uVerMax = sizeof(vertices) / sizeof(vertices[0]);

	//�ޯ̧�\����.
	D3D11_BUFFER_DESC bd;
	bd.Usage			= D3D11_USAGE_DEFAULT;	//�g�p���@(��̫��).
	bd.ByteWidth		= sizeof(VERTEX) * uVerMax;//���_�̻���.
	bd.BindFlags		= D3D11_BIND_VERTEX_BUFFER;//���_�ޯ̧�Ƃ��Ĉ���.
	bd.CPUAccessFlags	= 0;	//CPU����ͱ������Ȃ�.
	bd.MiscFlags		= 0;	//���̑����׸�(���g�p).
	bd.StructureByteStride = 0;	//�\���̻̂���(���g�p).

	//���ؿ���ް��\����.
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = vertices;	//����؂̒��_���.

	//���_�ޯ̧�̍쐬.
	if (FAILED(m_pDevice11->CreateBuffer(
		&bd, &InitData, &m_pVertexBuffer)))
	{
		_ASSERT_EXPR( false, "���_�ޯ̧�쐬���s" );
		return E_FAIL;
	}

	//���_�ޯ̧���.
	UINT stride = sizeof(VERTEX);	//�ް��Ԋu.
	UINT offset = 0;
	m_pContext11->IASetVertexBuffers(
		0, 1,
		&m_pVertexBuffer, &stride, &offset);



	return S_OK;
}

//�����ݸޗp.
void CSprite::Render()
{
	// �p�����[�^�̌v�Z
	DirectX::XMVECTOR eye_pos		= DirectX::XMVectorSet(0.0f, 70.0f, 500.0f, 1.0f);
	DirectX::XMVECTOR eye_lookat	= DirectX::XMVectorSet(0.0f, 70.0f, 0.0f, 1.0f);
	DirectX::XMVECTOR eye_up		= DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
	DirectX::XMMATRIX World	= DirectX::XMMatrixRotationY( 0.1f );
	DirectX::XMMATRIX View	= DirectX::XMMatrixLookAtLH( eye_pos, eye_lookat, eye_up );
	DirectX::XMMATRIX Proj	= DirectX::XMMatrixPerspectiveFovLH( DirectX::XM_PIDIV4, (FLOAT)WND_W/(FLOAT)WND_H, 0.1f, 800.0f );
	DirectX::XMMATRIX mTrans, mRot, mScale;

	//�g��k���s��.
	mScale = 
		DirectX::XMMatrixScaling( m_vScale.x, m_vScale.y, m_vScale.z );
	//��]�s��.
	DirectX::XMMATRIX mYaw, mPitch, mRoll;
	mYaw = DirectX::XMMatrixRotationY( m_vRotation.y );
	mPitch = DirectX::XMMatrixRotationY( m_vRotation.x );
	mRoll = DirectX::XMMatrixRotationY( m_vRotation.z );
	mRot = mYaw * mPitch * mRoll;

	//���s�s��(���s�ړ�).
	mTrans = DirectX::XMMatrixTranslation(
		m_vPosition.x, m_vPosition.y, m_vPosition.z );

	//ܰ��ލ��W�ϊ�.
	//�d�v: �g�k�s�� * ��]�s�� * ���s�s��.
//	World = mScale * mRot * mTrans;


	//�g�p���鼪��ނ̓o�^.
	m_pContext11->VSSetShader( m_pVertexShader, nullptr, 0 );
	m_pContext11->PSSetShader( m_pPixelShader, nullptr, 0 );

	//����ނ̺ݽ����ޯ̧�Ɋe���ް���n��.
	D3D11_MAPPED_SUBRESOURCE pData;
	SHADER_CONSTANT_BUFFER cb;	//�ݽ����ޯ̧.
	//�ޯ̧�����ް��̏��������J�n����map.
	if (SUCCEEDED(
		m_pContext11->Map(m_pConstantBuffer,
			0, D3D11_MAP_WRITE_DISCARD, 0, &pData)))
	{
		//ܰ���,�ޭ�,��ۼު���ݍs���n��.
		DirectX::XMMATRIX m = XMMatrixTranspose( World * View * Proj );

		m = XMMatrixTranspose( World * View * Proj );

		cb.mWVP = m;

		//�װ.
		cb.vColor = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

		memcpy_s(pData.pData, pData.RowPitch,
			(void*)(&cb), sizeof(cb));

		m_pContext11->Unmap(m_pConstantBuffer, 0);
	}

	//���̺ݽ����ޯ̧���ǂ̼���ނŎg�����H.
	m_pContext11->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	m_pContext11->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);

	//���_�ޯ̧���.
	UINT stride = sizeof(VERTEX);//�ް��̊Ԋu.
	UINT offset = 0;
	m_pContext11->IASetVertexBuffers(0, 1,
		&m_pVertexBuffer, &stride, &offset);

	//���_���߯�ڲ��Ă��.
	m_pContext11->IASetInputLayout(m_pVertexLayout);
	//����è�ށE���ۼް���.
	m_pContext11->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//����è�ނ������ݸ�.
	m_pContext11->Draw(4, 0);//�����(���_4��).

}
