#pragma once

//警告についてのｺｰﾄﾞ分析を無効にする.4005:再定義.
#pragma warning(disable:4005)
#define _CRTDBG_MAP_ALLOC
#include <Windows.h>
#include <crtdbg.h>
#include <memory>

#include <D3D11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

// ライブラリ読み込み.
#pragma comment( lib, "winmm.lib" )
#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "d3dcompiler.lib" )

#include "Utility/Log/MyLog.h"
#include "Direct11/TextuerLoader/WICTextureLoader.h"

//=================================================
//	定数.
//=================================================
const int WND_W = 1280;	// ウィンドウの幅.
const int WND_H = 720;	// ウィンドウの高さ.
const int FPS = 60;

class CCamera;
class CDX9SkinMesh;
class CDX9StaticMesh;
class CSprite;
class CLightBase;
class CFrameRate;

//===========================================================
//	マクロ.
//===========================================================
// 解放.
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)			if(p!=nullptr){(p)->Release();(p)=nullptr;}
#endif	// #ifndef SAFE_RELEASE.

// 破棄.
#ifndef SAFE_DELETE
#define SAFE_DELETE(p)			if(p!=nullptr){delete (p);(p)=nullptr;}
#endif	// #ifndef SAFE_DELETE.

// 破棄(配列).
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p)	if(p!=nullptr){delete[] (p);(p) = nullptr;}
#endif	// #ifndef SAFE_DELETE_ARRAY.

// 破壊.
#ifndef SAFE_DESTROY
#define SAFE_DESTROY(p)			if(p!=nullptr){(p)->Destroy();(p)=nullptr;}
#endif	// #ifndef SAFE_DESTROY.

// _declspec()	:DLLから(関数,クラス,クラスのメンバ関数を)エクスポートする.
// align()		:(強制的に)16byte で使用する.
#define ALIGN16	_declspec(align(16))

#include <string>
#include <codecvt>

template<class T = std::string >
void ERROR_MESSAGE ( const std::string& error )
{
	if( error.empty() ){
		_ASSERT_EXPR( false, L"Not Error Message" );
	}
	// w_charへの変換.
	int size_needed = MultiByteToWideChar( CP_UTF8, 0, &error[0], (int)error.size(), NULL, 0 );
	std::wstring wstrTo( size_needed, 0 );
	MultiByteToWideChar( CP_UTF8, 0, &error[0], (int)error.size(), &wstrTo[0], size_needed );

	CLog::Print( error );
	_ASSERT_EXPR( false, wstrTo.c_str() );
	MessageBox( nullptr, error.c_str(), "Warning", MB_OK );
}