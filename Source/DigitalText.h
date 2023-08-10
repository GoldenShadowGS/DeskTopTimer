#pragma once
#include "PCH.h"

class DigitalText
{
public:
	void Init(ID2D1Factory2* pD2DFactory, ID2D1DeviceContext* dc, float size);
	void DrawDigits(ID2D1DeviceContext* dc, int* keyarray, const D2D1::Matrix3x2F& transform);
	float GetDigitSpacing();
	float GetDigitWidth();
	float GetDigitHeight();
	float GetColonWidth();
private:
	void RasterizeDigit(ID2D1BitmapRenderTarget* bitmapRT, BYTE digit, const D2D1::Matrix3x2F& transform, ID2D1SolidColorBrush* brush);
	ComPtr<ID2D1PathGeometry> Geometry;
	ComPtr<ID2D1BitmapBrush> BitmapBrush[12]; // Holds the 10 digits and colon and dot
	float m_Length = 0.0f;
	float m_CornerLength = 0.0f;
	float m_Width = 0.0f;
};