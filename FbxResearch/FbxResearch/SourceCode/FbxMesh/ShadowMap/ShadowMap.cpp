#include "ShadowMap.h"
#include "..\FbxModel\FbxModel.h"
#include "..\FbxAnimation\FbxAnimationController.h"
#include "..\..\Light\Light.h"

#include <string>

CShadowMap::CShadowMap()
{
}

CShadowMap::~CShadowMap()
{
}


//-----------------------------------------.
//				�쐬.
//-----------------------------------------.
HRESULT CShadowMap::Create( ID3D11DeviceContext* pContext11 )
{
	if( pContext11 == nullptr ) return E_FAIL;
	// �R���e�L�X�g�̎擾.
	m_pContext11 = pContext11;
	// �f�o�C�X�̎擾.
	m_pContext11->GetDevice( &m_pDevice11 );
	if( m_pDevice11 == nullptr ) return E_FAIL;

	// �o�b�t�@�[�̍쐬.
	if( FAILED( CreateBuffer() )) return E_FAIL;
	// �V�F�[�_�[�̍쐬.
	if( FAILED( CreateShader() )) return E_FAIL;

	return S_OK;
}

//-----------------------------------------.
//				�j��.
//-----------------------------------------.
void CShadowMap::Destroy()
{
	SAFE_RELEASE( m_pCBufferPerMesh );
	SAFE_RELEASE( m_pVertexLayout );
	SAFE_RELEASE( m_pPixelShader );
	SAFE_RELEASE( m_pVertexShader );

	m_pDevice11 = nullptr;
	m_pContext11 = nullptr;
}

//-----------------------------------------.
//				�`��.
//-----------------------------------------.
void CShadowMap::Render(
	CFbxModel& mdoel,
	CLight& light )
{
	// �g�p����V�F�[�_�[�̐ݒ�.
	m_pContext11->VSSetShader( m_pVertexShader, nullptr, 0 );
	m_pContext11->PSSetShader( m_pPixelShader, nullptr, 0 );
	// �v���e�B�u�g�|���W�[���Z�b�g.
	m_pContext11->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	// ���[���h�s��擾.
	DirectX::XMMATRIX mWorld = mdoel.GetWorldMatrix();

	UINT stride = sizeof(VERTEX);
	UINT offset = 0;
	// ���b�V���f�[�^���`��.
	for( auto& m : mdoel.GetMeshData() ){
		// ���_�o�b�t�@���Z�b�g.
		m_pContext11->IASetVertexBuffers( 0, 1, &m.pVertexBuffer, &stride, &offset );
		m_pContext11->IASetIndexBuffer( m.pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		m_pContext11->IASetInputLayout( m_pVertexLayout );

		D3D11_MAPPED_SUBRESOURCE pData;
		if( SUCCEEDED ( m_pContext11->Map( 
			m_pCBufferPerMesh, 0, 
			D3D11_MAP_WRITE_DISCARD, 0, 
			&pData )))
		{
			CBUFFER_PER_MESH cb;
			// ���C�g�� wvp�@��]�u���ēn��.
			cb.mLightWVP = DirectX::XMMatrixTranspose( mWorld * light.GetVP() );

			memcpy_s( pData.pData, pData.RowPitch, (void*)(&cb), sizeof(cb) );
			m_pContext11->Unmap( m_pCBufferPerMesh, 0 );
		}
		// ��Őݒ肵���R���X�^���g�o�b�t�@���ǂ̃V�F�[�_�[�Ŏg����.
		m_pContext11->VSSetConstantBuffers( 0, 1, &m_pCBufferPerMesh );
		m_pContext11->PSSetConstantBuffers( 0, 1, &m_pCBufferPerMesh );

		// �|���S���������_�����O.
		m_pContext11->DrawIndexed( m.PolygonVertexCount, 0, 0 );
	}
}

// �萔�o�b�t�@�̍쐬.
HRESULT CShadowMap::CreateBuffer()
{
	//--------------------------------------.
	// �萔�o�b�t�@�̐ݒ�
	//--------------------------------------.
	if( FAILED( CreateConstantBuffer( 
		sizeof(CBUFFER_PER_MESH), &m_pCBufferPerMesh ) )){
		_ASSERT_EXPR( false, TEXT("���b�V�����̒萔�o�b�t�@�쐬���s") );
		MessageBox( nullptr, TEXT("���b�V�����̒萔�o�b�t�@�쐬���s"), TEXT("Warning"), MB_OK );
		return E_FAIL;
	}
	return S_OK;
}

// �萔�o�b�t�@�쐬.
HRESULT CShadowMap::CreateConstantBuffer( const size_t& byte, ID3D11Buffer** buffer )
{
	D3D11_BUFFER_DESC cb;
	cb.ByteWidth			= (UINT)byte;
	cb.Usage				= D3D11_USAGE_DYNAMIC;
	cb.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;
	cb.MiscFlags			= 0;
	cb.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;
	cb.StructureByteStride	= 0;
	if( FAILED( m_pDevice11->CreateBuffer( 
		&cb, nullptr, buffer ))){
		return E_FAIL;
	}
	return S_OK;
}

// �V�F�[�_�[�̍쐬.
HRESULT CShadowMap::CreateShader()
{
	ID3DBlob* pCompileVS = nullptr;
	ID3DBlob* pCompilePS = nullptr;
	ID3DBlob* pErrerBlog = nullptr;
	UINT uCompileFlag = 0;

#ifdef _DEBUG
	// �V�F�[�_�[�Ńu���C�N��\�邽�߂̏���.
	uCompileFlag = D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
#endif	// #ifdef _DEBUG.

	// �V�F�[�_�[�ǂݍ��݊֐�.
	auto shaderCompile = [&]( 
		const wchar_t* fileName, 
		const char* entryPosint, 
		const char* version, 
		ID3DBlob** ppOutBlog )
	{
		if( FAILED(
			D3DCompileFromFile( 
				fileName,							// �V�F�[�_�[��.
				nullptr,							// �}�N����`(�V�F�[�_�[���Ń}�N����`���Ă���ꍇ�Ɏg�p).
				D3D_COMPILE_STANDARD_FILE_INCLUDE,	// �C���N���[�h��`(�V�F�[�_�[���ŃC���N���[�h��`���Ă���ꍇ�Ɏg�p).
				entryPosint,						// �G���g���[�|�C���g.
				version,							// �V�F�[�_�[�̃o�[�V����.
				uCompileFlag,						// �t���O.
				0,									// �t���O.
				ppOutBlog,							// (out)�V�F�[�_�[�̏��.
				&pErrerBlog ))){					// (out)�G���[���Ȃ�.
			std::string msg;
			msg.resize( pErrerBlog->GetBufferSize() );
			std::copy_n(static_cast<char*>(pErrerBlog->GetBufferPointer()), pErrerBlog->GetBufferSize(), msg.begin());
			_ASSERT_EXPR( false, msg.c_str() );
			MessageBoxA( nullptr, msg.c_str(), "Warning", MB_OK );
			return E_FAIL;
		}
		return S_OK;
	};
	//-----------------------------------------.
	// ���_�V�F�[�_�[.
	//-----------------------------------------.
	// ���_�V�F�[�_�[�̓ǂݍ���.
	if( FAILED( shaderCompile( SHADOW_SHADER_NAME, "VS_Main", "vs_5_0", &pCompileVS ) )) return E_FAIL;
	// ���_�V�F�[�_�[�̍쐬.
	if( FAILED(
		m_pDevice11->CreateVertexShader(
			pCompileVS->GetBufferPointer(),	// �ǂݍ��񂾃V�F�[�_�[�̃|�C���^.
			pCompileVS->GetBufferSize(),	// ���_�V�F�[�_�[�̃T�C�Y.
			nullptr,						// "���I�V�F�[�_�[�����N"���g�p���Ȃ��̂�null.
			&m_pVertexShader ))){			// (out)���_�V�F�[�_�[.
		_ASSERT_EXPR( false, TEXT("���_�V�F�[�_�[�쐬���s") );
		MessageBox( nullptr, TEXT("���_�V�F�[�_�[�쐬���s"), TEXT("Warning"), MB_OK );
		return E_FAIL;
	}
	//-----------------------------------------.
	// �s�N�Z���V�F�[�_�[.
	//-----------------------------------------.
	// �s�N�Z���V�F�[�_�[�̓ǂݍ���.
	if( FAILED( shaderCompile( SHADOW_SHADER_NAME, "PS_Main", "ps_5_0", &pCompilePS ) )) return E_FAIL;
	// �s�N�Z���V�F�[�_�[�̍쐬.
	if( FAILED(
		m_pDevice11->CreatePixelShader(
			pCompilePS->GetBufferPointer(),	// �ǂݍ��񂾃V�F�[�_�[�̃|�C���^.
			pCompilePS->GetBufferSize(),	// �s�N�Z���V�F�[�_�[�̃T�C�Y.
			nullptr,						// "���I�V�F�[�_�[�����N"���g�p���Ȃ��̂�null.
			&m_pPixelShader ))){			// (out)�s�N�Z���V�F�[�_�[.
		_ASSERT_EXPR( false, TEXT("�s�N�Z���V�F�[�_�[�쐬���s") );
		MessageBox( nullptr, TEXT("�s�N�Z���V�F�[�_�[�쐬���s"), TEXT("Warning"), MB_OK );
		return E_FAIL;
	}
	//-----------------------------------------.
	//	���_�C���v�b�g.
	//-----------------------------------------.
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",			0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",			0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",		0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONE_INDEX",		0, DXGI_FORMAT_R32G32B32A32_UINT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONE_WEIGHT",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	int numElements = sizeof(layout)/sizeof(layout[0]);	//�v�f���Z�o.
	if (FAILED(
		m_pDevice11->CreateInputLayout(
			layout,
			numElements,
			pCompileVS->GetBufferPointer(),
			pCompileVS->GetBufferSize(),
			&m_pVertexLayout ))){
		_ASSERT_EXPR( false, TEXT("���_���C�A�E�g�쐬���s") );
		MessageBox( nullptr, TEXT("���_���C�A�E�g�쐬���s"), TEXT("Warning"), MB_OK );
		return E_FAIL;
	}

	SAFE_RELEASE( pCompileVS );
	SAFE_RELEASE( pCompilePS );

	return S_OK;
}