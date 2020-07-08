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

#include <D3D11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

// ���C�u�����ǂݍ���.
#pragma comment( lib, "winmm.lib" )
#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "d3dcompiler.lib" )

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

// FbxMatrix��DirectX::XMMATRIX�ɕϊ�����.
DirectX::XMMATRIX FbxMatrixConvertDXMMatrix( const FbxMatrix& matrix );