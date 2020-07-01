#ifndef MAIN_H
#define MAIN_H

#include <memory>
#include <string>
#include <Windows.h>

#include "..\FbxMesh\FbxMesh.h"

//-------------------------.
//	�O���錾.
class CDirectX11;
class CFbxMesh;
class CCamera;
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
	std::unique_ptr<CFbxMesh>	m_pFbxMesh;
	std::unique_ptr<CFbxMesh>	m_pFbxGround;
	std::unique_ptr<CCamera>	m_pCamera;
	CFbxAnimationController		m_pAc;

};

#endif	// #ifndef MAIN_H.
