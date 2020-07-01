#pragma once

#include <fbxsdk.h>	// FBXSDK.

//----------------------------------.
// ライブラリ読み込み.
//----------------------------------.
#if _DLL	// ランタイムライブラリ.

// MD, MDd.
#pragma comment( lib, "libfbxsdk-md.lib" )

/***********************************
↓ FBXSDK ver.2019-1 以降で仕様.
******/
//#pragma comment( lib, "libxml2-md.lib" )
//#pragma comment( lib, "zlib-md.lib" )

#else

// MT, MTd.
#pragma comment( lib, "libfbxsdk-mt.lib" )

/***********************************
↓ FBXSDK ver.2019-1 以降で仕様.
******/
//#pragma comment( lib, "libxml2-mt.lib" )
//#pragma comment( lib, "zlib-mt.lib" )

#endif	// #if _DLL.

#include "..\..\Global.h"

// FbxMatrixをDirectX::XMMATRIXに変換する.
DirectX::XMMATRIX FbxMatrixConvertDXMMatrix( const FbxMatrix& matrix );