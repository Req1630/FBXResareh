#include "Main.h"

#include <fstream>
#include <sstream>

#include <windows.h>

#include "..\Direct11\D3DX11.h"
#include "..\Global.h"
#include "..\Utility\FileManager\FileManager.h"
#include "..\Utility\FrameRate\FrameRate.h"
#include "..\FbxMesh\FbxAnimation\FbxAnimationController.h"
#include "..\FbxMesh\FbxAnimation\FbxAnimationLoader.h"

#include "..\Camera\Camera.h"
#include "..\Light\Light.h"

#include "..\ImGui\ImGuiManager\ImGuiManager.h"
#include "..\Sprite\CSprite.h"

#include "..\FbxMesh\FbxModelLoader\FbxModelLoader.h"
#include "..\FbxMesh\FbxRenderer\FbxRenderer.h"
#include "..\FbxMesh\FbxModel\FbxModel.h"

LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );

CMain::CMain()
	: m_hWnd			( nullptr )
	, m_pDirectX11		( nullptr )
	, m_pFrameRate		( nullptr )
	, m_pCamera			( nullptr )
	, m_pLight			( nullptr )
	, m_Sprite			( nullptr )
	, m_FbxRenderer		( nullptr )
	, m_FbxModelLoader	( nullptr )
	, m_fbxAnimLoader	( nullptr )
	, m_FbxModel		( nullptr )
	, m_FbxBone			( nullptr )
	, m_FbxGround		( nullptr )
{
	m_pDirectX11 = std::make_unique<CDirectX11>();
	m_pFrameRate = std::make_unique<CFrameRate>( static_cast<float>(FPS) );
	m_pCamera = std::make_unique<CCamera>();
	m_pLight = std::make_unique<CLight>();
	m_Sprite = std::make_unique<CSprite>();

	m_FbxRenderer = std::make_unique<CFbxRenderer>();
	m_FbxModelLoader = std::make_unique<CFbxModelLoader>();
	m_fbxAnimLoader = std::make_unique<CFbxAnimationLoader>();

	m_FbxModel = std::make_shared<CFbxModel>();
	m_FbxBone = std::make_shared<CFbxModel>();
	m_FbxGround = std::make_shared<CFbxModel>();
}

CMain::~CMain()
{
}

//====================================.
//	�������֐�.
//====================================.
HRESULT CMain::Init()
{
	// DirectX11�̍\�z.
	if( FAILED( m_pDirectX11->Create( m_hWnd ) )) return E_FAIL;
	// ImGui�̏�����.
	if( FAILED( CImGuiManager::Init( m_hWnd, 
		m_pDirectX11->GetDevice(), 
		m_pDirectX11->GetContext() ))) return E_FAIL;

	m_FbxRenderer->Create( m_pDirectX11->GetContext() );
	m_FbxModelLoader->Create( m_pDirectX11->GetDevice() );
	m_fbxAnimLoader->Create();
	m_Sprite->Init( m_pDirectX11->GetContext() );

	return S_OK;
}

//====================================.
//	����֐�.
//====================================.
void CMain::Release()
{
	m_fbxAnimLoader->Destroy();
	m_FbxModelLoader->Destroy();
	m_FbxRenderer->Destroy();

	m_Sprite->Release();
	CImGuiManager::Release();
	m_pDirectX11->Release();
}

//====================================.
//	�ǂݍ��݊֐�.
//====================================.
HRESULT CMain::Load()
{
	const char* fileName[] =
	{
		"Data\\Model\\unitychan\\unitychan_DAMAGED00.fbx",

		"Data\\Model\\humanoid.fbx",
		"Data\\Model\\grid\\grid1.fbx",
		"Data\\Model\\UE_Animation\\Walk_02_Cheerful_Loop_IP.FBX",
		"Data\\Model\\UE_Animation\\Walk_04_Texting_Loop_IP.FBX",
		"Data\\Model\\Liz\\Model\\Liz_Model.fbx",
		"Data\\Model\\Liz\\Animations\\Liz_Idle.fbx",
		"Data\\Model\\Liz\\Animations\\Liz_Jump.fbx",
		"Data\\Model\\Liz\\Animations\\Liz_Run.fbx",
		"Data\\Model\\Liz\\Animations\\Liz_Walk.fbx",
		"Data\\Model\\sayaka_fbx\\sayaka_fbx.fbx",
		"Data\\Model\\bomb\\bomb.fbx",
		"Data\\Model\\JumpingDown\\JumpingDown.fbx",
	};
	const char* boxModelName = "Data\\Model\\box.fbx";
	
	m_FbxModelLoader->LoadModel( m_FbxGround.get(), fileName[2] );
	m_FbxModelLoader->LoadModel( m_FbxModel.get(), fileName[12] );
	m_FbxModelLoader->LoadModel( m_FbxBone.get(), boxModelName );

//	SAnimationDataList animDataList;
//	m_fbxAnimLoader->LoadAnim( &animDataList, fileName[8] );
//	m_AC.AddAnimationData( animDataList );
//	m_fbxAnimLoader->LoadAnim( &animDataList, fileName[7] );
//	m_AC.AddAnimationData( animDataList );

	return S_OK;
}

//====================================.
//	�X�V����.
//====================================.
void CMain::Update()
{
	LARGE_INTEGER start, end;
	QueryPerformanceCounter(&start);

	// �f���^�^�C���̎擾.
	const float	deltaTime = static_cast<float>(m_pFrameRate->GetDeltaTime());

	// ��ʂ̃N���A.
	m_pDirectX11->ClearBackBuffer();

	// ImGui�̃t���[��������.
	CImGuiManager::SetingNewFrame();
	
	const char* boneName = "Head";
	const float boneBoxScale = 0.001f;

	static DirectX::XMFLOAT3 objectPos = { 0.0f, 0.0f, 0.0f };
	static DirectX::XMFLOAT3 objectRot = { 0.0f, 0.0f, 0.0f };
	static DirectX::XMFLOAT3 objectScale = { 0.05f, 0.05f, 0.05f };
	// �J��������.
	{
		static DirectX::XMFLOAT3 cameraPos = { 0.0f, 6.0f, 15.0f };
		static DirectX::XMFLOAT3 lightPos = { 0.0f, 10.0f, 15.0f };
		if( GetAsyncKeyState('C') & 0x8000 ){
			if( GetAsyncKeyState(VK_NUMPAD8) & 0x8000 ) cameraPos.y += 0.1f;
			if( GetAsyncKeyState(VK_NUMPAD2) & 0x8000 ) cameraPos.y -= 0.1f;
			if( GetAsyncKeyState(VK_NUMPAD4) & 0x8000 ) cameraPos.x += 0.1f;
			if( GetAsyncKeyState(VK_NUMPAD6) & 0x8000 ) cameraPos.x -= 0.1f;
			if( GetAsyncKeyState(VK_NUMPAD7) & 0x8000 ) cameraPos.z += 0.1f;
			if( GetAsyncKeyState(VK_NUMPAD9) & 0x8000 ) cameraPos.z -= 0.1f;
		}
		if( GetAsyncKeyState('L') & 0x8000 ){
			if( GetAsyncKeyState(VK_NUMPAD8) & 0x8000 ) lightPos.y += 0.1f;
			if( GetAsyncKeyState(VK_NUMPAD2) & 0x8000 ) lightPos.y -= 0.1f;
			if( GetAsyncKeyState(VK_NUMPAD4) & 0x8000 ) lightPos.x += 0.1f;
			if( GetAsyncKeyState(VK_NUMPAD6) & 0x8000 ) lightPos.x -= 0.1f;
			if( GetAsyncKeyState(VK_NUMPAD7) & 0x8000 ) lightPos.z += 0.1f;
			if( GetAsyncKeyState(VK_NUMPAD9) & 0x8000 ) lightPos.z -= 0.1f;
		}
		m_pCamera->SetPosition( cameraPos );
		m_pCamera->SetLookPosition( { 0.0f, 3.0f, 0.0f } );
		m_pCamera->InitViewProj();
		m_pLight->SetPosition( lightPos );
		m_pLight->SetLookPosition( { 0.0f, 0.0f, 0.0f } );
	}

	if( GetAsyncKeyState('A') & 0x0001 ){
		m_AC.ChangeNextAnimation();
	}

	// �I�u�W�F�N�g����.
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
	if( GetAsyncKeyState('Q') & 0x0001 ) objNum++;

	static DirectX::XMFLOAT3 spritePos = { 0.0f, WND_H*0.3f*0.0f, 0.0f };
	if( GetAsyncKeyState('T') & 0x8000 ){
		if( GetAsyncKeyState(VK_NUMPAD8) & 0x8000 ) spritePos.y -= 2.0f;
		if( GetAsyncKeyState(VK_NUMPAD2) & 0x8000 ) spritePos.y += 2.0f;
		if( GetAsyncKeyState(VK_NUMPAD4) & 0x8000 ) spritePos.x -= 2.0f;
		if( GetAsyncKeyState(VK_NUMPAD6) & 0x8000 ) spritePos.x += 2.0f;
		if( GetAsyncKeyState(VK_NUMPAD7) & 0x8000 ) spritePos.z += 2.0f;
		if( GetAsyncKeyState(VK_NUMPAD9) & 0x8000 ) spritePos.z -= 2.0f;
	}
	static DirectX::XMFLOAT3 spritePos2 = { 0.0f, WND_H*0.3f*1.0f, 0.0f };
	if( GetAsyncKeyState('N') & 0x8000 ){
		if( GetAsyncKeyState(VK_NUMPAD8) & 0x8000 ) spritePos2.y -= 2.0f;
		if( GetAsyncKeyState(VK_NUMPAD2) & 0x8000 ) spritePos2.y += 2.0f;
		if( GetAsyncKeyState(VK_NUMPAD4) & 0x8000 ) spritePos2.x -= 2.0f;
		if( GetAsyncKeyState(VK_NUMPAD6) & 0x8000 ) spritePos2.x += 2.0f;
		if( GetAsyncKeyState(VK_NUMPAD7) & 0x8000 ) spritePos2.z += 2.0f;
		if( GetAsyncKeyState(VK_NUMPAD9) & 0x8000 ) spritePos2.z -= 2.0f;
	}
	static DirectX::XMFLOAT3 spritePos3 = { 0.0f, WND_H*0.3f*2.0f, 0.0f };
	if( GetAsyncKeyState('V') & 0x8000 ){
		if( GetAsyncKeyState(VK_NUMPAD8) & 0x8000 ) spritePos3.y -= 2.0f;
		if( GetAsyncKeyState(VK_NUMPAD2) & 0x8000 ) spritePos3.y += 2.0f;
		if( GetAsyncKeyState(VK_NUMPAD4) & 0x8000 ) spritePos3.x -= 2.0f;
		if( GetAsyncKeyState(VK_NUMPAD6) & 0x8000 ) spritePos3.x += 2.0f;
		if( GetAsyncKeyState(VK_NUMPAD7) & 0x8000 ) spritePos3.z += 2.0f;
		if( GetAsyncKeyState(VK_NUMPAD9) & 0x8000 ) spritePos3.z -= 2.0f;
	}
	static DirectX::XMFLOAT3 spritePos4 = { WND_W-WND_W*0.4f, 0.0f, 0.0f };
	if( GetAsyncKeyState('V') & 0x8000 ){
		if( GetAsyncKeyState(VK_NUMPAD8) & 0x8000 ) spritePos4.y -= 2.0f;
		if( GetAsyncKeyState(VK_NUMPAD2) & 0x8000 ) spritePos4.y += 2.0f;
		if( GetAsyncKeyState(VK_NUMPAD4) & 0x8000 ) spritePos4.x -= 2.0f;
		if( GetAsyncKeyState(VK_NUMPAD6) & 0x8000 ) spritePos4.x += 2.0f;
		if( GetAsyncKeyState(VK_NUMPAD7) & 0x8000 ) spritePos4.z += 2.0f;
		if( GetAsyncKeyState(VK_NUMPAD9) & 0x8000 ) spritePos4.z -= 2.0f;
	}
	m_pDirectX11->SetBackBuffer();

	// GBuffer�̒ǉ�.
#if 0
	m_Sprite->SetPosition( spritePos );
	m_Sprite->SetScale( { 0.3f, 0.3f, 0.3f } );
	m_Sprite->Render( m_pDirectX11->GetColorMap() );

	m_Sprite->SetPosition( spritePos2 );
	m_Sprite->SetScale( { 0.3f, 0.3f, 0.3f } );
	m_Sprite->Render( m_pDirectX11->GetNormalMap() );

	m_Sprite->SetPosition( spritePos3 );
	m_Sprite->SetScale( { 0.3f, 0.3f, 0.3f } );
	m_Sprite->Render( m_pDirectX11->GetZDepthMap() );

	m_Sprite->SetPosition( spritePos4 );
	m_Sprite->SetScale( { 0.4f, 0.4f, 0.4f } );
	m_Sprite->Render( m_pDirectX11->GetGBuffer() );
#endif
	{
		{
			for( int i = 0; i < objNum; i++ ){
				m_FbxModel->SetPosition( objectPos );
				m_FbxModel->SetRotation( objectRot );
				m_FbxModel->SetScale( objectScale );
				m_FbxModel->SetAnimSpeed( deltaTime );
				m_FbxRenderer->Render(
					*m_FbxModel.get(),
					*m_pCamera.get(),
					*m_pLight.get());
//					&m_AC);
			}
			
		}


//		m_FbxBone->SetPosition( m_FbxModel->GetBonePosition( boneName ) );
//		m_FbxBone->SetRotation( objectRot );
//		m_FbxBone->SetScale( boneBoxScale );
//		m_FbxBone->SetAnimSpeed( 0.01 );
//		m_FbxRenderer->Render(
//			*m_FbxBone.get(),
//			*m_pCamera.get(),
//			*m_pLight.get() );
	}

	

	// ImGui�\��.
	{
		ImGui::SetNextWindowSize( 
			{ 200.0f, 500.0f }, 
			ImGuiCond_::ImGuiCond_Once );
		ImGui::SetNextWindowPos( 
			{ 0.0f, 0.0f }, 
			ImGuiCond_::ImGuiCond_Once );

		bool isOpen = true;
		ImGui::GetStyle().Colors[ImGuiCol_::ImGuiCol_WindowBg] 
			= { 0.3f, 0.3f, 0.3f, 0.9f };
		ImGui::Begin( "Info", &isOpen );
		
		ImGui::Text( "FPS : %f", (float)m_pFrameRate->GetFPS() );
		ImGui::Text( "DeltaTime : %f", (float)m_pFrameRate->GetDeltaTime() );
		ImGui::Text( "objNum : %d", objNum );

		QueryPerformanceCounter(&end);
		ImGui::Text( "main : %d", (end.QuadPart-start.QuadPart) );

		ImGui::End();

	}

	// ImGui�ŏI�`��.
	CImGuiManager::Render();

	m_pDirectX11->SwapChainPresent();
}

//====================================.
//	���b�Z�[�W���[�v.
//====================================.
void CMain::Loop()
{
	Load();	// �ǂݍ��݊֐�.

	// ���b�Z�[�W���[�v.
	MSG msg = { 0 };
	ZeroMemory( &msg, sizeof(msg) );

	while( msg.message != WM_QUIT )
	{
		if( PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE )){
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		} else {
			// �t���[�����[�g�̑ҋ@����.
			if( m_pFrameRate->Wait() == true ) continue;
			Update();
		}
	}
}

//====================================.
// �E�B���h�E�������֐�.
//====================================.
HRESULT CMain::InitWindow( HINSTANCE hInstance )
{
	ReadWindowInfo();

	// �E�B���h�E�̒�`.
	WNDCLASSEX wc = {};
	UINT windowStyle = CS_HREDRAW | CS_VREDRAW;

	wc.cbSize			= sizeof(wc);
	wc.style			= windowStyle;
	wc.lpfnWndProc		= WndProc;
	wc.hInstance		= hInstance;
	wc.hIcon			= LoadIcon( nullptr, IDI_APPLICATION );
	wc.hCursor			= LoadCursor( nullptr, IDC_ARROW );
	wc.hbrBackground	= (HBRUSH)GetStockObject( LTGRAY_BRUSH );
	wc.lpszClassName	= TEXT("FbxResearch");
	wc.hIconSm			= LoadIcon( nullptr, IDI_APPLICATION );

	// �E�B���h�E�N���X��Windows�ɓo�^.
	if( !RegisterClassEx( &wc ) ){
		ERROR_MESSAGE( "Registration of window class failed" );
		CLog::Print( "Registration of window class failed" );
		return E_FAIL;
	}

	// �E�B���h�E�̈�̒���.
	RECT rect = { 0, 0, WND_W, WND_H }; // ��`�\����.
	DWORD dwStyle;	// �E�B���h�E�X�^�C��.
	dwStyle		= WS_OVERLAPPEDWINDOW;

	if( AdjustWindowRect( &rect, dwStyle, FALSE ) == 0 ){
		MessageBox( nullptr, TEXT("�E�B���h�E�̈�̒����Ɏ��s"),
			TEXT("�G���[���b�Z�[�W"), MB_OK );
		return E_FAIL;
	}
	RECT deskRect = {};	// ��ʃT�C�Y.
	SystemParametersInfo( SPI_GETWORKAREA, 0, &deskRect, 0 );
	int pos_x = 0, pos_y = 0;
	// ��ʒ����ɔz�u�ł���悤�Ɍv�Z.
	pos_x = (((deskRect.right - deskRect.left) - (rect.right - rect.left)) / 2 + deskRect.left);
	pos_y = (((deskRect.bottom - deskRect.top) - (rect.bottom - rect.top)) / 2 + deskRect.top);

	// �E�B���h�E�̍쐬.
	m_hWnd = CreateWindow(
		TEXT("FbxResearch"),	// �A�v����.
		TEXT("Fbx"),			// �E�B���h�E�^�C�g��.
		WS_OVERLAPPEDWINDOW,	// �E�B���h�E���(����).
		pos_x, pos_y,			// �\���ʒux,y���W.
		rect.right - rect.left,	// �E�B���h�E��.
		rect.bottom - rect.top,	// �E�B���h�E����.
		nullptr,				// �e�E�B���h�E�n���h��.
		nullptr,				// ���j���[�ݒ�.
		hInstance,				// �C���X�^���X�ԍ�.
		nullptr );				// �E�B���h�E�쐬���ɔ�������C�x���g�ɓn���f�[�^.
	if( !m_hWnd ){
		ERROR_MESSAGE( "Window class creation failed" );
		return E_FAIL;
	}

	// �E�B���h�E�̕\��.
	ShowWindow( m_hWnd, SW_SHOW );
	UpdateWindow( m_hWnd );

	return S_OK;
}

//====================================.
// �E�B���h�E���ǂݍ���.
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