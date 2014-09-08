/*

Simple timer class used to calculaet time deltas using the high performance timer (basically the CPU clock it seems)
As Timegettime anmd other win32 functions only have about 5ms resolution, 
they're not really suitable for this kind of app.

*/

#pragma once

#include <windows.h>

class Timer
{
public:
	Timer(void);
	bool QuerySupport(void) const;
	bool SetTimeBase(void);
	float GetElapsedMsecs(void);
	bool HavePassed(float Msecs) const;

private:
	bool m_bSupported;
	LARGE_INTEGER m_iFrequency, m_iBaseCount; 
};
