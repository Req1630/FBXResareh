#ifndef LIGHT_H
#define LIGHT_H

#include "..\Global.h"
#include "..\Direct11\AlignedAllocationPolicy\AlignedAllocationPolicy.h"

class CLight : public AlignedAllocationPolicy<16>
{
	const float FLOAT_WND_W = static_cast<float>(WND_W);	// (float)��ʕ�.
	const float FLOAT_WND_H = static_cast<float>(WND_H);	// (float)��ʕ�.
	const float INITIAL_VIEWING_ANGLE	= static_cast<float>(3.141519/4.0);	// ��������p.
	const float SCREEN_ASPECT_RATIO		= 1.0f;			// ��ʃA�X�y�N�g��.
	const float INITIAL_MAX_CLIPPING_DISTANCE = 1000.0f;					// �����ő�`�拗��.
	const float INITIAL_MIN_CLIPPING_DISTANCE = 10.0f;						// �����ŏ��`�拗��.
	const DirectX::XMFLOAT3 VIEW_UP_VECTOR	= { 0.0f, 1.0f, 0.0f };	// ������x�N�g��.

public:
	CLight();
	~CLight();

	DirectX::XMMATRIX GetVP();
	DirectX::XMFLOAT4 GetDirection();

	// �J�������W�ݒ�֐�.
	void SetPosition( const DirectX::XMFLOAT3& vPos )			{ m_Position = vPos; }
	// �J�����������W�ݒ�֐�.
	void SetLookPosition( const DirectX::XMFLOAT3& vLookPos )	{ m_vLookPosition = vLookPos; }
private:
	DirectX::XMMATRIX m_ProjMatrix;
	DirectX::XMMATRIX m_ViewMatrix;
	DirectX::XMFLOAT3 m_Position;
	DirectX::XMFLOAT3 m_vLookPosition;

	float m_ViewingAngle;			// ����p.
	float m_MaxClippingDistance;	// �ő�N���b�s���O����.
	float m_MinClippingDistance;	// �ŏ��N���b�s���O����.
};

#endif	// #ifndef LIGHT_H.