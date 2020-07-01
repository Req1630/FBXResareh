#ifndef FRAME_RATE_H
#define FRAME_RATE_H

#include <Windows.h>

class CFrameRate
{
	static double FRAME_TIME;
public:
	CFrameRate( double fps = 60.0 );
	~CFrameRate();

	void Update();
	void Wait();

	static double GetFrameTime(){ return m_OutFPS; }
	static double GetFPS(){ return FRAME_TIME; }

private:

private:
	static double	m_OutFPS;
	double			m_FrameTime;
	LARGE_INTEGER	m_StartTime;
	LARGE_INTEGER	m_NowTime;
	LARGE_INTEGER	m_FreqTime;
	int		m_Count;
	DWORD	m_SleepTime;
};

#endif	// #ifndef FRAME_RATE_H.