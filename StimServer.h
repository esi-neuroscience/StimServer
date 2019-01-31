
// StimServer.h : main header file for the StimServer application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols

#include <D2d1_1.h>
#include <d3d11_1.h>

// #define WM_SEQUENCE_ERR (WM_APP + 1)


// CStimServerApp:
// See StimServer.cpp for the implementation of this class
//

class CStimServerApp : public CWinAppEx
{
public:
	CStimServerApp();


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
public:
	CWinThread* m_pDisplayThread;
private:
	CWinThread* m_pPipeThread;
	HANDLE m_hStimServerDone;
public:
	ID2D1DeviceContext* m_pContext;
//	HANDLE m_hArrayMutex;
//	HANDLE m_hDrawMutex;
	ID3D11Device* m_pD3Ddevice;
	ID3D11DeviceContext* m_pImmediateContext;
	ID3D11VertexShader* m_pVertexShader;
	bool m_drawMode;
	void BeginDraw(void);
	void EndDraw(void);
	afx_msg void OnHelpAbouthardware();
	CString m_featureLevelString;
//	static void setVertexShaderCol(void);
	static ID3D11VertexShader* m_pVertexShaderPart;
	static ID3D11VertexShader* m_pVertexShaderShift;
	static ID3D11RenderTargetView* m_pRenderTargetView;
	static ID2D1Factory1 * m_d2dFactory;
	static ID2D1SolidColorBrush* m_pWhiteBrush;
	static D2D1::ColorF m_defaultDrawColor;
	static D2D1::ColorF m_defaultOutlineColor;
	static D2D1_MATRIX_3X2_F m_contextTransform;

	static HRESULT createVertexShaderCol(void);
	static HRESULT createVertexShaderPart(void);
	static HRESULT createVertexShaderShift(void);
//	static HRESULT createPixelShaderMin(void);
	static HRESULT CompileShader(const char shaderSource[], SIZE_T SrcDataSize, const char shaderName[], const char shaderModel[], ID3DBlob** pSourceBlob);
	static BOOL CheckCommandLength(DWORD commandLength, DWORD requiredLength, LPCWSTR command);
	bool m_deferredMode;
	short m_errorMask;
	short m_errorCode;
};

extern CStimServerApp theApp;
//extern D2D1_SIZE_F g_ScreenSize;
