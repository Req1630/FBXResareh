#pragma once

//警告についてのｺｰﾄﾞ分析を無効にする.4005:再定義.
#pragma warning(disable:4005)

#include "..\Global.h"

#include <vector>

/**************************************************
*	ｽﾌﾟﾗｲﾄｸﾗｽ.
**/
class CSprite
{
public:
	//======================================
	//	構造体.
	//======================================
	//ｺﾝｽﾀﾝﾄﾊﾞｯﾌｧのｱﾌﾟﾘ側の定義.
	//※ｼｪｰﾀﾞ内のｺﾝｽﾀﾝﾄﾊﾞｯﾌｧと一致している必要あり.
	struct C_BUFFER
	{
		ALIGN16 DirectX::XMMATRIX	mW;				// ワールド行列.
		ALIGN16 DirectX::XMMATRIX	mWVP;
		ALIGN16 DirectX::XMFLOAT4	vColor;			// カラー(RGBAの型に合わせる).
		ALIGN16 DirectX::XMFLOAT2	vUV;			// UV座標.
		ALIGN16 DirectX::XMFLOAT2	vViewPort;		// UV座標.
	};
	//頂点の構造体.
	struct VERTEX
	{
		DirectX::XMFLOAT3 Pos;	//頂点座標.
		DirectX::XMFLOAT2 Tex;
	};

public:
	CSprite();	//ｺﾝｽﾄﾗｸﾀ.
	~CSprite();	//ﾃﾞｽﾄﾗｸﾀ.

	//初期化.
	HRESULT Init( ID3D11DeviceContext* pContext11 );

	//解放.
	void Release();

	//ｼｪｰﾀﾞ作成.
	HRESULT CreateShader();
	//ﾓﾃﾞﾙ作成.
	HRESULT CreateModel();
	// サンプラの作成.
	HRESULT InitSample();

	//ﾚﾝﾀﾞﾘﾝｸﾞ用.
	void Render( ID3D11ShaderResourceView* pSRV );
	void Render( std::vector<ID3D11ShaderResourceView*> gbuffers );

	//座標情報を設定.
	void SetPosition(const DirectX::XMFLOAT3& vPos) {	m_vPosition = vPos;	}
	//座標xを設定.
	void SetPositionX( float x ){ m_vPosition.x = x; }
	//座標yを設定.
	void SetPositionY( float y ){ m_vPosition.y = y; }
	//座標zを設定.
	void SetPositionZ( float z ){ m_vPosition.z = z; }

	//回転情報を設定.
	void SetRotation(const DirectX::XMFLOAT3& vRot){	m_vRotation = vRot;	}
	//回転軸Yを設定(Yaw).
	void SetRotationY(float y){	m_vRotation.y = y;	}
	//回転軸Xを設定(Pitch).
	void SetRotationX(float x){	m_vRotation.x = x;	}
	//回転軸Zを設定(Roll).
	void SetRotationZ(float z){	m_vRotation.z = z;	}

	//拡大縮小情報を設定.
	void SetScale( const DirectX::XMFLOAT3& vScale ){ m_vScale = vScale; }

private:
	ID3D11Device*			m_pDevice11;
	ID3D11DeviceContext*	m_pContext11;

	ID3D11VertexShader*		m_pVertexShader;	//頂点ｼｪｰﾀﾞ.
	ID3D11InputLayout*		m_pVertexLayout;	//頂点ﾚｲｱｳﾄ.
	ID3D11PixelShader*		m_pPixelShader;		//ﾋﾟｸｾﾙｼｪｰﾀﾞ.
	ID3D11PixelShader*		m_pPixelShaderLast;		//ﾋﾟｸｾﾙｼｪｰﾀﾞ.
	ID3D11Buffer*			m_pConstantBuffer;	//ｺﾝｽﾀﾝﾄﾊﾞｯﾌｧ.
	ID3D11Buffer*			m_pVertexBuffer;	//頂点ﾊﾞｯﾌｧ.
	ID3D11SamplerState*		m_pSampleLinear;		// サンプラ:テクスチャに各種フィルタをかける.

	DirectX::XMFLOAT3		m_vPosition;	//座標.
	DirectX::XMFLOAT3		m_vRotation;	//回転
	DirectX::XMFLOAT3		m_vScale;		//拡縮.
};