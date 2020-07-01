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
// �J�����̍X�V����.
//--------------------------------------------.
void CCamera::Update()
{
}

//--------------------------------------------.
// �r���[�s��A�v���W�F�N�V�����s��̏�����.
//--------------------------------------------.
void CCamera::InitViewProj()
{
	DirectX::XMVECTOR pos = 
		DirectX::XMVectorSet( m_vPosition.x, m_vPosition.y, m_vPosition.z, 1.0f );
	DirectX::XMVECTOR lookPos = 
		DirectX::XMVectorSet( m_vLookPosition.x, m_vLookPosition.y, m_vLookPosition.z, 1.0f );
	DirectX::XMVECTOR upVector = 
		DirectX::XMVectorSet( VIEW_UP_VECTOR.x, VIEW_UP_VECTOR.y, VIEW_UP_VECTOR.z, 1.0f );
	// �r���[(�J����)�ϊ�.
	m_ViewMatrix = DirectX::XMMatrixLookAtLH( 
		pos,		// �J�������W.
		lookPos,	// �J�����������W.
		upVector );	// �J�����x�N�g��.

	// �v���W�F�N�V����(�ˉe)�ϊ�.
	m_ProjMatrix = DirectX::XMMatrixPerspectiveFovLH(
		m_ViewingAngle,				// ����p.
		SCREEN_ASPECT_RATIO,		// ��ʃA�X�y�N�g��.
		m_MinClippingDistance,		// �ŏ��`�拗��.
		m_MaxClippingDistance );	// �ő�`�拗��.
	
}

//--------------------------------------------.
// �p����̃J�������̐ݒ�֐�.
//--------------------------------------------.
void CCamera::SetCamera( std::shared_ptr<CCamera> camera )
{
	this->m_vPosition		= camera->GetPosition();
	this->m_vLookPosition	= camera->GetLookPosition();
}