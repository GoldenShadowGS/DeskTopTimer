#pragma once
#include "PCH.h"

class Renderer
{
public:
	void Init(HWND hWnd);
	ID2D1DeviceContext* GetDeviceContext() { return dc.Get(); }
	IDXGISwapChain1* GetSwapChain() { return swapChain.Get(); }
	ID2D1Factory2* GetFactory() { return d2Factory.Get(); }
	BOOL isValid() { BOOL valid = FALSE; dcompDevice->CheckDeviceState(&valid); return valid; }
private:
	ComPtr<ID3D11Device> direct3dDevice;
	ComPtr<IDXGIDevice> dxgiDevice;
	ComPtr<IDXGIFactory2> dxFactory;
	ComPtr<IDXGISwapChain1> swapChain;
	ComPtr<ID2D1Factory2> d2Factory;
	ComPtr<ID2D1Device1> d2Device;
	ComPtr<ID2D1DeviceContext> dc;
	ComPtr<IDXGISurface2> surface;
	ComPtr<ID2D1Bitmap1> bitmap;
	ComPtr<IDCompositionDevice> dcompDevice;
	ComPtr<IDCompositionTarget> target;
	ComPtr<IDCompositionVisual> visual;
};

struct ComException
{
	HRESULT result;
	ComException(HRESULT const value) : result(value)
	{}
};

inline void HR(HRESULT const result)
{
	if (S_OK != result)
	{
		throw ComException(result);
	}
}