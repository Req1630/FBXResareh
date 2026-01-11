#pragma once

#include <fbxsdk.h>	// FBXSDK.

//----------------------------------.
// ライブラリ読み込み.
//----------------------------------.
#if _DLL	// ランタイムライブラリ.

// MD, MDd.
#if _DEBUG
#pragma comment( lib, "debug\\libfbxsdk-md.lib" )
/***********************************
↓ FBXSDK ver.2019-1 以降で仕様.
******/
#pragma comment( lib, "debug\\alembic-md.lib" )
#pragma comment( lib, "debug\\libxml2-md.lib" )
#pragma comment( lib, "debug\\zlib-md.lib" )
#else
#pragma comment( lib, "release\\libfbxsdk-md.lib" )
/***********************************
↓ FBXSDK ver.2019-1 以降で仕様.
******/
#pragma comment( lib, "release\\alembic-md.lib" )
#pragma comment( lib, "release\\libxml2-md.lib" )
#pragma comment( lib, "release\\zlib-md.lib" )
#endif	// #if _DEBUG.

#else

// MT, MTd.
#if _DEBUG
#pragma comment( lib, "debug\\libfbxsdk-mt.lib" )
#else
#pragma comment( lib, "release\\libfbxsdk-mt.lib" )
#endif	// #if _DEBUG.

/***********************************
↓ FBXSDK ver.2019-1 以降で仕様.
******/
//#pragma comment( lib, "libxml2-mt.lib" )
//#pragma comment( lib, "zlib-mt.lib" )

#endif	// #if _DLL.

#include <D3D11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

// ライブラリ読み込み.
#pragma comment( lib, "winmm.lib" )
#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "d3dcompiler.lib" )

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

// FbxMatrixをDirectX::XMMATRIXに変換する.
DirectX::XMMATRIX FbxMatrixConvertDXMMatrix( const FbxMatrix& matrix );