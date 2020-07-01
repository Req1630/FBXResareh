#include "CSprite.h"
#include "..\Direct11\D3DX11.h"

//ｼｪｰﾀﾞﾌｧｲﾙ名(ﾃﾞｨﾚｸﾄﾘも含む).
const wchar_t* SHADER_NAME = L"Data\\Shader\\Sprite.hlsl";

//ｺﾝｽﾄﾗｸﾀ.
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

//ﾃﾞｽﾄﾗｸﾀ.
CSprite::~CSprite()
{
	//解放処理を呼び出す.
	Release();

	//別のところで管理しているのでここではnullptrを入れて初期化.
	m_pContext11 = nullptr;
	m_pDevice11 = nullptr;
}

//初期化.
//	ID3D11Device* pDevice11 外部で作成して持ってくる。
//	ID3D11DeviceContext* pContext11 外部で作成して持ってくる。
HRESULT CSprite::Init()
{
	m_pDevice11 = CDirectX11::GetDevice();		//実態は別のところにある.他とも共有している.
	m_pContext11 = CDirectX11::GetContext();	//実態は別のところにある.他とも共有している.

	//ｼｪｰﾀﾞ作成.
	if( FAILED( CreateShader() ))
	{
		return E_FAIL;
	}
	//板ﾎﾟﾘｺﾞﾝ作成.
	if( FAILED( CreateModel() ))
	{
		return E_FAIL;
	}

	return S_OK;
}

//解放.
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
//	HLSLﾌｧｲﾙを読み込みｼｪｰﾀﾞを作成する.
//	HLSL: High Level Shading Language の略.
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

	//HLSLからﾊﾞｰﾃｯｸｽｼｪｰﾀﾞのﾌﾞﾛﾌﾞを作成.
	if (FAILED(
		D3DCompileFromFile(
			SHADER_NAME,	//ｼｪｰﾀﾞﾌｧｲﾙ名(HLSLﾌｧｲﾙ).
			nullptr, 
			nullptr, 
			"VS_Main",
			"vs_5_0", 
			uCompileFlag,
			0, 
			&pCompiledShader,
			nullptr )))
	{
		_ASSERT_EXPR( false, "hlsl読み込み失敗" );
		return E_FAIL;
	}
	SAFE_RELEASE(pErrors);

	//上記で作成したﾌﾞﾛﾌﾞから「ﾊﾞｰﾃｯｸｽｼｪｰﾀﾞ」を作成.
	if (FAILED(
		m_pDevice11->CreateVertexShader(
			pCompiledShader->GetBufferPointer(),
			pCompiledShader->GetBufferSize(),
			nullptr,
			&m_pVertexShader)))	//(out)ﾊﾞｰﾃｯｸｽｼｪｰﾀﾞ.
	{
		_ASSERT_EXPR(false, "ﾊﾞｰﾃｯｸｽｼｪｰﾀﾞ作成失敗");
		return E_FAIL;
	}

	//頂点ｲﾝﾌﾟｯﾄﾚｲｱｳﾄを定義.
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{
			"POSITION",						//位置.
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,	//DXGIのﾌｫｰﾏｯﾄ(32bit float型*3).
			0,
			0,								//ﾃﾞｰﾀの開始位置.
			D3D11_INPUT_PER_VERTEX_DATA, 0
		},
		{
			"TEXCOORD",						//ﾃｸｽﾁｬ位置.
			0,
			DXGI_FORMAT_R32G32_FLOAT,		//DXGIのﾌｫｰﾏｯﾄ(32bit float型*2).
			0,
			12,								//ﾃﾞｰﾀの開始位置.
			D3D11_INPUT_PER_VERTEX_DATA, 0
		}
	};
	//頂点ｲﾝﾌﾟｯﾄﾚｲｱｳﾄの配列要素数を算出.
	UINT numElements = sizeof(layout) / sizeof(layout[0]);

	//頂点ｲﾝﾌﾟｯﾄﾚｲｱｳﾄを作成.
	if (FAILED(
		m_pDevice11->CreateInputLayout(
			layout,
			numElements,
			pCompiledShader->GetBufferPointer(),
			pCompiledShader->GetBufferSize(),
			&m_pVertexLayout)))	//(out)頂点ｲﾝﾌﾟｯﾄﾚｲｱｳﾄ.
	{
		_ASSERT_EXPR( false, "頂点ｲﾝﾌﾟｯﾄﾚｲｱｳﾄ作成失敗" );
		return E_FAIL;
	}
	SAFE_RELEASE(pCompiledShader);

	//HLSLからﾋﾟｸｾﾙｼｪｰﾀﾞのﾌﾞﾛﾌﾞを作成.
	if (FAILED(
		D3DCompileFromFile(
			SHADER_NAME,	//ｼｪｰﾀﾞﾌｧｲﾙ名(HLSLﾌｧｲﾙ).
			nullptr, 
			nullptr, 
			"PS_Main",
			"ps_5_0", 
			uCompileFlag,
			0, 
			&pCompiledShader,
			nullptr )))
	{
		_ASSERT_EXPR( false, "hlsl読み込み失敗" );
		return E_FAIL;
	}
	SAFE_RELEASE(pErrors);

	//上記で作成したﾌﾞﾛﾌﾞから「ﾋﾟｸｾﾙｼｪｰﾀﾞ」を作成.
	if (FAILED(
		m_pDevice11->CreatePixelShader(
			pCompiledShader->GetBufferPointer(),
			pCompiledShader->GetBufferSize(),
			nullptr,
			&m_pPixelShader)))	//(out)ﾋﾟｸｾﾙｼｪｰﾀﾞ.
	{
		_ASSERT_EXPR( false, "ﾋﾟｸｾﾙｼｪｰﾀﾞ作成失敗" );
		return E_FAIL;
	}
	SAFE_RELEASE(pCompiledShader);

	//ｺﾝｽﾀﾝﾄ(定数)ﾊﾞｯﾌｧ作成.
	//ｼｪｰﾀﾞに特定の数値を送るﾊﾞｯﾌｧ.
	//ここでは変換行列渡し用.
	//ｼｪｰﾀﾞに World, View, Projection 行列を渡す.
	D3D11_BUFFER_DESC cb;
	cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;	//ｺﾝｽﾀﾝﾄﾊﾞｯﾌｧを指定.
	cb.ByteWidth = sizeof(SHADER_CONSTANT_BUFFER);//ｺﾝｽﾀﾝﾄﾊﾞｯﾌｧのｻｲｽﾞ.
	cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	//書き込みでｱｸｾｽ.
	cb.MiscFlags = 0;	//その他のﾌﾗｸﾞ(未使用).
	cb.StructureByteStride = 0;	//構造体のｻｲｽﾞ(未使用).
	cb.Usage = D3D11_USAGE_DYNAMIC;	//使用方法:直接書き込み.

	//ｺﾝｽﾀﾝﾄﾊﾞｯﾌｧの作成.
	if (FAILED(
		m_pDevice11->CreateBuffer(
			&cb,
			nullptr,
			&m_pConstantBuffer)))
	{
		_ASSERT_EXPR( false, "ｺﾝｽﾀﾝﾄﾊﾞｯﾌｧ作成失敗" );
		return E_FAIL;
	}


	return S_OK;
}


//ﾓﾃﾞﾙ作成.
HRESULT CSprite::CreateModel()
{
	//板ﾎﾟﾘ(四角形)の頂点を作成.
	VERTEX vertices[]=
	{
		//頂点座標(x,y,z)				 
		DirectX::XMFLOAT3(-1.0f,-1.0f, 0.0f ),
		DirectX::XMFLOAT3(-1.0f, 1.0f, 0.0f ),
		DirectX::XMFLOAT3( 1.0f,-1.0f, 0.0f ),
		DirectX::XMFLOAT3( 1.0f, 1.0f, 0.0f ),
	};
	//最大要素数を算出する.
	UINT uVerMax = sizeof(vertices) / sizeof(vertices[0]);

	//ﾊﾞｯﾌｧ構造体.
	D3D11_BUFFER_DESC bd;
	bd.Usage			= D3D11_USAGE_DEFAULT;	//使用方法(ﾃﾞﾌｫﾙﾄ).
	bd.ByteWidth		= sizeof(VERTEX) * uVerMax;//頂点のｻｲｽﾞ.
	bd.BindFlags		= D3D11_BIND_VERTEX_BUFFER;//頂点ﾊﾞｯﾌｧとして扱う.
	bd.CPUAccessFlags	= 0;	//CPUからはｱｸｾｽしない.
	bd.MiscFlags		= 0;	//その他のﾌﾗｸﾞ(未使用).
	bd.StructureByteStride = 0;	//構造体のｻｲｽﾞ(未使用).

	//ｻﾌﾞﾘｿｰｽﾃﾞｰﾀ構造体.
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = vertices;	//板ﾎﾟﾘの頂点をｾｯﾄ.

	//頂点ﾊﾞｯﾌｧの作成.
	if (FAILED(m_pDevice11->CreateBuffer(
		&bd, &InitData, &m_pVertexBuffer)))
	{
		_ASSERT_EXPR( false, "頂点ﾊﾞｯﾌｧ作成失敗" );
		return E_FAIL;
	}

	//頂点ﾊﾞｯﾌｧをｾｯﾄ.
	UINT stride = sizeof(VERTEX);	//ﾃﾞｰﾀ間隔.
	UINT offset = 0;
	m_pContext11->IASetVertexBuffers(
		0, 1,
		&m_pVertexBuffer, &stride, &offset);



	return S_OK;
}

//ﾚﾝﾀﾞﾘﾝｸﾞ用.
void CSprite::Render()
{
	// パラメータの計算
	DirectX::XMVECTOR eye_pos		= DirectX::XMVectorSet(0.0f, 70.0f, 500.0f, 1.0f);
	DirectX::XMVECTOR eye_lookat	= DirectX::XMVectorSet(0.0f, 70.0f, 0.0f, 1.0f);
	DirectX::XMVECTOR eye_up		= DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
	DirectX::XMMATRIX World	= DirectX::XMMatrixRotationY( 0.1f );
	DirectX::XMMATRIX View	= DirectX::XMMatrixLookAtLH( eye_pos, eye_lookat, eye_up );
	DirectX::XMMATRIX Proj	= DirectX::XMMatrixPerspectiveFovLH( DirectX::XM_PIDIV4, (FLOAT)WND_W/(FLOAT)WND_H, 0.1f, 800.0f );
	DirectX::XMMATRIX mTrans, mRot, mScale;

	//拡大縮小行列.
	mScale = 
		DirectX::XMMatrixScaling( m_vScale.x, m_vScale.y, m_vScale.z );
	//回転行列.
	DirectX::XMMATRIX mYaw, mPitch, mRoll;
	mYaw = DirectX::XMMatrixRotationY( m_vRotation.y );
	mPitch = DirectX::XMMatrixRotationY( m_vRotation.x );
	mRoll = DirectX::XMMatrixRotationY( m_vRotation.z );
	mRot = mYaw * mPitch * mRoll;

	//平行行列(平行移動).
	mTrans = DirectX::XMMatrixTranslation(
		m_vPosition.x, m_vPosition.y, m_vPosition.z );

	//ﾜｰﾙﾄﾞ座標変換.
	//重要: 拡縮行列 * 回転行列 * 平行行列.
//	World = mScale * mRot * mTrans;


	//使用するｼｪｰﾀﾞの登録.
	m_pContext11->VSSetShader( m_pVertexShader, nullptr, 0 );
	m_pContext11->PSSetShader( m_pPixelShader, nullptr, 0 );

	//ｼｪｰﾀﾞのｺﾝｽﾀﾝﾄﾊﾞｯﾌｧに各種ﾃﾞｰﾀを渡す.
	D3D11_MAPPED_SUBRESOURCE pData;
	SHADER_CONSTANT_BUFFER cb;	//ｺﾝｽﾀﾝﾄﾊﾞｯﾌｧ.
	//ﾊﾞｯﾌｧ内のﾃﾞｰﾀの書き換え開始時にmap.
	if (SUCCEEDED(
		m_pContext11->Map(m_pConstantBuffer,
			0, D3D11_MAP_WRITE_DISCARD, 0, &pData)))
	{
		//ﾜｰﾙﾄﾞ,ﾋﾞｭｰ,ﾌﾟﾛｼﾞｪｸｼｮﾝ行列を渡す.
		DirectX::XMMATRIX m = XMMatrixTranspose( World * View * Proj );

		m = XMMatrixTranspose( World * View * Proj );

		cb.mWVP = m;

		//ｶﾗｰ.
		cb.vColor = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

		memcpy_s(pData.pData, pData.RowPitch,
			(void*)(&cb), sizeof(cb));

		m_pContext11->Unmap(m_pConstantBuffer, 0);
	}

	//このｺﾝｽﾀﾝﾄﾊﾞｯﾌｧをどのｼｪｰﾀﾞで使うか？.
	m_pContext11->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	m_pContext11->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);

	//頂点ﾊﾞｯﾌｧをｾｯﾄ.
	UINT stride = sizeof(VERTEX);//ﾃﾞｰﾀの間隔.
	UINT offset = 0;
	m_pContext11->IASetVertexBuffers(0, 1,
		&m_pVertexBuffer, &stride, &offset);

	//頂点ｲﾝﾌﾟｯﾄﾚｲｱｳﾄをｾｯﾄ.
	m_pContext11->IASetInputLayout(m_pVertexLayout);
	//ﾌﾟﾘﾐﾃｨﾌﾞ・ﾄﾎﾟﾛｼﾞｰをｾｯﾄ.
	m_pContext11->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//ﾌﾟﾘﾐﾃｨﾌﾞをﾚﾝﾀﾞﾘﾝｸﾞ.
	m_pContext11->Draw(4, 0);//板ﾎﾟﾘ(頂点4つ分).

}
