#include "stdafx.h"
#include "DisplayProcedure.h"
#include <wincodec.h>
#include <d3d11_1.h>
#include <dxgi.h>	// includes dxgi1_2.h
//#include <dxgi1_2.h>
#include <d2d1_1.h>
#include <wrl\client.h>
#include <d3dcompiler.inl>
#include <DirectXMath.h>	// XMFLOAT3
#include <math.h>

using namespace Microsoft::WRL;
using namespace DirectX;

#include "StimServer.h"
#include "MainFrm.h"
#include "StimServerDoc.h"
#ifdef PERFSTAT
#include "PerfStat.h"
#endif
/*
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
*/
#pragma comment(lib, "d3dcompiler.lib")

extern CStimServerApp theApp;
//extern CRITICAL_SECTION g_criticalDrawSection;
//extern CRITICAL_SECTION g_criticalDeviceSection;
float g_frameRate;
D2D1_SIZE_F g_ScreenSize;
CString g_adapterName;

bool CStimServerDoc::m_valid = true;

CDeviceVertexBuffer CStimulus::VertexBufferUnitQuad;
CDeviceVertexBuffer CStimulusPixel::m_VertexBuffer;
D3D11_VIEWPORT CStimulusPixel::m_vp;

UINT DisplayProcedure( LPVOID pParam )
{
	POSITION pos = theApp.GetFirstDocTemplatePosition();
	CDocTemplate* temp = theApp.GetNextDocTemplate(pos);
	pos = temp->GetFirstDocPosition();
	CStimServerDoc* pDoc = (CStimServerDoc*) temp->GetNextDoc(pos);

//	HWND wndOutput = ((CMainFrame*) pParam)->m_wndOutput.m_wndOutputBuild.m_hWnd;

	HRESULT hr;

	/*
	DWM_TIMING_INFO timingInfo;
	timingInfo.cbSize = sizeof(DWM_TIMING_INFO);
	hr = DwmGetCompositionTimingInfo(NULL, &timingInfo);
	// 0x80263001 DWM_E_COMPOSITIONDISABLED
	ASSERT(timingInfo.rateRefresh.uiDenominator == 1);
	TRACE("Rate: %u\n", timingInfo.rateRefresh.uiNumerator);
	*/

	IDXGIFactory2 * pFactory;
	hr = CreateDXGIFactory1(__uuidof(IDXGIFactory2), (void**)(&pFactory) );
	ASSERT(hr == S_OK);

	// pFactory->IsWindowedStereoEnabled();
	IDXGIAdapter2 * pAdapter;
	//IDXGIOutput1 pOutput;			// IDXGIOutput2 with Dxgi1_3.h (Windows 8.1) --> Hardware Overlay Planes	
	DXGI_ADAPTER_DESC2 pDesc;
	DXGI_OUTPUT_DESC outputDesc;
	MONITORINFO monitorInfo;
	monitorInfo.cbSize = sizeof(MONITORINFO);
	BOOL haveSecondary = FALSE;
	UINT i = 0; 
	DXGI_MODE_DESC1 modeDesc;
	UINT width;
	UINT height;
	while(pFactory->EnumAdapters(i++, (IDXGIAdapter**) &pAdapter) != DXGI_ERROR_NOT_FOUND) 
	{ 
		pAdapter->GetDesc2(&pDesc);
		TRACE("%d. Adapter: %S\n", i, CString(pDesc.Description));
		g_adapterName = _T("Adapter: ") + CString(pDesc.Description);
		UINT j = 0; 
		while (pAdapter->EnumOutputs(j++, (IDXGIOutput**) &theApp.pOutput) != DXGI_ERROR_NOT_FOUND)
		{
			theApp.pOutput->GetDesc(&outputDesc);
			VERIFY(GetMonitorInfo(outputDesc.Monitor, &monitorInfo));
			haveSecondary = (monitorInfo.dwFlags & MONITORINFOF_PRIMARY) == 0;
			if (haveSecondary) {
				width = outputDesc.DesktopCoordinates.right - outputDesc.DesktopCoordinates.left;
				height = outputDesc.DesktopCoordinates.bottom - outputDesc.DesktopCoordinates.top;
				TRACE(_T(" Width: %u, Height: %u\n"), width, height);
				TRACE(" Device: %S\n", CString(outputDesc.DeviceName));
				TRACE(" %d %d %d %d\n", outputDesc.DesktopCoordinates.left, outputDesc.DesktopCoordinates.top, outputDesc.DesktopCoordinates.right, outputDesc.DesktopCoordinates.bottom);

				//++ Get the refresh rate from the supported modes
				UINT num = 0;
				DXGI_FORMAT format = DXGI_FORMAT_B8G8R8A8_UNORM;
				UINT flags         = 0;	// DXGI_ENUM_MODES_INTERLACED;

				theApp.pOutput->GetDisplayModeList1( format, flags, &num, 0);
						
				DXGI_MODE_DESC1 * pDescs = new DXGI_MODE_DESC1[num];
				theApp.pOutput->GetDisplayModeList1( format, flags, &num, pDescs);
				TRACE(_T("Number of Modes: %u\n"), num);

				//+ we choose the mode with the size of the desktop (monitor)
				//  and the highest refresh rate
				DXGI_MODE_DESC1 * pDes = NULL;
				g_frameRate = 0.0f;
//				float ratef = 0.0f;
				float rateftemp = 0.0f;
				for (UINT iMode = 0; iMode < num; iMode++)
				{
					pDes = &pDescs[iMode];
					if ((pDes->Width == width) && (pDes->Height == height))
					{
						TRACE(_T("Width: %u, Height: %u, Rate: %u/%u\n"),
							pDes->Width, pDes->Height,
							pDes->RefreshRate.Numerator, pDes->RefreshRate.Denominator);

						rateftemp = (float) pDes->RefreshRate.Numerator
							/ (float) pDes->RefreshRate.Denominator;
						if (rateftemp > g_frameRate)
						{
							g_frameRate = rateftemp;
							modeDesc = *pDes;
						}
					}
				}
				TRACE(_T("Width: %u, Height: %u, Rate: %f\n"),
					modeDesc.Width, modeDesc.Height, g_frameRate);
				//-
				delete [] pDescs;
				//--
				break;
			}
//			pOutput->Release();
		}
//		pAdapter->Release();
		if (haveSecondary) break;
	}
	pFactory->Release();
	if (!haveSecondary) {
		AfxMessageBox(_T("Could not find a secondary display monitor."), MB_OK, 0);
		return FALSE;
	}
	/* TRACE ADAPTER DESCRIPTION */
	TRACE("Dedicated Video: %u, Dedicated Sytem: %u, Shared System: %u\n", pDesc.DedicatedVideoMemory, pDesc.DedicatedSystemMemory, pDesc.SharedSystemMemory);
	//	TRACE("Graphics: %d, Compute: %d\n", pDesc.GraphicsPreemptionGranularity, pDesc.ComputePreemptionGranularity);

	CString OpenGLClass = AfxRegisterWndClass(CS_OWNDC, NULL, NULL, NULL);
	RECT OpenGLRect = outputDesc.DesktopCoordinates;
	CWnd m_OpenGLWnd;
	VERIFY(m_OpenGLWnd.CreateEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, OpenGLClass, _T("Stimulus Display"), 
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP | WS_VISIBLE,
		OpenGLRect, NULL, NULL, NULL));

	//IDXGISwapChain1 *pSwapChain = NULL;		// IDXGISwapChain2 with DXGI1_3.h

	D3D_FEATURE_LEVEL FeatureLevel;
//	ComPtr<ID3D11DeviceContext> immediateContext;


	hr = E_FAIL;
	D3D_FEATURE_LEVEL MaxSupportedFeatureLevel = D3D_FEATURE_LEVEL_9_1;
	D3D_FEATURE_LEVEL FeatureLevels[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_PREVENT_INTERNAL_THREADING_OPTIMIZATIONS;
#ifdef _DEBUG
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	hr = D3D11CreateDevice(
		NULL,		// neccessary with D3D11 and HARDWARE-DRIVER-TYPE
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		flags,
//		D3D11_CREATE_DEVICE_BGRA_SUPPORT,
//		D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_BGRA_SUPPORT,
		FeatureLevels,
		_countof(FeatureLevels),
		D3D11_SDK_VERSION,
		&theApp.m_pD3Ddevice,
		&FeatureLevel,
		&theApp.m_pImmediateContext);
	ASSERT(hr == S_OK);


//	CString featureLevel;
	switch (FeatureLevel) {
	case D3D_FEATURE_LEVEL_11_1: theApp.m_featureLevelString = "11.1"; break;
	case D3D_FEATURE_LEVEL_11_0: theApp.m_featureLevelString = "11.0"; break;
	case D3D_FEATURE_LEVEL_10_1: theApp.m_featureLevelString = "10.1"; break;
	case D3D_FEATURE_LEVEL_10_0: theApp.m_featureLevelString = "10.0"; break;
	case D3D_FEATURE_LEVEL_9_3: theApp.m_featureLevelString = "9.3"; break;
	case D3D_FEATURE_LEVEL_9_2: theApp.m_featureLevelString = "9.2"; break;
	case D3D_FEATURE_LEVEL_9_1: theApp.m_featureLevelString = "9.1"; break;
	default: theApp.m_featureLevelString = "unknown";
	}

	IDXGIDevice2* dxgiDevice;
	// Obtain the underlying DXGI device of the Direct3D11 device.
	theApp.m_pD3Ddevice->QueryInterface(__uuidof(IDXGIDevice2), (void**)(&dxgiDevice) );

//	ID2D1Factory1 * m_d2dFactory;
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), (void**)(&theApp.m_d2dFactory) );
	ASSERT(hr == S_OK);

	ID2D1Device *m_d2dDevice;
	// Obtain the Direct2D device for 2-D rendering.
//	hr = m_d2dFactory->CreateDevice(dxgiDevice.Get(), &m_d2dDevice);
	hr = theApp.m_d2dFactory->CreateDevice(dxgiDevice, &m_d2dDevice);
	ASSERT(hr == S_OK);
	//	m_d2dFactory->Release();

//	ID2D1DeviceContext *m_d2dContext;
	// Get Direct2D device's corresponding device context object.
	hr = m_d2dDevice->CreateDeviceContext(
		D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS,
//		D2D1_DEVICE_CONTEXT_OPTIONS_NONE,	// !! multithreading option
//		&m_d2dContext
		&theApp.m_pContext
		);
	ASSERT(hr == S_OK);
	m_d2dDevice->Release();

	UINT numQualLevels;
	hr = theApp.m_pD3Ddevice->CheckMultisampleQualityLevels(DXGI_FORMAT_B8G8R8A8_UNORM, 2, &numQualLevels);
	ASSERT(hr == S_OK);
	TRACE("Number of quality levels: %u\n", numQualLevels);
#ifdef _DEBUG
#define BUFFERCOUNT 1
#else
#define BUFFERCOUNT 1
#endif
	// Allocate a descriptor.
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {0};
//	swapChainDesc.Width = 0;                           // use automatic sizing
//	swapChainDesc.Height = 0;
	swapChainDesc.Width = width;                           // use automatic sizing
	swapChainDesc.Height = height;
	swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // this is the most common swapchain format
//	swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
	swapChainDesc.Stereo = false; 
	swapChainDesc.SampleDesc.Count = 1;                // don't use multi-sampling
	swapChainDesc.SampleDesc.Quality = 0;
	//	swapChainDesc.SampleDesc.Count = 2;
	//	swapChainDesc.SampleDesc.Quality = D3D11_CENTER_MULTISAMPLE_PATTERN;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = BUFFERCOUNT;                     // use double buffering to enable flip
	//	swapChainDesc.Scaling = DXGI_SCALING_NONE;		// unsupported on Windows 7
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
//	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // all apps must use this SwapEffect
//	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;
//	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
//	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_STRAIGHT;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC swapChainFullscreenDesc = {0};

	swapChainFullscreenDesc.RefreshRate = modeDesc.RefreshRate;
	swapChainFullscreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainFullscreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainFullscreenDesc.Windowed = FALSE;	// ??
/*
	// Identify the physical adapter (GPU or card) this device runs on.
	ComPtr<IDXGIAdapter> dxgiAdapter;
	hr = dxgiDevice->GetAdapter(&dxgiAdapter);
	ASSERT(hr == S_OK);
*/
/*
	// Get the factory object that created the DXGI device.
	ComPtr<IDXGIFactory2> dxgiFactory;
	hr = dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
	ASSERT(hr == S_OK);
	//	dxgiAdapter->Release();
*/
	// Get the final swap chain for this window from the DXGI factory.
//	hr = dxgiFactory->CreateSwapChainForHwnd(
	hr = pFactory->CreateSwapChainForHwnd(
		theApp.m_pD3Ddevice,
		m_OpenGLWnd.m_hWnd,
		&swapChainDesc,
		&swapChainFullscreenDesc,
		nullptr,    // allow on all displays	!!
		&theApp.pSwapChain
		);
	if (hr == DXGI_STATUS_OCCLUDED) hr = S_OK;
	ASSERT(hr == S_OK);

	hr = theApp.pSwapChain->SetFullscreenState(TRUE, theApp.pOutput);
	ASSERT(hr == S_OK);
	hr = theApp.pSwapChain->GetContainingOutput((IDXGIOutput**)&theApp.pOutput);
	ASSERT(hr == S_OK);

	// 0x887A0004 - DXGI_ERROR_UNSUPPORTED (stereo)

	//	dxgiFactory->Release();

	//	device->Release();
/*	
	// Get the frame rate for later reference
	hr = pSwapChain->GetFullscreenDesc(&swapChainFullscreenDesc);
	ASSERT(hr == S_OK);
	ASSERT(swapChainFullscreenDesc.RefreshRate.Denominator == 1);
	TRACE("Refresh Rate: %u\n", swapChainFullscreenDesc.RefreshRate.Numerator);
*/	

	// Ensure that DXGI doesn't queue more than one frame at a time.
	hr = dxgiDevice->SetMaximumFrameLatency(1);
	ASSERT(hr == S_OK);
	/*
	IDXGISurface *pBackBuffer;
	// Get a surface in the swap chain
	hr = pSwapChain->GetBuffer(
		0,
		IID_PPV_ARGS(&pBackBuffer)
		);
	ASSERT(hr == S_OK);
	*/
	/*
	ID2D1Factory1 * m_pD2DFactory;
	//	HRESULT hr = CreateD2D1Factory1(__uuidof(ID2D1Factory1), (void**)(&m_pD2DFactory) );
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), (void**)(&m_pD2DFactory) );
	ASSERT(hr == S_OK);
	*/
//	ID2D1RenderTarget *pRenderTarget = NULL;
	/*
	if (SUCCEEDED(hr))
	{
		// Create the DXGI Surface Render Target.
		FLOAT dpiX;
		FLOAT dpiY;
		theApp.m_d2dFactory->GetDesktopDpi(&dpiX, &dpiY);

		D2D1_RENDER_TARGET_PROPERTIES props =
			D2D1::RenderTargetProperties(
			D2D1_RENDER_TARGET_TYPE_DEFAULT,
			D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
			dpiX,
			dpiY
			);
		/*
		// Create a Direct2D render target which can draw into the surface in the swap chain
		hr = theApp.m_d2dFactory->CreateDxgiSurfaceRenderTarget(
			pBackBuffer,
			&props,
			&pRenderTarget
			);
		ASSERT(hr == S_OK);
	}
	pBackBuffer->Release();
	*/
//	m_d2dFactory->Release();



	//	ID2D1RenderTarget *pRenderTarget = NULL;
	D2D1_BITMAP_PROPERTIES1 bitmapProperties;
//	bitmapProperties.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
	bitmapProperties.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
//	bitmapProperties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;		//!!
	bitmapProperties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;		//!!
	bitmapProperties.dpiX = 0;
	bitmapProperties.dpiY = 0;
	bitmapProperties.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;	// !!
	bitmapProperties.colorContext = NULL;

	// Direct2D needs the dxgi version of the backbuffer surface pointer.
	ComPtr<IDXGISurface> dxgiBackBuffer;
	hr = theApp.pSwapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer));
	ASSERT(hr == S_OK);

	ID2D1Bitmap1 *m_d2dTargetBitmap = NULL;
	// Get a D2D surface from the DXGI back buffer to use as the D2D render target.
	hr = theApp.m_pContext->CreateBitmapFromDxgiSurface(
		dxgiBackBuffer.Get(),
		&bitmapProperties,
		&m_d2dTargetBitmap
		);
	ASSERT(hr == S_OK);

//	dxgiBackBuffer->Release();

	// Now we can set the Direct2D render target.
	theApp.m_pContext->SetTarget(m_d2dTargetBitmap);
	m_d2dTargetBitmap->Release();

	TRACE("D2D Device Context\n");
	FLOAT dpiX;
	FLOAT dpiY;
	theApp.m_pContext->GetDpi(&dpiX, &dpiY);
	TRACE(" DPIx: %f, DPIy: %f\n", dpiX, dpiY);
//	D2D1_SIZE_F contextSize = theApp.m_pContext->GetSize();
	g_ScreenSize = theApp.m_pContext->GetSize();
//	TRACE(" SizeX: %f, SizeY: %f\n", contextSize.width, contextSize.height);
	TRACE(" SizeX: %f, SizeY: %f\n", g_ScreenSize.width, g_ScreenSize.height);

    // Create a render target view
    ID3D11Texture2D* pBackBufferPS = NULL;
    hr = theApp.pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBufferPS );
    if( FAILED( hr ) )
        return hr; 

//	ID3D11RenderTargetView *g_pRenderTargetView = NULL;
	hr = theApp.m_pD3Ddevice->CreateRenderTargetView( pBackBufferPS, NULL, &theApp.m_pRenderTargetView );
    pBackBufferPS->Release();
    if( FAILED( hr ) )
        return hr;

    theApp.m_pImmediateContext->OMSetRenderTargets( 1, &theApp.m_pRenderTargetView, NULL );

//	D3D11_RENDER_TARGET_BLEND_DESC RenderTarget[8];
	D3D11_BLEND_DESC BlendStateDesc = {0};
	BlendStateDesc.AlphaToCoverageEnable = false;
	BlendStateDesc.IndependentBlendEnable = false;
	BlendStateDesc.RenderTarget[0].BlendEnable = true;
//	BlendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_COLOR;
	BlendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
//	BlendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	BlendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	BlendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
//	BlendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
//	BlendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA_SAT;
	BlendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
//	BlendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_DEST_ALPHA;
//	BlendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	BlendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	BlendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	BlendStateDesc.RenderTarget[0].RenderTargetWriteMask = 
		D3D11_COLOR_WRITE_ENABLE_RED |
		D3D11_COLOR_WRITE_ENABLE_GREEN |
		D3D11_COLOR_WRITE_ENABLE_BLUE;
	// D3D11_COLOR_WRITE_ENABLE_ALL;
	ID3D11BlendState *pBlendState;
	theApp.m_pD3Ddevice->CreateBlendState(&BlendStateDesc, &pBlendState);
	float BlendFactor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    theApp.m_pImmediateContext->OMSetBlendState(pBlendState, BlendFactor, 0xffffffff);
	pBlendState->Release();

    // Compile the vertex shader
	const char vertexShader[] = 
"float4 VS( float4 Pos : POSITION ) : SV_POSITION\n"
"{\n"
"    return Pos;\n"
"}\n"
;
//	TRACE("%s", vertexShader);
//	TRACE("Shader Size: %u\n", sizeof(vertexShader));
    ID3DBlob* pVSBlob = NULL;
	theApp.CompileShader(vertexShader, sizeof(vertexShader), "VS", "vs_4_0", &pVSBlob);

	// Create the vertex shader
	hr = theApp.m_pD3Ddevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &theApp.m_pVertexShader );
	if( FAILED( hr ) )
	{	
		pVSBlob->Release();
        return hr;
	}

	ID3D11InputLayout *pVertexLayout = NULL;
    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
	UINT numElements = ARRAYSIZE( layout );

    // Create the input layout
	hr = theApp.m_pD3Ddevice->CreateInputLayout( layout, numElements, pVSBlob->GetBufferPointer(),
                                          pVSBlob->GetBufferSize(), &pVertexLayout );
	pVSBlob->Release();
	if( FAILED( hr ) )
        return hr;

    // Set the input layout
    theApp.m_pImmediateContext->IASetInputLayout( pVertexLayout );
	pVertexLayout->Release();

	XMFLOAT3 vertices[] =
    {
		XMFLOAT3(-1.0f,-1.0f, 0.0f ),
		XMFLOAT3(-1.0f, 1.0f, 0.0f ),
		XMFLOAT3( 1.0f,-1.0f, 0.0f ),
		XMFLOAT3( 1.0f, 1.0f, 0.0f )
    };

//	hr = CStimulusPS::m_VertexBuffer.Init(vertices, sizeof( XMFLOAT3 ) * 4);
	hr = CStimulus::VertexBufferUnitQuad.Init(vertices, sizeof( XMFLOAT3 ) * 4);
    if( FAILED( hr ) )
        return hr;

	XMFLOAT3 vertex = XMFLOAT3(0.0f, 0.0f, 0.0f);
	hr = CStimulusPixel::m_VertexBuffer.Init(&vertex, sizeof( XMFLOAT3 ));
    if( FAILED( hr ) )
        return hr;
//	CStimulusPixel::m_vp.Width = contextSize.width;
//	CStimulusPixel::m_vp.Height = contextSize.height;
	CStimulusPixel::m_vp.Width = g_ScreenSize.width;
	CStimulusPixel::m_vp.Height = g_ScreenSize.height;
	CStimulusPixel::m_vp.MinDepth = 0.0f;
	CStimulusPixel::m_vp.MaxDepth = 1.0f;

	hr = theApp.m_pContext->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::White),
		&theApp.m_pWhiteBrush
		);
	ASSERT(hr == S_OK);




	D2D1_MATRIX_3X2_F contextTransform = 
	{1.f, 0.f,
	 0.f, 1.f,
	g_ScreenSize.width/2.f, g_ScreenSize.height/2.f};
	theApp.m_contextTransform = contextTransform;
	theApp.m_pContext->SetTransform(contextTransform);

/*
	DXGI_FRAME_STATISTICS frameStats, prevFrameStats;

	pSwapChain->GetFrameStatistics(&frameStats);
//	PresentCount = frameStats.PresentCount;
//	SyncRefreshCount = frameStats.SyncRefreshCount;
	prevFrameStats = frameStats;
*/

	DXGI_PRESENT_PARAMETERS presentPars = {0};
	hr = theApp.pSwapChain->Present1(
		1,
		0,
		&presentPars);
	ASSERT(hr == S_OK);



//	pDoc->AddPhotoDiode(D2D1::RectF(g_ScreenSize.width/-2.f, g_ScreenSize.height/-2.f,
//		g_ScreenSize.width/-2.f+15.f, g_ScreenSize.height/-2.f+15.f));
	CPhotoDiode::Init();
//	VERIFY(theApp.m_pDisplayThread->SetThreadPriority(THREAD_PRIORITY_ABOVE_NORMAL));
//	VERIFY(theApp.m_pDisplayThread->SetThreadPriority(THREAD_PRIORITY_HIGHEST));
	//	unsigned int nErrors = 0;
	/*
	LARGE_INTEGER performanceFrequency;
	LARGE_INTEGER preCount;
	LARGE_INTEGER postCount;
	ASSERT(QueryPerformanceFrequency(&performanceFrequency));
	*/
#ifdef PERFSTAT
	CPerfStat perfStat(1000);
#endif
	VERIFY(SetEvent(CStimServerApp::m_hDisplayThreadReady));
//	bool docValid = true;
	do
	{
//		if  (!(docValid = pDoc->Draw())) break;
		pDoc->Draw();
//		pOutput->WaitForVBlank();

//		EnterCriticalSection(&g_criticalDeviceSection);
//		ASSERT(QueryPerformanceCounter(&preCount));
#ifdef PERFSTAT
		perfStat.Pre();
#endif

	

		hr = theApp.pSwapChain->Present1(
			1,
			0,
//			DXGI_PRESENT_DO_NOT_SEQUENCE,
//			DXGI_PRESENT_RESTART,
			&presentPars);
//		ASSERT(QueryPerformanceCounter(&postCount));
#ifdef PERFSTAT
		perfStat.Post();
#endif
//		LeaveCriticalSection(&g_criticalDeviceSection);
//		pOutput->WaitForVBlank();



		if (hr == DXGI_ERROR_DEVICE_REMOVED)
		{
			hr = theApp.m_pD3Ddevice->GetDeviceRemovedReason();
			CString DXGIerror;
			switch (hr)
			{
			case DXGI_ERROR_DEVICE_REMOVED:
				AfxMessageBox(_T("Device Removed"));
				break;
			default:
				DXGIerror.Format(_T("Device Removed: %X"), hr);
				AfxMessageBox(DXGIerror);
				break;
			}
			/*
			0x887A0005 DXGI_ERROR_DEVICE_REMOVED
			0x887A0006 DXGI_ERROR_DEVICE_HUNG
			The application's device failed due to badly formed commands sent by the application. This is an design-time issue that should be investigated and fixed.
			*/
			ASSERT(SUCCEEDED(hr));
		}
		ASSERT(SUCCEEDED(hr));
		/*
		pSwapChain->GetFrameStatistics(&frameStats);
		if ((frameStats.PresentCount != prevFrameStats.PresentCount +1) || (frameStats.SyncRefreshCount != prevFrameStats.SyncRefreshCount+1)) {
			TRACE("%u %u %u %u\n", frameStats.PresentCount, prevFrameStats.PresentCount, frameStats.SyncRefreshCount, prevFrameStats.SyncRefreshCount);
			DXGI_FRAME_STATISTICS *pFrameStatsBuffer = new DXGI_FRAME_STATISTICS[2];
			pFrameStatsBuffer[0] = prevFrameStats;
			pFrameStatsBuffer[1] = frameStats;
			if (nErrors < 100) {
				VERIFY(::PostMessage(wndOutput, WM_SEQUENCE_ERR, (WPARAM) pFrameStatsBuffer, 0));
				nErrors++;
			}
		}
		prevFrameStats = frameStats;
		*/
	} while (CStimServerDoc::m_valid);
//	} while (docValid);
	theApp.pSwapChain->Release();
	theApp.m_pContext->Release();
	theApp.pOutput->Release();
	TRACE("End of Display Thread\n");
	if (!SetEvent(pDoc->m_hCloseDocument)) {
		DWORD error = GetLastError();
		ASSERT(false);
	}
	return 0;
}