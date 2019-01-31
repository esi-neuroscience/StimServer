
// StimServer.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "StimServer.h"
#include "MainFrm.h"

#include "StimServerDoc.h"
#include "StimServerView.h"
#include "DisplayProcedure.h"
#include "PipeProcedure.h"
#include "HardwareDlg.h"
#include <d3dcompiler.inl>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

ID3D11VertexShader* CStimulus::m_pVertexShaderCol = NULL;
ID3D11VertexShader* CStimServerApp::m_pVertexShaderPart;
ID3D11VertexShader* CStimServerApp::m_pVertexShaderShift;
//ID3D11PixelShader* CStimServerApp::m_pPixelShaderMin;
ID3D11RenderTargetView* CStimServerApp::m_pRenderTargetView;
ID2D1Factory1* CStimServerApp::m_d2dFactory;
ID2D1SolidColorBrush* CStimServerApp::m_pWhiteBrush;
D2D1::ColorF CStimServerApp::m_defaultDrawColor = D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f);
D2D1::ColorF CStimServerApp::m_defaultOutlineColor = D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f);
D2D1_MATRIX_3X2_F CStimServerApp::m_contextTransform;

CStimServerDoc* g_pDoc;

CRITICAL_SECTION g_criticalMapSection;
CRITICAL_SECTION g_criticalDrawSection;
//CRITICAL_SECTION g_criticalDeviceSection;
//D2D1_SIZE_F ScreenSize;

// CStimServerApp

BEGIN_MESSAGE_MAP(CStimServerApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CStimServerApp::OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
	ON_COMMAND(ID_HELP_ABOUTHARDWARE, &CStimServerApp::OnHelpAbouthardware)
END_MESSAGE_MAP()


// CStimServerApp construction

CStimServerApp::CStimServerApp()
	: m_pContext(NULL)
	, m_drawMode(false)
	, m_featureLevelString(_T(""))
	, m_deferredMode(false)
	, m_errorMask(0)
	, m_errorCode(0)
{
	m_bHiColorIcons = TRUE;

	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("StimServer.AppID.NoVersion"));

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	m_pVertexShaderPart = NULL;
	m_pVertexShaderShift = NULL;
//	m_pPixelShaderMin = NULL;
	m_pRenderTargetView = NULL;
	m_hStimServerDone = ::CreateEvent(NULL, FALSE, FALSE, _T("StimServerDone"));
	ASSERT(m_hStimServerDone != NULL);
}

// The one and only CStimServerApp object

CStimServerApp theApp;

// CStimServerApp initialization

BOOL CStimServerApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();


	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	EnableTaskbarInteraction(FALSE);

	// AfxInitRichEdit2() is required to use RichEdit control	
	// AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)


	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CStimServerDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CStimServerView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);


	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);



	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	POSITION pos = pDocTemplate->GetFirstDocPosition();
	g_pDoc = (CStimServerDoc*) pDocTemplate->GetNextDoc(pos);
	VERIFY(CAnim::CreateEvent());

	InitializeCriticalSection(&g_criticalMapSection);
	InitializeCriticalSection(&g_criticalDrawSection);
//	InitializeCriticalSection(&g_criticalDeviceSection);
//	m_hArrayMutex = CreateMutex(NULL, FALSE, NULL);
//	ASSERT(m_hArrayMutex);
//	m_hDrawMutex = CreateMutex(NULL, FALSE, NULL);
//	ASSERT(m_hDrawMutex);
	m_pDisplayThread = AfxBeginThread(DisplayProcedure, m_pMainWnd);
	m_pPipeThread = AfxBeginThread(PipeProcedure, NULL);

	// The one and only window has been initialized, so show and update it
//	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->ShowWindow(SW_MINIMIZE);
	m_pMainWnd->UpdateWindow();
	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand
	VERIFY(SetEvent(m_hStimServerDone));
	return TRUE;
}

int CStimServerApp::ExitInstance()
{
	//TODO: handle additional resources you may have added
	m_pWhiteBrush->Release();
	m_pVertexShader->Release();
	m_pRenderTargetView->Release();
	m_pImmediateContext->Release();
	m_pD3Ddevice->Release();
	m_d2dFactory->Release();
	AfxOleTerm(FALSE);
//	VERIFY(CloseHandle(m_hDrawMutex));
//	VERIFY(CloseHandle(m_hArrayMutex));
	VERIFY(CloseHandle(m_hStimServerDone));
	DeleteCriticalSection(&g_criticalMapSection);
	DeleteCriticalSection(&g_criticalDrawSection);
	return CWinAppEx::ExitInstance();
}

// CStimServerApp message handlers


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// App command to run the dialog
void CStimServerApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CStimServerApp customization load/save methods

void CStimServerApp::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
	bNameValid = strName.LoadString(IDS_EXPLORER);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EXPLORER);
}

void CStimServerApp::LoadCustomState()
{
}

void CStimServerApp::SaveCustomState()
{
}

// CStimServerApp message handlers


void CStimServerApp::BeginDraw(void)
{
	m_pContext->BeginDraw();
	m_drawMode = true;
}

void CStimServerApp::EndDraw(void)
{
	HRESULT hr = m_pContext->EndDraw();
	m_drawMode = false;
	if (hr != S_OK)
	{
		switch (hr)
		{
		case D2DERR_RECREATE_TARGET:	// 0x8899000c
		// 0x88990001 D2DERR_WRONG_STATE The object was not in the correct state to process the method.
		default:
			ASSERT(false);
		}
	}
}


void CStimServerApp::OnHelpAbouthardware()
{
	CHardwareDlg hardwareDlg;
	hardwareDlg.DoModal();
}


HRESULT CStimServerApp::createVertexShaderCol(void)
{
	HRESULT hr;
    // Compile the vertex shader
	const char vertexShader[] = 
"cbuffer VS_PARAMS : register (b1)\n"
"{\n"
"    float4  color : packoffset(c0.x);\n"
//"    float   size  : packoffset(c1.x);\n"
"};\n"
"struct OUTPUT {\n"
"    float4 Position   : SV_POSITION;\n"
"    float4 Color      : COLOR;\n"
"    float4 TexCoord   : TEXCOORD;\n"
//"	 float	Size	   : PSIZE;\n"
"};\n"
"OUTPUT VScol( float4 Pos : POSITION )\n"
"{\n"
"	 OUTPUT output;\n"
"	 output.Position = Pos;\n"
"	 output.Color = color;\n"
//"	 output.Size = size;\n"
"	 output.TexCoord.x = 0.5f*(Pos.x+1.0f);\n"
"	 output.TexCoord.y = 0.5f*(1.0f-Pos.y);\n"
"    return output;\n"
"}\n"
;
    ID3DBlob* pVSBlob = NULL;
	CompileShader(vertexShader, sizeof(vertexShader), "VScol", "vs_4_0", &pVSBlob);

	// Create the vertex shader
//	ID3D11VertexShader* pVertexShader;
	hr = theApp.m_pD3Ddevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &CStimulus::m_pVertexShaderCol );
	if( FAILED( hr ) )
	{	
		pVSBlob->Release();
        return hr;
	}
	return hr;
}

HRESULT CStimServerApp::createVertexShaderPart(void)
{
	HRESULT hr;
    // Compile the vertex shader
	const char vertexShader[] = 
"cbuffer VS_ANIMATION : register (b0)\n"
"{\n"
//"    float2  shift : packoffset(c0.x);\n"
"    float  shift : packoffset(c0.x);\n"
"};\n"
"cbuffer VS_PARAMS : register (b1)\n"
"{\n"
"    float4  color : packoffset(c0.x);\n"
"    float   size  : packoffset(c1.x);\n"
"    float   angle : packoffset(c1.y);\n"
"};\n"
"struct OUTPUT {\n"
"    float4 Position   : SV_POSITION;\n"
"    float4 Color      : COLOR;\n"
"	 float	Size	   : PSIZE;\n"
"};\n"
"OUTPUT VSpart( float4 Pos : POSITION )\n"
"{\n"
"    float2 p;\n"	// Richtungsvektor
"    sincos(radians(Pos.z+angle), p.y, p.x);\n"
//"    float2 delta = sign(p)*fmod(abs(p)*float2(shift, shift), float2(2.0f,2.0f)) -sign(p);\n"
"	 OUTPUT output;\n"
//"	 output.Position.xy = fmod(Pos.xy+delta+float2(3.0f,3.0f), float2(2.0f,2.0f)) - float2(1.0f,1.0f);\n"
"	 float2 temp = Pos.xy+p*float2(shift, shift);\n"
"	 output.Position.xy = sign(temp)*fmod(abs(temp)+float2(1.0f,1.0f), float2(2.0f,2.0f)) - sign(temp);\n"
"	 output.Position.z = 0.0f;\n"
"	 output.Position.w = Pos.w;\n"
"	 output.Color = color;\n"
"	 output.Size = size;\n"
"    return output;\n"
"}\n"
;
    ID3DBlob* pVSBlob = NULL;
	theApp.CompileShader(vertexShader, sizeof(vertexShader), "VSpart", "vs_4_0", &pVSBlob);

	// Create the vertex shader
//	ID3D11VertexShader* pVertexShader;
	hr = theApp.m_pD3Ddevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &m_pVertexShaderPart );
	if( FAILED( hr ) )
	{	
		pVSBlob->Release();
        return hr;
	}
	return hr;
}

HRESULT CStimServerApp::createVertexShaderShift(void)
{
	HRESULT hr;
    // Compile the vertex shader
	const char vertexShader[] = 
"cbuffer VS_ANIMATION : register (b0)\n"
"{\n"
"    float2  shift : packoffset(c0.x);\n"
"};\n"
//"cbuffer VS_PARAMS : register (b1)\n"
//"{\n"
//"    float4  color : packoffset(c0.x);\n"
//"    float   size  : packoffset(c1.x);\n"
//"};\n"
//"struct OUTPUT {\n"
//"    float4 Position   : SV_POSITION;\n"
//"    float4 Color      : COLOR;\n"
//"	 float	Size	   : PSIZE;\n"
//"};\n"
"float4 VSshift( float4 Pos : POSITION ) : SV_Position\n"
"{\n"
"	 float4 Position;\n"
"	 Position.xy = Pos.xy+shift;\n"
"	 Position.zw = Pos.zw;\n"
//"	 output.Color = color;\n"
//"	 output.Size = size;\n"
"    return Position;\n"
"}\n"
;
    ID3DBlob* pVSBlob = NULL;
	theApp.CompileShader(vertexShader, sizeof(vertexShader), "VSshift", "vs_4_0", &pVSBlob);

	// Create the vertex shader
//	ID3D11VertexShader* pVertexShader;
	hr = theApp.m_pD3Ddevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &m_pVertexShaderShift );
	if( FAILED( hr ) )
	{	
		pVSBlob->Release();
        return hr;
	}
	return hr;
}

/*
HRESULT CStimServerApp::createPixelShaderMin(void)
{
	HRESULT hr;
    // Compile the pixel shader
	const char pixelShader[] = 
"float4 PSmin( float4 Pos : SV_POSITION, float4 Color : COLOR ) : SV_Target\n"
"{\n"
"    return Color;\n"
"//    return float4(1.0f, 1.0f, 1.0f, 1.0f);\n"
"}\n"
;
    ID3DBlob* pPSBlob = NULL;
	theApp.CompileShader(pixelShader, sizeof(pixelShader), "PSmin", "ps_4_0", &pPSBlob);

	// Create the pixel shader
	hr = theApp.m_pD3Ddevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_pPixelShaderMin );
	if( FAILED( hr ) )
	{	
		pPSBlob->Release();
        return hr;
	}
	return hr;
}


void CStimServerApp::setPixelShaderMin(void)
{
	theApp.m_pImmediateContext->PSSetShader( m_pPixelShaderMin, NULL, 0 );
//	TRACE("Setting Pixel Shader\n");
}
*/

HRESULT CStimServerApp::CompileShader(const char shaderSource[], SIZE_T SrcDataSize, const char shaderName[], const char shaderModel[], ID3DBlob** pSourceBlob)
{
    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif
	TRACE("%s", shaderSource);
	TRACE("Shader size: %u\n", SrcDataSize);
    ID3DBlob* pErrorBlob;
	HRESULT hr = D3DCompile(
		shaderSource,
		SrcDataSize,
		NULL,
		NULL,
		NULL,
		shaderName,
		shaderModel,	// gs_5_0
        dwShaderFlags,
		0,
		pSourceBlob,
		&pErrorBlob
		);
    if( FAILED(hr) )
    {
        if( pErrorBlob != NULL )
            OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
		ASSERT(false);
    }
    if( pErrorBlob ) pErrorBlob->Release();
	return hr;
}


BOOL CStimServerApp::CheckCommandLength(DWORD commandLength, DWORD requiredLength, LPCWSTR command)
{
	if (commandLength == requiredLength) return true;
	theApp.m_errorMask |= 2;
	CString errString;
	errString.Format(_T("Command length error in %s. Actual length is %u (expected: %u)."),
		command, commandLength, requiredLength);
	COutputList::AddString(errString);
	return false;
}
