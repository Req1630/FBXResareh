#ifndef FRAME_RATE_H
#define FRAME_RATE_H

#include <Windows.h>

const double DEFAULT_FRAME_RATE = 60.0;	// デフォルトFPS.

/********************************************
*	フレームレートの計測クラス.
**/
class CFrameRate
{
	const double FRAME_RATE;		// フレームレート.
	const double MIN_FRAME_TIME;	// 最小フレームタイム.

	const double MIN_FRAME_RATE = 10.0;					// 最低フレームレート.
	const double MAX_FRAME_TIME = 1.0 / MIN_FRAME_RATE;	// 最高フレームタイム.

public:
	CFrameRate();
	CFrameRate( const double& fps = 60.0 );
	~CFrameRate();

	// 待機関数.
	//	true の場合そのフレームを終了する.
	bool Wait();

	// FPSの取得.
	inline double GetFPS() const { return m_FPS; }
	// デルタタイムの取得.
	inline double GetDeltaTime() const { return m_FrameTime; }

private:
	double			m_FPS;			// 現在のFPS.
	double			m_FrameTime;	// フレームタイム(デルタタイム).
	LARGE_INTEGER	m_StartTime;	// 開始時間.
	LARGE_INTEGER	m_NowTime;		// 現在の時間.
	LARGE_INTEGER	m_FreqTime;		// 起動した時間.
};

#endif	// #ifndef FRAME_RATE_H.