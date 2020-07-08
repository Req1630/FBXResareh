#ifndef MAIN_H
#define MAIN_H

#include <memory>
#include <string>
#include <Windows.h>

#include "..\FbxMesh\FbxAnimation\FbxAnimationController.h"

//-------------------------.
//	�O���錾.
class CDirectX11;
class CCamera;
class CLight;
class CFrameRate;
class CSprite;

class CFbxMesh;
class CFbxRenderer;
class CFbxModelLoader;
class CFbxAnimationLoader;
class CFbxModel;
//-------------------------.

/**************************************************
*	���C���N���X.
**/
class CMain
{
	std::string WND_TITLE;
	std::string APP_NAME;
	const char* WINDOW_INFO_TEXT_PATH = "Data\\WindowInfo.txt";
public:
	CMain();
	~CMain();
	// �E�B���h�E�������֐�.
	HRESULT InitWindow( HINSTANCE hInstance );

	// ���b�Z�[�W���[�v.
	void Loop();

	// �������֐�.
	HRESULT Init();
	// ����֐�.
	void Release();

private:
	// �X�V����.
	void Update();

	// �ǂݍ��݊֐�.
	HRESULT Load();
	// �E�B���h�E���ǂݍ���.
	void ReadWindowInfo();

private:
	HWND m_hWnd;
	std::unique_ptr<CDirectX11>	m_pDirectX11;	// DirectX11.
	std::unique_ptr<CFrameRate>	m_pFPS;
	std::unique_ptr<CCamera>	m_pCamera;
	std::unique_ptr<CLight>		m_pLight;
	std::unique_ptr<CSprite>	m_Sprite;

	std::unique_ptr<CFbxRenderer>			m_FbxRenderer;
	std::unique_ptr<CFbxModelLoader>		m_FbxModelLoader;
	std::unique_ptr<CFbxAnimationLoader>	m_fbxAnimLoader;
	std::shared_ptr<CFbxModel>				m_FbxModel;
	std::shared_ptr<CFbxModel>				m_FbxGround;

	CFbxAnimationController	m_AC;

};

#endif	// #ifndef MAIN_H.
