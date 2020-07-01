#include "FrameRate.h"
#include <thread>
#include <chrono>

double CFrameRate::FRAME_TIME = 0.0;
double CFrameRate::m_OutFPS = 0.0;

CFrameRate::CFrameRate( double fps )
	: m_FrameTime	( 0.0 )
	, m_StartTime	()
	, m_NowTime		()
	, m_FreqTime	()
	, m_Count		( 0 )
	, m_SleepTime	( 0 )
{
	FRAME_TIME = 60;
	QueryPerformanceFrequency( &m_FreqTime );
	QueryPerformanceCounter( &m_StartTime );
}

CFrameRate::~CFrameRate()
{
}

void CFrameRate::Update()
{
	if( m_Count == static_cast<int>(FRAME_TIME) ){
		QueryPerformanceCounter( &m_NowTime );	// ���݂̎��Ԃ��擾.
		// 1000ms / ((���݂̎���ms - 1�t���[���ڂ̎���ms) / �t���[�� ).
		m_OutFPS = 1000.0 / 
			(static_cast<double>((m_NowTime.QuadPart - m_StartTime.QuadPart) * 
				1000 / m_FreqTime.QuadPart ) / FRAME_TIME);
		m_Count = 0;	// �J�E���g�̏�����.
		m_StartTime = m_NowTime;	// ���݂̎��Ԃ�ۑ�.
	}
	m_Count++;
}

void CFrameRate::Wait()
{
	// Sleep�����鎞�Ԃ̌v�Z.

    QueryPerformanceCounter( &m_NowTime );	// ���݂̎��Ԃ��擾.
    // Sleep�����鎞��ms = 1�t���[���ڂ��猻�݂̃t���[���܂ł̕`��ɂ�����ׂ�����ms - 
	//						1�t���[���ڂ��猻�݂̃t���[���܂Ŏ��ۂɂ�����������ms.
    //                   = (1000ms / 60)*�t���[���� - (���݂̎���ms - 1�t���[���ڂ̎���ms).
	m_SleepTime = static_cast<DWORD>((1000.0 / FRAME_TIME) * m_Count - 
		(m_NowTime.QuadPart - m_StartTime.QuadPart) * 1000 / m_FreqTime.QuadPart);
    if( m_SleepTime > 0 && m_SleepTime < 18 ){
		// �傫���ϓ����Ȃ����SleepTime��1�`17�̊Ԃɔ[�܂�.
        timeBeginPeriod(1);
        Sleep( m_SleepTime );
        timeEndPeriod(1);
    } else {
		// �傫���ϓ����������ꍇ.
        timeBeginPeriod(1);
        Sleep(1);
        timeEndPeriod(1);
    }
}