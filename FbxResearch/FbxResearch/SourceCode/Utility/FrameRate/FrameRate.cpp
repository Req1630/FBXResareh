#include "FrameRate.h"
#include <thread>
#include <chrono>

#pragma comment( lib, "winmm.lib" )

CFrameRate::CFrameRate()
	: CFrameRate	( DEFAULT_FRAME_RATE )
{
}

CFrameRate::CFrameRate( const double& fps )
	: FRAME_RATE		( fps )
	, MIN_FRAME_TIME	( 1.0 / fps )
	, m_FPS				( fps )
	, m_FrameTime		( 0.0 )
	, m_StartTime		()
	, m_NowTime			()
	, m_FreqTime		()
{
	QueryPerformanceFrequency( &m_FreqTime );
	QueryPerformanceCounter( &m_StartTime );
}

CFrameRate::~CFrameRate()
{
}

// 待機関数.
bool CFrameRate::Wait()
{
	// 現在の時間を取得.
	QueryPerformanceCounter( &m_NowTime );
	// (今の時間 - 前フレームの時間) / 周波数 = 経過時間(秒単位).
	m_FrameTime =
		(static_cast<double>(m_NowTime.QuadPart) - static_cast<double>(m_StartTime.QuadPart)) / 
		static_cast<double>(m_FreqTime.QuadPart);

	// 処理時間に余裕がある場合はその分待機.
	if( m_FrameTime < MIN_FRAME_TIME ){
		// 待機時間を計算.
		DWORD sleepTime = static_cast<DWORD>((MIN_FRAME_TIME - m_FrameTime) * 1000.0);

		timeBeginPeriod(1);
		Sleep( sleepTime );
		timeEndPeriod(1); 

		return true;
	}

	if( m_FrameTime > 0.0 ) m_FPS = (m_FPS*0.99) + (0.01/m_FrameTime);
	if( m_FrameTime > MAX_FRAME_TIME ) m_FrameTime = MAX_FRAME_TIME;

	m_StartTime = m_NowTime;

	return false;
}