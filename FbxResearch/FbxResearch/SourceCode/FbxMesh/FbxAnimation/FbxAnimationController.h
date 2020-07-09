#pragma once

#include "FbxAnimationData.h"

/***************************************************************************
	�A�j���[�V�����𑀍삷��N���X.
----------------------------------------------------------------------------
*	��   ��Ɏg�p����֐�   ��.
*	���̃A�j���[�V�����ɕύX.
*	void ChangeNextAnimation().
*
----------------------------------------------------------------------------
*	�w��̃A�j���[�V�����ɕύX.
*	index���A�j���[�V�������X�g��葽����΁A
*	���X�g�̏��߂���Đ�.
*	void ChangeAnimation( int& index ).
*
----------------------------------------------------------------------------
*	�A�j���[�V�������x�̐ݒ�.
*	void SetAnimSpeed( const double& speed ).
*
----------------------------------------------------------------------------
*	���݂̃A�j���[�V�������x�̎擾.
*	double GetAnimSpeed().
*
***/
class CFbxAnimationController
{
public:
	CFbxAnimationController();
	~CFbxAnimationController();

	//-----------------------------------------.
	// �t���[���̍X�V.
	//-----------------------------------------.
	void FrameUpdate();

	//-----------------------------------------.
	//	�A�j���[�V�����f�[�^�̐ݒ�.
	//-----------------------------------------.

	// �A�j���[�V�����f�[�^�̐ݒ�.
	void SetAnimDataList( const SAnimationDataList& animDataList );
	// �A�j���[�V�����f�[�^�̒ǉ�.
	void AddAnimationData( const SAnimationDataList& animationData );

	//-----------------------------------------.
	//	�A�j���[�V�����̃p�����[�^�̐ݒ�E�擾.
	//-----------------------------------------.

	// ���̃A�j���[�V�����ɕύX.
	void ChangeNextAnimation();
	// �w��̃A�j���[�V�����ɕύX.
	void ChangeAnimation( int& animNumber );

	// �A�j���[�V�������x�̐ݒ�.
	void SetAnimSpeed( const double& speed ){ m_AnimSpeed = speed; }
	// �A�j���[�V�������x�̎擾.
	double GetAnimSpeed() const { return m_AnimSpeed; }

	// �t���[�����̍s��擾.
	bool GetFrameLinkMatrix( const int& meshNo, const int& i, FbxMatrix* outMatrix );

private:
	double m_AnimSpeed;							// �A�j���[�V�������x.
	int m_NowAnimNumber;						// ���݂̃A�j���[�V�����ԍ�.
	std::vector<SAnimationData>	m_AnimDataList;	// �A�j���[�V�����f�[�^���X�g.
	SAnimationData	m_NowAnimation;				// ���݂̃A�j���[�V�����f�[�^.
};