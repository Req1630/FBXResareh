#ifndef CAMERA_H
#define CAMERA_H

#include "..\Global.h"
#include "..\Direct11\AlignedAllocationPolicy\AlignedAllocationPolicy.h"

//===========================.
//	カメラクラス.
//===========================.
class CCamera : public AlignedAllocationPolicy<16>
{
private: //------- 定数 -------.
	const float FLOAT_WND_W = static_cast<float>(WND_W);	// (float)画面幅.
	const float FLOAT_WND_H = static_cast<float>(WND_H);	// (float)画面幅.
	const float INITIAL_VIEWING_ANGLE	= static_cast<float>(3.141519/4.0);	// 初期視野角.
	const float SCREEN_ASPECT_RATIO		= FLOAT_WND_W/FLOAT_WND_H;			// 画面アスペクト比.
	const float INITIAL_MAX_CLIPPING_DISTANCE = 100.0f;						// 初期最大描画距離.
	const float INITIAL_MIN_CLIPPING_DISTANCE = 0.1f;						// 初期最小描画距離.
	const DirectX::XMFLOAT3 VIEW_UP_VECTOR	= { 0.0f, 1.0f, 0.0f };	// 上方向ベクトル.

public: //------- 関数 -------.
	CCamera();
	virtual ~CCamera();

	// カメラの更新処理.
	void Update();
	// ビュー行列、プロジェクション行列の初期化.
	void InitViewProj();

	// ビュー行列取得関数.
	DirectX::XMMATRIX GetViewMatrix() const { return m_ViewMatrix; }
	// プロジェクション行列取得関数.
	DirectX::XMMATRIX GetProjMatrix() const { return m_ProjMatrix; }
	// カメラ座標取得関数.
	DirectX::XMFLOAT3 GetPosition()		const { return m_vPosition; }
	// カメラ注視座標取得関数.
	DirectX::XMFLOAT3 GetLookPosition()	const { return m_vLookPosition; }
	// カメラ座標設定関数.
	void SetPosition( const DirectX::XMFLOAT3& vPos )			{ m_vPosition = vPos; }
	// カメラ注視座標設定関数.
	void SetLookPosition( const DirectX::XMFLOAT3& vLookPos )	{ m_vLookPosition = vLookPos; }

	// 継承先のカメラ情報の設定関数.
	void SetCamera( std::shared_ptr<CCamera> camera );

protected: //------- 変数 -------.
	DirectX::XMFLOAT3	m_vPosition;		// カメラ位置.
	DirectX::XMFLOAT3	m_vLookPosition;	// カメラ注視位置.

private: //------- 変数 -------.
	DirectX::XMMATRIX m_ViewMatrix;		// ビュー行列.
	DirectX::XMMATRIX m_ProjMatrix;		// プロジェクション行列.

	float m_ViewingAngle;			// 視野角.
	float m_MaxClippingDistance;	// 最大クリッピング距離.
	float m_MinClippingDistance;	// 最小クリッピング距離.
};

#endif	// #ifndef CAMERA_H.