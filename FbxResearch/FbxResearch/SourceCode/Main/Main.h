#ifndef MAIN_H
#define MAIN_H

#include <memory>
#include <string>
#include <Windows.h>

#include "..\FbxMesh\FbxAnimation\FbxAnimationController.h"

//-------------------------.
//	前方宣言.
class CDirectX11;
class CCamera;
class CFrameRate;
class CSprite;

class CFbxMesh;
class CFbxRenderer;
class CFbxModelLoader;
class CFbxModel;
//-------------------------.

/**************************************************
*	メインクラス.
**/
class CMain
{
	std::string WND_TITLE;
	std::string APP_NAME;
	const char* WINDOW_INFO_TEXT_PATH = "Data\\WindowInfo.txt";
public:
	CMain();
	~CMain();
	// ウィンドウ初期化関数.
	HRESULT InitWindow( HINSTANCE hInstance );

	// メッセージループ.
	void Loop();

	// 初期化関数.
	HRESULT Init();
	// 解放関数.
	void Release();

private:
	// 更新処理.
	void Update();

	// 読み込み関数.
	HRESULT Load();
	// ウィンドウ情報読み込み.
	void ReadWindowInfo();

private:
	HWND m_hWnd;
	std::unique_ptr<CDirectX11>	m_pDirectX11;	// DirectX11.
	std::unique_ptr<CFrameRate>	m_pFPS;
	std::unique_ptr<CFbxMesh>	m_pFbxMesh;
	std::unique_ptr<CFbxMesh>	m_pFbxGround;
	std::unique_ptr<CCamera>	m_pCamera;
	CFbxAnimationController		m_Ac;

	std::unique_ptr<CFbxRenderer>		m_FbxRenderer;
	std::unique_ptr<CFbxModelLoader>	m_FbxModelLoader;
	std::shared_ptr<CFbxModel>			m_FbxModel;
	std::unique_ptr<CSprite>			m_Sprite;

};

#endif	// #ifndef MAIN_H.
