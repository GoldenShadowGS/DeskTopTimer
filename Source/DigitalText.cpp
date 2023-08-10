#include "PCH.h"
#include "DigitalText.h"
#include "Renderer.h"

void DigitalText::Init(ID2D1Factory2* pD2DFactory, ID2D1DeviceContext* dc, float size)
{
	m_Length = size / 2.0f;
	m_CornerLength = m_Length * 0.6129032f;
	m_Width = m_Length * 0.3870967f;

	//pulls in segment so there is a gap between them.
	const float pull = m_Length * 0.12f;
	const float length = m_Length - pull;
	const float cornerlength = m_CornerLength - pull;
	const float width = m_Width;
	ComPtr<ID2D1GeometrySink> Sink;
	HR(pD2DFactory->CreatePathGeometry(Geometry.ReleaseAndGetAddressOf()));
	HR(Geometry->Open(Sink.ReleaseAndGetAddressOf()));
	Sink->SetFillMode(D2D1_FILL_MODE_WINDING);
	D2D1_POINT_2F p1 = { -length, 0.0f };
	D2D1_POINT_2F p2 = { -cornerlength,-width };
	D2D1_POINT_2F p3 = { cornerlength  ,-width };
	D2D1_POINT_2F p4 = { length, 0.0f };
	D2D1_POINT_2F p5 = { cornerlength, width };
	D2D1_POINT_2F p6 = { -cornerlength, width };

	Sink->BeginFigure(p1, D2D1_FIGURE_BEGIN_FILLED);
	Sink->AddLine(p2);
	Sink->AddLine(p3);
	Sink->AddLine(p4);
	Sink->AddLine(p5);
	Sink->AddLine(p6);
	Sink->EndFigure(D2D1_FIGURE_END_CLOSED);
	HR(Sink->Close());

	ComPtr<ID2D1SolidColorBrush> SolidBlackBrush;
	HR(dc->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f), SolidBlackBrush.ReleaseAndGetAddressOf()));

	D2D1_SIZE_F BitmapSize = { GetDigitWidth() + 2.0f, GetDigitHeight() + 2.0f }; // Adding a 1 pixel border
	auto CreateBitmaps = [&] (BYTE digit, ID2D1SolidColorBrush* brush)
		{
			ComPtr<ID2D1BitmapRenderTarget> BitmapRenderTarget;
			ComPtr<ID2D1Bitmap> Bitmap;
			HR(dc->CreateCompatibleRenderTarget(BitmapSize, &BitmapRenderTarget));
			BitmapRenderTarget->BeginDraw();

			D2D1::Matrix3x2F transform = D2D1::Matrix3x2F::Translation(BitmapSize.width / 2.0f, BitmapSize.height / 2.0f);
			RasterizeDigit(BitmapRenderTarget.Get(), digit, transform, brush);

			HR(BitmapRenderTarget->EndDraw());
			HR(BitmapRenderTarget->GetBitmap(Bitmap.ReleaseAndGetAddressOf()));

			D2D1_BITMAP_BRUSH_PROPERTIES bitmapProperties = { D2D1_EXTEND_MODE_CLAMP,	D2D1_EXTEND_MODE_CLAMP,	D2D1_BITMAP_INTERPOLATION_MODE_LINEAR };
			D2D1_BRUSH_PROPERTIES brushProperties = { 1.0f, D2D1::Matrix3x2F::Translation(-1.0f, -1.0f) };
			ComPtr<ID2D1BitmapBrush> BMBrush;
			dc->CreateBitmapBrush(Bitmap.Get(), bitmapProperties, brushProperties, BMBrush.ReleaseAndGetAddressOf());
			return BMBrush;
		};
	for (int i = 0; i < 12; i++)
	{
		BitmapBrush[i] = CreateBitmaps(i, SolidBlackBrush.Get());
	}
}

void DigitalText::DrawDigits(ID2D1DeviceContext* dc, int* keyarray, const D2D1::Matrix3x2F& transform)
{
	static D2D1::Matrix3x2F translation[10] =
	{
		D2D1::Matrix3x2F::Identity(),
		translation[0] * D2D1::Matrix3x2F::Translation(GetDigitSpacing(), 0.0f),
		translation[1] * D2D1::Matrix3x2F::Translation(GetColonWidth(), 0.0f),
		translation[2] * D2D1::Matrix3x2F::Translation(GetColonWidth(), 0.0f),
		translation[3] * D2D1::Matrix3x2F::Translation(GetDigitSpacing(), 0.0f),
		translation[4] * D2D1::Matrix3x2F::Translation(GetColonWidth(), 0.0f),
		translation[5] * D2D1::Matrix3x2F::Translation(GetColonWidth(), 0.0f),
		translation[6] * D2D1::Matrix3x2F::Translation(GetDigitSpacing(), 0.0f),
		translation[7] * D2D1::Matrix3x2F::Translation(GetColonWidth(), 0.0f),
		D2D1::Matrix3x2F::Scale(0.5f, 0.5f) * translation[8] * D2D1::Matrix3x2F::Translation(GetColonWidth(), GetDigitHeight() / 2.0f)
	}; // Preset table of translation matrices for each digit

	static D2D1::Matrix3x2F StartingOffset[8] =
	{
		D2D1::Matrix3x2F::Identity(),
		StartingOffset[0] * D2D1::Matrix3x2F::Translation(-GetDigitSpacing(), 0.0f),
		StartingOffset[1] * D2D1::Matrix3x2F::Translation(-GetColonWidth(), 0.0f),
		StartingOffset[2] * D2D1::Matrix3x2F::Translation(-GetColonWidth(), 0.0f),
		StartingOffset[3] * D2D1::Matrix3x2F::Translation(-GetDigitSpacing(), 0.0f),
		StartingOffset[4] * D2D1::Matrix3x2F::Translation(-GetColonWidth(), 0.0f),
		StartingOffset[5] * D2D1::Matrix3x2F::Translation(-GetColonWidth(), 0.0f),
		StartingOffset[6] * D2D1::Matrix3x2F::Translation(-GetDigitSpacing(), 0.0f)
	}; // Preset table of translation matrices for each digit

	D2D1_RECT_F rect = D2D1::RectF(0.0f, 0.0f, GetDigitWidth(), GetDigitHeight());
	BYTE Digits[10] =
	{
		static_cast<BYTE>(keyarray[0]),
		static_cast<BYTE>(keyarray[1]),
		10,
		static_cast<BYTE>(keyarray[2]),
		static_cast<BYTE>(keyarray[3]),
		10,
		static_cast<BYTE>(keyarray[4]),
		static_cast<BYTE>(keyarray[5]),
		11,
		static_cast<BYTE>(keyarray[6]),
	};
	int startindex = 0;
	for (int i = 0; i < 5; i++)
	{
		if (keyarray[i])
		{
			break;
		}
		startindex++;
		if (startindex == 2 || startindex == 5 || startindex == 8)
			startindex++;
	}
	for (int i = startindex; i < 10; i++)
	{
		dc->SetTransform(translation[i] * StartingOffset[startindex] * transform);
		dc->FillRectangle(rect, BitmapBrush[Digits[i]].Get());
	}
}

float DigitalText::GetDigitSpacing()
{
	static float cachedresult = GetDigitWidth() * 1.1f;
	return cachedresult;
}

float DigitalText::GetDigitWidth()
{
	static float cachedresult = m_Length * 2.0f + m_Width * 2.0f;
	return cachedresult;
}

float DigitalText::GetDigitHeight()
{
	static float cachedresult = m_Length * 4.0f + m_Width * 2.0f;
	return cachedresult;
}

float DigitalText::GetColonWidth()
{
	static float cachedresult = GetDigitWidth() * 0.85f;
	return cachedresult;
}

void DigitalText::RasterizeDigit(ID2D1BitmapRenderTarget* bitmapRT, BYTE digit, const D2D1::Matrix3x2F& transform, ID2D1SolidColorBrush* brush)
{
	const float Offset = m_Length;
	const float bigoffset = Offset * 2.0f;

	const D2D1::Matrix3x2F transforms[7] =
	{
		D2D1::Matrix3x2F::Translation(0.0f, -bigoffset),
		D2D1::Matrix3x2F::Rotation(90, { 0.0f, 0.0f }) * D2D1::Matrix3x2F::Translation(-Offset, -Offset),
		D2D1::Matrix3x2F::Rotation(90, { 0.0f, 0.0f }) * D2D1::Matrix3x2F::Translation(Offset, -Offset),
		D2D1::Matrix3x2F::Translation(0.0f, 0.0f),
		D2D1::Matrix3x2F::Rotation(90, { 0.0f, 0.0f }) * D2D1::Matrix3x2F::Translation(-Offset, Offset),
		D2D1::Matrix3x2F::Rotation(90, { 0.0f, 0.0f }) * D2D1::Matrix3x2F::Translation(Offset, Offset),
		D2D1::Matrix3x2F::Translation(0.0f, bigoffset)
	};

	auto drawsegment = [&] (int segmentindex)
		{
			bitmapRT->SetTransform(transforms[segmentindex] * transform);
			bitmapRT->FillGeometry(Geometry.Get(), brush);
		};

	float dotradius = 5.0f;
	switch (digit)
	{
	case 10: // colon
	{
		D2D1_ELLIPSE ellipse = { {0.0f, -m_Length}, dotradius, dotradius };
		bitmapRT->SetTransform(transform);
		bitmapRT->FillEllipse(ellipse, brush);
		ellipse = { {0.0f, m_Length}, dotradius, dotradius };
		bitmapRT->FillEllipse(ellipse, brush);
		break;
	}
	case 11: // dot
	{
		D2D1_ELLIPSE ellipse = { {0.0f, m_Length * 2.0f}, dotradius, dotradius };
		bitmapRT->SetTransform(transform);
		bitmapRT->FillEllipse(ellipse, brush);
		break;
	}
	case 0:
		drawsegment(0);
		drawsegment(1);
		drawsegment(2);
		drawsegment(4);
		drawsegment(5);
		drawsegment(6);
		break;
	case 1:
		drawsegment(2);
		drawsegment(5);
		break;
	case 2:
		drawsegment(0);
		drawsegment(2);
		drawsegment(3);
		drawsegment(4);
		drawsegment(6);
		break;
	case 3:
		drawsegment(0);
		drawsegment(2);
		drawsegment(3);
		drawsegment(5);
		drawsegment(6);
		break;
	case 4:
		drawsegment(1);
		drawsegment(2);
		drawsegment(3);
		drawsegment(5);
		break;
	case 5:
		drawsegment(0);
		drawsegment(1);
		drawsegment(3);
		drawsegment(5);
		drawsegment(6);
		break;
	case 6:
		drawsegment(0);
		drawsegment(1);
		drawsegment(3);
		drawsegment(4);
		drawsegment(5);
		drawsegment(6);
		break;
	case 7:
		drawsegment(0);
		drawsegment(2);
		drawsegment(5);
		break;
	case 8:
		drawsegment(0);
		drawsegment(1);
		drawsegment(2);
		drawsegment(3);
		drawsegment(4);
		drawsegment(5);
		drawsegment(6);
		break;
	case 9:
		drawsegment(0);
		drawsegment(1);
		drawsegment(2);
		drawsegment(3);
		drawsegment(5);
		drawsegment(6);
		break;
	}
}
