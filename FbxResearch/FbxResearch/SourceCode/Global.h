#pragma once

//�x���ɂ��Ă̺��ޕ��͂𖳌��ɂ���.4005:�Ē�`.
#pragma warning(disable:4005)
#define _CRTDBG_MAP_ALLOC
#include <Windows.h>
#include <crtdbg.h>
#include <memory>

#include <D3D11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

// ���C�u�����ǂݍ���.
#pragma comment( lib, "winmm.lib" )
#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "d3dcompiler.lib" )

#include "Utility/Log/MyLog.h"
#include "Direct11/TextuerLoader/WICTextureLoader.h"

//=================================================
//	�萔.
//=================================================
const int WND_W = 1280;	// �E�B���h�E�̕�.
const int WND_H = 720;	// �E�B���h�E�̍���.
const int FPS = 60;

class CCamera;
class CDX9SkinMesh;
class CDX9StaticMesh;
class CSprite;
class CLightBase;
class CFrameRate;

//===========================================================
//	�}�N��.
//===========================================================
// ���.
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)			if(p!=nullptr){(p)->Release();(p)=nullptr;}
#endif	// #ifndef SAFE_RELEASE.

// �j��.
#ifndef SAFE_DELETE
#define SAFE_DELETE(p)			if(p!=nullptr){delete (p);(p)=nullptr;}
#endif	// #ifndef SAFE_DELETE.

// �j��(�z��).
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p)	if(p!=nullptr){delete[] (p);(p) = nullptr;}
#endif	// #ifndef SAFE_DELETE_ARRAY.

// �j��.
#ifndef SAFE_DESTROY
#define SAFE_DESTROY(p)			if(p!=nullptr){(p)->Destroy();(p)=nullptr;}
#endif	// #ifndef SAFE_DESTROY.

// _declspec()	:DLL����(�֐�,�N���X,�N���X�̃����o�֐���)�G�N�X�|�[�g����.
// align()		:(�����I��)16byte �Ŏg�p����.
#define ALIGN16	_declspec(align(16))

#include <string>
#include <codecvt>

template<class T = std::string >
void ERROR_MESSAGE ( const std::string& error )
{
	if( error.empty() ){
		_ASSERT_EXPR( false, L"Not Error Message" );
	}
	// w_char�ւ̕ϊ�.
	int size_needed = MultiByteToWideChar( CP_UTF8, 0, &error[0], (int)error.size(), NULL, 0 );
	std::wstring wstrTo( size_needed, 0 );
	MultiByteToWideChar( CP_UTF8, 0, &error[0], (int)error.size(), &wstrTo[0], size_needed );

	CLog::Print( error );
	_ASSERT_EXPR( false, wstrTo.c_str() );
	MessageBox( nullptr, error.c_str(), "Warning", MB_OK );
}