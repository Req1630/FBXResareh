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

	// �������֐�.
	static HRESULT Init( 
		HWND hWnd, 
		ID3D11Device* pDevice11,
		ID3D11DeviceContext* pContext11 );

	// ����֐�.
	static HRESULT Release();

	// �t���[���̐ݒ�.
	static void SetingNewFrame();

	// �`��.
	static void Render();

private:
	// �C���X�^���X�̎擾.
	static CImGuiManager* GetInstance();
};

#endif	// #ifndef IMGUI_MANAGER_H.