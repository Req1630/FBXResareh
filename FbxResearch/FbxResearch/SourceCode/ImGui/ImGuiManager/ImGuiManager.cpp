#include "ImGuiManager.h"

CImGuiManager::CImGuiManager()
{
}

CImGuiManager::~CImGuiManager()
{
}

//----------------------.
// 初期化関数.
//----------------------.
HRESULT CImGuiManager::Init( 
	HWND hWnd, 
	ID3D11Device* pDevice11,
	ID3D11DeviceContext* pContext11 )
{
	IMGUI_CHECKVERSION();

	const float fontSize = GetInstance()->FONT_SIZE; 

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImFont* font = io.Fonts->AddFontFromFileTTF(
		GetInstance()->FONT_FILE_PATH, 
		fontSize,
		nullptr, 
		io.Fonts->GetGlyphRangesJapanese());

	ImGuiStyle& style = ImGui::GetStyle();

	style.ScaleAllSizes( fontSize*0.06f ); // UIの大きさを一括で変更できます。
	io.FontGlobalScale = fontSize*0.06f; // フォントの大きさを一括で変更できます。

	if( ImGui_ImplWin32_Init( hWnd ) == false ) return E_FAIL;
	if( ImGui_ImplDX11_Init( pDevice11, pContext11 ) == false ) return E_FAIL;

	ImGui::StyleColorsDark();

	return S_OK;
}

//----------------------.
// 解放関数.
//----------------------.
HRESULT CImGuiManager::Release()
{
	// ImGuiの解放.
	ImGui_ImplWin32_Shutdown();
	ImGui_ImplDX11_Shutdown();
	ImGui::DestroyContext();

	return S_OK;
}

//----------------------.
// フレームの設定.
//----------------------.
void CImGuiManager::SetingNewFrame()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

//----------------------.
// 描画.
//----------------------.
void CImGuiManager::Render()
{
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData() );
}

//----------------------.
// インスタンスの取得.
//----------------------.
CImGuiManager* CImGuiManager::GetInstance()
{
	static std::unique_ptr<CImGuiManager> pInstance = 
		std::make_unique<CImGuiManager>();
	return pInstance.get();
}