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
	, m_pSampleLinear	( nullptr )
	, m_vPosition		( 0.0f, 0.0f, 0.0f )
	, m_vRotation		( 0.0f, 0.0f, 0.0f )
	, m_vScale			( 1.0f, 1.0f, 1.0f )
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
HRESULT CSprite::Init( ID3D11DeviceContext* pContext11 )
{
	if( pContext11 == nullptr ) return E_FAIL;
	// �R���e�L�X�g�̎擾.
	m_pContext11 = pContext11;
	// �f�o�C�X�̎擾.
	m_pContext11->GetDevice( &m_pDevice11 );
	if( m_pDevice11 == nullptr ) return E_FAIL;

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
	// �T���v���̍쐬.
	if( FAILED( InitSample() ))
	{
		return E_FAIL;
	}

	return S_OK;
}

//���.
void CSprite::Release()
{
	SAFE_RELEASE( m_pSampleLinear );
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
	ID3DBlob* pCompileVS = nullptr;
	ID3DBlob* pCompilePS = nullptr;
	UINT uCompileFlag = 0;
#ifdef _DEBUG
	uCompileFlag =
		D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
#endif	// #ifdef _DEBUG
	//----------------------------.
	// ���_�V�F�[�_�[.
	//----------------------------.
	if( FAILED(
		D3DCompileFromFile( 
			SHADER_NAME, 
			nullptr, 
			nullptr, 
			"VS_MainUI",
			"vs_5_0", 
			uCompileFlag,
			0, 
			&pCompileVS, 
			nullptr ))){
			ERROR_MESSAGE( "vs hlsl Loading Failure." );
			return E_FAIL;
	}
	if( FAILED(
		m_pDevice11->CreateVertexShader(
			pCompileVS->GetBufferPointer(),
			pCompileVS->GetBufferSize(),
			nullptr,
			&m_pVertexShader ))){
		ERROR_MESSAGE( "vs hlsl Creating Failure." );
		return E_FAIL;
	}
	//----------------------------.
	// �s�N�Z���V�F�[�_�[.
	//----------------------------.
	if( FAILED(
		D3DCompileFromFile( 
			SHADER_NAME, 
			nullptr, 
			nullptr, 
			"PS_Main", 
			"ps_5_0", 
			uCompileFlag, 
			0, 
			&pCompilePS, 
			nullptr ))){
		ERROR_MESSAGE( "ps hlsl Loading Failure." );
		return E_FAIL;
	}
	if( FAILED(
		m_pDevice11->CreatePixelShader(
			pCompilePS->GetBufferPointer(),
			pCompilePS->GetBufferSize(),
			nullptr,
			&m_pPixelShader ))){
		ERROR_MESSAGE( "ps hlsl Creating Failure." );
		return E_FAIL;
	}
	//----------------------------.
	// �s�N�Z���V�F�[�_�[.
	//----------------------------.
	if( FAILED(
		D3DCompileFromFile( 
			SHADER_NAME, 
			nullptr, 
			nullptr, 
			"PS_MainLast", 
			"ps_5_0", 
			uCompileFlag, 
			0, 
			&pCompilePS, 
			nullptr ))){
		ERROR_MESSAGE( "ps hlsl Loading Failure." );
		return E_FAIL;
	}
	if( FAILED(
		m_pDevice11->CreatePixelShader(
			pCompilePS->GetBufferPointer(),
			pCompilePS->GetBufferSize(),
			nullptr,
			&m_pPixelShaderLast ))){
		ERROR_MESSAGE( "ps hlsl Creating Failure." );
		return E_FAIL;
	}
	//----------------------------.
	//	���_�C���v�b�g.
	//----------------------------.
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",		0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	int numElements = sizeof(layout)/sizeof(layout[0]);	//�v�f���Z�o.
	if (FAILED(
		m_pDevice11->CreateInputLayout(
			layout,
			numElements,
			pCompileVS->GetBufferPointer(),
			pCompileVS->GetBufferSize(),
			&m_pVertexLayout ))){
		ERROR_MESSAGE( "vs layout Creating Failure." );
		return E_FAIL;
	}

	SAFE_RELEASE( pCompileVS );
	SAFE_RELEASE( pCompilePS );

	return S_OK;
}

HRESULT CSprite::InitSample()
{
	// �e�N�X�`���p�̃T���v���\����.
	D3D11_SAMPLER_DESC samDesc;
	ZeroMemory( &samDesc, sizeof( samDesc ) );
	samDesc.Filter		= D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samDesc.AddressU	= D3D11_TEXTURE_ADDRESS_WRAP;
	samDesc.AddressV	= D3D11_TEXTURE_ADDRESS_WRAP;
	samDesc.AddressW	= D3D11_TEXTURE_ADDRESS_WRAP;
	samDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samDesc.MinLOD         = 0;
	samDesc.MaxLOD         = D3D11_FLOAT32_MAX;
	// �T���v���쐬.
	if( FAILED( m_pDevice11->CreateSamplerState(
		&samDesc, &m_pSampleLinear ) ) )
	{
		ERROR_MESSAGE( "SamplerState creation failed" );
		return E_FAIL;
	}
	

	D3D11_BUFFER_DESC cb;

	cb.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;
	cb.ByteWidth			= sizeof(C_BUFFER);
	cb.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;
	cb.MiscFlags			= 0;
	cb.StructureByteStride	= 0;
	cb.Usage				= D3D11_USAGE_DYNAMIC;

	return m_pDevice11->CreateBuffer( &cb, nullptr, &m_pConstantBuffer );

	return S_OK;
}


//���ٍ쐬.
HRESULT CSprite::CreateModel()
{
	//�����(�l�p�`)�̒��_���쐬.
	VERTEX vertices[]=
	{
		//���_���W(x,y,z)				 
		DirectX::XMFLOAT3( 0.0f, WND_H, 0.0f ),		DirectX::XMFLOAT2( 0.0f, 1.0f ),
		DirectX::XMFLOAT3( 0.0f, 0.0f, 0.0f ),		DirectX::XMFLOAT2( 0.0f, 0.0f ),
		DirectX::XMFLOAT3( WND_W, WND_H, 0.0f ),	DirectX::XMFLOAT2( 1.0f, 1.0f ),
		DirectX::XMFLOAT3( WND_W, 0.0f, 0.0f ),		DirectX::XMFLOAT2( 1.0f, 0.0f ),
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

	return S_OK;
}

//�����ݸޗp.
void CSprite::Render( ID3D11ShaderResourceView* pSRV  )
{
	// �V�F�[�_�[�̃R���X�^���g�o�b�t�@�Ɋe��f�[�^��n��.
	D3D11_MAPPED_SUBRESOURCE pData;
	C_BUFFER cb;	// �R���X�^���g�o�b�t�@.

	DirectX::XMMATRIX mW;

	DirectX::XMMATRIX mTrn, mRot, mScale;

	mTrn = DirectX::XMMatrixTranslation(
		m_vPosition.x, m_vPosition.y, m_vPosition.z );

	mRot = DirectX::XMMatrixRotationRollPitchYaw(
		m_vRotation.x, m_vRotation.y, m_vRotation.z );

	mScale = DirectX::XMMatrixScaling(
		m_vScale.x, m_vScale.y, m_vScale.z );

	mW = mScale * mRot * mTrn;


	// �o�b�t�@���̃f�[�^�̏��������J�n����Map.
	if( SUCCEEDED( m_pContext11->Map( 
		m_pConstantBuffer, 
		0, 
		D3D11_MAP_WRITE_DISCARD, 
		0, 
		&pData ))){

		cb.mW = DirectX::XMMatrixTranspose(mW);

		// �r���[�|�[�g�̕�,������n��.
		cb.vViewPort.x	= static_cast<float>(WND_W);
		cb.vViewPort.y	= static_cast<float>(WND_H);

		memcpy_s( 
			pData.pData, 
			pData.RowPitch,
			(void*)(&cb),
			sizeof(cb) );

		m_pContext11->Unmap( m_pConstantBuffer, 0 );
	}
	// �g�p����V�F�[�_�̃Z�b�g.
	m_pContext11->VSSetShader( m_pVertexShader, nullptr, 0 );	// ���_�V�F�[�_.
	m_pContext11->PSSetShader( m_pPixelShader, nullptr, 0 );	// �s�N�Z���V�F�[�_.
	// ���̃R���X�^���g�o�b�t�@���ǂ̃V�F�[�_�Ŏg�p���邩�H.
	m_pContext11->VSSetConstantBuffers( 0, 1, &m_pConstantBuffer );	// ���_�V�F�[�_.
	m_pContext11->PSSetConstantBuffers( 0, 1, &m_pConstantBuffer );	// �s�N�Z���V�F�[�_�[.

	// ���_�o�b�t�@���Z�b�g.
	UINT stride = sizeof(VERTEX); // �f�[�^�̊Ԋu.
	UINT offset = 0;
	m_pContext11->IASetVertexBuffers( 0, 1, &m_pVertexBuffer, &stride, &offset );

	// ���_�C���v�b�g���C�A�E�g���Z�b�g.
	m_pContext11->IASetInputLayout( m_pVertexLayout );
;
	//����è�ށE���ۼް���.
	m_pContext11->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

	//ø����𼪰�ނɓn��.
	m_pContext11->PSSetSamplers( 0, 1, &m_pSampleLinear );
	m_pContext11->PSSetShaderResources( 0, 1, &pSRV );

	//����è�ނ������ݸ�.
	m_pContext11->Draw( 4, 0 );//�����(���_4��).
}

void CSprite::Render( std::vector<ID3D11ShaderResourceView*> gbuffers  )
{
	// �V�F�[�_�[�̃R���X�^���g�o�b�t�@�Ɋe��f�[�^��n��.
	D3D11_MAPPED_SUBRESOURCE pData;
	C_BUFFER cb;	// �R���X�^���g�o�b�t�@.

	DirectX::XMMATRIX mW;

	DirectX::XMMATRIX mTrn, mRot, mScale;

	mTrn = DirectX::XMMatrixTranslation(
		m_vPosition.x, m_vPosition.y, m_vPosition.z );

	mRot = DirectX::XMMatrixRotationRollPitchYaw(
		m_vRotation.x, m_vRotation.y, m_vRotation.z );

	mScale = DirectX::XMMatrixScaling(
		m_vScale.x, m_vScale.y, m_vScale.z );

	mW = mScale * mRot * mTrn;


	// �o�b�t�@���̃f�[�^�̏��������J�n����Map.
	if( SUCCEEDED( m_pContext11->Map( 
		m_pConstantBuffer, 
		0, 
		D3D11_MAP_WRITE_DISCARD, 
		0, 
		&pData ))){

		cb.mW = DirectX::XMMatrixTranspose(mW);

		// �r���[�|�[�g�̕�,������n��.
		cb.vViewPort.x	= static_cast<float>(WND_W);
		cb.vViewPort.y	= static_cast<float>(WND_H);

		memcpy_s( 
			pData.pData, 
			pData.RowPitch,
			(void*)(&cb),
			sizeof(cb) );

		m_pContext11->Unmap( m_pConstantBuffer, 0 );
	}
	// �g�p����V�F�[�_�̃Z�b�g.
	m_pContext11->VSSetShader( m_pVertexShader, nullptr, 0 );	// ���_�V�F�[�_.
	m_pContext11->PSSetShader( m_pPixelShaderLast, nullptr, 0 );	// �s�N�Z���V�F�[�_.
																// ���̃R���X�^���g�o�b�t�@���ǂ̃V�F�[�_�Ŏg�p���邩�H.
	m_pContext11->VSSetConstantBuffers( 0, 1, &m_pConstantBuffer );	// ���_�V�F�[�_.
	m_pContext11->PSSetConstantBuffers( 0, 1, &m_pConstantBuffer );	// �s�N�Z���V�F�[�_�[.

																	// ���_�o�b�t�@���Z�b�g.
	UINT stride = sizeof(VERTEX); // �f�[�^�̊Ԋu.
	UINT offset = 0;
	m_pContext11->IASetVertexBuffers( 0, 1, &m_pVertexBuffer, &stride, &offset );

	// ���_�C���v�b�g���C�A�E�g���Z�b�g.
	m_pContext11->IASetInputLayout( m_pVertexLayout );
	;
	//����è�ށE���ۼް���.
	m_pContext11->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

	//ø����𼪰�ނɓn��.
	m_pContext11->PSSetSamplers( 0, 1, &m_pSampleLinear );
	m_pContext11->PSSetShaderResources( 0, 1, &gbuffers[0] );
	m_pContext11->PSSetShaderResources( 1, 1, &gbuffers[1] );
	m_pContext11->PSSetShaderResources( 2, 1, &gbuffers[2] );

	//����è�ނ������ݸ�.
	m_pContext11->Draw( 4, 0 );//�����(���_4��).
}