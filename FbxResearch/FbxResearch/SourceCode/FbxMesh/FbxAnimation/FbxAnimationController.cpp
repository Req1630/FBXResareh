#include "FbxAnimationController.h"

CFbxAnimationController::CFbxAnimationController()
{
}

CFbxAnimationController::~CFbxAnimationController()
{
}

// �t���[���̍X�V.
void CFbxAnimationController::FrameUpdate()
{
	// �A�j���[�V�����f�[�^��������ΏI��.
	if( m_AnimDataList.empty() == true ) return;
	if( m_AnimSpeed >= 0.0 ){
		// �o�ߎ��Ԃ����Z.
		m_NowAnimation.NowTime += m_AnimSpeed;
		// �o�ߎ��Ԃ��I�����Ă����.
		if( m_NowAnimation.NowTime >= m_NowAnimation.EndTime )
			m_NowAnimation.NowTime = m_NowAnimation.StartTime;	// �J�n���Ԃ�ݒ�.
	} else {
		// �o�ߎ��Ԃ����Z.
		m_NowAnimation.NowTime += m_AnimSpeed;
		// �o�ߎ��Ԃ��J�n���ԂȂ�.
		if( m_NowAnimation.NowTime <= m_NowAnimation.StartTime )
			m_NowAnimation.NowTime = m_NowAnimation.EndTime;	// �I�����Ԃ�ݒ�.
	}
}

// �A�j���[�V�����f�[�^�̐ݒ�.
void CFbxAnimationController::SetAnimDataList( const std::vector<SAnimationData>& animDataList )
{
	m_AnimDataList = animDataList;
	m_NowAnimation = m_AnimDataList[0];
	m_AnimSpeed = m_NowAnimation.AnimSpeed;
}

// ���̃A�j���[�V�����ɕύX.
void CFbxAnimationController::ChangeNextAnimation()
{
	m_NowAnimNumber++;	// �A�j���[�V�����ԍ��̉��Z.
	ChangeAnimation( m_NowAnimNumber );
}

// �w��̃A�j���[�V�����ɕύX.
void CFbxAnimationController::ChangeAnimation( int& animNumber )
{
	if( m_AnimDataList.empty() == true ) return;
	// �A�j���[�V�����ԍ����A�j���[�V�������X�g��葽�����.
	// �A�j���[�V�������X�g�̏��߂���Đ�.
	if( animNumber >= (int)m_AnimDataList.size() ) animNumber = 0;
	m_NowAnimation = m_AnimDataList[animNumber];
	m_AnimSpeed = m_NowAnimation.AnimSpeed;
}

FbxMatrix CFbxAnimationController::GetFrameLinkMatrix( const int& meshNo, const int& i )
{
	return m_NowAnimation.GetFrameLinkMatrix( meshNo, i, m_NowAnimation.NowTime );
}

FbxMatrix CFbxAnimationController::GetFrameMatrix( const int& meshNo )
{
	return m_NowAnimation.GetFrameMatrix( meshNo, m_NowAnimation.NowTime );
}