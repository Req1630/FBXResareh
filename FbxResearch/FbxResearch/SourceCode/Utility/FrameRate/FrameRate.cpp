#include "FrameRate.h"
#include <thread>
#include <chrono>

CFrameRate::CFrameRate( float fps )
	: m_Rate		( 1000.0f / fps )
	, m_SsyncOld	( 0 )
	, m_SyncNow		( 0 )
{
	m_SsyncOld = timeGetTime();
	// ���ԏ����̂��߁A�ŏ��P�ʂ�1�~���b�ɕύX.
	timeBeginPeriod( 1 );
}

CFrameRate::~CFrameRate()
{
}

bool CFrameRate::Update()
{
	if( m_SyncNow - m_SsyncOld < m_Rate )
	{
		return false;
	}
	m_SsyncOld = m_SyncNow;	// ���ݎ��Ԃɒu������.

	return true;
}

void CFrameRate::Wait()
{
	Sleep( 1 );
	m_SyncNow = timeGetTime();	//���݂̎��Ԃ��擾.
}