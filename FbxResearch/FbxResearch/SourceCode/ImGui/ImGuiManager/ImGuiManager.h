#ifndef IMGUI_MANAGER_H
#define IMGUI_MANAGER_H

#include "..\..\Global.h"

#include "..\imgui.h"
#include "..\imgui_impl_dx11.h"
#include "..\imgui_impl_win32.h"

#include <memory>

class CImGuiManager
{
	const char* FONT_FILE_PATH  = "Data\\Texture\\_Font\\mplus-1p-medium.ttf";
	const float FONT_SIZE = 18.0f;
public:
	CImGuiManager();
	~CImGuiManager();

	// 初期化関数.
	static HRESULT Init( 
		HWND hWnd, 
		ID3D11Device* pDevice11,
		ID3D11DeviceContext* pContext11 );

	// 解放関数.
	static HRESULT Release();

	// フレームの設定.
	static void SetingNewFrame();

	// 描画.
	static void Render();

private:
	// インスタンスの取得.
	static CImGuiManager* GetInstance();
};

#endif	// #ifndef IMGUI_MANAGER_H.