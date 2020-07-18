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
	// �r���[(�J����)�ϊ�.
	m_ViewMatrix = DirectX::XMMatrixLookAtLH( 
		{ m_Position.x, m_Position.y, m_Position.z },		// �J�������W.
		{ m_vLookPosition.x, m_vLookPosition.y, m_vLookPosition.z },	// �J�����������W.
		{ VIEW_UP_VECTOR.x, VIEW_UP_VECTOR.y, VIEW_UP_VECTOR.z } );	// �J�����x�N�g��.
	
	// �v���W�F�N�V����(���s)�ϊ�.
	m_ProjMatrix = DirectX::XMMatrixOrthographicLH(
		m_ViewingAngle,				// ����p.
		SCREEN_ASPECT_RATIO,		// ��ʃA�X�y�N�g��.
		m_MinClippingDistance,		// �ŏ��`�拗��.
		m_MaxClippingDistance );	// �ő�`�拗��.

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