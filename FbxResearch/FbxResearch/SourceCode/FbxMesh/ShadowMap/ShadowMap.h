#pragma once

#include "..\FbxInclude\FbxInclude.h"

class CFbxModel;
class CFbxAnimationController;
class CCamera;
class CLight;

class CShadowMap
{
	const wchar_t* SHADOW_SHADER_NAME	= L"Data\\Shader\\ShadowMap.hlsl";

	/***************************************
	*			↓	構造体　↓.
	***************************************/
	//======================================.
	//	コンスタントバッファ(メッシュ毎).
	//======================================.
	struct CBUFFER_PER_MESH
	{
		DirectX::XMMATRIX mLightWVP;
	};
public:
	CShadowMap();
	~CShadowMap();


	//-----------------------------------------.
	//				作成.
	//-----------------------------------------.
	HRESULT Create( ID3D11DeviceContext* pContext11 );

	//-----------------------------------------.
	//				破壊.
	//-----------------------------------------.
	void Destroy();

	//-----------------------------------------.
	//				描画.
	//-----------------------------------------.
	void Render(
		CFbxModel& mdoel,
		CLight& light );

private:
	//-----------------------------------------.
	//		描画用データの作成系.
	//-----------------------------------------.

	// 定数バッファの作成.
	HRESULT CreateBuffer();
	// 定数バッファ作成.
	HRESULT CreateConstantBuffer( const size_t& byte, ID3D11Buffer** buffer );

	// シェーダーの作成.
	HRESULT CreateShader();

private:
	/***************************************
	*			DirectX11.
	***************************************/
	ID3D11Device*			m_pDevice11;	// デバイス11.
	ID3D11DeviceContext*	m_pContext11;	// コンテキスト11.

	/***************************************
	*			シェーダー.
	***************************************/
	ID3D11VertexShader*	m_pVertexShader;	// 頂点シェーダー.
	ID3D11PixelShader*	m_pPixelShader;		// ピクセルシェーダー.

	/***************************************
	*			描画用データ.
	***************************************/
	ID3D11InputLayout*	m_pVertexLayout;	// 頂点レイアウト.
	ID3D11Buffer*		m_pCBufferPerMesh;	// コンスタントバッファ.
};