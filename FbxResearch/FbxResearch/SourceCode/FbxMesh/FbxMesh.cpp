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

// 作成.
HRESULT CFbxMesh::Create( ID3D11DeviceContext* pContext11, const char* fileName )
{
	if( pContext11 == nullptr ) return E_FAIL;
	// コンテキストの取得.
	m_pContext11 = pContext11;
	// デバイスの取得.
	m_pContext11->GetDevice( &m_pDevice11 );
	if( m_pDevice11 == nullptr ) return E_FAIL;

	//------------------------------.
	// マネージャーの作成.
	//------------------------------.
	m_pFbxManager = FbxManager::Create();
	if( m_pFbxManager == nullptr ){
		ERROR_MESSAGE( "FbxManager Create Failure." );
		return E_FAIL;
	}

	{
	//------------------------------.
	// インポーターの作成.
	//------------------------------.
	FbxImporter* pFbxImpoter = FbxImporter::Create( m_pFbxManager, "imp" );
	if( pFbxImpoter == nullptr ){
		ERROR_MESSAGE( "FbxImpoter Create Failure." );
		return E_FAIL;
	}

	//------------------------------.
	// FBXファイルの読み込み.
	//------------------------------.
	// ファイル名の設定.
	FbxString fbxFileName( fileName );
	if( pFbxImpoter->Initialize( fbxFileName.Buffer() ) == false ){
		ERROR_MESSAGE( "FbxFile Loading Failure." );
		return E_FAIL;
	}

	//------------------------------.
	// シーンオブジェクトの作成.
	//------------------------------.
	m_pFbxScene = FbxScene::Create( m_pFbxManager, "fbxScene" );
	if( m_pFbxScene == nullptr ){
		ERROR_MESSAGE( "FbxScene Create Failure." );
		return E_FAIL;
	}

	//------------------------------.
	// インポーターとシーンオブジェクトの関連付け.
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
	// ポリゴンを三角形にする.
	// 多角形ポリゴンがあれば作りなおすので時間がかかる.
	convertReslut = geometryConverter.Triangulate( m_pFbxScene, true );
	geometryConverter.RemoveBadPolygonsFromMeshes( m_pFbxScene );
	// マテリアルごとのメッシュを作成する.
	convertReslut = geometryConverter.SplitMeshesPerMaterial( m_pFbxScene, true );

	//----------------------------.
	// FbxSkeletonの数を取得.
	//----------------------------.
	int skeletonNum = m_pFbxScene->GetSrcObjectCount<FbxSkeleton>();
	int skeletonNo = 0;
	m_Skeletons.resize( skeletonNum );
	for( auto& s : m_Skeletons ){
		s = m_pFbxScene->GetSrcObject<FbxSkeleton>(skeletonNo);
		skeletonNo++;
	}

	//----------------------------.
	// FbxMeshの数を取得.
	//----------------------------.
	int meshNum = m_pFbxScene->GetSrcObjectCount<FbxMesh>();
	int meshNo = 0;
	m_MeshData.resize( meshNum );
	for( auto& m : m_MeshData ){
		// メッシュデータの取得.
		FbxMesh* pMesh = m_pFbxScene->GetSrcObject<FbxMesh>(meshNo);
		// マテリアルの取得.
		GetMaterial( pMesh, m, fileName );
		// メッシュデータの読み込み.
		LoadMesh( pMesh, m );
		// 頂点バッファの作成.
		if( FAILED( CreateVertexBuffers( m ) )) return E_FAIL;
		// インデックスバッファの作成.
		if( FAILED( CreateIndexBuffers( m ) )) return E_FAIL;
		meshNo++;
	}

	//----------------------------.
	//	アニメーション.
	//----------------------------.
	m_pAnimLoader = std::make_unique<CFbxAnimationLoader>();
	std::vector<SAnimationData>	animDataList;
	m_pAnimLoader->LoadAnimationData( m_pFbxScene, m_MeshClusterData, m_Skeletons, &animDataList );
	if( animDataList.empty() == false ){
		// 上で設定したアニメーションデータがあれば.
		// アニメーションコントローラーを作成して.
		// アニメーションデータを追加.
		m_pAc = new CFbxAnimationController;
		m_pAc->SetAnimDataList( animDataList );
	}
	}

	// バッファーの作成.
	if( FAILED( CreateBuffer() )) return E_FAIL;
	// サンプラーの作成.
	if( FAILED( CreateSampler() )) return E_FAIL;
	// シェーダーの作成.
	if( FAILED( CreateShader() )) return E_FAIL;

	return S_OK;
}

// 破壊.
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
//				破壊.
//-----------------------------------------.
HRESULT CFbxMesh::LoadModel( const char* fileName )
{
	//------------------------------.
	// インポーターの作成.
	//------------------------------.
	FbxImporter* pFbxImpoter = FbxImporter::Create( m_pFbxManager, "imp" );
	if( pFbxImpoter == nullptr ){
		ERROR_MESSAGE( "FbxImpoter Create Failure." );
		return E_FAIL;
	}

	//------------------------------.
	// FBXファイルの読み込み.
	//------------------------------.
	// ファイル名の設定.
	FbxString fbxFileName( fileName );
	if( pFbxImpoter->Initialize( fbxFileName.Buffer() ) == false ){
		ERROR_MESSAGE( "FbxFile Loading Failure." );
		return E_FAIL;
	}

	//------------------------------.
	// シーンオブジェクトの作成.
	//------------------------------.
	m_pFbxScene = FbxScene::Create( m_pFbxManager, "fbxScene" );
	if( m_pFbxScene == nullptr ){
		ERROR_MESSAGE( "FbxScene Create Failure." );
		return E_FAIL;
	}

	//------------------------------.
	// インポーターとシーンオブジェクトの関連付け.
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
	// ポリゴンを三角形にする.
	// 多角形ポリゴンがあれば作りなおすので時間がかかる.
	convertReslut = geometryConverter.Triangulate( m_pFbxScene, true );
	if( convertReslut == false ){
		ERROR_MESSAGE( "Triangulate Failure." );
		return E_FAIL;
	}
	geometryConverter.RemoveBadPolygonsFromMeshes( m_pFbxScene );
	// メッシュをマテリアルごとに分割する.
	convertReslut = geometryConverter.SplitMeshesPerMaterial( m_pFbxScene, true );
	if( convertReslut == false ){
		ERROR_MESSAGE( "SplitMeshesPerMaterial Failure." );
		return E_FAIL;
	}


	//----------------------------.
	// FbxSkeletonの数を取得.
	//----------------------------.
	int skeletonNum = m_pFbxScene->GetSrcObjectCount<FbxSkeleton>();
	int skeletonNo = 0;
	m_Skeletons.resize( skeletonNum );
	for( auto& s : m_Skeletons ){
		s = m_pFbxScene->GetSrcObject<FbxSkeleton>(skeletonNo);
		skeletonNo++;
	}

	//----------------------------.
	// FbxMeshの数を取得.
	//----------------------------.
	int meshNum = m_pFbxScene->GetSrcObjectCount<FbxMesh>();
	int meshNo = 0;
	m_MeshData.resize( meshNum );
	for( auto& m : m_MeshData ){
		// メッシュデータの取得.
		FbxMesh* pMesh = m_pFbxScene->GetSrcObject<FbxMesh>(meshNo);
		// マテリアルの取得.
		GetMaterial( pMesh, m, fileName );
		// メッシュデータの読み込み.
		LoadMesh( pMesh, m );
		// 頂点バッファの作成.
		if( FAILED( CreateVertexBuffers( m ) )) return E_FAIL;
		// インデックスバッファの作成.
		if( FAILED( CreateIndexBuffers( m ) )) return E_FAIL;
		meshNo++;
	}

	//----------------------------.
	//	アニメーションの読み込み.
	//----------------------------.
	m_pAnimLoader = std::make_unique<CFbxAnimationLoader>();
	std::vector<SAnimationData>	animDataList;
	m_pAnimLoader->LoadAnimationData( m_pFbxScene, m_MeshClusterData, m_Skeletons, &animDataList );
	if( animDataList.empty() == false ){
		// 上で設定したアニメーションデータがあれば.
		// アニメーションコントローラーを作成して.
		// アニメーションデータを追加.
		m_pAc = new CFbxAnimationController;
		m_pAc->SetAnimDataList( animDataList );
	}

	return S_OK;
}

// 描画.
void CFbxMesh::Render(
	const DirectX::XMMATRIX& view,
	const DirectX::XMMATRIX& proj,
	CFbxAnimationController* pAc )
{
	// ワールド行列取得.
	DirectX::XMMATRIX World	= GetWorldMatrix();
	int meshNo = 0;
	UINT stride = sizeof(VERTEX);
	UINT offset = 0;

	if( pAc == nullptr ){
		// アニメーションフレームの更新.
		if( m_pAc != nullptr ) m_pAc->FrameUpdate();
	} else {
		// アニメーションフレームの更新.
		if( pAc != nullptr ) pAc->FrameUpdate();
	}

	// メッシュデータ分描画.
	for( auto& m : m_MeshData ){
		// アニメーションの行列計算.
		AnimMatrixCalculation( meshNo, m, pAc );
		meshNo++;

		// 頂点バッファをセット.
		m_pContext11->IASetVertexBuffers( 0, 1, &m.pVertexBuffer, &stride, &offset );
		m_pContext11->IASetIndexBuffer( m.pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		m_pContext11->IASetInputLayout( m_pVertexLayout );
		// プリティブトポロジーをセット.
		m_pContext11->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		// 使用するシェーダーの設定.
		m_pContext11->VSSetShader( m_pVertexShader, nullptr, 0 );
		m_pContext11->PSSetShader( m_pPixelShader, nullptr, 0 );

		D3D11_MAPPED_SUBRESOURCE pdata;
		if( SUCCEEDED ( m_pContext11->Map( 
			m_pCBufferPerMesh, 0, 
			D3D11_MAP_WRITE_DISCARD, 0, 
			&pdata )))
		{
			CBUFFER_PER_MESH cb;
			// ワールド行列を転置して渡す.
			cb.mW	= DirectX::XMMatrixTranspose( World );
			// world, View, Proj を転置して渡す.
			cb.mWVP	= DirectX::XMMatrixTranspose( World * view * proj );
			// アニメーションがあるかどうか( 0.0 : 無し, 1.0 : 有り).
			cb.IsAnimation.x = m_pAc == nullptr ? 0.0f : 1.0f;
			memcpy_s(
				pdata.pData, pdata.RowPitch,
				(void*)(&cb), sizeof(cb) );
			m_pContext11->Unmap( m_pCBufferPerMesh, 0 );
		}
		// 上で設定したコンスタントバッファをどのシェーダーで使うか.
		m_pContext11->VSSetConstantBuffers( 0, 1, &m_pCBufferPerMesh );
		m_pContext11->PSSetConstantBuffers( 0, 1, &m_pCBufferPerMesh );
		
		// マテリアル分.
		if( SUCCEEDED ( m_pContext11->Map( 
			m_pCBufferPerMaterial, 0, 
			D3D11_MAP_WRITE_DISCARD, 0, 
			&pdata )))
		{
			CBUFFER_PER_MATERIAL cb;
			cb.Ambient	= m.Material.Ambient;	// アンビエント.
			cb.Diffuse	= m.Material.Diffuse;	// ディフューズ.
			cb.Specular = m.Material.Specular;	// スペキュラ.

			memcpy_s( 
				pdata.pData, pdata.RowPitch, 
				(void*)(&cb), sizeof(cb) );

			m_pContext11->Unmap( m_pCBufferPerMaterial, 0 );
		}
		// 上で設定したコンスタントバッファをどのシェーダーで使うか.
		m_pContext11->VSSetConstantBuffers( 1, 1, &m_pCBufferPerMaterial );
		m_pContext11->PSSetConstantBuffers( 1, 1, &m_pCBufferPerMaterial );
		// サンプラをセット.
		m_pContext11->PSSetSamplers( 0, 1, &m_pSampleLinear );

		if( m_Textures.count(m.Material.Name) > 0 ){
			// テクスチャがあれば.
			// テクスチャをシェーダーに渡す.
			m_pContext11->PSSetShaderResources( 0, 1, &m_Textures.at(m.Material.Name) );
		} else {
			// テクスチャ無し.
			ID3D11ShaderResourceView* notex = { nullptr };
			m_pContext11->PSSetShaderResources( 0, 1, &notex );
		}
		// ポリゴンをレンダリング.
		m_pContext11->DrawIndexed( m.PolygonVertexCount, 0, 0 );
	}
}

// ワールド行列を取得.
DirectX::XMMATRIX CFbxMesh::GetWorldMatrix()
{
	DirectX::XMMATRIX mTarn, mRot, mScale;
	// 平行移動行列.
	mTarn = DirectX::XMMatrixTranslation(
		m_Position.x, m_Position.y, m_Position.z );
	// 回転行列.
	mRot = DirectX::XMMatrixRotationRollPitchYaw(
		m_Rotation.x, m_Rotation.y, m_Rotation.z );
	// 拡大縮小行列.
	mScale = DirectX::XMMatrixScaling(
		m_Scale.x, m_Scale.y, m_Scale.z );

	// ワールド行列作成.
	// 拡縮*回転*移動.
	return mScale * mRot * mTarn;
}

// アニメーションコントローラーの取得.
CFbxAnimationController CFbxMesh::GetAnimationController()
{
	if( m_pAc != nullptr ) return *m_pAc;
	return CFbxAnimationController();
}

// バッファの作成.
HRESULT CFbxMesh::CreateBuffer()
{
	//--------------------------------------.
	// 定数バッファの設定
	//--------------------------------------.
	if( FAILED( CreateConstantBuffer( 
		sizeof(CBUFFER_PER_MESH), &m_pCBufferPerMesh ) )) return E_FAIL;
	if( FAILED( CreateConstantBuffer(
		sizeof(CBUFFER_PER_MATERIAL), &m_pCBufferPerMaterial ) )) return E_FAIL;
	if( FAILED( CreateConstantBuffer(
		sizeof(CBUFFER_PER_BONE), &m_pCBufferPerBone ) )) return E_FAIL;
	return S_OK;
}

// 頂点バッファ作成.
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

	// 頂点バッファの作成.
	if( FAILED( m_pDevice11->CreateBuffer(
		&bd, &data, &meshData.pVertexBuffer ))){
		return E_FAIL;
	}
	return S_OK;
}

// インデックスバッファ作成.
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
	
	// インデックスバッファの作成.
	if( FAILED( m_pDevice11->CreateBuffer(
		&bd, &data, &meshData.pIndexBuffer ))){
		return E_FAIL;
	}
	return S_OK;
}

// サンプラー作成.
HRESULT CFbxMesh::CreateSampler()
{
	D3D11_SAMPLER_DESC samDesc;
	ZeroMemory(&samDesc, sizeof(samDesc));
	samDesc.Filter		= D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samDesc.AddressU	= D3D11_TEXTURE_ADDRESS_WRAP;
	samDesc.AddressV	= D3D11_TEXTURE_ADDRESS_WRAP;
	samDesc.AddressW	= D3D11_TEXTURE_ADDRESS_WRAP;
	// サンプラ作成.
	if( FAILED( m_pDevice11->CreateSamplerState(
		&samDesc, &m_pSampleLinear ))){
		return E_FAIL;
	}
	return S_OK;
}

// コンスタントバッファ作成.
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

// シェーダーの作成.
HRESULT CFbxMesh::CreateShader()
{
	ID3DBlob* pCompileVS = nullptr;
	ID3DBlob* pCompilePS = nullptr;
	UINT uCompileFlag = 0;

#ifdef _DEBUG
	// シェーダーでブレイクを貼るための処理.
	uCompileFlag = D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
#endif	// #ifdef _DEBUG.

	//----------------------------.
	// 頂点シェーダー.
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
	// ピクセルシェーダー.
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
	//	頂点インプット.
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
	int numElements = sizeof(layout)/sizeof(layout[0]);	//要素数算出.
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

// アニメーション用の行列計算.
void CFbxMesh::AnimMatrixCalculation( const int& meahNo, FBXMeshData& meshData, CFbxAnimationController* pAc  )
{
	// アニメーションデータが無ければ終了.
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
	// 上で設定したコンスタントバッファをどのシェーダーで使うか.
	m_pContext11->VSSetConstantBuffers( 2, 1, &m_pCBufferPerBone );
	m_pContext11->PSSetConstantBuffers( 2, 1, &m_pCBufferPerBone );

	pAC = nullptr;
}

// マテリアル取得.
void CFbxMesh::GetMaterial( FbxMesh* pMesh, FBXMeshData& mesh, const char* fileName )
{
	// マテリアル情報が無いので終了.
	if( pMesh->GetElementMaterialCount() == 0 ) return;

	// Mesh側のマテリアル情報を取得.
	FbxLayerElementMaterial* material = pMesh->GetElementMaterial(0);
	int index = material->GetIndexArray().GetAt(0);

	// FBXのマテリアルを取得.
	FbxSurfaceMaterial* pMat = pMesh->GetNode()->GetSrcObject<FbxSurfaceMaterial>(index);

	// LambertかPhongか.
	if( pMat->GetClassId().Is( FbxSurfaceLambert::ClassId )){
		// Ambientの情報を取得.
		FbxProperty ambient = pMat->FindProperty( FbxSurfaceMaterial::sAmbient );
		// diffuseの情報を取得.
		FbxProperty diffuse = pMat->FindProperty( FbxSurfaceMaterial::sDiffuse );
		// Specularの情報を取得.
		FbxProperty specular = pMat->FindProperty( FbxSurfaceMaterial::sSpecular );

		MATERIAL tmpMat;	// 仮マテリアルを用意.

		// アンビエント取得.
		if( ambient.IsValid() == true ){
			tmpMat.Ambient.x = (float)ambient.Get<FbxDouble4>()[0];
			tmpMat.Ambient.y = (float)ambient.Get<FbxDouble4>()[1];
			tmpMat.Ambient.z = (float)ambient.Get<FbxDouble4>()[2];
			tmpMat.Ambient.w = (float)ambient.Get<FbxDouble4>()[3];
		} else {
			tmpMat.Ambient = { 1.0f, 1.0f, 1.0f, 1.0f };
		}

		// アンビエント取得.
		if( diffuse.IsValid() == true ){
			tmpMat.Diffuse.x = (float)diffuse.Get<FbxDouble4>()[0];
			tmpMat.Diffuse.y = (float)diffuse.Get<FbxDouble4>()[1];
			tmpMat.Diffuse.z = (float)diffuse.Get<FbxDouble4>()[2];
			tmpMat.Diffuse.w = (float)diffuse.Get<FbxDouble4>()[3];
		} else {
			tmpMat.Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
		}

		// アンビエント取得.
		if( specular.IsValid() == true ){
			tmpMat.Specular.x = (float)specular.Get<FbxDouble4>()[0];
			tmpMat.Specular.y = (float)specular.Get<FbxDouble4>()[1];
			tmpMat.Specular.z = (float)specular.Get<FbxDouble4>()[2];
			tmpMat.Specular.w = (float)specular.Get<FbxDouble4>()[3];
		} else {
			tmpMat.Specular = { 1.0f, 1.0f, 1.0f, 1.0f };
		}
		// マテリアル名取得.
		tmpMat.Name = pMat->GetName();

		// マテリアルリストに追加.
		mesh.Material = tmpMat;

		//-----------------------.
		// テクスチャ作成.
		//-----------------------.
		FbxFileTexture* texture = nullptr;
		// FbxFileTexture == シングルテクスチャ.
		int textureCount = diffuse.GetSrcObjectCount<FbxFileTexture>();
		if( textureCount > 0 ){
			texture = diffuse.GetSrcObject<FbxFileTexture>(0);
			mesh.UVSetCount = textureCount;
		} else {
			// マルチテクスチャ(レイヤーテクスチャ)の可能性があるので検索.
			int layerCount = diffuse.GetSrcObjectCount<FbxLayeredTexture>();
			if( layerCount > 0 ){
				/******************************
					マルチテクスチャの実装は後.
				*******************************/
				texture = diffuse.GetSrcObject<FbxFileTexture>(0);
				mesh.UVSetCount = layerCount;
			}
		}
		if( texture == nullptr ) return;
		// テクスチャの読み込み.
		if( FAILED( LoadTexture( texture, fileName, pMat->GetName() ))) return;
	}
}

//-----------------------------------------.
// テクスチャの読み込み.
//-----------------------------------------.
HRESULT CFbxMesh::LoadTexture( FbxFileTexture* texture, const char* fileName, const char* keyName )
{
	// ファイル名を取得.
	std::string texturePath = texture->GetRelativeFileName();
	std::string path = fileName;

	// ファイルパスを設定.
	int pos = path.find_last_of('\\')+1;
	path = path.substr( 0, pos );
	path += texturePath;

	// wchar_t に変換.
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> cv;
	std::wstring wstr_file_name = cv.from_bytes(path);
	
	// テクスチャリソース作成.
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
// メッシュの読み込み.
//-----------------------------------------.
void CFbxMesh::LoadMesh( FbxMesh* pMesh, FBXMeshData& meshData )
{
	LoadIndices( pMesh, meshData );	// インデックス情報.

	int ctrlCount = pMesh->GetControlPointsCount();
	// ウェイト.
	std::vector<std::vector<float>> weights( ctrlCount );
	// ボーンインデックス.
	std::vector<std::vector<int>> bones( ctrlCount );
	// スキン情報の取得.
	LoadSkin( pMesh, meshData, weights, bones );

	int polygonCount = pMesh->GetPolygonCount();
	// 頂点の数.
	int vertexCounter = 0;
	// ポリゴンの数分.
	for( int i = 0; i < polygonCount; i++ ){
		// ポリゴンのサイズ.
		int polygonSize = pMesh->GetPolygonSize(i);
		if( polygonSize != 3 ){
			// 三角ポリゴンじゃない.
			ERROR_MESSAGE("Not a triangular polygon");
			return;
		}

		// 一つのポリゴンの頂点3つ分.
		for( int j = 0; j < polygonSize; j++ ){
			// 頂点のインデックスを取得.
			int ctrlPointIndex = pMesh->GetPolygonVertex( i, j );
			// 頂点情報を用意.
			VERTEX vertex = {};

			// 頂点座標を取得.
			LoadVertices( pMesh, vertex, ctrlPointIndex );
			// 法線の取得.
			LoadNormals( pMesh, vertex, ctrlPointIndex, vertexCounter );
			// 頂点カラーの取得.
			LoadColors( pMesh, vertex, ctrlPointIndex, vertexCounter );
			// UV座標の取得.
			for( int k = 0; k < 2; k++ ){
				LoadUV( pMesh, vertex, ctrlPointIndex, pMesh->GetTextureUVIndex( i, j ), k );
			}
			// ウェイトとボーンインデックスの設定.
			SetBoneWeight( vertex, weights[ctrlPointIndex], bones[ctrlPointIndex] );

			// 頂点情報を追加.
			meshData.Vertices.emplace_back( vertex );
			// 頂点数を加算.
			vertexCounter++;
		}
	}
}

//-----------------------------------------.
// インデックス情報読み込み.
//-----------------------------------------.
void CFbxMesh::LoadIndices( FbxMesh* pMesh, FBXMeshData& meshData )
{
	// ポリゴン数の取得.
	int polygonCount = pMesh->GetPolygonCount();
	meshData.PolygonVertexCount = pMesh->GetPolygonVertexCount();
	// ポリゴンの数だけ連番として保存.
	for( int i = 0; i < polygonCount; i++ ){
		meshData.Indices.emplace_back( i * 3 + 2 );
		meshData.Indices.emplace_back( i * 3 + 1 );
		meshData.Indices.emplace_back( i * 3 + 0 );
	}
}

//-----------------------------------------.
// 頂点情報読み込み.
//-----------------------------------------.
void CFbxMesh::LoadVertices( FbxMesh* pMesh, VERTEX& vertex, int ctrlPointIndex )
{
	vertex.Pos.x = (float)pMesh->GetControlPoints()[ctrlPointIndex][0];
	vertex.Pos.y = (float)pMesh->GetControlPoints()[ctrlPointIndex][1];
	vertex.Pos.z = (float)pMesh->GetControlPoints()[ctrlPointIndex][2];
}

//-----------------------------------------.
// 法線情報読み込み.
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
// 頂点カラー読み込み.
//-----------------------------------------.
void CFbxMesh::LoadColors( FbxMesh* pMesh, VERTEX& vertex, int ctrlPointIndex, int vertexCounter )
{
	// 頂点カラーデータの数を確認.
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
// UV情報読み込み.
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
// スキン情報読み込み.
//-----------------------------------------.
void CFbxMesh::LoadSkin( FbxMesh* pMesh, FBXMeshData& meshData, std::vector<std::vector<float>>& weights, std::vector<std::vector<int>>& bones )
{
	// ダウンキャストしてスキン情報を取得.
	FbxSkin* pSkin = (FbxSkin*)pMesh->GetDeformer( 0, FbxDeformer::eSkin );
	if( pSkin == nullptr ) return;

	m_MeshClusterData.emplace_back();

	// ボーンの数.
	int boneCount = pSkin->GetClusterCount();
	for( int boneIndex = 0; boneIndex < boneCount; boneIndex++ ){
		// ボーン情報取得.
		FbxCluster* pCluster = pSkin->GetCluster( boneIndex );
		FbxNode* pNode = pCluster->GetLink();

		m_MeshClusterData.back().ClusterKey[pNode->GetName()] = pNode;
		m_MeshClusterData.back().ClusterName.emplace_back( pNode->GetName() );

		FbxAMatrix bindPoseMatrix;
		FbxAMatrix newbindpose;

		pCluster->GetTransformLinkMatrix( bindPoseMatrix );
		pCluster->GetTransformMatrix( newbindpose );

		newbindpose = bindPoseMatrix.Inverse() * newbindpose;
		// 初期ボーン座標の設定.
		meshData.Skin.InitBonePositions.emplace_back( newbindpose );

		// ボーンインデックスとウェイトの設定.
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
	// ウェイトを重い順にソート.
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
// ウェイトとボーンの設定.
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


// アニメーション速度の設定.
void CFbxMesh::SetAnimSpeed( const double& speed )
{
	if( m_pAc == nullptr ) return;
	m_pAc->SetAnimSpeed( speed );
}
// アニメーション速度の設定.
void CFbxMesh::ChangeAnimation( int& index )
{
	if( m_pAc == nullptr ) return;
	m_pAc->ChangeAnimation( index );
}

//-----------------------------------------.
// ノードをたどる再起関数.
//-----------------------------------------.
void CFbxMesh::RecurrenceNode( FbxNode* pNode )
{
	// 子ノードの数を取得.
	int childNodeNum = pNode->GetChildCount();
	GetDataByNodeType( pNode );
	// ノードタイプ別でデータを取得.
	for( int i = 0; i < childNodeNum; i++ ){
		FbxNode* pFbxChild = pNode->GetChild(i);
		RecurrenceNode( pFbxChild );	// 再起.
	}
}

//-----------------------------------------.
// ノードタイプ別取得関数.
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