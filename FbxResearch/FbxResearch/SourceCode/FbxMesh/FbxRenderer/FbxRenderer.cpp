#include "FbxRenderer.h"

CFbxRenderer::CFbxRenderer()
{
}

CFbxRenderer::~CFbxRenderer()
{
}

//-----------------------------------------.
//				描画.
//-----------------------------------------.
void CFbxRenderer::Render(
	CFbxModel mdoel,
	const DirectX::XMMATRIX& view, 
	const DirectX::XMMATRIX& proj,
	CFbxAnimationController* pAc )
{
	// ワールド行列取得.
//	DirectX::XMMATRIX World	= GetWorldMatrix();
	int meshNo = 0;
	UINT stride = sizeof(VERTEX);
	UINT offset = 0;
	// メッシュデータ分描画.
	for( auto& m : mdoel.m_MeshData ){
		// アニメーションの行列計算.
		AnimMatrixCalculation( mdoel, meshNo, m, pAc );
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
//			cb.mW	= DirectX::XMMatrixTranspose( World );
			// world, View, Proj を転置して渡す.
//			cb.mWVP	= DirectX::XMMatrixTranspose( World * view * proj );
			// アニメーションがあるかどうか( 0.0 : 無し, 1.0 : 有り).
			cb.IsAnimation.x = mdoel.m_pAc == nullptr ? 0.0f : 1.0f;
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

		if( mdoel.m_Textures.count(m.Material.Name) > 0 ){
			// テクスチャがあれば.
			// テクスチャをシェーダーに渡す.
			m_pContext11->PSSetShaderResources( 0, 1, &mdoel.m_Textures.at(m.Material.Name) );
		} else {
			// テクスチャ無し.
			ID3D11ShaderResourceView* notex = { nullptr };
			m_pContext11->PSSetShaderResources( 0, 1, &notex );
		}
		// ポリゴンをレンダリング.
		m_pContext11->DrawIndexed( m.PolygonVertexCount, 0, 0 );
	}
}

// アニメーション用の行列計算.
void CFbxRenderer::AnimMatrixCalculation(
	CFbxModel mdoel,
	const int& meahNo,
	FBXMeshData& meshData,
	CFbxAnimationController* pAc  )
{
	// アニメーションデータが無ければ終了.
	if( mdoel.m_pAc == nullptr ) return;

	CFbxAnimationController* pAC = nullptr;
	if( pAc == nullptr ){
		pAC = mdoel.m_pAc;
	} else {
		pAC = pAc;
	}

	// アニメーションフレームの更新.
	pAC->FrameUpdate();

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

//-----------------------------------------.
// 定数バッファの作成.
//-----------------------------------------.
HRESULT CFbxRenderer::CreateBuffer()
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

//-----------------------------------------.
// サンプラー作成.
//-----------------------------------------.
HRESULT CFbxRenderer::CreateSampler()
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

//-----------------------------------------.
// 定数バッファ作成.
//-----------------------------------------.
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

//-----------------------------------------.
// シェーダーの作成.
//-----------------------------------------.
HRESULT CFbxRenderer::CreateShader()
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