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
//	作成.
////////////////////////////////////////////////.
HRESULT CFbxRenderer::Create( ID3D11DeviceContext* pContext11 )
{
	if( pContext11 == nullptr ) return E_FAIL;
	// コンテキストの取得.
	m_pContext11 = pContext11;
	// デバイスの取得.
	m_pContext11->GetDevice( &m_pDevice11 );
	if( m_pDevice11 == nullptr ) return E_FAIL;

	// バッファーの作成.
	if( FAILED( CreateBuffer() )) return E_FAIL;
	// サンプラーの作成.
	if( FAILED( CreateSampler() )) return E_FAIL;
	// シェーダーの作成.
	if( FAILED( CreateShader() )) return E_FAIL;

	return S_OK;
}

////////////////////////////////////////////////.
//	破壊.
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
//	描画.
////////////////////////////////////////////////.
void CFbxRenderer::Render(
	CFbxModel& mdoel,
	CCamera& camera,
	CLight& light,
	CFbxAnimationController* pAc )
{
	// 使用するシェーダーの設定.
	m_pContext11->VSSetShader( m_pVertexShader, nullptr, 0 );
	m_pContext11->PSSetShader( m_pPixelShader, nullptr, 0 );
	// プリティブトポロジーをセット.
	m_pContext11->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	// ワールド行列取得.
	DirectX::XMMATRIX mWorld = mdoel.GetWorldMatrix();
	// ワールド、ビュー、プロジェクション行列.
	DirectX::XMMATRIX mWVP = mWorld * camera.GetViewMatrix() * camera.GetProjMatrix();

	UINT stride = sizeof(VERTEX);
	UINT offset = 0;

	// アニメーションフレームの更新.
	AnimationFrameUpdate( mdoel, pAc );

	int meshNo = 0;
	// メッシュデータ分描画.
	for( auto& m : mdoel.GetMeshData() ){
		// アニメーションの行列計算.
		AnimMatrixCalculation( mdoel, meshNo, m, pAc );
		meshNo++;

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
			// ワールド行列を転置して渡す.
			cb.mW	= DirectX::XMMatrixTranspose( mWorld );
			// world, View, Proj を転置して渡す.
			cb.mWVP	= DirectX::XMMatrixTranspose( mWVP );
			// ライトの wvp　を転置して渡す.
			cb.mLightWVP = DirectX::XMMatrixTranspose( mWorld*light.GetVP() );
			// カメラの座標を渡す.
			cb.CameraPos = { camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z, 0.0f };
			// ライトの方向を渡す.
			cb.LightDir = light.GetDirection();

			memcpy_s( pData.pData, pData.RowPitch, (void*)(&cb), sizeof(cb) );
			m_pContext11->Unmap( m_pCBufferPerMesh, 0 );
		}
		// 上で設定したコンスタントバッファをどのシェーダーで使うか.
		m_pContext11->VSSetConstantBuffers( 0, 1, &m_pCBufferPerMesh );
		m_pContext11->PSSetConstantBuffers( 0, 1, &m_pCBufferPerMesh );
		
		// マテリアル分.
		if( SUCCEEDED ( m_pContext11->Map( 
			m_pCBufferPerMaterial, 0, 
			D3D11_MAP_WRITE_DISCARD, 0, 
			&pData )))
		{
			CBUFFER_PER_MATERIAL cb;
			cb.Ambient	= m.Material.Ambient;	// アンビエント.
			cb.Diffuse	= m.Material.Diffuse;	// ディフューズ.
			cb.Specular = m.Material.Specular;	// スペキュラ.

			memcpy_s( pData.pData, pData.RowPitch, (void*)(&cb), sizeof(cb) );
			m_pContext11->Unmap( m_pCBufferPerMaterial, 0 );
		}
		// 上で設定したコンスタントバッファをどのシェーダーで使うか.
		m_pContext11->VSSetConstantBuffers( 1, 1, &m_pCBufferPerMaterial );
		m_pContext11->PSSetConstantBuffers( 1, 1, &m_pCBufferPerMaterial );
		// サンプラをセット.
		m_pContext11->PSSetSamplers( 0, 1, &m_pSampleLinear );

		// テクスチャの確認.
		if( mdoel.GetTextures().count(m.Material.Name) > 0 ){
			// テクスチャがあれば.
			// テクスチャをシェーダーに渡す.
			m_pContext11->PSSetShaderResources( 0, 1, &mdoel.GetTextures().at(m.Material.Name) );
		} else {
			// テクスチャ無し.
			ID3D11ShaderResourceView* notex = { nullptr };
			m_pContext11->PSSetShaderResources( 0, 1, &notex );
		}

		// ポリゴンをレンダリング.
		m_pContext11->DrawIndexed( m.PolygonVertexCount, 0, 0 );
	}
}

////////////////////////////////////////////////.
// アニメーションフレームの更新.
////////////////////////////////////////////////.
void CFbxRenderer::AnimationFrameUpdate( CFbxModel& mdoel, CFbxAnimationController* pAc )
{
	// アニメーションフレームの更新.
	if( pAc == nullptr ){
		// モデルデータのアニメーションコントローラーを使用.
		if( mdoel.GetPtrAC() != nullptr ){
			// アニメーション用の頂点シェーダーの設定.
			m_pContext11->VSSetShader( m_pVertexAnimShader, nullptr, 0 );
			mdoel.GetPtrAC()->FrameUpdate();
		}
	} else {
		// デフォルト引数のアニメーションコントローラーを使用.
		if( pAc != nullptr ){
			// アニメーション用の頂点シェーダーの設定.
			m_pContext11->VSSetShader( m_pVertexAnimShader, nullptr, 0 );
			pAc->FrameUpdate();
		}
	}
}

////////////////////////////////////////////////.
// アニメーション用の行列計算.
////////////////////////////////////////////////.
void CFbxRenderer::AnimMatrixCalculation(
	CFbxModel& mdoel,
	const int& meahNo,
	FBXMeshData& meshData,
	CFbxAnimationController* pAc  )
{
	CBUFFER_PER_BONE cb;
	if( pAc == nullptr ){
		// モデルデータのアニメーションコントローラーを使用.
		if( GetBoneConstBuffer( meshData.Skin, meahNo, mdoel.GetPtrAC(), &cb ) == false ) return;
	} else {
		// デフォルト引数のアニメーションコントローラーを使用.
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
	// 上で設定したコンスタントバッファをどのシェーダーで使うか.
	m_pContext11->VSSetConstantBuffers( 2, 1, &m_pCBufferPerBone );
	m_pContext11->PSSetConstantBuffers( 2, 1, &m_pCBufferPerBone );

}

////////////////////////////////////////////////.
// ボーンの定数バッファの取得.
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
			_ASSERT_EXPR( false, "モデルとアニメーションのボーンの数が合いません" );
			MessageBox( nullptr, "モデルとアニメーションのボーンの数が合いません", "Warning", MB_OK );
		}
		vertexTransformMatrix = frameMatrix * b;
		pOutCB->Bone[boneIndex] = FbxMatrixConvertDXMMatrix( vertexTransformMatrix );
		boneIndex++;
	}

	return true;
}

////////////////////////////////////////////////.
// 定数バッファの作成.
////////////////////////////////////////////////.
HRESULT CFbxRenderer::CreateBuffer()
{
	//--------------------------------------.
	// 定数バッファの設定
	//--------------------------------------.
	if( FAILED( CreateConstantBuffer( 
		sizeof(CBUFFER_PER_MESH), &m_pCBufferPerMesh ) )){
		_ASSERT_EXPR( false, "メッシュ毎の定数バッファ作成失敗" );
		MessageBox( nullptr, "メッシュ毎の定数バッファ作成失敗", "Warning", MB_OK );
		return E_FAIL;
	}
	if( FAILED( CreateConstantBuffer(
		sizeof(CBUFFER_PER_MATERIAL), &m_pCBufferPerMaterial ) )){
		_ASSERT_EXPR( false, "マテリアル毎の定数バッファ作成失敗" );
		MessageBox( nullptr, "マテリアル毎の定数バッファ作成失敗", "Warning", MB_OK );
		return E_FAIL;
	}
	if( FAILED( CreateConstantBuffer(
		sizeof(CBUFFER_PER_BONE), &m_pCBufferPerBone ) )){
		_ASSERT_EXPR( false, "ボーン毎の定数バッファ作成失敗" );
		MessageBox( nullptr, "ボーン毎の定数バッファ作成失敗", "Warning", MB_OK );
		return E_FAIL;
	}
	return S_OK;
}

////////////////////////////////////////////////.
// サンプラー作成.
////////////////////////////////////////////////.
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
		_ASSERT_EXPR( false, "サンプラー作成失敗" );
		MessageBox( nullptr, "サンプラー作成失敗", "Warning", MB_OK );
		return E_FAIL;
	}
	return S_OK;
}

////////////////////////////////////////////////.
// 定数バッファ作成.
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
// シェーダーの作成.
////////////////////////////////////////////////.
HRESULT CFbxRenderer::CreateShader()
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
			MessageBox( nullptr, msg.c_str(), "Warning", MB_OK );
			return E_FAIL;
		}
		return S_OK;
	};
	//-----------------------------------------.
	// 頂点シェーダー.
	//-----------------------------------------.
	// 頂点シェーダーの読み込み.
	if( FAILED( shaderCompile( VS_SHADER_NAME, "VS_Main", "vs_5_0", &pCompileVS ) )) return E_FAIL;
	// 頂点シェーダーの作成.
	if( FAILED(
		m_pDevice11->CreateVertexShader(
			pCompileVS->GetBufferPointer(),	// 読み込んだシェーダーのポインタ.
			pCompileVS->GetBufferSize(),	// 頂点シェーダーのサイズ.
			nullptr,						// "動的シェーダーリンク"を使用しないのでnull.
			&m_pVertexShader ))){			// (out)頂点シェーダー.
		_ASSERT_EXPR( false, "頂点シェーダー作成失敗" );
		MessageBox( nullptr, "頂点シェーダー作成失敗", "Warning", MB_OK );
		return E_FAIL;
	}
	// 頂点シェーダー(アニメーション)の読み込み.
	if( FAILED( shaderCompile( VS_ANIM_SHADER_NAME, "VS_Main", "vs_5_0", &pCompileVS ) )) return E_FAIL;
	// 頂点シェーダー(アニメーション)の作成.
	if( FAILED(
		m_pDevice11->CreateVertexShader(
			pCompileVS->GetBufferPointer(),	// 読み込んだシェーダーのポインタ.
			pCompileVS->GetBufferSize(),	// 頂点シェーダーのサイズ.
			nullptr,						// "動的シェーダーリンク"を使用しないのでnull.
			&m_pVertexAnimShader ))){		// (out)頂点シェーダー.
		_ASSERT_EXPR( false, "頂点シェーダー作成失敗" );
		MessageBox( nullptr, "頂点シェーダー作成失敗", "Warning", MB_OK );
		return E_FAIL;
	}
	//-----------------------------------------.
	// ピクセルシェーダー.
	//-----------------------------------------.
	// ピクセルシェーダーの読み込み.
	if( FAILED( shaderCompile( PS_SHADER_NAME, "PS_Main", "ps_5_0", &pCompilePS ) )) return E_FAIL;
	// ピクセルシェーダーの作成.
	if( FAILED(
		m_pDevice11->CreatePixelShader(
			pCompilePS->GetBufferPointer(),	// 読み込んだシェーダーのポインタ.
			pCompilePS->GetBufferSize(),	// ピクセルシェーダーのサイズ.
			nullptr,						// "動的シェーダーリンク"を使用しないのでnull.
			&m_pPixelShader ))){			// (out)ピクセルシェーダー.
		_ASSERT_EXPR( false, "ピクセルシェーダー作成失敗" );
		MessageBox( nullptr, "ピクセルシェーダー作成失敗", "Warning", MB_OK );
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
		_ASSERT_EXPR( false, "頂点レイアウト作成失敗" );
		MessageBox( nullptr, "頂点レイアウト作成失敗", "Warning", MB_OK );
		return E_FAIL;
	}

	SAFE_RELEASE( pCompileVS );
	SAFE_RELEASE( pCompilePS );

	return S_OK;
}