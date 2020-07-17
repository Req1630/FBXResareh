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
	float m_Rate;		// ƒŒ[ƒg.
	DWORD m_SsyncOld;	// ‰ß‹ŠÔ.
	DWORD m_SyncNow;	// Œ»İŠÔ.
};

#endif	// #ifndef FRAME_RATE_H.