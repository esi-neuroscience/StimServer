#include "StdAfx.h"
#include "StimServer.h"
#include "OutputWnd.h"

bool LoadD2DBitmap(LPCWSTR wzFilename, ID2D1Bitmap** pBitmap)
{
	HRESULT hr = S_OK;
	IWICBitmapDecoder *decoder;
	IWICBitmapFrameDecode *bitmapSource;
	//    IWICStream *stream;
	IWICFormatConverter *converter;
	//    IWICBitmapScaler *scaler;
	IWICImagingFactory *wicFactory;

	hr = CoCreateInstance(
		CLSID_WICImagingFactory, nullptr,
		CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wicFactory) );
	ASSERT(hr == S_OK);
	if (FAILED(hr)) return false;
	hr = wicFactory->CreateDecoderFromFilename(
		wzFilename,
		nullptr,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad,
		&decoder);
	if (FAILED(hr)) {
		CString errString = _T("Error reading image file: ");
		errString += wzFilename;
		switch (hr) {
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
		return false;
	}
	TRACE("Pic Init post decoder\n");

	// Create the initial frame.
	hr = decoder->GetFrame(0, &bitmapSource);
	ASSERT(hr == S_OK);
	decoder->Release();

	if (SUCCEEDED(hr))
	{
		// Convert the image format to 32bppPBGRA
		// (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
		hr = wicFactory->CreateFormatConverter(&converter);
		ASSERT(hr == S_OK);
		wicFactory->Release();
	}

	if (SUCCEEDED(hr))
	{
		hr = converter->Initialize(
			bitmapSource,
			GUID_WICPixelFormat32bppPBGRA,
			WICBitmapDitherTypeNone,
			nullptr,
			0.f,
			WICBitmapPaletteTypeMedianCut);
		ASSERT(hr == S_OK);
		//        }
		bitmapSource->Release();
		TRACE("Bitmap created\n");
	}
	if (SUCCEEDED(hr))
	{
		// Create a Direct2D bitmap from the WIC bitmap.
		hr = theApp.m_pContext->CreateBitmapFromWicBitmap(
			converter,
			pBitmap);
		// handle D2DERR_RECREATE_TARGET
		ASSERT(hr == S_OK);
		converter->Release();
	}
	return SUCCEEDED(hr);
}


bool LoadD2DBitmaps(LPCWSTR wzFilename, ID2D1Bitmap** pBitmaps)
{
	HRESULT hr = S_OK;
	IWICBitmapDecoder *decoder;
	IWICBitmapFrameDecode *bitmapSource;
	//    IWICStream *stream;
	IWICFormatConverter *converter;
	//    IWICBitmapScaler *scaler;
	IWICImagingFactory *wicFactory;

	hr = CoCreateInstance(
		CLSID_WICImagingFactory, nullptr,
		CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wicFactory) );
	ASSERT(hr == S_OK);
	if (FAILED(hr)) return false;
	hr = wicFactory->CreateDecoderFromFilename(
		wzFilename,
		nullptr,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad,
		&decoder);
	if (FAILED(hr)) {
		CString errString = _T("Error reading image file: ");
		errString += wzFilename;
		switch (hr) {
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
		return false;
	}
	TRACE("Pics Init post decoder\n");

	UINT nFrames;
	hr = decoder->GetFrameCount(&nFrames);
	ASSERT(hr == S_OK);
	TRACE("Number of Frames: %u\n", nFrames);
	// Create the initial frame.
	hr = decoder->GetFrame(0, &bitmapSource);
	ASSERT(hr == S_OK);
	decoder->Release();

	if (SUCCEEDED(hr))
	{
		// Convert the image format to 32bppPBGRA
		// (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
		hr = wicFactory->CreateFormatConverter(&converter);
		ASSERT(hr == S_OK);
		wicFactory->Release();
	}

	if (SUCCEEDED(hr))
	{
		// If a new width or height was specified, create an
		// IWICBitmapScaler and use it to resize the image.
		/*
		if (destinationWidth != 0 || destinationHeight != 0)
		{
		unsigned int originalWidth, originalHeight;
		hr = bitmapSource->GetSize(&originalWidth, &originalHeight);
		if (SUCCEEDED(hr))
		{
		if (destinationWidth == 0)
		{
		float scalar = static_cast<float>(destinationHeight) / static_cast<float>(originalHeight);
		destinationWidth = static_cast<unsigned int>(scalar * static_cast<float>(originalWidth));
		}
		else if (destinationHeight == 0)
		{
		float scalar = static_cast<float>(destinationWidth) / static_cast<float>(originalWidth);
		destinationHeight = static_cast<unsigned int>(scalar * static_cast<float>(originalHeight));
		}

		hr = wicFactory->CreateBitmapScaler(&scaler);
		if (SUCCEEDED(hr))
		{
		hr = scaler->Initialize(
		bitmapSource,
		destinationWidth,
		destinationHeight,
		WICBitmapInterpolationModeCubic);
		}
		if (SUCCEEDED(hr))
		{
		hr = converter->Initialize(
		scaler,
		GUID_WICPixelFormat32bppPBGRA,
		WICBitmapDitherTypeNone,
		nullptr,
		0.f,
		WICBitmapPaletteTypeMedianCut);
		}
		}
		}
		else // Don't scale the image.
		{
		*/
		hr = converter->Initialize(
			bitmapSource,
			GUID_WICPixelFormat32bppPBGRA,
			WICBitmapDitherTypeNone,
			nullptr,
			0.f,
			WICBitmapPaletteTypeMedianCut);
		ASSERT(hr == S_OK);
		//        }
		bitmapSource->Release();
		TRACE("Bitmap created\n");
	}
	if (SUCCEEDED(hr))
	{
		// Create a Direct2D bitmap from the WIC bitmap.
		hr = theApp.m_pContext->CreateBitmapFromWicBitmap(
			converter,
			pBitmaps);
		// handle D2DERR_RECREATE_TARGET
		ASSERT(hr == S_OK);
		converter->Release();
	}
	return SUCCEEDED(hr);
}
