#include "FbxAnimationController.h"

CFbxAnimationController::CFbxAnimationController()
	: m_AnimSpeed		( 0.0 )
	, m_NowAnimNumber	( 0 )
	, m_AnimDataList	()
	, m_NowAnimation	()
{
}

CFbxAnimationController::~CFbxAnimationController()
{
	m_AnimDataList.clear();
}

///////////////////////////////////////////////////.
// �t���[���̍X�V.
///////////////////////////////////////////////////.
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

///////////////////////////////////////////////////.
// �A�j���[�V�����f�[�^�̐ݒ�.
///////////////////////////////////////////////////.
void CFbxAnimationController::SetAnimDataList( const SAnimationDataList& animDataList )
{
	// �A�j���[�V�����f�[�^���󂾂�����I��.
	if( animDataList.AnimList.empty() == true ) return;
	m_AnimDataList = animDataList.AnimList;			// �A�j���[�V�����f�[�^�̐ݒ�.
	m_NowAnimation = m_AnimDataList[0];		// �A�j���[�V�����̍ŏ��̃f�[�^�����݂̃f�[�^�ɐݒ�.
	m_AnimSpeed = m_NowAnimation.AnimSpeed;	// �A�j���[�V�������x�̐ݒ�.
}

///////////////////////////////////////////////////.
// �A�j���[�V�����f�[�^�̒ǉ�.
///////////////////////////////////////////////////.
void CFbxAnimationController::AddAnimationData( const SAnimationDataList& animationData )
{
	// �A�j���[�V�������X�g���̃A�j���[�V�����ǉ�.
	for( auto& a : animationData.AnimList ) m_AnimDataList.emplace_back( a );
	// �A�j���[�V�����f�[�^����Ȃ�I��.
	if( m_AnimDataList.empty() == true ) return;
	m_NowAnimation = m_AnimDataList[0];		// �A�j���[�V�����̍ŏ��̃f�[�^�����݂̃f�[�^�ɐݒ�.
	m_AnimSpeed = m_NowAnimation.AnimSpeed;	// �A�j���[�V�������x�̐ݒ�.
}

///////////////////////////////////////////////////.
// ���̃A�j���[�V�����ɕύX.
///////////////////////////////////////////////////.
void CFbxAnimationController::ChangeNextAnimation()
{
	m_NowAnimNumber++;	// �A�j���[�V�����ԍ��̉��Z.
	ChangeAnimation( m_NowAnimNumber );	// �A�j���[�V�����̕ύX.
}

///////////////////////////////////////////////////.
// �w��̃A�j���[�V�����ɕύX.
///////////////////////////////////////////////////.
void CFbxAnimationController::ChangeAnimation( int& animNumber )
{
	// �A�j���[�V�����f�[�^����Ȃ�I��.
	if( m_AnimDataList.empty() == true ) return;
	// �A�j���[�V�����ԍ����A�j���[�V�������X�g��葽�����.
	// �A�j���[�V�������X�g�̏��߂���Đ�.
	if( animNumber >= (int)m_AnimDataList.size() ) animNumber = 0;
	m_NowAnimation = m_AnimDataList[animNumber];
	m_AnimSpeed = m_NowAnimation.AnimSpeed;
}

///////////////////////////////////////////////////.
// �t���[�����̍s��擾.
///////////////////////////////////////////////////.
bool CFbxAnimationController::GetFrameLinkMatrix( const int& meshNo, const int& i, FbxMatrix* outMatrix )
{
	return m_NowAnimation.GetFrameLinkMatrix( meshNo, i, m_NowAnimation.NowTime, outMatrix );
}