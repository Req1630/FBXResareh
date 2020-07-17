#include "FrameRate.h"
#include <thread>
#include <chrono>

CFrameRate::CFrameRate( float fps )
	: m_Rate		( 1000.0f / fps )
	, m_SsyncOld	( 0 )
	, m_SyncNow		( 0 )
{
	m_SsyncOld = timeGetTime();
	// 時間処理のため、最小単位を1ミリ秒に変更.
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
	m_SsyncOld = m_SyncNow;	// 現在時間に置き換え.

	return true;
}

void CFrameRate::Wait()
{
	Sleep( 1 );
	m_SyncNow = timeGetTime();	//現在の時間を取得.
}