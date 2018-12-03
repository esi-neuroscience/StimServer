// Symbol.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "StimServer.h"
#include "Symbol.h"


extern CRITICAL_SECTION g_criticalMapSection;
extern CRITICAL_SECTION g_criticalDrawSection;

// CSymbol
// CSymbol-Memberfunktionen

CSymbol::CSymbol()
	: m_pRenderTarget(NULL)
	, m_pD2DrenderTarget(NULL)
//	, m_pRenderTargetView(NULL)
	, m_pShaderResView(NULL)
	, m_flags(0)
{
	m_size = D2D1::SizeU(0, 0);
}

CSymbol::~CSymbol()
{
	if (m_pShaderResView)
	{
		m_pShaderResView->Release();
		m_pShaderResView = NULL;
	}
//	m_pRenderTargetView->Release();
	if (m_pRenderTarget)
	{
		m_pRenderTarget->Release();
		m_pRenderTarget = NULL;
	}
//	m_pD2DrenderTarget->Release();
}

void CSymbol::CreateTextureTarget()
{
	HRESULT hr;
	if (m_pRenderTarget) m_pRenderTarget->Release();
	// Create the render target texture
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory( &desc, sizeof(desc) );
//	desc.Width = 2*m_size.width;
//	desc.Height = 2*m_size.height;
	desc.Width = m_size.width;
	desc.Height = m_size.height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
//	desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;	// no CPU access needed -- we'll render to texture
	desc.MiscFlags = 0;

	hr = theApp.m_pD3Ddevice->CreateTexture2D( &desc, NULL, &m_pRenderTarget );
	ASSERT(hr == S_OK);

	IDXGISurface *pDXGIsurface = NULL;
	hr = m_pRenderTarget->QueryInterface(&pDXGIsurface);
	ASSERT(hr == S_OK);

    // Create a D2D render target which can draw into our offscreen D3D
    // surface.
    D2D1_RENDER_TARGET_PROPERTIES props =
        D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_DEFAULT,
//            D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_UNKNOWN),
            D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
            0,
            0
//            96,
//            96
            );

//	if (m_pD2DrenderTarget) m_pD2DrenderTarget->Release();

    hr = theApp.m_d2dFactory->CreateDxgiSurfaceRenderTarget(
        pDXGIsurface,
        &props,
        &m_pD2DrenderTarget
        );
	ASSERT(hr == S_OK);
	pDXGIsurface->Release();
	//+ try the aliased mode 
//	m_pD2DrenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
	//-
	m_pD2DrenderTarget->SetAntialiasMode(m_flag.aliased);
}


void CSymbol::Init()
{
	CreateTextureTarget();

	// Create the sample state	--> STATIC ?
    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory( &sampDesc, sizeof(sampDesc) );
//    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
//    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
//    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
//    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

//	ID3D11SamplerState* pSampler;
    HRESULT hr = theApp.m_pD3Ddevice->CreateSamplerState( &sampDesc, &m_pSampler );
	ASSERT(hr == S_OK);
//	theApp.m_pImmediateContext->PSSetSamplers(0, 1, &pSampler);
}


void CSymbol::DrawBrushedTexture()
{
	HRESULT hr;
	ID2D1SolidColorBrush* pWhiteBrush;
	hr = m_pD2DrenderTarget->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::White),
		&pWhiteBrush
		);
	ASSERT(hr == S_OK);

	m_pD2DrenderTarget->BeginDraw();
	m_pD2DrenderTarget->Clear(D2D1::ColorF(1.0f,1.0f,1.0f,0.0f));
	DrawBrushedTexture(pWhiteBrush);
	/*
	m_pD2DrenderTarget->FillEllipse(D2D1::Ellipse(
		D2D1::Point2F((float) m_size.width/2.0f, (float) m_size.height/2.0f),
		(float) m_size.width/2.0f, (float)m_size.height/2.0f), pWhiteBrush );
	*/
	hr = m_pD2DrenderTarget->EndDraw();
	ASSERT(hr == S_OK);
	
	pWhiteBrush->Release();
	m_pD2DrenderTarget->Release();
	m_pD2DrenderTarget = NULL;
	if (m_pShaderResView) m_pShaderResView->Release();
	TRACE("Disc: Pre Shader Resource View\n"); 
	hr = theApp.m_pD3Ddevice->CreateShaderResourceView( m_pRenderTarget, NULL, &m_pShaderResView );
	TRACE("Disc: Post Shader Resource View\n"); 
	ASSERT(hr == S_OK);
}


CDisc::CDisc(unsigned short size)
{
	m_size = D2D1::SizeU(size, size);
	EnterCriticalSection(&g_criticalDrawSection);
	CSymbol::Init();
//	DrawTexture();
	CSymbol::DrawBrushedTexture();
	LeaveCriticalSection(&g_criticalDrawSection);
}


CDisc::~CDisc(void)
{
	CSymbol::~CSymbol();
}

/*
//void CDisc::DrawTexture(unsigned short size, ID2D1RenderTarget* pD2DrenderTarget)
void CDisc::DrawTexture()
{
	HRESULT hr;
	ID2D1SolidColorBrush* pWhiteBrush;
	hr = m_pD2DrenderTarget->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::White),
		&pWhiteBrush
		);
	ASSERT(hr == S_OK);

	m_pD2DrenderTarget->BeginDraw();
	m_pD2DrenderTarget->Clear(D2D1::ColorF(1.0f,1.0f,1.0f,0.0f));
	m_pD2DrenderTarget->FillEllipse(D2D1::Ellipse(
		D2D1::Point2F((float) m_size.width/2.0f, (float) m_size.height/2.0f),
		(float) m_size.width/2.0f, (float)m_size.height/2.0f), pWhiteBrush );
	hr = m_pD2DrenderTarget->EndDraw();
	ASSERT(hr == S_OK);
	
	pWhiteBrush->Release();
	m_pD2DrenderTarget->Release();
	m_pD2DrenderTarget = NULL;
	if (m_pShaderResView) m_pShaderResView->Release();
	TRACE("Disc: Pre Shader Resource View\n"); 
	hr = theApp.m_pD3Ddevice->CreateShaderResourceView( m_pRenderTarget, NULL, &m_pShaderResView );
	TRACE("Disc: Post Shader Resource View\n"); 
	ASSERT(hr == S_OK);
}
*/

void CDisc::DrawTexture()
{
	CSymbol::DrawBrushedTexture();
}


void CDisc::DrawBrushedTexture(ID2D1SolidColorBrush* pBrush)
{
	m_pD2DrenderTarget->FillEllipse(D2D1::Ellipse(
		D2D1::Point2F((float) m_size.width/2.0f, (float) m_size.height/2.0f),
		(float) m_size.width/2.0f, (float)m_size.height/2.0f), pBrush );
}


CCircle::CCircle(unsigned short size)
{
	m_size = D2D1::SizeU(size, size);
	EnterCriticalSection(&g_criticalDrawSection);
	CSymbol::Init();
//	DrawTexture();
	CSymbol::DrawBrushedTexture();
	LeaveCriticalSection(&g_criticalDrawSection);
}


CCircle::~CCircle(void)
{
}

/*
void CCircle::DrawTexture()
{
	HRESULT hr;
	ID2D1SolidColorBrush* pWhiteBrush;
	hr = m_pD2DrenderTarget->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::White),
		&pWhiteBrush
		);
	ASSERT(hr == S_OK);

	m_pD2DrenderTarget->BeginDraw();
	m_pD2DrenderTarget->Clear(D2D1::ColorF(1.0f,1.0f,1.0f,0.0f));
//	float size = (float) m_size;
	float strokeWidth = 5.0f;
//	float radius = m_size.width - (strokeWidth-1.0f)/2.0f;	// ! m_size.height
	float radius = ((float) m_size.width - strokeWidth-1.0f)/2.0f;	// ! m_size.height
	m_pD2DrenderTarget->DrawEllipse(D2D1::Ellipse(
		D2D1::Point2F((float) m_size.width/2.0f, (float) m_size.height/2.0f),
		radius, radius), pWhiteBrush, strokeWidth );
	hr = m_pD2DrenderTarget->EndDraw();
	ASSERT(hr == S_OK);
	
	pWhiteBrush->Release();
	m_pD2DrenderTarget->Release();
	m_pD2DrenderTarget = NULL;
	if (m_pShaderResView) m_pShaderResView->Release();
	TRACE("Circle: Pre Shader Resource View\n"); 
	hr = theApp.m_pD3Ddevice->CreateShaderResourceView( m_pRenderTarget, NULL, &m_pShaderResView );
	TRACE("Circle: Post Shader Resource View\n"); 
	ASSERT(hr == S_OK);
}
*/

void CCircle::DrawTexture()
{
	CSymbol::DrawBrushedTexture();
}


void CCircle::DrawBrushedTexture(ID2D1SolidColorBrush* pBrush)
{
	float strokeWidth = 5.0f;
//	float radius = m_size.width - (strokeWidth-1.0f)/2.0f;	// ! m_size.height
	float radius = ((float) m_size.width - strokeWidth-1.0f)/2.0f;	// ! m_size.height
	m_pD2DrenderTarget->DrawEllipse(D2D1::Ellipse(
		D2D1::Point2F((float) m_size.width/2.0f, (float) m_size.height/2.0f),
		radius, radius), pBrush, strokeWidth );
}


CPicture::CPicture(ID2D1Bitmap* pBitmap)
{
//	m_pBitmap = pBitmap;
	HRESULT hr;
	m_size = pBitmap->GetPixelSize();
	EnterCriticalSection(&g_criticalDrawSection);
	CSymbol::Init();
	hr = m_pD2DrenderTarget->CreateSharedBitmap(
		__uuidof(ID2D1Bitmap),
		pBitmap,
		NULL,
		&m_pBitmap);
	ASSERT(hr == S_OK);
	DrawTexture();
	LeaveCriticalSection(&g_criticalDrawSection);
	m_pBitmap->Release();
	TRACE("Picture Size: %ux%u\n", m_size.width, m_size.height);
}


CPicture::~CPicture(void)
{
}


void CPicture::DrawTexture()
{
	HRESULT hr;
	m_pD2DrenderTarget->BeginDraw();
	m_pD2DrenderTarget->DrawBitmap(m_pBitmap,  // D2D1::RectF(0.0f, 0.0f, m_size.width, m_size.height),
		NULL, 
		1.0f,	// opacity
		D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
	hr = m_pD2DrenderTarget->EndDraw();
	ASSERT(hr == S_OK);
	
	m_pD2DrenderTarget->Release();
	m_pD2DrenderTarget = NULL;
	if (m_pShaderResView) m_pShaderResView->Release();
	TRACE("Picture: Pre Shader Resource View\n"); 
	hr = theApp.m_pD3Ddevice->CreateShaderResourceView( m_pRenderTarget, NULL, &m_pShaderResView );
	TRACE("Picture: Post Shader Resource View\n"); 
	ASSERT(hr == S_OK);
//	m_pBitmap->Release();
}