#pragma once

#include <fbxsdk.h>	// FBXSDK.

//----------------------------------.
// ���C�u�����ǂݍ���.
//----------------------------------.
#if _DLL	// �����^�C�����C�u����.

// MD, MDd.
#pragma comment( lib, "libfbxsdk-md.lib" )

/***********************************
�� FBXSDK ver.2019-1 �ȍ~�Ŏd�l.
******/
//#pragma comment( lib, "libxml2-md.lib" )
//#pragma comment( lib, "zlib-md.lib" )

#else

// MT, MTd.
#pragma comment( lib, "libfbxsdk-mt.lib" )

/***********************************
�� FBXSDK ver.2019-1 �ȍ~�Ŏd�l.
******/
//#pragma comment( lib, "libxml2-mt.lib" )
//#pragma comment( lib, "zlib-mt.lib" )

#endif	// #if _DLL.

#include "..\..\Global.h"

// FbxMatrix��DirectX::XMMATRIX�ɕϊ�����.
DirectX::XMMATRIX FbxMatrixConvertDXMMatrix( const FbxMatrix& matrix );