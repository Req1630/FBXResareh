#pragma once

#include <fbxsdk.h>	// FBXSDK.

//----------------------------------.
// ライブラリ読み込み.
//----------------------------------.
#if _DLL	// ランタイムライブラリ.

// MD, MDd.
#if _DEBUG
#pragma comment( lib, "debug\\libfbxsdk-md.lib" )
#else
#pragma comment( lib, "release\\libfbxsdk-md.lib" )
#endif	// #if _DEBUG.

/***********************************
↓ FBXSDK ver.2019-1 以降で仕様.
******/
//#pragma comment( lib, "libxml2-md.lib" )
//#pragma comment( lib, "zlib-md.lib" )

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

#include "..\..\Global.h"

// FbxMatrixをDirectX::XMMATRIXに変換する.
DirectX::XMMATRIX FbxMatrixConvertDXMMatrix( const FbxMatrix& matrix );