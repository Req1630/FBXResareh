#pragma once

#include <fbxsdk.h>	// FBXSDK.

//----------------------------------.
// ���C�u�����ǂݍ���.
//----------------------------------.
#if _DLL	// �����^�C�����C�u����.

// MD, MDd.
#if _DEBUG
#pragma comment( lib, "debug\\libfbxsdk-md.lib" )
#else
#pragma comment( lib, "release\\libfbxsdk-md.lib" )
#endif	// #if _DEBUG.

/***********************************
�� FBXSDK ver.2019-1 �ȍ~�Ŏd�l.
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
�� FBXSDK ver.2019-1 �ȍ~�Ŏd�l.
******/
//#pragma comment( lib, "libxml2-mt.lib" )
//#pragma comment( lib, "zlib-mt.lib" )

#endif	// #if _DLL.

#include "..\..\Global.h"

// FbxMatrix��DirectX::XMMATRIX�ɕϊ�����.
DirectX::XMMATRIX FbxMatrixConvertDXMMatrix( const FbxMatrix& matrix );