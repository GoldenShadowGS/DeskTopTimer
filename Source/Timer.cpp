#include "PCH.h"
#include "Timer.h"
#include "Resource.h"

Timer::Timer()
{
	m_EventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (m_EventHandle == INVALID_HANDLE_VALUE)
		throw std::exception("Invalid Event Handle");
}

Timer::~Timer()
{
	Stop();
	if (m_EventHandle)
		CloseHandle(m_EventHandle);
}

void Timer::Start()
{
	if (!m_bStarted)
	{
		m_ThreadSafeTimer.Start();
		m_bStarted = TRUE;
		if (!m_bTimerThreadRunning)
		{
			m_bTimerThreadRunning = TRUE;
			m_ClockThread = std::jthread(&Timer::ReDrawSync, this);
			// Spawns a thread to synchronize drawing to tenth of a second
		}
	}
}

void Timer::Stop()
{
	if (m_bStarted)
	{
		m_bStarted = FALSE;
		SetEvent(m_EventHandle); // Trigger Event to Kill Waiting Thread
		m_ThreadSafeTimer.Stop();
	}
}

void Timer::Reset()
{
	Stop();
	m_ThreadSafeTimer.Reset();
}

INT64 Timer::GetRemainingTime()
{
	INT64 timeremaining = m_AlarmTime - m_ThreadSafeTimer.GetTime(m_bStarted);
	if (timeremaining > 0)
	{
		return timeremaining;
	}
	else
	{
		Stop();
		return 0;
	}
}

void Timer::SetAlarm(INT64 ms)
{
	m_AlarmTime = ms;
}

void Timer::ReDraw()
{
	if (AppWindow)
	{
		PostMessageA(AppWindow, WM_APP + 1, 0, 0);
		//RECT rc;
		//GetClientRect(AppWindow, &rc);
		//InvalidateRect(AppWindow, &rc, TRUE);
	}
}

void Timer::ReDrawSync()
{
	const INT64 SyncInterval = 100; // 100 milliseconds
	while (m_bStarted)
	{
		if (m_bStarted)
		{
			// Syncs redraws to tenths of a second intervals
			INT64 CurrentTime = m_ThreadSafeTimer.GetTime(m_bStarted) - 50;
			INT64 difference = CurrentTime - ((CurrentTime / SyncInterval) * SyncInterval);
			INT64 sleeptime = SyncInterval - difference;
			WaitForSingleObject(m_EventHandle, (UINT)sleeptime);
			ReDraw();
		}
	}
	m_bTimerThreadRunning = FALSE;
}
