#include "FbxRenderer.h"
#include "..\FbxModel\FbxModel.h"
#include "..\FbxAnimation\FbxAnimationController.h"
#include "..\..\Camera\Camera.h"
#include "..\..\Light\Light.h"

CFbxRenderer::CFbxRenderer()
	: m_pDevice11			( nullptr )
	, m_pContext11			( nullptr )
	, m_pVertexShader		( nullptr )
	, m_pVertexAnimShader	( nullptr )
	, m_pPixelShader		( nullptr )
	, m_pVertexLayout		( nullptr )
	, m_pCBufferPerMesh		( nullptr )
	, m_pCBufferPerMaterial	( nullptr )
	, m_pCBufferPerBone		( nullptr )
	, m_pSampleLinear		( nullptr )
{
}

CFbxRenderer::~CFbxRenderer()
{
}

////////////////////////////////////////////////.
//	�쐬.
////////////////////////////////////////////////.
HRESULT CFbxRenderer::Create( ID3D11DeviceContext* pContext11 )
{
	if( pContext11 == nullptr ) return E_FAIL;
	// �R���e�L�X�g�̎擾.
	m_pContext11 = pContext11;
	// �f�o�C�X�̎擾.
	m_pContext11->GetDevice( &m_pDevice11 );
	if( m_pDevice11 == nullptr ) return E_FAIL;

	// �o�b�t�@�[�̍쐬.
	if( FAILED( CreateBuffer() )) return E_FAIL;
	// �T���v���[�̍쐬.
	if( FAILED( CreateSampler() )) return E_FAIL;
	// �V�F�[�_�[�̍쐬.
	if( FAILED( CreateShader() )) return E_FAIL;

	return S_OK;
}

////////////////////////////////////////////////.
//	�j��.
////////////////////////////////////////////////.
void CFbxRenderer::Destroy()
{
	SAFE_RELEASE( m_pSampleLinear );
	SAFE_RELEASE( m_pCBufferPerBone );
	SAFE_RELEASE( m_pCBufferPerMaterial );
	SAFE_RELEASE( m_pCBufferPerMesh );
	SAFE_RELEASE( m_pVertexLayout );
	SAFE_RELEASE( m_pPixelShader );
	SAFE_RELEASE( m_pVertexAnimShader );
	SAFE_RELEASE( m_pVertexShader );

	m_pDevice11 = nullptr;
	m_pContext11 = nullptr;
}

////////////////////////////////////////////////.
//	�`��.
////////////////////////////////////////////////.
void CFbxRenderer::Render(
	CFbxModel& mdoel,
	CCamera& camera,
	CLight& light,
	CFbxAnimationController* pAc )
{
	// �g�p����V�F�[�_�[�̐ݒ�.
	m_pContext11->VSSetShader( m_pVertexShader, nullptr, 0 );
	m_pContext11->PSSetShader( m_pPixelShader, nullptr, 0 );
	// �v���e�B�u�g�|���W�[���Z�b�g.
	m_pContext11->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	// ���[���h�s��擾.
	DirectX::XMMATRIX mWorld = mdoel.GetWorldMatrix();
	// ���[���h�A�r���[�A�v���W�F�N�V�����s��.
	DirectX::XMMATRIX mWVP = mWorld * camera.GetViewMatrix() * camera.GetProjMatrix();

	UINT stride = sizeof(VERTEX);
	UINT offset = 0;

	// �A�j���[�V�����t���[���̍X�V.
	AnimationFrameUpdate( mdoel, pAc );

	int meshNo = 0;
	// ���b�V���f�[�^���`��.
	for( auto& m : mdoel.GetMeshData() ){
		// �A�j���[�V�����̍s��v�Z.
		AnimMatrixCalculation( mdoel, meshNo, m, pAc );
		meshNo++;

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
			// ���[���h�s���]�u���ēn��.
			cb.mW	= DirectX::XMMatrixTranspose( mWorld );
			// world, View, Proj ��]�u���ēn��.
			cb.mWVP	= DirectX::XMMatrixTranspose( mWVP );
			// ���C�g�� wvp�@��]�u���ēn��.
			cb.mLightWVP = DirectX::XMMatrixTranspose( mWorld*light.GetVP() );
			// �J�����̍��W��n��.
			cb.CameraPos = { camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z, 0.0f };
			// ���C�g�̕�����n��.
			cb.LightDir = light.GetDirection();

			memcpy_s( pData.pData, pData.RowPitch, (void*)(&cb), sizeof(cb) );
			m_pContext11->Unmap( m_pCBufferPerMesh, 0 );
		}
		// ��Őݒ肵���R���X�^���g�o�b�t�@���ǂ̃V�F�[�_�[�Ŏg����.
		m_pContext11->VSSetConstantBuffers( 0, 1, &m_pCBufferPerMesh );
		m_pContext11->PSSetConstantBuffers( 0, 1, &m_pCBufferPerMesh );
		
		// �}�e���A����.
		if( SUCCEEDED ( m_pContext11->Map( 
			m_pCBufferPerMaterial, 0, 
			D3D11_MAP_WRITE_DISCARD, 0, 
			&pData )))
		{
			CBUFFER_PER_MATERIAL cb;
			cb.Ambient	= m.Material.Ambient;	// �A���r�G���g.
			cb.Diffuse	= m.Material.Diffuse;	// �f�B�t���[�Y.
			cb.Specular = m.Material.Specular;	// �X�y�L����.

			memcpy_s( pData.pData, pData.RowPitch, (void*)(&cb), sizeof(cb) );
			m_pContext11->Unmap( m_pCBufferPerMaterial, 0 );
		}
		// ��Őݒ肵���R���X�^���g�o�b�t�@���ǂ̃V�F�[�_�[�Ŏg����.
		m_pContext11->VSSetConstantBuffers( 1, 1, &m_pCBufferPerMaterial );
		m_pContext11->PSSetConstantBuffers( 1, 1, &m_pCBufferPerMaterial );
		// �T���v�����Z�b�g.
		m_pContext11->PSSetSamplers( 0, 1, &m_pSampleLinear );

		// �e�N�X�`���̊m�F.
		if( mdoel.GetTextures().count(m.Material.Name) > 0 ){
			// �e�N�X�`���������.
			// �e�N�X�`�����V�F�[�_�[�ɓn��.
			m_pContext11->PSSetShaderResources( 0, 1, &mdoel.GetTextures().at(m.Material.Name) );
		} else {
			// �e�N�X�`������.
			ID3D11ShaderResourceView* notex = { nullptr };
			m_pContext11->PSSetShaderResources( 0, 1, &notex );
		}

		// �|���S���������_�����O.
		m_pContext11->DrawIndexed( m.PolygonVertexCount, 0, 0 );
	}
}

////////////////////////////////////////////////.
// �A�j���[�V�����t���[���̍X�V.
////////////////////////////////////////////////.
void CFbxRenderer::AnimationFrameUpdate( CFbxModel& mdoel, CFbxAnimationController* pAc )
{
	// �A�j���[�V�����t���[���̍X�V.
	if( pAc == nullptr ){
		// ���f���f�[�^�̃A�j���[�V�����R���g���[���[���g�p.
		if( mdoel.GetPtrAC() != nullptr ){
			// �A�j���[�V�����p�̒��_�V�F�[�_�[�̐ݒ�.
			m_pContext11->VSSetShader( m_pVertexAnimShader, nullptr, 0 );
			mdoel.GetPtrAC()->FrameUpdate();
		}
	} else {
		// �f�t�H���g�����̃A�j���[�V�����R���g���[���[���g�p.
		if( pAc != nullptr ){
			// �A�j���[�V�����p�̒��_�V�F�[�_�[�̐ݒ�.
			m_pContext11->VSSetShader( m_pVertexAnimShader, nullptr, 0 );
			pAc->FrameUpdate();
		}
	}
}

////////////////////////////////////////////////.
// �A�j���[�V�����p�̍s��v�Z.
////////////////////////////////////////////////.
void CFbxRenderer::AnimMatrixCalculation(
	CFbxModel& mdoel,
	const int& meahNo,
	FBXMeshData& meshData,
	CFbxAnimationController* pAc  )
{
	CBUFFER_PER_BONE cb;
	if( pAc == nullptr ){
		// ���f���f�[�^�̃A�j���[�V�����R���g���[���[���g�p.
		if( GetBoneConstBuffer( meshData.Skin, meahNo, mdoel.GetPtrAC(), &cb ) == false ) return;
	} else {
		// �f�t�H���g�����̃A�j���[�V�����R���g���[���[���g�p.
		if( GetBoneConstBuffer( meshData.Skin, meahNo, pAc, &cb ) == false ) return;
	}

	D3D11_MAPPED_SUBRESOURCE pdata;
	if( SUCCEEDED ( m_pContext11->Map( 
		m_pCBufferPerBone, 0,
		D3D11_MAP_WRITE_DISCARD, 0,
		&pdata )))
	{
		memcpy_s( 
			pdata.pData, pdata.RowPitch,
			(void*)(&cb), sizeof(cb) );
		m_pContext11->Unmap( m_pCBufferPerBone, 0 );
	}
	// ��Őݒ肵���R���X�^���g�o�b�t�@���ǂ̃V�F�[�_�[�Ŏg����.
	m_pContext11->VSSetConstantBuffers( 2, 1, &m_pCBufferPerBone );
	m_pContext11->PSSetConstantBuffers( 2, 1, &m_pCBufferPerBone );

}

////////////////////////////////////////////////.
// �{�[���̒萔�o�b�t�@�̎擾.
////////////////////////////////////////////////.
bool CFbxRenderer::GetBoneConstBuffer(
	const SkinData& skinData,
	const int& meahNo,
	CFbxAnimationController* pAc,
	CBUFFER_PER_BONE* pOutCB )
{
	if( pAc == nullptr ) return false;

	int boneIndex = 0;
	FbxMatrix frameMatrix;
	FbxMatrix vertexTransformMatrix;
	for( auto& b : skinData.InitBonePositions ){
		if( boneIndex >= BONE_COUNT_MAX ) break;
		if( pAc->GetFrameLinkMatrix( meahNo, boneIndex, &frameMatrix ) == false ){
			_ASSERT_EXPR( false, "���f���ƃA�j���[�V�����̃{�[���̐��������܂���" );
			MessageBox( nullptr, "���f���ƃA�j���[�V�����̃{�[���̐��������܂���", "Warning", MB_OK );
		}
		vertexTransformMatrix = frameMatrix * b;
		pOutCB->Bone[boneIndex] = FbxMatrixConvertDXMMatrix( vertexTransformMatrix );
		boneIndex++;
	}

	return true;
}

////////////////////////////////////////////////.
// �萔�o�b�t�@�̍쐬.
////////////////////////////////////////////////.
HRESULT CFbxRenderer::CreateBuffer()
{
	//--------------------------------------.
	// �萔�o�b�t�@�̐ݒ�
	//--------------------------------------.
	if( FAILED( CreateConstantBuffer( 
		sizeof(CBUFFER_PER_MESH), &m_pCBufferPerMesh ) )){
		_ASSERT_EXPR( false, "���b�V�����̒萔�o�b�t�@�쐬���s" );
		MessageBox( nullptr, "���b�V�����̒萔�o�b�t�@�쐬���s", "Warning", MB_OK );
		return E_FAIL;
	}
	if( FAILED( CreateConstantBuffer(
		sizeof(CBUFFER_PER_MATERIAL), &m_pCBufferPerMaterial ) )){
		_ASSERT_EXPR( false, "�}�e���A�����̒萔�o�b�t�@�쐬���s" );
		MessageBox( nullptr, "�}�e���A�����̒萔�o�b�t�@�쐬���s", "Warning", MB_OK );
		return E_FAIL;
	}
	if( FAILED( CreateConstantBuffer(
		sizeof(CBUFFER_PER_BONE), &m_pCBufferPerBone ) )){
		_ASSERT_EXPR( false, "�{�[�����̒萔�o�b�t�@�쐬���s" );
		MessageBox( nullptr, "�{�[�����̒萔�o�b�t�@�쐬���s", "Warning", MB_OK );
		return E_FAIL;
	}
	return S_OK;
}

////////////////////////////////////////////////.
// �T���v���[�쐬.
////////////////////////////////////////////////.
HRESULT CFbxRenderer::CreateSampler()
{
	D3D11_SAMPLER_DESC samDesc;
	ZeroMemory(&samDesc, sizeof(samDesc));
	samDesc.Filter		= D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samDesc.AddressU	= D3D11_TEXTURE_ADDRESS_WRAP;
	samDesc.AddressV	= D3D11_TEXTURE_ADDRESS_WRAP;
	samDesc.AddressW	= D3D11_TEXTURE_ADDRESS_WRAP;
	// �T���v���쐬.
	if( FAILED( m_pDevice11->CreateSamplerState(
		&samDesc, &m_pSampleLinear ))){
		_ASSERT_EXPR( false, "�T���v���[�쐬���s" );
		MessageBox( nullptr, "�T���v���[�쐬���s", "Warning", MB_OK );
		return E_FAIL;
	}
	return S_OK;
}

////////////////////////////////////////////////.
// �萔�o�b�t�@�쐬.
////////////////////////////////////////////////.
HRESULT CFbxRenderer::CreateConstantBuffer( const size_t& byte, ID3D11Buffer** buffer )
{
	D3D11_BUFFER_DESC cb;
	cb.ByteWidth	= byte;
	cb.Usage		= D3D11_USAGE_DYNAMIC;
	cb.BindFlags	= D3D11_BIND_CONSTANT_BUFFER;
	cb.MiscFlags	= 0;
	cb.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;
	cb.StructureByteStride	= 0;
	if( FAILED( m_pDevice11->CreateBuffer( 
		&cb, nullptr, buffer ))){
		return E_FAIL;
	}
	return S_OK;
}

////////////////////////////////////////////////.
// �V�F�[�_�[�̍쐬.
////////////////////////////////////////////////.
HRESULT CFbxRenderer::CreateShader()
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
			MessageBox( nullptr, msg.c_str(), "Warning", MB_OK );
			return E_FAIL;
		}
		return S_OK;
	};
	//-----------------------------------------.
	// ���_�V�F�[�_�[.
	//-----------------------------------------.
	// ���_�V�F�[�_�[�̓ǂݍ���.
	if( FAILED( shaderCompile( VS_SHADER_NAME, "VS_Main", "vs_5_0", &pCompileVS ) )) return E_FAIL;
	// ���_�V�F�[�_�[�̍쐬.
	if( FAILED(
		m_pDevice11->CreateVertexShader(
			pCompileVS->GetBufferPointer(),	// �ǂݍ��񂾃V�F�[�_�[�̃|�C���^.
			pCompileVS->GetBufferSize(),	// ���_�V�F�[�_�[�̃T�C�Y.
			nullptr,						// "���I�V�F�[�_�[�����N"���g�p���Ȃ��̂�null.
			&m_pVertexShader ))){			// (out)���_�V�F�[�_�[.
		_ASSERT_EXPR( false, "���_�V�F�[�_�[�쐬���s" );
		MessageBox( nullptr, "���_�V�F�[�_�[�쐬���s", "Warning", MB_OK );
		return E_FAIL;
	}
	// ���_�V�F�[�_�[(�A�j���[�V����)�̓ǂݍ���.
	if( FAILED( shaderCompile( VS_ANIM_SHADER_NAME, "VS_Main", "vs_5_0", &pCompileVS ) )) return E_FAIL;
	// ���_�V�F�[�_�[(�A�j���[�V����)�̍쐬.
	if( FAILED(
		m_pDevice11->CreateVertexShader(
			pCompileVS->GetBufferPointer(),	// �ǂݍ��񂾃V�F�[�_�[�̃|�C���^.
			pCompileVS->GetBufferSize(),	// ���_�V�F�[�_�[�̃T�C�Y.
			nullptr,						// "���I�V�F�[�_�[�����N"���g�p���Ȃ��̂�null.
			&m_pVertexAnimShader ))){		// (out)���_�V�F�[�_�[.
		_ASSERT_EXPR( false, "���_�V�F�[�_�[�쐬���s" );
		MessageBox( nullptr, "���_�V�F�[�_�[�쐬���s", "Warning", MB_OK );
		return E_FAIL;
	}
	//-----------------------------------------.
	// �s�N�Z���V�F�[�_�[.
	//-----------------------------------------.
	// �s�N�Z���V�F�[�_�[�̓ǂݍ���.
	if( FAILED( shaderCompile( PS_SHADER_NAME, "PS_Main", "ps_5_0", &pCompilePS ) )) return E_FAIL;
	// �s�N�Z���V�F�[�_�[�̍쐬.
	if( FAILED(
		m_pDevice11->CreatePixelShader(
			pCompilePS->GetBufferPointer(),	// �ǂݍ��񂾃V�F�[�_�[�̃|�C���^.
			pCompilePS->GetBufferSize(),	// �s�N�Z���V�F�[�_�[�̃T�C�Y.
			nullptr,						// "���I�V�F�[�_�[�����N"���g�p���Ȃ��̂�null.
			&m_pPixelShader ))){			// (out)�s�N�Z���V�F�[�_�[.
		_ASSERT_EXPR( false, "�s�N�Z���V�F�[�_�[�쐬���s" );
		MessageBox( nullptr, "�s�N�Z���V�F�[�_�[�쐬���s", "Warning", MB_OK );
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
		_ASSERT_EXPR( false, "���_���C�A�E�g�쐬���s" );
		MessageBox( nullptr, "���_���C�A�E�g�쐬���s", "Warning", MB_OK );
		return E_FAIL;
	}

	SAFE_RELEASE( pCompileVS );
	SAFE_RELEASE( pCompilePS );

	return S_OK;
}