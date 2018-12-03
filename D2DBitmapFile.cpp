#include "StdAfx.h"
#include "StimServer.h"
#include "D2DBitmapFile.h"
#include "OutputWnd.h"


CD2DBitmapFile::CD2DBitmapFile(void)
{
}


CD2DBitmapFile::~CD2DBitmapFile(void)
{
}


bool CD2DBitmapFile::Init(LPCWSTR wzFilename)
{
	HRESULT hr = CoCreateInstance(
		CLSID_WICImagingFactory, nullptr,
		CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pWICfactory) );
	ASSERT(hr == S_OK);
	if (FAILED(hr)) return false;
	hr = m_pWICfactory->CreateDecoderFromFilename(
		wzFilename,
		nullptr,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad,
		&m_pDecoder);
	if (FAILED(hr))
	{
		CString errString = _T("Error reading image file: ");
		errString += wzFilename;
		switch (hr)
		{
		case 0x80070003:
			errString += "\r\n Cannot find the path specified.";
			break;
		case 0x8007052E:
			errString += "\r\n Logon Failure: unknown user name or bad password.";
			break;
		case 0x88982F50:
			errString += "\r\n Codec not found.";
			break;
		default:
			COutputList::AddString(errString);
			errString.Format(_T(" Error Code (hResult): %X"), hr);
		}
		COutputList::AddString(errString);
		m_pWICfactory->Release();
		return false;
	}
	return true;
}


HRESULT CD2DBitmapFile::GetFrameCount(UINT32* nFrames)
{
	return m_pDecoder->GetFrameCount(nFrames);
}


HRESULT CD2DBitmapFile::LoadBitmapFile(UINT32 iFrame, ID2D1Bitmap** pBitmap)
{
	IWICFormatConverter *pConverter;
	IWICBitmapFrameDecode *pBitmapSource;
	HRESULT hr = m_pDecoder->GetFrame(iFrame, &pBitmapSource);
	ASSERT(hr == S_OK);
	if (!SUCCEEDED(hr)) return hr;

	// Convert the image format to 32bppPBGRA
	// (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
	hr = m_pWICfactory->CreateFormatConverter(&pConverter);
	ASSERT(hr == S_OK);
	if (!SUCCEEDED(hr)) return hr;

	hr = pConverter->Initialize(
		pBitmapSource,
		GUID_WICPixelFormat32bppPBGRA,
		WICBitmapDitherTypeNone,
		nullptr,
		0.f,
		WICBitmapPaletteTypeMedianCut);
	ASSERT(hr == S_OK);
	if (!SUCCEEDED(hr)) return hr;

	// Create a Direct2D bitmap from the WIC bitmap.
	hr = theApp.m_pContext->CreateBitmapFromWicBitmap(
		pConverter,
		pBitmap);
	pConverter->Release();
	pBitmapSource->Release();
	return hr;
}

void CD2DBitmapFile::Cleanup(void)
{
	m_pDecoder->Release();
	m_pWICfactory->Release();
}
