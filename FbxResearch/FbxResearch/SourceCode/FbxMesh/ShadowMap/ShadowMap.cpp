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
//				作成.
//-----------------------------------------.
HRESULT CShadowMap::Create( ID3D11DeviceContext* pContext11 )
{
	if( pContext11 == nullptr ) return E_FAIL;
	// コンテキストの取得.
	m_pContext11 = pContext11;
	// デバイスの取得.
	m_pContext11->GetDevice( &m_pDevice11 );
	if( m_pDevice11 == nullptr ) return E_FAIL;

	// バッファーの作成.
	if( FAILED( CreateBuffer() )) return E_FAIL;
	// シェーダーの作成.
	if( FAILED( CreateShader() )) return E_FAIL;

	return S_OK;
}

//-----------------------------------------.
//				破壊.
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
//				描画.
//-----------------------------------------.
void CShadowMap::Render(
	CFbxModel& mdoel,
	CLight& light )
{
	// 使用するシェーダーの設定.
	m_pContext11->VSSetShader( m_pVertexShader, nullptr, 0 );
	m_pContext11->PSSetShader( m_pPixelShader, nullptr, 0 );
	// プリティブトポロジーをセット.
	m_pContext11->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	// ワールド行列取得.
	DirectX::XMMATRIX mWorld = mdoel.GetWorldMatrix();

	UINT stride = sizeof(VERTEX);
	UINT offset = 0;
	// メッシュデータ分描画.
	for( auto& m : mdoel.GetMeshData() ){
		// 頂点バッファをセット.
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
			// ライトの wvp　を転置して渡す.
			cb.mLightWVP = DirectX::XMMatrixTranspose( mWorld * light.GetVP() );

			memcpy_s( pData.pData, pData.RowPitch, (void*)(&cb), sizeof(cb) );
			m_pContext11->Unmap( m_pCBufferPerMesh, 0 );
		}
		// 上で設定したコンスタントバッファをどのシェーダーで使うか.
		m_pContext11->VSSetConstantBuffers( 0, 1, &m_pCBufferPerMesh );
		m_pContext11->PSSetConstantBuffers( 0, 1, &m_pCBufferPerMesh );

		// ポリゴンをレンダリング.
		m_pContext11->DrawIndexed( m.PolygonVertexCount, 0, 0 );
	}
}

// 定数バッファの作成.
HRESULT CShadowMap::CreateBuffer()
{
	//--------------------------------------.
	// 定数バッファの設定
	//--------------------------------------.
	if( FAILED( CreateConstantBuffer( 
		sizeof(CBUFFER_PER_MESH), &m_pCBufferPerMesh ) )){
		_ASSERT_EXPR( false, TEXT("メッシュ毎の定数バッファ作成失敗") );
		MessageBox( nullptr, TEXT("メッシュ毎の定数バッファ作成失敗"), TEXT("Warning"), MB_OK );
		return E_FAIL;
	}
	return S_OK;
}

// 定数バッファ作成.
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

// シェーダーの作成.
HRESULT CShadowMap::CreateShader()
{
	ID3DBlob* pCompileVS = nullptr;
	ID3DBlob* pCompilePS = nullptr;
	ID3DBlob* pErrerBlog = nullptr;
	UINT uCompileFlag = 0;

#ifdef _DEBUG
	// シェーダーでブレイクを貼るための処理.
	uCompileFlag = D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
#endif	// #ifdef _DEBUG.

	// シェーダー読み込み関数.
	auto shaderCompile = [&]( 
		const wchar_t* fileName, 
		const char* entryPosint, 
		const char* version, 
		ID3DBlob** ppOutBlog )
	{
		if( FAILED(
			D3DCompileFromFile( 
				fileName,							// シェーダー名.
				nullptr,							// マクロ定義(シェーダー側でマクロ定義している場合に使用).
				D3D_COMPILE_STANDARD_FILE_INCLUDE,	// インクルード定義(シェーダー側でインクルード定義している場合に使用).
				entryPosint,						// エントリーポイント.
				version,							// シェーダーのバーション.
				uCompileFlag,						// フラグ.
				0,									// フラグ.
				ppOutBlog,							// (out)シェーダーの情報.
				&pErrerBlog ))){					// (out)エラー情報など.
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
	// 頂点シェーダー.
	//-----------------------------------------.
	// 頂点シェーダーの読み込み.
	if( FAILED( shaderCompile( SHADOW_SHADER_NAME, "VS_Main", "vs_5_0", &pCompileVS ) )) return E_FAIL;
	// 頂点シェーダーの作成.
	if( FAILED(
		m_pDevice11->CreateVertexShader(
			pCompileVS->GetBufferPointer(),	// 読み込んだシェーダーのポインタ.
			pCompileVS->GetBufferSize(),	// 頂点シェーダーのサイズ.
			nullptr,						// "動的シェーダーリンク"を使用しないのでnull.
			&m_pVertexShader ))){			// (out)頂点シェーダー.
		_ASSERT_EXPR( false, TEXT("頂点シェーダー作成失敗") );
		MessageBox( nullptr, TEXT("頂点シェーダー作成失敗"), TEXT("Warning"), MB_OK );
		return E_FAIL;
	}
	//-----------------------------------------.
	// ピクセルシェーダー.
	//-----------------------------------------.
	// ピクセルシェーダーの読み込み.
	if( FAILED( shaderCompile( SHADOW_SHADER_NAME, "PS_Main", "ps_5_0", &pCompilePS ) )) return E_FAIL;
	// ピクセルシェーダーの作成.
	if( FAILED(
		m_pDevice11->CreatePixelShader(
			pCompilePS->GetBufferPointer(),	// 読み込んだシェーダーのポインタ.
			pCompilePS->GetBufferSize(),	// ピクセルシェーダーのサイズ.
			nullptr,						// "動的シェーダーリンク"を使用しないのでnull.
			&m_pPixelShader ))){			// (out)ピクセルシェーダー.
		_ASSERT_EXPR( false, TEXT("ピクセルシェーダー作成失敗") );
		MessageBox( nullptr, TEXT("ピクセルシェーダー作成失敗"), TEXT("Warning"), MB_OK );
		return E_FAIL;
	}
	//-----------------------------------------.
	//	頂点インプット.
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
	int numElements = sizeof(layout)/sizeof(layout[0]);	//要素数算出.
	if (FAILED(
		m_pDevice11->CreateInputLayout(
			layout,
			numElements,
			pCompileVS->GetBufferPointer(),
			pCompileVS->GetBufferSize(),
			&m_pVertexLayout ))){
		_ASSERT_EXPR( false, TEXT("頂点レイアウト作成失敗") );
		MessageBox( nullptr, TEXT("頂点レイアウト作成失敗"), TEXT("Warning"), MB_OK );
		return E_FAIL;
	}

	SAFE_RELEASE( pCompileVS );
	SAFE_RELEASE( pCompilePS );

	return S_OK;
}