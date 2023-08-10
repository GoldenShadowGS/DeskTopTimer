#pragma once
#include "PCH.h"

class Timer
{
	class ThreadSafeTimer
	{
	public:
		void Reset()
		{
			const std::lock_guard<std::mutex> lock(m_Mutex);
			m_Duration = 0;
		}
		void Start()
		{
			const std::lock_guard<std::mutex> lock(m_Mutex);
			start = std::chrono::steady_clock::now();
		}
		void Stop()
		{
			const std::lock_guard<std::mutex> lock(m_Mutex);
			m_Duration += GetActiveDuration();
		}
		INT64 GetTime(BOOL isStarted)
		{
			const std::lock_guard<std::mutex> lock(m_Mutex);
			if (isStarted)
				return m_Duration + GetActiveDuration();
			else
				return m_Duration;
		}
	private:
		INT64 GetActiveDuration()
		{
			std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
			return std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
		}
		std::mutex m_Mutex;
		std::chrono::time_point<std::chrono::steady_clock> start;
		INT64 m_Duration = 0;
	};
public:
	Timer();
	~Timer();
	void Start();
	BOOL isStarted() { return m_bStarted; }
	void Stop();
	void Reset();
	INT64 GetRemainingTime();
	void SetAlarm(INT64 time);
	void SetAppWindow(HWND hwnd) { AppWindow = hwnd; }
private:
	void ReDraw();
	void ReDrawSync(); // Thread Function
	ThreadSafeTimer m_ThreadSafeTimer;
	HWND AppWindow = nullptr;
	INT64 m_AlarmTime = 0;
	HANDLE m_EventHandle = {};
	std::jthread m_ClockThread;
	std::atomic<BOOL> m_bStarted = FALSE;
	std::atomic<BOOL> m_bTimerThreadRunning = FALSE;
};
