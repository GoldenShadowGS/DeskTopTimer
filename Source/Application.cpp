#include "PCH.h"
#include "Application.h"
#include "Resource.h"

const WCHAR* gTitle = L"Desktop Timer";
const WCHAR* gWindowClass = L"MainWindowClass";

static ATOM RegisterWindowClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex = {};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = Application::WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wcex.lpszMenuName = 0;
	wcex.lpszClassName = gWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALLICON));
	return RegisterClassExW(&wcex);
}

BOOL Application::Init(HINSTANCE hInstance)
{
	hInst = hInstance;
	RegisterWindowClass(hInstance);

	const DWORD style = WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX;  // WS_POPUP
	const DWORD exstyle = WS_EX_NOREDIRECTIONBITMAP;

	RECT winRect = { 0, 0, m_WindowWidth, m_WindowHeight };
	AdjustWindowRectEx(&winRect, style, false, exstyle);

	hWindow = CreateWindowExW(exstyle, gWindowClass, gTitle, style, CW_USEDEFAULT, 0,
		winRect.right - winRect.left, winRect.bottom - winRect.top, nullptr, nullptr, hInst, this);

	if (!hWindow)
		return FALSE;

	m_Timer.SetAppWindow(hWindow);
	//Sound Init
	m_SoundManager.Init(8); // channels
	Alarm.Load(SOUND_ALARM);
	StartSound.Load(SOUND_START);
	StopSound.Load(SOUND_STOP);
	ClickSound.Load(SOUND_CLICK);
	SmallClickSound.Load(SOUND_SMALLCLICK);

	// Graphics Resource Init
	m_Renderer.Init(hWindow);
	ID2D1Factory2* factory = m_Renderer.GetFactory();
	ID2D1DeviceContext* dc = m_Renderer.GetDeviceContext();
	HR(dc->CreateSolidColorBrush(D2D1::ColorF(0.79f, 0.75f, 0.75f, 1.0f), BKGBrush.ReleaseAndGetAddressOf()));
	HR(dc->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f), BlackBrush.ReleaseAndGetAddressOf()));
	m_DigitalText.Init(factory, dc, 32);

	ShowWindow(hWindow, SW_SHOW);
	UpdateWindow(hWindow);

	return TRUE;
}


int Application::Run()
{
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}

void Application::Update()
{
	if (m_AlarmSet && !m_Timer.GetRemainingTime())
	{
		SetWindowPos(hWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		m_AlarmSet = FALSE;
		FlashWindow(hWindow, TRUE);
		AlarmLoopIntervalID = SetTimer(hWindow, TIMER_LOOPINTERVAL, 1200, nullptr);
		m_SoundManager.Play(Alarm, 1.0f, 1.0f);
	}

	if (m_Timer.isStarted())
		RetrieveDigits(m_Timer.GetRemainingTime());

	// Skip Drawing if Window is Minimized
	if (!IsIconic(hWindow))
	{
		ID2D1DeviceContext* dc = m_Renderer.GetDeviceContext();
		dc->BeginDraw();

		D2D1::ColorF backGroundColor = D2D1::ColorF(0.8f, 0.8f, 0.8f, 1.0f);

		dc->Clear(backGroundColor);
		dc->SetTransform(D2D1::Matrix3x2F::Identity());

		D2D1::Matrix3x2F translationdown = D2D1::Matrix3x2F::Translation(0.0f, m_DigitalText.GetDigitHeight());
		D2D1::Matrix3x2F skew = D2D1::Matrix3x2F::Skew(-10.0f, 0.0f);
		D2D1::Matrix3x2F translationup = D2D1::Matrix3x2F::Translation(32.0f, -m_DigitalText.GetDigitHeight() * 1.5f + m_WindowHeight * 0.5f);
		D2D1::Matrix3x2F transform = translationup * skew * translationdown;

		m_DigitalText.DrawDigits(dc, keyarray, transform);

		HR(dc->EndDraw());
		HR(m_Renderer.GetSwapChain()->Present(1, 0));
	}

	// Update Window Title
	if (m_Timer.GetRemainingTime())
	{
		int hours = keyarray[0] * 10 + keyarray[1];
		int minutes1 = keyarray[2];
		int minutes2 = keyarray[3];
		int seconds1 = keyarray[4];
		int seconds2 = keyarray[5];
		int tenths = keyarray[6];
		WCHAR buffer[64];
		if (hours)
			swprintf_s(buffer, 64, L"%i:%i%i:%i%i.%i\n", hours, minutes1, minutes2, seconds1, seconds2, tenths);
		else if (minutes1)
			swprintf_s(buffer, 64, L"%i%i:%i%i.%i\n", minutes1, minutes2, seconds1, seconds2, tenths);
		else if (minutes2)
			swprintf_s(buffer, 64, L"%i:%i%i.%i\n", minutes2, seconds1, seconds2, tenths);
		else if (seconds1)
			swprintf_s(buffer, 64, L"%i%i.%i\n", seconds1, seconds2, tenths);
		else
			swprintf_s(buffer, 64, L"%i.%i\n", seconds2, tenths);
		SetWindowTextW(hWindow, buffer);
	}
	else if (AlarmLoopIntervalID)
	{
		SetWindowTextW(hWindow, L"ALARM");
	}
	else
		SetWindowTextW(hWindow, gTitle);
}

void Application::SubmitDigits()
{
	m_Timer.Reset();
	m_Timer.SetAlarm(GetTimeFromDigitArray(keyarray));
}

void Application::RetrieveDigits(INT64 ms)
{
	INT64 tenths = (ms / 100);
	INT64 Seconds = (ms / 1000) % 60;
	INT64 Minutes = (ms / 60000) % 60;
	INT64 Hours = (ms / 3600000);
	keyarray[0] = (Hours / 10) % 10;
	keyarray[1] = Hours % 10;
	keyarray[2] = (Minutes / 10) % 10;
	keyarray[3] = Minutes % 10;
	keyarray[4] = (Seconds / 10) % 10;
	keyarray[5] = Seconds % 10;
	keyarray[6] = tenths % 10;
}

void Application::KeyPressed(int key)
{
	if (!m_Timer.isStarted())
	{
		if (key >= '0' && key <= '9')
		{
			if (Insert(key - 48))
				m_SoundManager.Play(SmallClickSound, 1.0f, 1.0f);
			SubmitDigits();
		}
		else if (key == 8) // 8 BackSpace
		{
			if (BackSpace())
				m_SoundManager.Play(SmallClickSound, 1.0f, 1.0f);
			SubmitDigits();
		}
		Update();
	}
	if (key == 13 || key == 32) // Enter
	{
		ToggleTimer();
		Update();
	}
}

void Application::ToggleTimer()
{
	if (m_Timer.GetRemainingTime())
	{
		if (!m_Timer.isStarted())
		{
			m_SoundManager.Play(StartSound, 1.0f, 1.0f);
			m_Timer.Start();
			m_AlarmSet = TRUE;
		}
		else
		{
			m_SoundManager.Play(StopSound, 1.0f, 1.0f);
			m_Timer.Stop();
			m_AlarmSet = FALSE;
		}
	}
	Update();
}

void Application::EndAlarm()
{
	if (AlarmLoopIntervalID)
	{
		KillTimer(hWindow, AlarmLoopIntervalID);
		AlarmLoopIntervalID = {};
		SetWindowPos(hWindow, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}
}

LRESULT CALLBACK Application::InternalWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int MouseDelay = 0;
	switch (message)
	{
	case WM_PAINT:
	{
		Update();
		PAINTSTRUCT ps;
		BeginPaint(hWindow, &ps);
		EndPaint(hWindow, &ps);
	}
	break;
	case WM_KILLFOCUS:
	case WM_MOUSELEAVE:
	{
		m_MouseinWindow = FALSE;
		RECT rc;
		GetClientRect(hWnd, &rc);
		InvalidateRect(hWnd, &rc, TRUE);
	}
	break;
	case WM_LBUTTONDOWN:
	{
		if (AlarmLoopIntervalID)
		{
			EndAlarm();
		}
		else
			ToggleTimer();
		Update();

	}
	break;
	case WM_KEYDOWN:
	{
		if (AlarmLoopIntervalID)
		{
			EndAlarm();
			Update();
		}
		else if (wParam == VK_DELETE && !m_Timer.isStarted())
		{
			if (m_Timer.GetRemainingTime())
				m_SoundManager.Play(ClickSound, 1.0f, 1.0f);
			Clear();
			SubmitDigits();
			Update();
		}
	}
	break;
	case WM_CHAR:
	{
		KeyPressed((int)wParam);
	}
	break;
	case WM_APP + 1:
	{
		Update();
	}
	break;
	case WM_TIMER:
	{
		m_SoundManager.Play(Alarm, 1.0f, 1.0f);
	}
	break;
	case WM_WINDOWPOSCHANGING:
	{
		if (AlarmLoopIntervalID)
			EndAlarm();
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

LRESULT CALLBACK Application::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static Application* app = nullptr;
	if (app)
		return app->InternalWndProc(hWnd, message, wParam, lParam);
	else
	{
		if (message == WM_CREATE)
		{
			app = reinterpret_cast<Application*>(((CREATESTRUCTW*)lParam)->lpCreateParams);
			return 0;
		}
		else
		{
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
}

INT64 GetTimeFromDigitArray(int* array)
{
	INT64 result = 0;
	result += array[0] * 36000000LL;
	result += array[1] * 3600000LL;
	result += array[2] * 600000LL;
	result += array[3] * 60000LL;
	result += array[4] * 10000LL;
	result += array[5] * 1000LL;
	return result;
}
