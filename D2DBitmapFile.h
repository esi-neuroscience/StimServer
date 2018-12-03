#pragma once
class CD2DBitmapFile
{
public:
	CD2DBitmapFile(void);
	~CD2DBitmapFile(void);
	bool Init(LPCWSTR wzFilename);
	HRESULT GetFrameCount(UINT32* nFrames);
	HRESULT LoadBitmapFile(UINT32 iFrame, ID2D1Bitmap** pBitmap);
	void Cleanup(void);
private:
	IWICBitmapDecoder *m_pDecoder;
	IWICImagingFactory *m_pWICfactory;
};

