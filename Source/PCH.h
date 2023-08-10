#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <windowsx.h>

#include <xaudio2.h>

#include <wrl.h>

#include <dxgi1_3.h>
#include <d3d11_2.h>
#include <d2d1_2.h>
#include <d2d1_2helper.h>
#include <dcomp.h>

#include <vector>
#include <string>
#include <chrono>
#include <fstream>
#include <thread>
#include <atomic>
#include <assert.h>
using namespace Microsoft::WRL;
