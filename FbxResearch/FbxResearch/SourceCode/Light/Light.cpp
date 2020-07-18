#include "Light.h"

CLight::CLight()
	: m_ProjMatrix			()
	, m_ViewMatrix			()
	, m_Position			( 0.0f, 0.0f, -1.0f )
	, m_vLookPosition		( 0.0f, 0.5f, 0.0f )
	, m_ViewingAngle		( INITIAL_VIEWING_ANGLE )
	, m_MaxClippingDistance	( INITIAL_MAX_CLIPPING_DISTANCE )
	, m_MinClippingDistance	( INITIAL_MIN_CLIPPING_DISTANCE )
{
}

CLight::~CLight()
{
}

DirectX::XMMATRIX CLight::GetVP()
{
	// ビュー(カメラ)変換.
	m_ViewMatrix = DirectX::XMMatrixLookAtLH( 
		{ m_Position.x, m_Position.y, m_Position.z },		// カメラ座標.
		{ m_vLookPosition.x, m_vLookPosition.y, m_vLookPosition.z },	// カメラ注視座標.
		{ VIEW_UP_VECTOR.x, VIEW_UP_VECTOR.y, VIEW_UP_VECTOR.z } );	// カメラベクトル.
	
	// プロジェクション(平行)変換.
	m_ProjMatrix = DirectX::XMMatrixOrthographicLH(
		m_ViewingAngle,				// 視野角.
		SCREEN_ASPECT_RATIO,		// 画面アスペクト比.
		m_MinClippingDistance,		// 最小描画距離.
		m_MaxClippingDistance );	// 最大描画距離.

	return m_ViewMatrix * m_ProjMatrix;
}

DirectX::XMFLOAT4 CLight::GetDirection()
{
	return 
	{
		m_Position.x - m_vLookPosition.x,
		m_Position.y - m_vLookPosition.y,
		m_Position.z - m_vLookPosition.z,
		0.0f,
	};
}