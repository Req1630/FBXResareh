#pragma once

#include "FbxAnimationData.h"

class CFbxAnimationController
{
public:
	CFbxAnimationController();
	~CFbxAnimationController();

	// �t���[���̍X�V.
	void FrameUpdate();

	// �A�j���[�V�����f�[�^�̐ݒ�.
	void SetAnimDataList( const std::vector<SAnimationData>& animDataList );

	// ���̃A�j���[�V�����ɕύX.
	void ChangeNextAnimation();
	// �w��̃A�j���[�V�����ɕύX.
	void ChangeAnimation( int& animNumber );
	// �t���[�����̍s��擾.
	FbxMatrix GetFrameLinkMatrix( const int& meshNo, const int& i );
	// �t���[�����̍s��擾.
	FbxMatrix GetFrameMatrix( const int& meshNo );

	// �A�j���[�V�������x�̐ݒ�.
	void SetAnimSpeed( const double& speed ){ m_AnimSpeed = speed; }
	// �A�j���[�V�������x�̎擾.
	double GetAnimSpeed() const { return m_AnimSpeed; }

	// �A�j���[�V�����f�[�^�̒ǉ�.
	void AddAnimationData( const std::vector<SAnimationData>& animationData )
	{
		for( auto& a : animationData )
			m_AnimDataList.emplace_back( a ); 
	}

private:
	double m_AnimSpeed;							// �A�j���[�V�������x.
	std::vector<SAnimationData>	m_AnimDataList;	// �A�j���[�V�����f�[�^���X�g.
	SAnimationData	m_NowAnimation;				// ���݂̃A�j���[�V�����f�[�^.
	int m_NowAnimNumber;						// ���݂̃A�j���[�V�����ԍ�.
};