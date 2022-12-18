#ifndef FRAME_RATE_H
#define FRAME_RATE_H

#include <Windows.h>

const double DEFAULT_FRAME_RATE = 60.0;	// �f�t�H���gFPS.

/********************************************
*	�t���[�����[�g�̌v���N���X.
**/
class CFrameRate
{
	const double FRAME_RATE;		// �t���[�����[�g.
	const double MIN_FRAME_TIME;	// �ŏ��t���[���^�C��.

	const double MIN_FRAME_RATE = 10.0;					// �Œ�t���[�����[�g.
	const double MAX_FRAME_TIME = 1.0 / MIN_FRAME_RATE;	// �ō��t���[���^�C��.

public:
	CFrameRate();
	CFrameRate( const double& fps = 60.0 );
	~CFrameRate();

	// �ҋ@�֐�.
	//	true �̏ꍇ���̃t���[�����I������.
	bool Wait();

	// FPS�̎擾.
	inline double GetFPS() const { return m_FPS; }
	// �f���^�^�C���̎擾.
	inline double GetDeltaTime() const { return m_FrameTime; }

private:
	double			m_FPS;			// ���݂�FPS.
	double			m_FrameTime;	// �t���[���^�C��(�f���^�^�C��).
	LARGE_INTEGER	m_StartTime;	// �J�n����.
	LARGE_INTEGER	m_NowTime;		// ���݂̎���.
	LARGE_INTEGER	m_FreqTime;		// �N����������.
};

#endif	// #ifndef FRAME_RATE_H.