#include "PCH.h"
#include "Renderer.h"
#include "Timer.h"
#include "DigitalText.h"
#include "Sound.h"

INT64 GetTimeFromDigitArray(int* array);

class Application
{
public:
	BOOL Init(HINSTANCE hInstance);
	int Run();
	void Update();
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
private:
	void KeyPressed(int key);
	void SubmitDigits();
	void RetrieveDigits(INT64 ms);
	void ToggleTimer();
	void EndAlarm();
	LRESULT CALLBACK InternalWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	HINSTANCE hInst = nullptr;
	HWND hWindow = nullptr;
	int m_WindowWidth = 460;
	int m_WindowHeight = 100;
	int m_Lastmousex = 0;
	int m_Lastmousey = 0;
	BOOL m_MouseinWindow = FALSE;
	BOOL m_AlarmSet = FALSE;
	Renderer m_Renderer;
	Timer m_Timer;
	SoundManager m_SoundManager;
	WavFile Alarm;
	WavFile StartSound;
	WavFile StopSound;
	WavFile ClickSound;
	WavFile SmallClickSound;
	UINT_PTR AlarmLoopIntervalID = {};
	ComPtr<ID2D1SolidColorBrush> BKGBrush;
	ComPtr<ID2D1SolidColorBrush> BlackBrush;
	DigitalText m_DigitalText;
	inline BOOL Insert(int value)
	{
		if (!keyarray[0] && !keyarray[1])
		{
			for (int i = 2; i < 6; i++)
			{
				int destIndex = i - 1;
				keyarray[destIndex] = keyarray[i];
			}
			keyarray[5] = value;
			keyarray[6] = 0;
			return TRUE;
		}
		return FALSE;
	}
	inline BOOL BackSpace()
	{
		BOOL result = FALSE;
		for (int i = 0; i < 6; i++)
		{
			if (keyarray[i])
				result = TRUE;
		}
		for (int i = 4; i >= 0; i--)
		{
			int destIndex = i + 1;
			keyarray[destIndex] = keyarray[i];
		}
		keyarray[0] = 0;
		keyarray[6] = 0;
		return result;
	}
	inline void Clear()
	{
		for (int i = 0; i < 7; i++)
		{
			keyarray[i] = 0;
		}
	}
	int keyarray[7] = {};
};