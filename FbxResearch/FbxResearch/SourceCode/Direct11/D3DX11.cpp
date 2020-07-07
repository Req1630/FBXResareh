#include "D3DX11.h"

#include <array>

ID3D11Device* CDirectX11::m_pDevice11 = nullptr;
ID3D11DeviceContext* CDirectX11::m_pContext11 = nullptr;

CDirectX11::CDirectX11()
	: m_hWnd					( nullptr )
	, m_pSwapChain				( nullptr )
	, m_pBackBuffer_TexRTV		( nullptr )
	, m_pBackBuffer_DSTex		( nullptr )
	, m_pBackBuffer_DSTexDSV	( nullptr )
	, GBufferRTV				( g_bufferNum )
	, GBufferSRV				( g_bufferNum )
	, GBufferTex				( g_bufferNum )
	, m_pColorMapRTV			( nullptr )
	, m_pColorMapTex			( nullptr )
	, m_pColorSRV				( nullptr )
	, m_pNormalMapRTV			( nullptr )
	, m_pNormalMapTex			( nullptr )
	, m_pNormalSRV				( nullptr )
	, m_pPositionMapRTV			( nullptr )
	, m_pPositionMapTex			( nullptr )
	, m_pPositionSRV			( nullptr )
{
}

CDirectX11::~CDirectX11()
{
}

//-----------------------------------.
// DirectX11�\�z�֐�.
//-----------------------------------.
HRESULT CDirectX11::Create( HWND hWnd )
{
	m_hWnd = hWnd;

	if( FAILED(InitDevice11()) )	return E_FAIL;
	if( FAILED(InitTexRTV()) )		return E_FAIL;
	if( FAILED(InitDSTex()) )		return E_FAIL;
	if( FAILED(InitViewports()) )	return E_FAIL;
	if( FAILED(InitRasterizer()) )	return E_FAIL;

	return S_OK;
}

//-----------------------------------.
// DirectX11����֐�.
//-----------------------------------.
HRESULT CDirectX11::Release()
{
	SAFE_RELEASE(m_pPositionSRV);
	SAFE_RELEASE(m_pPositionMapTex);
	SAFE_RELEASE(m_pPositionMapRTV);
	SAFE_RELEASE(m_pColorMapRTV);
	SAFE_RELEASE(m_pColorMapTex);
	SAFE_RELEASE(m_pColorSRV);
	SAFE_RELEASE(m_pNormalMapRTV);
	SAFE_RELEASE(m_pNormalMapTex);
	SAFE_RELEASE(m_pNormalSRV);

	for( auto& rtv : GBufferRTV ) SAFE_RELEASE(rtv);
	for( auto& srv : GBufferSRV ) SAFE_RELEASE(srv);
	for( auto& tex : GBufferTex ) SAFE_RELEASE(tex);

	SAFE_RELEASE(m_pBackBuffer_DSTexDSV);
	SAFE_RELEASE(m_pBackBuffer_DSTex);
	SAFE_RELEASE(m_pBackBuffer_TexRTV);
	SAFE_RELEASE(m_pSwapChain);
	SAFE_RELEASE(m_pContext11);
	SAFE_RELEASE(m_pDevice11);

	return S_OK;
}

//-----------------------------------.
// �N���A�o�b�N�o�b�t�@.
//-----------------------------------.
void CDirectX11::ClearBackBuffer()
{
	// �J���[�o�b�N�o�b�t�@.
	m_pContext11->ClearRenderTargetView( m_pBackBuffer_TexRTV, CLEAR_BACK_COLOR1 );
	
	// ���ް���ޯ��ޭ������߽��ݼ��ޭ����߲��ײ݂ɾ��.
	m_pContext11->OMSetRenderTargets( 3, &GBufferRTV[0], m_pBackBuffer_DSTexDSV );

	for( auto& rtv : GBufferRTV ){
		m_pContext11->ClearRenderTargetView( rtv, CLEAR_BACK_COLOR2 );
	}

	// �f�v�X�X�e���V���o�b�t�@.
	m_pContext11->ClearDepthStencilView(
		m_pBackBuffer_DSTexDSV,
		D3D11_CLEAR_DEPTH,
		1.0f, 0 );
}

//-----------------------------------.
// �X���b�v�`�F�[���v���[���g.
//-----------------------------------.
void CDirectX11::SwapChainPresent()
{
	m_pSwapChain->Present( 0, 0 ); 
}

void CDirectX11::SetBackBuffer()
{
	m_pContext11->OMSetRenderTargets( 1, &m_pBackBuffer_TexRTV, m_pBackBuffer_DSTexDSV );
	// �f�v�X�X�e���V���o�b�t�@.
	m_pContext11->ClearDepthStencilView(
		m_pBackBuffer_DSTexDSV,
		D3D11_CLEAR_DEPTH,
		1.0f, 0 );
}

//-----------------------------------.
// �f�o�C�X11�̍쐬.
//-----------------------------------.
HRESULT CDirectX11::InitDevice11()
{
	// �X���b�v�`�F�[���\����.
	DXGI_SWAP_CHAIN_DESC sd = {0};
	sd.BufferCount			= 1;								// �o�b�N�o�b�t�@�̐�.
	sd.BufferDesc.Width		= WND_W;							// �o�b�N�o�b�t�@�̕�.
	sd.BufferDesc.Height	= WND_H;							// �o�b�N�o�b�t�@�̍���.
	sd.BufferDesc.Format	= DXGI_FORMAT_R8G8B8A8_UNORM;		// �t�H�[�}�b�g(32�ޯĶװ).
	sd.BufferDesc.RefreshRate.Numerator		= 60;				// ���t���b�V�����[�g(����) ��FPS:60.
	sd.BufferDesc.RefreshRate.Denominator	= 1;				// ���t���b�V�����[�g(���q).
	sd.BufferUsage			= DXGI_USAGE_RENDER_TARGET_OUTPUT;	// �g����(�\����).
	sd.OutputWindow			= m_hWnd;							// �E�B���h�E�n���h��.
	sd.SampleDesc.Count		= 1;								// �}���`�T���v���̐�.
	sd.SampleDesc.Quality	= 0;								// �}���`�T���v���̃N�I���e�B.
	sd.Windowed				= TRUE;								// �E�B���h�E���[�h(�t���X�N���[������FALSE).

	//�쐬�����݂�@�\���x���̗D����w��.
	// (GPU���T�|�[�g����@�\��Ă̒�`).
	// D3D_FEATURE_LEVEL�񋓌^�̔z��.
	// D3D_FEATURE_LEVEL_11_0:Direct3D 11.0 �� GPU���x��.
	D3D_FEATURE_LEVEL pFeatureLevels = D3D_FEATURE_LEVEL_11_0;
	D3D_FEATURE_LEVEL* pFeatureLevel = nullptr;	// �z��̗v�f��.

	auto createDevice = [&]( const D3D_DRIVER_TYPE& type ) -> HRESULT
	{
		return D3D11CreateDeviceAndSwapChain(
			nullptr,			// �r�f�I�A�_�v�^�ւ̃|�C���^.
			type,				// �쐬����Ãf�o�C�X�̎��.
			nullptr,			// �\�t�g�E�F�A ���X�^���C�U����������DLL�̃n���h��.
			0,					// �L���ɂ��郉���^�C�����C���[.
			&pFeatureLevels,	// �쐬�����݂�@�\���x���̏������w�肷��z��ւ̃|�C���^.
			1,					// ���̗v�f��.
			D3D11_SDK_VERSION,	// SDK�̃o�[�W����.
			&sd,				// �X���b�v�`�F�[���̏������p�����[�^�̃|�C���^.
			&m_pSwapChain,		// (out) �����_�����O�Ɏg�p����X���b�v�`�F�[��.
			&m_pDevice11,		// (out) �쐬���ꂽ�f�o�C�X.
			pFeatureLevel,		// �@�\���ق̔z��ɂ���ŏ��̗v�f��\���|�C���^.
			&m_pContext11 );	// (out) �f�o�C�X�R���e�L�X�g.
	};

	if( FAILED(createDevice(D3D_DRIVER_TYPE_HARDWARE)) ){
		if( FAILED(createDevice(D3D_DRIVER_TYPE_WARP)) ){
			if( FAILED(createDevice(D3D_DRIVER_TYPE_REFERENCE)) ){
				MessageBox( nullptr,
					"�f�o�C�X�ƃX���b�v�`�F�[���쐬���s",
					"Error", MB_OK );
				return E_FAIL;
			}
		}
	}
	pFeatureLevel = nullptr;

	return S_OK;
}

//-----------------------------------.
// �����_�[�^�[�Q�b�g�r���[�̍쐬.
//-----------------------------------.
HRESULT CDirectX11::InitTexRTV()
{
	ID3D11Texture2D* pBackBuffer_Tex = nullptr;
	m_pSwapChain->GetBuffer(
		0,
		__uuidof(ID3D11Texture2D),	//__uuidof:���Ɋ֘A�t�����ꂽGUID���擾.
		(LPVOID*)&pBackBuffer_Tex);	//(out)�ޯ��ޯ̧ø���.

	//����ø����ɑ΂������ް���ޯ��ޭ�(RTV)���쐬.
	m_pDevice11->CreateRenderTargetView(
		pBackBuffer_Tex,
		nullptr,
		&m_pBackBuffer_TexRTV);	//(out)RTV.

	//�ޯ��ޯ̧ø��������.
	SAFE_RELEASE(pBackBuffer_Tex);

	return S_OK;
}

//-----------------------------------.
// �X�e���V���r���[�̍쐬.
//-----------------------------------.
HRESULT CDirectX11::InitDSTex()
{
	int wnd_Width = WND_W;
	int wnd_Height = WND_H;
	D3D11_TEXTURE2D_DESC descDepth;
	descDepth.Width					= wnd_Width;				// ��.
	descDepth.Height				= wnd_Height;				// ����.
	descDepth.MipLevels				= 1;						// Я��ϯ������:1.
	descDepth.ArraySize				= 1;						// �z��:1.
	descDepth.Format				= DXGI_FORMAT_D32_FLOAT;	// 32�ޯ�̫�ϯ�.
	descDepth.SampleDesc.Count		= 1;						// �������ق̐�.
	descDepth.SampleDesc.Quality	= 0;						// �������ق̸��è.
	descDepth.Usage					= D3D11_USAGE_DEFAULT;		// �g�p���@:��̫��.
	descDepth.BindFlags				= D3D11_BIND_DEPTH_STENCIL;	// �[�x(��ݼقƂ��Ďg�p).
	descDepth.CPUAccessFlags		= 0;						// CPU����ͱ������Ȃ�.
	descDepth.MiscFlags				= 0;						// ���̑��̐ݒ�Ȃ�.


	// ����ø����ɑ΂������߽��ݼ�(DSTex)���쐬.
	if( FAILED( m_pDevice11->CreateTexture2D( &descDepth, nullptr, &m_pBackBuffer_DSTex )) ){
		_ASSERT_EXPR( false, L"�f�v�X�X�e���V���쐬���s" );
		return E_FAIL;
	}
	// ����ø����ɑ΂������߽��ݼ��ޭ�(DSV)���쐬.
	if( FAILED( m_pDevice11->CreateDepthStencilView( m_pBackBuffer_DSTex, nullptr, &m_pBackBuffer_DSTexDSV)) ){
		_ASSERT_EXPR( false, L"�f�v�X�X�e���V���r���[�쐬���s" );
		return E_FAIL;
	}

	D3D11_TEXTURE2D_DESC texDepth;
	texDepth.Width				= wnd_Width;				// ��.
	texDepth.Height				= wnd_Height;				// ����.
	texDepth.MipLevels			= 1;						// Я��ϯ������:1.
	texDepth.ArraySize			= 1;						// �z��:1.
	texDepth.Format				= DXGI_FORMAT_R11G11B10_FLOAT;	// 32�ޯ�̫�ϯ�.
	texDepth.SampleDesc.Count	= 1;						// �������ق̐�.
	texDepth.SampleDesc.Quality	= 0;						// �������ق̸��è.
	texDepth.Usage				= D3D11_USAGE_DEFAULT;		// �g�p���@:��̫��.
	texDepth.BindFlags			= D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDepth.CPUAccessFlags		= 0;						// CPU����ͱ������Ȃ�.
	texDepth.MiscFlags			= 0;						// ���̑��̐ݒ�Ȃ�.
	

	for( int i = 0; i < g_bufferNum; i++ ){
		// ����ø����ɑ΂������߽��ݼ�(DSTex)���쐬.
		if( FAILED( m_pDevice11->CreateTexture2D( &texDepth, nullptr, &GBufferTex[i] )) ){
			_ASSERT_EXPR( false, L"�e�N�X�`���f�X�N�쐬���s" );
			return E_FAIL;
		}
		// �����_�[�^�[�Q�b�g�r���[�̐ݒ�
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
		memset( &rtvDesc, 0, sizeof( rtvDesc ) );
		rtvDesc.Format             = DXGI_FORMAT_R11G11B10_FLOAT;
		rtvDesc.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE2D;

		// RenderTargetView�쐬.
		if( FAILED( m_pDevice11->CreateRenderTargetView( GBufferTex[i], &rtvDesc, &GBufferRTV[i] ) )){
			_ASSERT_EXPR( false, L"RenderTargetView�쐬���s" );
			return E_FAIL;
		}

		// �V�F�[�_���\�[�X�r���[�̐ݒ�
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		memset( &srvDesc, 0, sizeof( srvDesc ) );
		srvDesc.Format              = rtvDesc.Format;
		srvDesc.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		// �e�N�X�`���쐬���Ɠ����t�H�[�}�b�g
		if( FAILED( m_pDevice11->CreateShaderResourceView( GBufferTex[i], &srvDesc, &GBufferSRV[i] ) )){
			_ASSERT_EXPR( false, L"�f�v�X�X�e���V���쐬���s" );
			return E_FAIL;
		}
	}

	return S_OK;
}

//-----------------------------------.
// �r���[�|�[�g�̍쐬.
//-----------------------------------.
HRESULT CDirectX11::InitViewports()
{
	D3D11_VIEWPORT vp;
	vp.Width	= (FLOAT)WND_W;	// ��.
	vp.Height	= (FLOAT)WND_H;	// ����.
	vp.MinDepth = 0.0f;			// �ŏ��[�x(��O).
	vp.MaxDepth = 1.0f;			// �ő�[�x(��).
	vp.TopLeftX = 0.0f;			// ����ʒux.
	vp.TopLeftY = 0.0f;			// ����ʒuy.

	m_pContext11->RSSetViewports( 1, &vp );
	return S_OK;
}

//-----------------------------------.
// ���X�^���C�U�̍쐬.
//-----------------------------------.
HRESULT CDirectX11::InitRasterizer()
{
	D3D11_RASTERIZER_DESC rdc;
	ZeroMemory(&rdc, sizeof(rdc));
	rdc.FillMode				= D3D11_FILL_SOLID;	// �h��Ԃ�(�د��).
	rdc.CullMode				= D3D11_CULL_NONE;	// BACK:�w�ʂ�`�悵�Ȃ�, FRONT:���ʂ�`�悵�Ȃ�.
	rdc.FrontCounterClockwise	= TRUE;				// ��غ�݂̕\�������肷���׸�.
	rdc.DepthClipEnable			= FALSE;			// �����ɂ��Ă̸د��ݸޗL��.

	ID3D11RasterizerState* pRs = nullptr;
	if( FAILED( m_pDevice11->CreateRasterizerState( &rdc, &pRs )) ){
		_ASSERT_EXPR( false, L"���X�^���C�U�[�쐬���s" );
	}
	m_pContext11->RSSetState( pRs );
	SAFE_RELEASE( pRs );

	return S_OK;
}