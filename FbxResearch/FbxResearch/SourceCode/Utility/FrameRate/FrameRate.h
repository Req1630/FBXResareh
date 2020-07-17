#ifndef FRAME_RATE_H
#define FRAME_RATE_H

#include <Windows.h>

class CFrameRate
{
public:
	CFrameRate( float fps = 60.0 );
	~CFrameRate();

	bool Update();
	void Wait();

private:
	float m_Rate;		// レート.
	DWORD m_SsyncOld;	// 過去時間.
	DWORD m_SyncNow;	// 現在時間.
};

#endif	// #ifndef FRAME_RATE_H.