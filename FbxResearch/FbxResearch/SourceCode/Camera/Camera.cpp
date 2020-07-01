#include "Camera.h"

CCamera::CCamera()
	: m_ViewMatrix			()
	, m_ProjMatrix			()
	, m_vPosition			( 0.0f, 0.0f, -1.0f )
	, m_vLookPosition		( 0.0f, 0.5f, 0.0f )
	, m_ViewingAngle		( INITIAL_VIEWING_ANGLE )
	, m_MaxClippingDistance	( INITIAL_MAX_CLIPPING_DISTANCE )
	, m_MinClippingDistance	( INITIAL_MIN_CLIPPING_DISTANCE )
{
}

CCamera::~CCamera()
{
}

//--------------------------------------------.
// カメラの更新処理.
//--------------------------------------------.
void CCamera::Update()
{
}

//--------------------------------------------.
// ビュー行列、プロジェクション行列の初期化.
//--------------------------------------------.
void CCamera::InitViewProj()
{
	DirectX::XMVECTOR pos = 
		DirectX::XMVectorSet( m_vPosition.x, m_vPosition.y, m_vPosition.z, 1.0f );
	DirectX::XMVECTOR lookPos = 
		DirectX::XMVectorSet( m_vLookPosition.x, m_vLookPosition.y, m_vLookPosition.z, 1.0f );
	DirectX::XMVECTOR upVector = 
		DirectX::XMVectorSet( VIEW_UP_VECTOR.x, VIEW_UP_VECTOR.y, VIEW_UP_VECTOR.z, 1.0f );
	// ビュー(カメラ)変換.
	m_ViewMatrix = DirectX::XMMatrixLookAtLH( 
		pos,		// カメラ座標.
		lookPos,	// カメラ注視座標.
		upVector );	// カメラベクトル.

	// プロジェクション(射影)変換.
	m_ProjMatrix = DirectX::XMMatrixPerspectiveFovLH(
		m_ViewingAngle,				// 視野角.
		SCREEN_ASPECT_RATIO,		// 画面アスペクト比.
		m_MinClippingDistance,		// 最小描画距離.
		m_MaxClippingDistance );	// 最大描画距離.
	
}

//--------------------------------------------.
// 継承先のカメラ情報の設定関数.
//--------------------------------------------.
void CCamera::SetCamera( std::shared_ptr<CCamera> camera )
{
	this->m_vPosition		= camera->GetPosition();
	this->m_vLookPosition	= camera->GetLookPosition();
}