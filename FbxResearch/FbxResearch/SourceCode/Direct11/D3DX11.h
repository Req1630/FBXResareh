#ifndef D3DX_H
#define D3DX_H

#include "..\Global.h"

class CDirectX11
{
private: //----- 定数 ------.
	const float CLEAR_BACK_COLOR[4] = { 0.1f, 0.1f, 0.6f, 1.0f };	// バックカラー.

public: //----- 関数 ------.
	CDirectX11();
	~CDirectX11();

	// DirectX11構築関数.
	HRESULT Create( HWND hWnd );
	// DirectX11解放関数.
	HRESULT Release();

	// クリアバックバッファ.
	void ClearBackBuffer();
	// スワップチェーンプレゼント.
	void SwapChainPresent();

	// デバイス11の取得関数.
	static ID3D11Device*		GetDevice() { return m_pDevice11; }
	// コンテキスト11の取得関数.
	static ID3D11DeviceContext*	GetContext() { return m_pContext11; }

private: //----- 関数 ------.
	// デバイス11の作成.
	HRESULT InitDevice11();
	// レンダーターゲットビューの作成.
	HRESULT InitTexRTV();
	// ステンシルビューの作成.
	HRESULT InitDSTex();
	// ビューポートの作成.
	HRESULT InitViewports();
	// ラスタライザの作成.
	HRESULT InitRasterizer();

private: //----- 変数 ------.
	HWND	m_hWnd;	// ウィンドウハンドル.

	static ID3D11Device*		m_pDevice11;	// デバイス11.
	static ID3D11DeviceContext*	m_pContext11;	// コンテキスト11.

	IDXGISwapChain*			m_pSwapChain;			// スワップチェーン.

	ID3D11RenderTargetView*	m_pBackBuffer_TexRTV;	// レンダーターゲットビュー.
	ID3D11Texture2D*		m_pBackBuffer_DSTex;	// テクスチャー2D.
	ID3D11DepthStencilView*	m_pBackBuffer_DSTexDSV;	// ステンシルビュー.

	ID3D11RenderTargetView*		m_pColorMap;	// カラーマップ.
	ID3D11Texture2D*			m_pColorMapTex;
	ID3D11ShaderResourceView*	m_pColorSRV;	// カラーマップシェーダーリソースビュー.
	ID3D11RenderTargetView*		m_pNormalMap;	// ノーマルマップ.
	ID3D11Texture2D*			m_pNormalMapTex;
	ID3D11ShaderResourceView*	m_pNormalSRV;	// ノーマルマップシェーダーリソースビュー.
	
};

#endif	// #ifndef D3DX_H.