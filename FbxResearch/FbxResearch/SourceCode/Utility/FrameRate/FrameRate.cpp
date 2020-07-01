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
		QueryPerformanceCounter( &m_NowTime );	// 現在の時間を取得.
		// 1000ms / ((現在の時間ms - 1フレーム目の時間ms) / フレーム ).
		m_OutFPS = 1000.0 / 
			(static_cast<double>((m_NowTime.QuadPart - m_StartTime.QuadPart) * 
				1000 / m_FreqTime.QuadPart ) / FRAME_TIME);
		m_Count = 0;	// カウントの初期化.
		m_StartTime = m_NowTime;	// 現在の時間を保存.
	}
	m_Count++;
}

void CFrameRate::Wait()
{
	// Sleepさせる時間の計算.

    QueryPerformanceCounter( &m_NowTime );	// 現在の時間を取得.
    // Sleepさせる時間ms = 1フレーム目から現在のフレームまでの描画にかかるべき時間ms - 
	//						1フレーム目から現在のフレームまで実際にかかった時間ms.
    //                   = (1000ms / 60)*フレーム数 - (現在の時間ms - 1フレーム目の時間ms).
	m_SleepTime = static_cast<DWORD>((1000.0 / FRAME_TIME) * m_Count - 
		(m_NowTime.QuadPart - m_StartTime.QuadPart) * 1000 / m_FreqTime.QuadPart);
    if( m_SleepTime > 0 && m_SleepTime < 18 ){
		// 大きく変動がなければSleepTimeは1～17の間に納まる.
        timeBeginPeriod(1);
        Sleep( m_SleepTime );
        timeEndPeriod(1);
    } else {
		// 大きく変動があった場合.
        timeBeginPeriod(1);
        Sleep(1);
        timeEndPeriod(1);
    }
}