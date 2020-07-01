#include "Main.h"

#include <fstream>
#include <sstream>

#include "..\Direct11\D3DX11.h"
#include "..\Global.h"
#include "..\Utility\FileManager\FileManager.h"
#include "..\Utility\FrameRate\FrameRate.h"
#include "..\FbxMesh\FbxAnimationData.h"
#include "..\FbxMesh\FbxAnimation\FbxAnimationLoader.h"

#include "..\Camera\Camera.h"

#include "..\ImGui\ImGuiManager\ImGuiManager.h"

LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );

CMain::CMain()
	: m_hWnd			( nullptr )
	, m_pDirectX11		( nullptr )
	, m_pFPS			( nullptr )
	, m_pFbxMesh		( nullptr )
	, m_pFbxGround		( nullptr )
	, m_pCamera			( nullptr )
{
	m_pDirectX11 = std::make_unique<CDirectX11>();
	m_pFPS = std::make_unique<CFrameRate>( FPS );
	m_pCamera = std::make_unique<CCamera>();
	m_pFbxMesh = std::make_unique<CFbxMesh>();
	m_pFbxGround = std::make_unique<CFbxMesh>();
}

CMain::~CMain()
{
}

//====================================.
//	初期化関数.
//====================================.
HRESULT CMain::Init()
{
	// DirectX11の構築.
	if( FAILED( m_pDirectX11->Create( m_hWnd ) )) return E_FAIL;
	// ImGuiの初期化.
	if( FAILED( CImGuiManager::Init( m_hWnd, 
		m_pDirectX11->GetDevice(), 
		m_pDirectX11->GetContext() ))) return E_FAIL;

	return S_OK;
}

//====================================.
//	解放関数.
//====================================.
void CMain::Release()
{
	m_pFbxGround->Destroy();
	m_pFbxMesh->Destroy();
	CImGuiManager::Release();
	m_pDirectX11->Release();
}

//====================================.
//	読み込み関数.
//====================================.
HRESULT CMain::Load()
{
	const char* fileName[] =
	{
		"Data\\Model\\unitychan\\unitychan.fbx",

		"Data\\Model\\humanoid.fbx",
		"Data\\Model\\grid\\grid1.fbx",
		"Data\\Model\\sayaka_fbx\\sayaka_fbx.fbx",
		"Data\\Model\\Stage\\stage.fbx",
		"Data\\Model\\Teddy_Attack1.fbx",
		"Data\\Model\\Box_Attack.fbx",
		"Data\\Model\\JumpingDown\\JumpingDown.fbx",
		"Data\\Model\\UE_Animation\\Walk_02_Cheerful_Loop_IP.FBX",
		"Data\\Model\\UE_Animation\\Walk_04_Texting_Loop_IP.FBX",
	};
	m_pFbxMesh->Create( m_pDirectX11->GetContext(), fileName[0] );
	m_pAc = m_pFbxMesh->GetAnimationController();

	m_pFbxGround->Create( m_pDirectX11->GetContext(), fileName[2] );


	CFbxAnimationLoader* pAnimData = new CFbxAnimationLoader;
	pAnimData->Create( fileName[8] );
//	m_pAc.AddAnimationData( pAnimData->GetAnimDateList() );
	pAnimData->Destroy();
	delete pAnimData;

	return S_OK;
}

//====================================.
//	更新処理.
//====================================.
void CMain::Update()
{
	// 画面のクリア.
	m_pDirectX11->ClearBackBuffer();

	// ImGuiのフレーム初期化.
	CImGuiManager::SetingNewFrame();

	static DirectX::XMFLOAT3 objectPos = { 0.0f, 0.0f, 0.0f };
	static DirectX::XMFLOAT3 objectRot = { -3.1415f/2.0f, 0.0f, 0.0f };
	static DirectX::XMFLOAT3 objectScale = { 10.05f, 10.05f, 10.05f };
	// カメラ制御.
	{
		static DirectX::XMFLOAT3 cameraPos = { 0.0f, 4.0f, 17.0f };
		if( GetAsyncKeyState('C') & 0x8000 ){
			if( GetAsyncKeyState(VK_NUMPAD8) & 0x8000 ) cameraPos.y += 0.1f;
			if( GetAsyncKeyState(VK_NUMPAD2) & 0x8000 ) cameraPos.y -= 0.1f;
			if( GetAsyncKeyState(VK_NUMPAD4) & 0x8000 ) cameraPos.x += 0.1f;
			if( GetAsyncKeyState(VK_NUMPAD6) & 0x8000 ) cameraPos.x -= 0.1f;
			if( GetAsyncKeyState(VK_NUMPAD7) & 0x8000 ) cameraPos.z += 0.1f;
			if( GetAsyncKeyState(VK_NUMPAD9) & 0x8000 ) cameraPos.z -= 0.1f;
		}
		m_pCamera->SetPosition( cameraPos );
		m_pCamera->SetLookPosition( { 0.0f, 0.0f, 0.0f } );
		m_pCamera->InitViewProj();
	}

	// 地面の表示.
	{
		m_pFbxGround->SetPosition( {0.0f, -1.0f, 0.0f} );
		m_pFbxGround->SetScale( 100.0f );
		m_pFbxGround->Render(
			m_pCamera->GetViewMatrix(), 
			m_pCamera->GetProjMatrix() );
	}

	// オブジェクト操作.
	{
		if( GetAsyncKeyState('P') & 0x8000 ){
			if( GetAsyncKeyState(VK_NUMPAD8) & 0x8000 ) objectPos.y += 0.1f;
			if( GetAsyncKeyState(VK_NUMPAD2) & 0x8000 ) objectPos.y -= 0.1f;
			if( GetAsyncKeyState(VK_NUMPAD4) & 0x8000 ) objectPos.x += 0.1f;
			if( GetAsyncKeyState(VK_NUMPAD6) & 0x8000 ) objectPos.x -= 0.1f;
			if( GetAsyncKeyState(VK_NUMPAD7) & 0x8000 ) objectPos.z += 0.1f;
			if( GetAsyncKeyState(VK_NUMPAD9) & 0x8000 ) objectPos.z -= 0.1f;
		}
		if( GetAsyncKeyState('R') & 0x8000 ){
			if( GetAsyncKeyState(VK_NUMPAD8) & 0x8000 ) objectRot.y += 0.1f;
			if( GetAsyncKeyState(VK_NUMPAD2) & 0x8000 ) objectRot.y -= 0.1f;
			if( GetAsyncKeyState(VK_NUMPAD4) & 0x8000 ) objectRot.x += 0.1f;
			if( GetAsyncKeyState(VK_NUMPAD6) & 0x8000 ) objectRot.x -= 0.1f;
			if( GetAsyncKeyState(VK_NUMPAD7) & 0x8000 ) objectRot.z += 0.1f;
			if( GetAsyncKeyState(VK_NUMPAD9) & 0x8000 ) objectRot.z -= 0.1f;
		}
		if( GetAsyncKeyState('S') & 0x8000 ){
			if( GetAsyncKeyState(VK_NUMPAD8) & 0x8000 ) objectScale.y += 0.01f;
			if( GetAsyncKeyState(VK_NUMPAD2) & 0x8000 ) objectScale.y -= 0.01f;
			if( GetAsyncKeyState(VK_NUMPAD4) & 0x8000 ) objectScale.x += 0.01f;
			if( GetAsyncKeyState(VK_NUMPAD6) & 0x8000 ) objectScale.x -= 0.01f;
			if( GetAsyncKeyState(VK_NUMPAD7) & 0x8000 ) objectScale.z += 0.01f;
			if( GetAsyncKeyState(VK_NUMPAD9) & 0x8000 ) objectScale.z -= 0.01f;
		}
	}

	static int objNum = 1;
	// オブジェクトの表示.
	{
		static int animNum = 0;
		if( GetAsyncKeyState('A') & 0x0001 ){
			animNum++;
			m_pAc.ChangeNextAnimation();
		}
		static const double speed = m_pAc.GetAnimSpeed()/4;
		if( GetAsyncKeyState('S') & 0x8000 ){
			m_pAc.SetAnimSpeed( -speed );
		} else {
			m_pAc.SetAnimSpeed( speed );
		}
		if( GetAsyncKeyState('Q') & 0x0001 ) objNum++;
		for( int i = 0; i < objNum; i++ ){
			m_pFbxMesh->SetPosition( objectPos );
			m_pFbxMesh->SetRotation( objectRot );
			m_pFbxMesh->SetScale( objectScale );
			m_pFbxMesh->Render(
				m_pCamera->GetViewMatrix(),
				m_pCamera->GetProjMatrix(),
				&m_pAc );
		}

	}

	// ImGui表示.
	{
		ImGui::SetNextWindowSize( 
			{ 200.0f, 100.0f }, 
			ImGuiCond_::ImGuiCond_Once );
		ImGui::SetNextWindowPos( 
			{ 0.0f, 0.0f }, 
			ImGuiCond_::ImGuiCond_Once );

		bool isOpen = true;
		ImGui::GetStyle().Colors[ImGuiCol_::ImGuiCol_WindowBg] 
			= { 0.3f, 0.3f, 0.3f, 0.9f };
		ImGui::Begin( "Info", &isOpen );
		
		ImGui::Text( "FPS : %f", (float)m_pFPS->GetFrameTime() );
		ImGui::Text( "objNum : %d", objNum );

		ImGui::End();

	}

	// ImGui最終描画.
	CImGuiManager::Render();

	m_pDirectX11->SwapChainPresent();
}

//====================================.
//	メッセージループ.
//====================================.
void CMain::Loop()
{
	Load();	// 読み込み関数.

	// メッセージループ.
	MSG msg = { 0 };
	ZeroMemory( &msg, sizeof(msg) );

	while( msg.message != WM_QUIT )
	{
		if( PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE )){
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		} else {
			m_pFPS->Update();
			Update();
			// フレームレートの待機処理.
			m_pFPS->Wait();
		}
	}
}

//====================================.
// ウィンドウ初期化関数.
//====================================.
HRESULT CMain::InitWindow( HINSTANCE hInstance )
{
	ReadWindowInfo();

	// ウィンドウの定義.
	WNDCLASSEX wc = {};
	UINT windowStyle = CS_HREDRAW | CS_VREDRAW;

	wc.cbSize			= sizeof(wc);
	wc.style			= windowStyle;
	wc.lpfnWndProc		= WndProc;
	wc.hInstance		= hInstance;
	wc.hIcon			= LoadIcon( nullptr, IDI_APPLICATION );
	wc.hCursor			= LoadCursor( nullptr, IDC_ARROW );
	wc.hbrBackground	= (HBRUSH)GetStockObject( LTGRAY_BRUSH );
	wc.lpszClassName	= "FbxResearch";
	wc.hIconSm			= LoadIcon( nullptr, IDI_APPLICATION );

	// ウィンドウクラスをWindowsに登録.
	if( !RegisterClassEx( &wc ) ){
		ERROR_MESSAGE( "Registration of window class failed" );
		CLog::Print( "Registration of window class failed" );
		return E_FAIL;
	}

	// ウィンドウ領域の調整.
	RECT rect = { 0, 0, WND_W, WND_H }; // 矩形構造体.
	DWORD dwStyle;	// ウィンドウスタイル.
	dwStyle		= WS_OVERLAPPEDWINDOW;

	if( AdjustWindowRect( &rect, dwStyle, FALSE ) == 0 ){
		MessageBox( nullptr, "ウィンドウ領域の調整に失敗",
			"エラーメッセージ", MB_OK );
		return E_FAIL;
	}
	RECT deskRect = {};	// 画面サイズ.
	SystemParametersInfo( SPI_GETWORKAREA, 0, &deskRect, 0 );
	int pos_x = 0, pos_y = 0;
	// 画面中央に配置できるように計算.
	pos_x = (((deskRect.right - deskRect.left) - (rect.right - rect.left)) / 2 + deskRect.left);
	pos_y = (((deskRect.bottom - deskRect.top) - (rect.bottom - rect.top)) / 2 + deskRect.top);

	// ウィンドウの作成.
	m_hWnd = CreateWindow(
		"FbxResearch",			// アプリ名.
		"Fbx",					// ウィンドウタイトル.
		WS_OVERLAPPEDWINDOW,	// ウィンドウ種別(普通).
		pos_x, pos_y,			// 表示位置x,y座標.
		rect.right - rect.left,	// ウィンドウ幅.
		rect.bottom - rect.top,	// ウィンドウ高さ.
		nullptr,				// 親ウィンドウハンドル.
		nullptr,				// メニュー設定.
		hInstance,				// インスタンス番号.
		nullptr );				// ウィンドウ作成時に発生するイベントに渡すデータ.
	if( !m_hWnd ){
		ERROR_MESSAGE( "Window class creation failed" );
		return E_FAIL;
	}

	// ウィンドウの表示.
	ShowWindow( m_hWnd, SW_SHOW );
	UpdateWindow( m_hWnd );

	return S_OK;
}

//====================================.
// ウィンドウ情報読み込み.
//====================================.
void CMain::ReadWindowInfo()
{
	std::vector<std::string> readList = CFileManager::TextLoading( WINDOW_INFO_TEXT_PATH );

	if( readList.empty() == true ){
		ERROR_MESSAGE( "The list of WindowInfo was empty" );
		return;
	}

	WND_TITLE = readList[0];
	APP_NAME = readList[1];
}