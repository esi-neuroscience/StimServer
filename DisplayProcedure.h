/*
// DisplayProcedure.h : main header file for DisplayProcedure
//

*/

#pragma once
#include "stdafx.h"
#include "StimServerDoc.h"
#include <dxgi.h>	// includes dxgi1_2.h

//UINT DisplayProcedure(LPVOID pParam);

class CDisplay 
{
public:	
	static UINT InitializeWindow();
	static UINT PresentLoop( LPVOID pParam );
	static UINT InvertGammaExponent(float exponent);

private:	
	static CWnd m_DirectXWnd;
	static CStimServerDoc* pDoc;
	static IDXGIOutput1* pOutput;
	static IDXGISwapChain1* pSwapChain;
	static DXGI_PRESENT_PARAMETERS presentPars;
	static DXGI_GAMMA_CONTROL_CAPABILITIES GammaCaps;
};

