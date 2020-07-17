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
	float m_Rate;		// ���[�g.
	DWORD m_SsyncOld;	// �ߋ�����.
	DWORD m_SyncNow;	// ���ݎ���.
};

#endif	// #ifndef FRAME_RATE_H.