#include "FbxMesh.h"
#include "FbxAnimation/FbxAnimationController.h"

CFbxMesh::CFbxMesh()
	: m_pFbxManager			( nullptr )
	, m_pFbxScene			( nullptr )
	, m_pVertexShader		( nullptr )
	, m_pPixelShader		( nullptr )
	, m_pVertexLayout		( nullptr )
	, m_pCBufferPerMesh		( nullptr )
	, m_pCBufferPerMaterial	( nullptr )
	, m_pCBufferPerBone		( nullptr )
	, m_pSampleLinear		( nullptr )
	, m_MeshData			()
	, m_Skeletons			()
	, m_Textures			()
	, m_pAc					( nullptr )
	, m_pAnimLoader			( nullptr )
	, m_Position			( 0.0f, 0.0f, 0.0f )
	, m_Rotation			( 0.0f, 0.0f, 0.0f )
	, m_Scale				( 1.0f, 1.0f, 1.0f )
{
}

CFbxMesh::~CFbxMesh()
{
}

// �쐬.
HRESULT CFbxMesh::Create( ID3D11DeviceContext* pContext11, const char* fileName )
{
	if( pContext11 == nullptr ) return E_FAIL;
	// �R���e�L�X�g�̎擾.
	m_pContext11 = pContext11;
	// �f�o�C�X�̎擾.
	m_pContext11->GetDevice( &m_pDevice11 );
	if( m_pDevice11 == nullptr ) return E_FAIL;

	//------------------------------.
	// �}�l�[�W���[�̍쐬.
	//------------------------------.
	m_pFbxManager = FbxManager::Create();
	if( m_pFbxManager == nullptr ){
		ERROR_MESSAGE( "FbxManager Create Failure." );
		return E_FAIL;
	}

	{
	//------------------------------.
	// �C���|�[�^�[�̍쐬.
	//------------------------------.
	FbxImporter* pFbxImpoter = FbxImporter::Create( m_pFbxManager, "imp" );
	if( pFbxImpoter == nullptr ){
		ERROR_MESSAGE( "FbxImpoter Create Failure." );
		return E_FAIL;
	}

	//------------------------------.
	// FBX�t�@�C���̓ǂݍ���.
	//------------------------------.
	// �t�@�C�����̐ݒ�.
	FbxString fbxFileName( fileName );
	if( pFbxImpoter->Initialize( fbxFileName.Buffer() ) == false ){
		ERROR_MESSAGE( "FbxFile Loading Failure." );
		return E_FAIL;
	}

	//------------------------------.
	// �V�[���I�u�W�F�N�g�̍쐬.
	//------------------------------.
	m_pFbxScene = FbxScene::Create( m_pFbxManager, "fbxScene" );
	if( m_pFbxScene == nullptr ){
		ERROR_MESSAGE( "FbxScene Create Failure." );
		return E_FAIL;
	}

	//------------------------------.
	// �C���|�[�^�[�ƃV�[���I�u�W�F�N�g�̊֘A�t��.
	//------------------------------.
	if( pFbxImpoter->Import( m_pFbxScene ) == false ){
		SAFE_DESTROY( m_pFbxManager );
		SAFE_DESTROY( m_pFbxScene );
		SAFE_DESTROY( pFbxImpoter );
		ERROR_MESSAGE( "FbxScene Create Failure." );
		return E_FAIL;
	}
	SAFE_DESTROY( pFbxImpoter );

	bool convertReslut = false;
	FbxGeometryConverter geometryConverter( m_pFbxManager );
	// �|���S�����O�p�`�ɂ���.
	// ���p�`�|���S��������΍��Ȃ����̂Ŏ��Ԃ�������.
	convertReslut = geometryConverter.Triangulate( m_pFbxScene, true );
	geometryConverter.RemoveBadPolygonsFromMeshes( m_pFbxScene );
	// �}�e���A�����Ƃ̃��b�V�����쐬����.
	convertReslut = geometryConverter.SplitMeshesPerMaterial( m_pFbxScene, true );

	//----------------------------.
	// FbxSkeleton�̐����擾.
	//----------------------------.
	int skeletonNum = m_pFbxScene->GetSrcObjectCount<FbxSkeleton>();
	int skeletonNo = 0;
	m_Skeletons.resize( skeletonNum );
	for( auto& s : m_Skeletons ){
		s = m_pFbxScene->GetSrcObject<FbxSkeleton>(skeletonNo);
		skeletonNo++;
	}

	//----------------------------.
	// FbxMesh�̐����擾.
	//----------------------------.
	int meshNum = m_pFbxScene->GetSrcObjectCount<FbxMesh>();
	int meshNo = 0;
	m_MeshData.resize( meshNum );
	for( auto& m : m_MeshData ){
		// ���b�V���f�[�^�̎擾.
		FbxMesh* pMesh = m_pFbxScene->GetSrcObject<FbxMesh>(meshNo);
		// �}�e���A���̎擾.
		GetMaterial( pMesh, m, fileName );
		// ���b�V���f�[�^�̓ǂݍ���.
		LoadMesh( pMesh, m );
		// ���_�o�b�t�@�̍쐬.
		if( FAILED( CreateVertexBuffers( m ) )) return E_FAIL;
		// �C���f�b�N�X�o�b�t�@�̍쐬.
		if( FAILED( CreateIndexBuffers( m ) )) return E_FAIL;
		meshNo++;
	}

	//----------------------------.
	//	�A�j���[�V����.
	//----------------------------.
	m_pAnimLoader = std::make_unique<CFbxAnimationLoader>();
	std::vector<SAnimationData>	animDataList;
	m_pAnimLoader->LoadAnimationData( m_pFbxScene, m_MeshClusterData, m_Skeletons, &animDataList );
	if( animDataList.empty() == false ){
		// ��Őݒ肵���A�j���[�V�����f�[�^�������.
		// �A�j���[�V�����R���g���[���[���쐬����.
		// �A�j���[�V�����f�[�^��ǉ�.
		m_pAc = new CFbxAnimationController;
		m_pAc->SetAnimDataList( animDataList );
	}
	}

	// �o�b�t�@�[�̍쐬.
	if( FAILED( CreateBuffer() )) return E_FAIL;
	// �T���v���[�̍쐬.
	if( FAILED( CreateSampler() )) return E_FAIL;
	// �V�F�[�_�[�̍쐬.
	if( FAILED( CreateShader() )) return E_FAIL;

	return S_OK;
}

// �j��.
void CFbxMesh::Destroy()
{
	SAFE_DELETE( m_pAc );

	SAFE_RELEASE( m_pSampleLinear );
	SAFE_RELEASE( m_pCBufferPerBone );
	SAFE_RELEASE( m_pCBufferPerMaterial );
	SAFE_RELEASE( m_pCBufferPerMesh );
	SAFE_RELEASE( m_pVertexLayout );
	SAFE_RELEASE( m_pPixelShader );
	SAFE_RELEASE( m_pVertexShader );

	for( auto& t : m_Textures ) SAFE_RELEASE( t.second );

	for( auto& m : m_MeshData ) m.Release();

	SAFE_DESTROY( m_pFbxScene );
	SAFE_DESTROY( m_pFbxManager );

	m_pDevice11 = nullptr;
	m_pContext11 = nullptr;
}

//-----------------------------------------.
//				�j��.
//-----------------------------------------.
HRESULT CFbxMesh::LoadModel( const char* fileName )
{
	//------------------------------.
	// �C���|�[�^�[�̍쐬.
	//------------------------------.
	FbxImporter* pFbxImpoter = FbxImporter::Create( m_pFbxManager, "imp" );
	if( pFbxImpoter == nullptr ){
		ERROR_MESSAGE( "FbxImpoter Create Failure." );
		return E_FAIL;
	}

	//------------------------------.
	// FBX�t�@�C���̓ǂݍ���.
	//------------------------------.
	// �t�@�C�����̐ݒ�.
	FbxString fbxFileName( fileName );
	if( pFbxImpoter->Initialize( fbxFileName.Buffer() ) == false ){
		ERROR_MESSAGE( "FbxFile Loading Failure." );
		return E_FAIL;
	}

	//------------------------------.
	// �V�[���I�u�W�F�N�g�̍쐬.
	//------------------------------.
	m_pFbxScene = FbxScene::Create( m_pFbxManager, "fbxScene" );
	if( m_pFbxScene == nullptr ){
		ERROR_MESSAGE( "FbxScene Create Failure." );
		return E_FAIL;
	}

	//------------------------------.
	// �C���|�[�^�[�ƃV�[���I�u�W�F�N�g�̊֘A�t��.
	//------------------------------.
	if( pFbxImpoter->Import( m_pFbxScene ) == false ){
		SAFE_DESTROY( m_pFbxManager );
		SAFE_DESTROY( m_pFbxScene );
		SAFE_DESTROY( pFbxImpoter );
		ERROR_MESSAGE( "FbxScene Create Failure." );
		return E_FAIL;
	}
	SAFE_DESTROY( pFbxImpoter );

	bool convertReslut = false;
	FbxGeometryConverter geometryConverter( m_pFbxManager );
	// �|���S�����O�p�`�ɂ���.
	// ���p�`�|���S��������΍��Ȃ����̂Ŏ��Ԃ�������.
	convertReslut = geometryConverter.Triangulate( m_pFbxScene, true );
	if( convertReslut == false ){
		ERROR_MESSAGE( "Triangulate Failure." );
		return E_FAIL;
	}
	geometryConverter.RemoveBadPolygonsFromMeshes( m_pFbxScene );
	// ���b�V�����}�e���A�����Ƃɕ�������.
	convertReslut = geometryConverter.SplitMeshesPerMaterial( m_pFbxScene, true );
	if( convertReslut == false ){
		ERROR_MESSAGE( "SplitMeshesPerMaterial Failure." );
		return E_FAIL;
	}


	//----------------------------.
	// FbxSkeleton�̐����擾.
	//----------------------------.
	int skeletonNum = m_pFbxScene->GetSrcObjectCount<FbxSkeleton>();
	int skeletonNo = 0;
	m_Skeletons.resize( skeletonNum );
	for( auto& s : m_Skeletons ){
		s = m_pFbxScene->GetSrcObject<FbxSkeleton>(skeletonNo);
		skeletonNo++;
	}

	//----------------------------.
	// FbxMesh�̐����擾.
	//----------------------------.
	int meshNum = m_pFbxScene->GetSrcObjectCount<FbxMesh>();
	int meshNo = 0;
	m_MeshData.resize( meshNum );
	for( auto& m : m_MeshData ){
		// ���b�V���f�[�^�̎擾.
		FbxMesh* pMesh = m_pFbxScene->GetSrcObject<FbxMesh>(meshNo);
		// �}�e���A���̎擾.
		GetMaterial( pMesh, m, fileName );
		// ���b�V���f�[�^�̓ǂݍ���.
		LoadMesh( pMesh, m );
		// ���_�o�b�t�@�̍쐬.
		if( FAILED( CreateVertexBuffers( m ) )) return E_FAIL;
		// �C���f�b�N�X�o�b�t�@�̍쐬.
		if( FAILED( CreateIndexBuffers( m ) )) return E_FAIL;
		meshNo++;
	}

	//----------------------------.
	//	�A�j���[�V�����̓ǂݍ���.
	//----------------------------.
	m_pAnimLoader = std::make_unique<CFbxAnimationLoader>();
	std::vector<SAnimationData>	animDataList;
	m_pAnimLoader->LoadAnimationData( m_pFbxScene, m_MeshClusterData, m_Skeletons, &animDataList );
	if( animDataList.empty() == false ){
		// ��Őݒ肵���A�j���[�V�����f�[�^�������.
		// �A�j���[�V�����R���g���[���[���쐬����.
		// �A�j���[�V�����f�[�^��ǉ�.
		m_pAc = new CFbxAnimationController;
		m_pAc->SetAnimDataList( animDataList );
	}

	return S_OK;
}

// �`��.
void CFbxMesh::Render(
	const DirectX::XMMATRIX& view,
	const DirectX::XMMATRIX& proj,
	CFbxAnimationController* pAc )
{
	// ���[���h�s��擾.
	DirectX::XMMATRIX World	= GetWorldMatrix();
	int meshNo = 0;
	UINT stride = sizeof(VERTEX);
	UINT offset = 0;

	if( pAc == nullptr ){
		// �A�j���[�V�����t���[���̍X�V.
		if( m_pAc != nullptr ) m_pAc->FrameUpdate();
	} else {
		// �A�j���[�V�����t���[���̍X�V.
		if( pAc != nullptr ) pAc->FrameUpdate();
	}

	// ���b�V���f�[�^���`��.
	for( auto& m : m_MeshData ){
		// �A�j���[�V�����̍s��v�Z.
		AnimMatrixCalculation( meshNo, m, pAc );
		meshNo++;

		// ���_�o�b�t�@���Z�b�g.
		m_pContext11->IASetVertexBuffers( 0, 1, &m.pVertexBuffer, &stride, &offset );
		m_pContext11->IASetIndexBuffer( m.pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		m_pContext11->IASetInputLayout( m_pVertexLayout );
		// �v���e�B�u�g�|���W�[���Z�b�g.
		m_pContext11->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		// �g�p����V�F�[�_�[�̐ݒ�.
		m_pContext11->VSSetShader( m_pVertexShader, nullptr, 0 );
		m_pContext11->PSSetShader( m_pPixelShader, nullptr, 0 );

		D3D11_MAPPED_SUBRESOURCE pdata;
		if( SUCCEEDED ( m_pContext11->Map( 
			m_pCBufferPerMesh, 0, 
			D3D11_MAP_WRITE_DISCARD, 0, 
			&pdata )))
		{
			CBUFFER_PER_MESH cb;
			// ���[���h�s���]�u���ēn��.
			cb.mW	= DirectX::XMMatrixTranspose( World );
			// world, View, Proj ��]�u���ēn��.
			cb.mWVP	= DirectX::XMMatrixTranspose( World * view * proj );
			// �A�j���[�V���������邩�ǂ���( 0.0 : ����, 1.0 : �L��).
			cb.IsAnimation.x = m_pAc == nullptr ? 0.0f : 1.0f;
			memcpy_s(
				pdata.pData, pdata.RowPitch,
				(void*)(&cb), sizeof(cb) );
			m_pContext11->Unmap( m_pCBufferPerMesh, 0 );
		}
		// ��Őݒ肵���R���X�^���g�o�b�t�@���ǂ̃V�F�[�_�[�Ŏg����.
		m_pContext11->VSSetConstantBuffers( 0, 1, &m_pCBufferPerMesh );
		m_pContext11->PSSetConstantBuffers( 0, 1, &m_pCBufferPerMesh );
		
		// �}�e���A����.
		if( SUCCEEDED ( m_pContext11->Map( 
			m_pCBufferPerMaterial, 0, 
			D3D11_MAP_WRITE_DISCARD, 0, 
			&pdata )))
		{
			CBUFFER_PER_MATERIAL cb;
			cb.Ambient	= m.Material.Ambient;	// �A���r�G���g.
			cb.Diffuse	= m.Material.Diffuse;	// �f�B�t���[�Y.
			cb.Specular = m.Material.Specular;	// �X�y�L����.

			memcpy_s( 
				pdata.pData, pdata.RowPitch, 
				(void*)(&cb), sizeof(cb) );

			m_pContext11->Unmap( m_pCBufferPerMaterial, 0 );
		}
		// ��Őݒ肵���R���X�^���g�o�b�t�@���ǂ̃V�F�[�_�[�Ŏg����.
		m_pContext11->VSSetConstantBuffers( 1, 1, &m_pCBufferPerMaterial );
		m_pContext11->PSSetConstantBuffers( 1, 1, &m_pCBufferPerMaterial );
		// �T���v�����Z�b�g.
		m_pContext11->PSSetSamplers( 0, 1, &m_pSampleLinear );

		if( m_Textures.count(m.Material.Name) > 0 ){
			// �e�N�X�`���������.
			// �e�N�X�`�����V�F�[�_�[�ɓn��.
			m_pContext11->PSSetShaderResources( 0, 1, &m_Textures.at(m.Material.Name) );
		} else {
			// �e�N�X�`������.
			ID3D11ShaderResourceView* notex = { nullptr };
			m_pContext11->PSSetShaderResources( 0, 1, &notex );
		}
		// �|���S���������_�����O.
		m_pContext11->DrawIndexed( m.PolygonVertexCount, 0, 0 );
	}
}

// ���[���h�s����擾.
DirectX::XMMATRIX CFbxMesh::GetWorldMatrix()
{
	DirectX::XMMATRIX mTarn, mRot, mScale;
	// ���s�ړ��s��.
	mTarn = DirectX::XMMatrixTranslation(
		m_Position.x, m_Position.y, m_Position.z );
	// ��]�s��.
	mRot = DirectX::XMMatrixRotationRollPitchYaw(
		m_Rotation.x, m_Rotation.y, m_Rotation.z );
	// �g��k���s��.
	mScale = DirectX::XMMatrixScaling(
		m_Scale.x, m_Scale.y, m_Scale.z );

	// ���[���h�s��쐬.
	// �g�k*��]*�ړ�.
	return mScale * mRot * mTarn;
}

// �A�j���[�V�����R���g���[���[�̎擾.
CFbxAnimationController CFbxMesh::GetAnimationController()
{
	if( m_pAc != nullptr ) return *m_pAc;
	return CFbxAnimationController();
}

// �o�b�t�@�̍쐬.
HRESULT CFbxMesh::CreateBuffer()
{
	//--------------------------------------.
	// �萔�o�b�t�@�̐ݒ�
	//--------------------------------------.
	if( FAILED( CreateConstantBuffer( 
		sizeof(CBUFFER_PER_MESH), &m_pCBufferPerMesh ) )) return E_FAIL;
	if( FAILED( CreateConstantBuffer(
		sizeof(CBUFFER_PER_MATERIAL), &m_pCBufferPerMaterial ) )) return E_FAIL;
	if( FAILED( CreateConstantBuffer(
		sizeof(CBUFFER_PER_BONE), &m_pCBufferPerBone ) )) return E_FAIL;
	return S_OK;
}

// ���_�o�b�t�@�쐬.
HRESULT CFbxMesh::CreateVertexBuffers( FBXMeshData& meshData )
{
	D3D11_BUFFER_DESC bd;
	bd.ByteWidth = sizeof(VERTEX)*meshData.Vertices.size();
	bd.Usage		= D3D11_USAGE_DYNAMIC;
	bd.BindFlags	= D3D11_BIND_VERTEX_BUFFER;
	bd.MiscFlags	= 0;
	bd.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;
	bd.StructureByteStride	= 0;
	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem			= &meshData.Vertices[0];
	data.SysMemPitch		= 0;
	data.SysMemSlicePitch	= 0;

	// ���_�o�b�t�@�̍쐬.
	if( FAILED( m_pDevice11->CreateBuffer(
		&bd, &data, &meshData.pVertexBuffer ))){
		return E_FAIL;
	}
	return S_OK;
}

// �C���f�b�N�X�o�b�t�@�쐬.
HRESULT CFbxMesh::CreateIndexBuffers( FBXMeshData& meshData )
{
	D3D11_BUFFER_DESC bd;
	bd.ByteWidth = sizeof(int)*meshData.Indices.size();
	bd.Usage		= D3D11_USAGE_DEFAULT;
	bd.BindFlags	= D3D11_BIND_INDEX_BUFFER;
	bd.MiscFlags	= 0;
	bd.CPUAccessFlags		= 0;
	bd.StructureByteStride	= 0;
	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem			= &meshData.Indices[0];
	data.SysMemPitch		= 0;
	data.SysMemSlicePitch	= 0;
	
	// �C���f�b�N�X�o�b�t�@�̍쐬.
	if( FAILED( m_pDevice11->CreateBuffer(
		&bd, &data, &meshData.pIndexBuffer ))){
		return E_FAIL;
	}
	return S_OK;
}

// �T���v���[�쐬.
HRESULT CFbxMesh::CreateSampler()
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
		return E_FAIL;
	}
	return S_OK;
}

// �R���X�^���g�o�b�t�@�쐬.
HRESULT CFbxMesh::CreateConstantBuffer( const size_t& byte, ID3D11Buffer** buffer )
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

// �V�F�[�_�[�̍쐬.
HRESULT CFbxMesh::CreateShader()
{
	ID3DBlob* pCompileVS = nullptr;
	ID3DBlob* pCompilePS = nullptr;
	UINT uCompileFlag = 0;

#ifdef _DEBUG
	// �V�F�[�_�[�Ńu���C�N��\�邽�߂̏���.
	uCompileFlag = D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
#endif	// #ifdef _DEBUG.

	//----------------------------.
	// ���_�V�F�[�_�[.
	//----------------------------.
	if( FAILED(
		D3DCompileFromFile( 
			SHADER_NAME, 
			nullptr, 
			nullptr, 
			"VS_Main",
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
	//	���_�C���v�b�g.
	//----------------------------.
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
		ERROR_MESSAGE( "vs layout Creating Failure." );
		return E_FAIL;
	}

	SAFE_RELEASE( pCompileVS );
	SAFE_RELEASE( pCompilePS );

	return S_OK;
}

// �A�j���[�V�����p�̍s��v�Z.
void CFbxMesh::AnimMatrixCalculation( const int& meahNo, FBXMeshData& meshData, CFbxAnimationController* pAc  )
{
	// �A�j���[�V�����f�[�^��������ΏI��.
	if( m_pAc == nullptr ) return;

	CFbxAnimationController* pAC = nullptr;
	if( pAc == nullptr ){
		pAC = m_pAc;
	} else {
		pAC = pAc;
	}

	FbxMatrix globalPosition = pAC->GetFrameMatrix( meahNo );
	CBUFFER_PER_BONE cb;
	int boneIndex = 0;
	FbxMatrix frameMatrix;
	FbxMatrix vertexTransformMatrix;
	for( auto& b : meshData.Skin.InitBonePositions ){
		frameMatrix = globalPosition.Inverse() * pAC->GetFrameLinkMatrix( meahNo, boneIndex );
		vertexTransformMatrix = frameMatrix * b;
		cb.Bone[boneIndex] = FbxMatrixConvertDXMMatrix( vertexTransformMatrix );
		boneIndex++;
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

	pAC = nullptr;
}

// �}�e���A���擾.
void CFbxMesh::GetMaterial( FbxMesh* pMesh, FBXMeshData& mesh, const char* fileName )
{
	// �}�e���A����񂪖����̂ŏI��.
	if( pMesh->GetElementMaterialCount() == 0 ) return;

	// Mesh���̃}�e���A�������擾.
	FbxLayerElementMaterial* material = pMesh->GetElementMaterial(0);
	int index = material->GetIndexArray().GetAt(0);

	// FBX�̃}�e���A�����擾.
	FbxSurfaceMaterial* pMat = pMesh->GetNode()->GetSrcObject<FbxSurfaceMaterial>(index);

	// Lambert��Phong��.
	if( pMat->GetClassId().Is( FbxSurfaceLambert::ClassId )){
		// Ambient�̏����擾.
		FbxProperty ambient = pMat->FindProperty( FbxSurfaceMaterial::sAmbient );
		// diffuse�̏����擾.
		FbxProperty diffuse = pMat->FindProperty( FbxSurfaceMaterial::sDiffuse );
		// Specular�̏����擾.
		FbxProperty specular = pMat->FindProperty( FbxSurfaceMaterial::sSpecular );

		MATERIAL tmpMat;	// ���}�e���A����p��.

		// �A���r�G���g�擾.
		if( ambient.IsValid() == true ){
			tmpMat.Ambient.x = (float)ambient.Get<FbxDouble4>()[0];
			tmpMat.Ambient.y = (float)ambient.Get<FbxDouble4>()[1];
			tmpMat.Ambient.z = (float)ambient.Get<FbxDouble4>()[2];
			tmpMat.Ambient.w = (float)ambient.Get<FbxDouble4>()[3];
		} else {
			tmpMat.Ambient = { 1.0f, 1.0f, 1.0f, 1.0f };
		}

		// �A���r�G���g�擾.
		if( diffuse.IsValid() == true ){
			tmpMat.Diffuse.x = (float)diffuse.Get<FbxDouble4>()[0];
			tmpMat.Diffuse.y = (float)diffuse.Get<FbxDouble4>()[1];
			tmpMat.Diffuse.z = (float)diffuse.Get<FbxDouble4>()[2];
			tmpMat.Diffuse.w = (float)diffuse.Get<FbxDouble4>()[3];
		} else {
			tmpMat.Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
		}

		// �A���r�G���g�擾.
		if( specular.IsValid() == true ){
			tmpMat.Specular.x = (float)specular.Get<FbxDouble4>()[0];
			tmpMat.Specular.y = (float)specular.Get<FbxDouble4>()[1];
			tmpMat.Specular.z = (float)specular.Get<FbxDouble4>()[2];
			tmpMat.Specular.w = (float)specular.Get<FbxDouble4>()[3];
		} else {
			tmpMat.Specular = { 1.0f, 1.0f, 1.0f, 1.0f };
		}
		// �}�e���A�����擾.
		tmpMat.Name = pMat->GetName();

		// �}�e���A�����X�g�ɒǉ�.
		mesh.Material = tmpMat;

		//-----------------------.
		// �e�N�X�`���쐬.
		//-----------------------.
		FbxFileTexture* texture = nullptr;
		// FbxFileTexture == �V���O���e�N�X�`��.
		int textureCount = diffuse.GetSrcObjectCount<FbxFileTexture>();
		if( textureCount > 0 ){
			texture = diffuse.GetSrcObject<FbxFileTexture>(0);
			mesh.UVSetCount = textureCount;
		} else {
			// �}���`�e�N�X�`��(���C���[�e�N�X�`��)�̉\��������̂Ō���.
			int layerCount = diffuse.GetSrcObjectCount<FbxLayeredTexture>();
			if( layerCount > 0 ){
				/******************************
					�}���`�e�N�X�`���̎����͌�.
				*******************************/
				texture = diffuse.GetSrcObject<FbxFileTexture>(0);
				mesh.UVSetCount = layerCount;
			}
		}
		if( texture == nullptr ) return;
		// �e�N�X�`���̓ǂݍ���.
		if( FAILED( LoadTexture( texture, fileName, pMat->GetName() ))) return;
	}
}

//-----------------------------------------.
// �e�N�X�`���̓ǂݍ���.
//-----------------------------------------.
HRESULT CFbxMesh::LoadTexture( FbxFileTexture* texture, const char* fileName, const char* keyName )
{
	// �t�@�C�������擾.
	std::string texturePath = texture->GetRelativeFileName();
	std::string path = fileName;

	// �t�@�C���p�X��ݒ�.
	int pos = path.find_last_of('\\')+1;
	path = path.substr( 0, pos );
	path += texturePath;

	// wchar_t �ɕϊ�.
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> cv;
	std::wstring wstr_file_name = cv.from_bytes(path);
	
	// �e�N�X�`�����\�[�X�쐬.
	if( FAILED( DirectX::CreateWICTextureFromFile(
		m_pDevice11,
		wstr_file_name.c_str(),
		nullptr,
		&m_Textures[keyName] ))){
		return E_FAIL;
	}
	return S_OK;
}

//-----------------------------------------.
// ���b�V���̓ǂݍ���.
//-----------------------------------------.
void CFbxMesh::LoadMesh( FbxMesh* pMesh, FBXMeshData& meshData )
{
	LoadIndices( pMesh, meshData );	// �C���f�b�N�X���.

	int ctrlCount = pMesh->GetControlPointsCount();
	// �E�F�C�g.
	std::vector<std::vector<float>> weights( ctrlCount );
	// �{�[���C���f�b�N�X.
	std::vector<std::vector<int>> bones( ctrlCount );
	// �X�L�����̎擾.
	LoadSkin( pMesh, meshData, weights, bones );

	int polygonCount = pMesh->GetPolygonCount();
	// ���_�̐�.
	int vertexCounter = 0;
	// �|���S���̐���.
	for( int i = 0; i < polygonCount; i++ ){
		// �|���S���̃T�C�Y.
		int polygonSize = pMesh->GetPolygonSize(i);
		if( polygonSize != 3 ){
			// �O�p�|���S������Ȃ�.
			ERROR_MESSAGE("Not a triangular polygon");
			return;
		}

		// ��̃|���S���̒��_3��.
		for( int j = 0; j < polygonSize; j++ ){
			// ���_�̃C���f�b�N�X���擾.
			int ctrlPointIndex = pMesh->GetPolygonVertex( i, j );
			// ���_����p��.
			VERTEX vertex = {};

			// ���_���W���擾.
			LoadVertices( pMesh, vertex, ctrlPointIndex );
			// �@���̎擾.
			LoadNormals( pMesh, vertex, ctrlPointIndex, vertexCounter );
			// ���_�J���[�̎擾.
			LoadColors( pMesh, vertex, ctrlPointIndex, vertexCounter );
			// UV���W�̎擾.
			for( int k = 0; k < 2; k++ ){
				LoadUV( pMesh, vertex, ctrlPointIndex, pMesh->GetTextureUVIndex( i, j ), k );
			}
			// �E�F�C�g�ƃ{�[���C���f�b�N�X�̐ݒ�.
			SetBoneWeight( vertex, weights[ctrlPointIndex], bones[ctrlPointIndex] );

			// ���_����ǉ�.
			meshData.Vertices.emplace_back( vertex );
			// ���_�������Z.
			vertexCounter++;
		}
	}
}

//-----------------------------------------.
// �C���f�b�N�X���ǂݍ���.
//-----------------------------------------.
void CFbxMesh::LoadIndices( FbxMesh* pMesh, FBXMeshData& meshData )
{
	// �|���S�����̎擾.
	int polygonCount = pMesh->GetPolygonCount();
	meshData.PolygonVertexCount = pMesh->GetPolygonVertexCount();
	// �|���S���̐������A�ԂƂ��ĕۑ�.
	for( int i = 0; i < polygonCount; i++ ){
		meshData.Indices.emplace_back( i * 3 + 2 );
		meshData.Indices.emplace_back( i * 3 + 1 );
		meshData.Indices.emplace_back( i * 3 + 0 );
	}
}

//-----------------------------------------.
// ���_���ǂݍ���.
//-----------------------------------------.
void CFbxMesh::LoadVertices( FbxMesh* pMesh, VERTEX& vertex, int ctrlPointIndex )
{
	vertex.Pos.x = (float)pMesh->GetControlPoints()[ctrlPointIndex][0];
	vertex.Pos.y = (float)pMesh->GetControlPoints()[ctrlPointIndex][1];
	vertex.Pos.z = (float)pMesh->GetControlPoints()[ctrlPointIndex][2];
}

//-----------------------------------------.
// �@�����ǂݍ���.
//-----------------------------------------.
void CFbxMesh::LoadNormals( FbxMesh* pMesh, VERTEX& vertex, int ctrlPointIndex, int vertexCounter )
{
	if( pMesh->GetElementNormalCount() < 1 ) return;

	FbxGeometryElementNormal* leNormal = pMesh->GetElementNormal(0);
	switch( leNormal->GetMappingMode() )
	{
	case FbxGeometryElement::eByControlPoint:
	{
		switch( leNormal->GetReferenceMode() )
		{
		case FbxGeometryElement::eDirect:
		{
			vertex.Normal.x = -(float)leNormal->GetDirectArray().GetAt(ctrlPointIndex)[0];
			vertex.Normal.y = (float)leNormal->GetDirectArray().GetAt(ctrlPointIndex)[1];
			vertex.Normal.z = (float)leNormal->GetDirectArray().GetAt(ctrlPointIndex)[2];
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int id = leNormal->GetIndexArray().GetAt(ctrlPointIndex);
			vertex.Normal.x = -(float)leNormal->GetDirectArray().GetAt(id)[0];
			vertex.Normal.y = (float)leNormal->GetDirectArray().GetAt(id)[1];
			vertex.Normal.z = (float)leNormal->GetDirectArray().GetAt(id)[2];
		}
		break;

		default:
			break;
		}
	}
	break;

	case FbxGeometryElement::eByPolygonVertex:
	{
		switch (leNormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			vertex.Normal.x = -(float)leNormal->GetDirectArray().GetAt(vertexCounter)[0];
			vertex.Normal.y = (float)leNormal->GetDirectArray().GetAt(vertexCounter)[1];
			vertex.Normal.z = (float)leNormal->GetDirectArray().GetAt(vertexCounter)[2];
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int id = leNormal->GetIndexArray().GetAt(vertexCounter);
			vertex.Normal.x = -(float)leNormal->GetDirectArray().GetAt(id)[0];
			vertex.Normal.y = (float)leNormal->GetDirectArray().GetAt(id)[1];
			vertex.Normal.z = (float)leNormal->GetDirectArray().GetAt(id)[2];
		}
		break;

		default:
			break;
		}
	}
	break;
	}
}

//-----------------------------------------.
// ���_�J���[�ǂݍ���.
//-----------------------------------------.
void CFbxMesh::LoadColors( FbxMesh* pMesh, VERTEX& vertex, int ctrlPointIndex, int vertexCounter )
{
	// ���_�J���[�f�[�^�̐����m�F.
	int colorCount = pMesh->GetElementVertexColorCount();
	if( colorCount == 0 ) return;

	FbxGeometryElementVertexColor* vertexColors = pMesh->GetElementVertexColor(0);
	switch( vertexColors->GetMappingMode() )
	{
	case FbxGeometryElement::eByControlPoint:
	{
		switch( vertexColors->GetReferenceMode() )
		{
		case FbxGeometryElement::eDirect:
		{
			vertex.Color.x = -(float)vertexColors->GetDirectArray().GetAt(ctrlPointIndex)[0];
			vertex.Color.y = (float)vertexColors->GetDirectArray().GetAt(ctrlPointIndex)[1];
			vertex.Color.z = (float)vertexColors->GetDirectArray().GetAt(ctrlPointIndex)[2];
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int id = vertexColors->GetIndexArray().GetAt(ctrlPointIndex);
			vertex.Color.x = -(float)vertexColors->GetDirectArray().GetAt(id)[0];
			vertex.Color.y = (float)vertexColors->GetDirectArray().GetAt(id)[1];
			vertex.Color.z = (float)vertexColors->GetDirectArray().GetAt(id)[2];
		}
		break;

		default:
			break;
		}
	}
	break;

	case FbxGeometryElement::eByPolygonVertex:
	{
		switch (vertexColors->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			vertex.Color.x = (float)vertexColors->GetDirectArray().GetAt(vertexCounter)[0];
			vertex.Color.y = (float)vertexColors->GetDirectArray().GetAt(vertexCounter)[1];
			vertex.Color.z = (float)vertexColors->GetDirectArray().GetAt(vertexCounter)[2];
		}
		break;

		case FbxGeometryElement::eIndexToDirect:
		{
			int id = vertexColors->GetIndexArray().GetAt(vertexCounter);
			vertex.Color.x = (float)vertexColors->GetDirectArray().GetAt(id)[0];
			vertex.Color.y = (float)vertexColors->GetDirectArray().GetAt(id)[1];
			vertex.Color.z = (float)vertexColors->GetDirectArray().GetAt(id)[2];
		}
		break;

		default:
			break;
		}
	}
	break;
	}
}

//-----------------------------------------.
// UV���ǂݍ���.
//-----------------------------------------.
void CFbxMesh::LoadUV( FbxMesh* pMesh, VERTEX& vertex, int ctrlPointIndex, int texUVIndex, int uvLayer )
{
	if( uvLayer >= 2 || pMesh->GetElementUVCount() <= uvLayer ) return;

	FbxGeometryElementUV* pVertexUV = pMesh->GetElementUV( uvLayer );

	switch( pVertexUV->GetMappingMode() )
	{
	case FbxGeometryElement::eByControlPoint:
	{
		switch( pVertexUV->GetReferenceMode() )
		{
		case FbxGeometryElement::eDirect:
		{
			vertex.UV.x = (float)pVertexUV->GetDirectArray().GetAt(ctrlPointIndex)[0];
			vertex.UV.y = -(float)pVertexUV->GetDirectArray().GetAt(ctrlPointIndex)[1];
		}
		break;
		case FbxGeometryElement::eIndexToDirect:
		{
			int id = pVertexUV->GetIndexArray().GetAt(ctrlPointIndex);
			vertex.UV.x = (float)pVertexUV->GetDirectArray().GetAt(id)[0];
			vertex.UV.y = -(float)pVertexUV->GetDirectArray().GetAt(id)[1];
		}
		break;
		default:
			break;
		}
	}
	break;

	case FbxGeometryElement::eByPolygonVertex:
	{
		switch (pVertexUV->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		case FbxGeometryElement::eIndexToDirect:
		{
			vertex.UV.x = (float)pVertexUV->GetDirectArray().GetAt(texUVIndex)[0];
			vertex.UV.y = -(float)pVertexUV->GetDirectArray().GetAt(texUVIndex)[1];
		}
		break;
		default:
			break;
		}
	}
	break;
	}
}

//-----------------------------------------.
// �X�L�����ǂݍ���.
//-----------------------------------------.
void CFbxMesh::LoadSkin( FbxMesh* pMesh, FBXMeshData& meshData, std::vector<std::vector<float>>& weights, std::vector<std::vector<int>>& bones )
{
	// �_�E���L���X�g���ăX�L�������擾.
	FbxSkin* pSkin = (FbxSkin*)pMesh->GetDeformer( 0, FbxDeformer::eSkin );
	if( pSkin == nullptr ) return;

	m_MeshClusterData.emplace_back();

	// �{�[���̐�.
	int boneCount = pSkin->GetClusterCount();
	for( int boneIndex = 0; boneIndex < boneCount; boneIndex++ ){
		// �{�[�����擾.
		FbxCluster* pCluster = pSkin->GetCluster( boneIndex );
		FbxNode* pNode = pCluster->GetLink();

		m_MeshClusterData.back().ClusterKey[pNode->GetName()] = pNode;
		m_MeshClusterData.back().ClusterName.emplace_back( pNode->GetName() );

		FbxAMatrix bindPoseMatrix;
		FbxAMatrix newbindpose;

		pCluster->GetTransformLinkMatrix( bindPoseMatrix );
		pCluster->GetTransformMatrix( newbindpose );

		newbindpose = bindPoseMatrix.Inverse() * newbindpose;
		// �����{�[�����W�̐ݒ�.
		meshData.Skin.InitBonePositions.emplace_back( newbindpose );

		// �{�[���C���f�b�N�X�ƃE�F�C�g�̐ݒ�.
		int*	boneVertexIndices = pCluster->GetControlPointIndices();
		double* boneVertexWeights = pCluster->GetControlPointWeights();
		int	numBoneVertexIndices = pCluster->GetControlPointIndicesCount();
		for( int boneVertexIndex = 0; boneVertexIndex < numBoneVertexIndices; boneVertexIndex++ ){
			int cpIndex = boneVertexIndices[boneVertexIndex];
			float boneWeight = (float)boneVertexWeights[boneVertexIndex];
			weights[cpIndex].emplace_back(boneWeight);
			bones[cpIndex].emplace_back(boneIndex);
		}
	}
	// �E�F�C�g���d�����Ƀ\�[�g.
	for( int i = 0; i < (int)weights.size(); i++ ){
		for( int m = (int)weights[i].size(); m > 1; m-- ){
			for( int n = 1; n < m; n++ ){
				if( weights[i][n-1] < weights[i][n] ){

					float tmpweight = weights[i][n-1];
					weights[i][n-1] = weights[i][n];
					weights[i][n] = tmpweight;

					int tmpbone = bones[i][n-1];
					bones[i][n-1] = bones[i][n];
					bones[i][n] = tmpbone;
				}
			}
		}
	}
}

//-----------------------------------------.
// �E�F�C�g�ƃ{�[���̐ݒ�.
//-----------------------------------------.
void CFbxMesh::SetBoneWeight( VERTEX& vertex, const std::vector<float>& weight, const std::vector<int>& bone )
{
	switch( weight.size() )
	{
	default:
	case 4:
		vertex.BoneWeight.w = weight[3];
		vertex.BoneIndex.w = bone[3];
	case 3:
		vertex.BoneWeight.z = weight[2];
		vertex.BoneIndex.z = bone[2];
	case 2:
		vertex.BoneWeight.y = weight[1];
		vertex.BoneIndex.y = bone[1];
	case 1:
		vertex.BoneWeight.x = weight[0];
		vertex.BoneIndex.x = bone[0];
		break;
	case 0:
		break;
	}
}


// �A�j���[�V�������x�̐ݒ�.
void CFbxMesh::SetAnimSpeed( const double& speed )
{
	if( m_pAc == nullptr ) return;
	m_pAc->SetAnimSpeed( speed );
}
// �A�j���[�V�������x�̐ݒ�.
void CFbxMesh::ChangeAnimation( int& index )
{
	if( m_pAc == nullptr ) return;
	m_pAc->ChangeAnimation( index );
}

//-----------------------------------------.
// �m�[�h�����ǂ�ċN�֐�.
//-----------------------------------------.
void CFbxMesh::RecurrenceNode( FbxNode* pNode )
{
	// �q�m�[�h�̐����擾.
	int childNodeNum = pNode->GetChildCount();
	GetDataByNodeType( pNode );
	// �m�[�h�^�C�v�ʂŃf�[�^���擾.
	for( int i = 0; i < childNodeNum; i++ ){
		FbxNode* pFbxChild = pNode->GetChild(i);
		RecurrenceNode( pFbxChild );	// �ċN.
	}
}

//-----------------------------------------.
// �m�[�h�^�C�v�ʎ擾�֐�.
//-----------------------------------------.
void CFbxMesh::GetDataByNodeType( FbxNode* pNode )
{
	FbxNodeAttribute* attribute = pNode->GetNodeAttribute();
	if( attribute == nullptr ) return;

	FbxNodeAttribute::EType type = attribute->GetAttributeType();
	switch( type )
	{
	case fbxsdk::FbxNodeAttribute::eUnknown:
		break;
	case fbxsdk::FbxNodeAttribute::eNull:
		break;
	case fbxsdk::FbxNodeAttribute::eMarker:
		break;
	case fbxsdk::FbxNodeAttribute::eSkeleton:
		m_Skeletons.emplace_back( pNode->GetSkeleton() );
		break;
	case fbxsdk::FbxNodeAttribute::eMesh:
		break;
	case fbxsdk::FbxNodeAttribute::eNurbs:
		break;
	case fbxsdk::FbxNodeAttribute::ePatch:
		break;
	case fbxsdk::FbxNodeAttribute::eCamera:
		break;
	case fbxsdk::FbxNodeAttribute::eCameraStereo:
		break;
	case fbxsdk::FbxNodeAttribute::eCameraSwitcher:
		break;
	case fbxsdk::FbxNodeAttribute::eLight:
		break;
	case fbxsdk::FbxNodeAttribute::eOpticalReference:
		break;
	case fbxsdk::FbxNodeAttribute::eOpticalMarker:
		break;
	case fbxsdk::FbxNodeAttribute::eNurbsCurve:
		break;
	case fbxsdk::FbxNodeAttribute::eTrimNurbsSurface:
		break;
	case fbxsdk::FbxNodeAttribute::eBoundary:
		break;
	case fbxsdk::FbxNodeAttribute::eNurbsSurface:
		break;
	case fbxsdk::FbxNodeAttribute::eShape:
		break;
	case fbxsdk::FbxNodeAttribute::eLODGroup:
		break;
	case fbxsdk::FbxNodeAttribute::eSubDiv:
		break;
	case fbxsdk::FbxNodeAttribute::eCachedEffect:
		break;
	case fbxsdk::FbxNodeAttribute::eLine:
		break;
	default:
		break;
	}
}