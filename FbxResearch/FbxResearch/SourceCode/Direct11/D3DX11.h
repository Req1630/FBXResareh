#ifndef D3DX_H
#define D3DX_H

#include "..\Global.h"

#include <vector>

class CDirectX11
{
private: //----- 定数 ------.
	const float CLEAR_BACK_COLOR1[4] = { 0.1f, 0.1f, 0.6f, 1.0f };	// バックカラー.
	const float CLEAR_BACK_COLOR2[4] = { 0.6f, 0.1f, 0.1f, 1.0f };	// バックカラー.
	const float CLEAR_BACK_COLOR3[4] = { 0.1f, 0.6f, 0.1f, 1.0f };	// バックカラー.

	const int g_bufferNum = 3;

	enum enGBUFFER
	{
		enGBUFFER_COLOR,
		enGBUFFER_NORMAL,
		enGBUFFER_Z_DEPTH,
	};
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

	void SetBackBuffer();

	std::vector<ID3D11ShaderResourceView*> GetGBuffer(){ return GBufferSRV; }
	ID3D11ShaderResourceView* GetColorMap() { return GBufferSRV[enGBUFFER_COLOR]; } 
	ID3D11ShaderResourceView* GetNormalMap() { return GBufferSRV[enGBUFFER_NORMAL]; } 
	ID3D11ShaderResourceView* GetZDepthMap() { return GBufferSRV[enGBUFFER_Z_DEPTH]; }

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
	

	std::vector<ID3D11RenderTargetView*>	GBufferRTV;	// G-Bufferのレンダーターゲットビュー.
	std::vector<ID3D11ShaderResourceView*>	GBufferSRV;	// G-Bufferのシェーダーリソースビュー.
	std::vector<ID3D11Texture2D*>			GBufferTex;	// G-Bufferのテクスチャ2D.

	ID3D11RenderTargetView*		m_pColorMapRTV;		// カラーマップ.
	ID3D11Texture2D*			m_pColorMapTex;
	ID3D11ShaderResourceView*	m_pColorSRV;		// カラーマップシェーダーリソースビュー.
	ID3D11RenderTargetView*		m_pNormalMapRTV;	// ノーマルマップ.
	ID3D11Texture2D*			m_pNormalMapTex;
	ID3D11ShaderResourceView*	m_pNormalSRV;		// ノーマルマップシェーダーリソースビュー.
	ID3D11RenderTargetView*		m_pPositionMapRTV;	// 座標マップ.
	ID3D11Texture2D*			m_pPositionMapTex;
	ID3D11ShaderResourceView*	m_pPositionSRV;		// 座標マップシェーダーリソースビュー.
	
};

#endif	// #ifndef D3DX_H.