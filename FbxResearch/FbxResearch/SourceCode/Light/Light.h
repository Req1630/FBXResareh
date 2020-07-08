#ifndef LIGHT_H
#define LIGHT_H

#include "..\Global.h"
#include "..\Direct11\AlignedAllocationPolicy\AlignedAllocationPolicy.h"

class CLight : public AlignedAllocationPolicy<16>
{
	const float FLOAT_WND_W = static_cast<float>(WND_W);	// (float)画面幅.
	const float FLOAT_WND_H = static_cast<float>(WND_H);	// (float)画面幅.
	const float INITIAL_VIEWING_ANGLE	= static_cast<float>(3.141519/4.0);	// 初期視野角.
	const float SCREEN_ASPECT_RATIO		= 1.0f;			// 画面アスペクト比.
	const float INITIAL_MAX_CLIPPING_DISTANCE = 1000.0f;					// 初期最大描画距離.
	const float INITIAL_MIN_CLIPPING_DISTANCE = 10.0f;						// 初期最小描画距離.
	const DirectX::XMFLOAT3 VIEW_UP_VECTOR	= { 0.0f, 1.0f, 0.0f };	// 上方向ベクトル.

public:
	CLight();
	~CLight();

	DirectX::XMMATRIX GetVP();
	DirectX::XMFLOAT4 GetDirection();

	// カメラ座標設定関数.
	void SetPosition( const DirectX::XMFLOAT3& vPos )			{ m_Position = vPos; }
	// カメラ注視座標設定関数.
	void SetLookPosition( const DirectX::XMFLOAT3& vLookPos )	{ m_vLookPosition = vLookPos; }
private:
	DirectX::XMMATRIX m_ProjMatrix;
	DirectX::XMMATRIX m_ViewMatrix;
	DirectX::XMFLOAT3 m_Position;
	DirectX::XMFLOAT3 m_vLookPosition;

	float m_ViewingAngle;			// 視野角.
	float m_MaxClippingDistance;	// 最大クリッピング距離.
	float m_MinClippingDistance;	// 最小クリッピング距離.
};

#endif	// #ifndef LIGHT_H.