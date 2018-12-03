#include "StdAfx.h"
#include "OutputWnd.h"
#include "StimulusPixel.h"
//#include <d2d1_1helper.h>
//using namespace D2D1;

extern CRITICAL_SECTION g_criticalDrawSection;

CStimulusPixel::CStimulusPixel(void)
{
	m_typeName = _T("pixel");
	HRESULT hr;
	if (CStimServerApp::m_pVertexShaderShift == NULL) {
		hr = CStimServerApp::createVertexShaderShift();
		ASSERT(SUCCEEDED(hr));
		if( FAILED( hr ) )
			ASSERT(false);
	}
	m_VSanimationData.shift = D2D1::Vector2F();
	hr = m_AnimationBuffer.Init(&m_VSanimationData, sizeof(m_VSanimationData));
	ASSERT(SUCCEEDED(hr));

    // Compile the pixel shader
	const char pixelShader[] = 
//"float4 PSmin( float4 Pos : SV_POSITION, float4 Color : COLOR ) : SV_Target\n"
"float4 PSmin( float4 Pos : SV_POSITION ) : SV_Target\n"
"{\n"
"//    return Color;\n"
"    return float4(1.0f, 1.0f, 1.0f, 1.0f);\n"
"}\n"
;
    ID3DBlob* pPSBlob = NULL;
	theApp.CompileShader(pixelShader, sizeof(pixelShader), "PSmin", "ps_4_0", &pPSBlob);

	// Create the pixel shader
	hr = theApp.m_pD3Ddevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_pPixelShaderMin );
	if( FAILED( hr ) )
	{	
		pPSBlob->Release();
        ASSERT(false);
	}
}


CStimulusPixel::~CStimulusPixel(void)
{
	m_AnimationBuffer.~CDeviceConstBuffer();
//	CStimulus::~CStimulus();
}


void CStimulusPixel::Draw(void)
{
	if (theApp.m_drawMode) theApp.EndDraw();

    // Set vertex buffer
//    UINT stride = sizeof( XMFLOAT3 );
    UINT stride = 12;
    UINT offset = 0;
	theApp.m_pImmediateContext->IASetVertexBuffers( 0, 1, &m_VertexBuffer.m_pDeviceBuffer, &stride, &offset );
    // Set primitive topology
    theApp.m_pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_POINTLIST );
	theApp.m_pImmediateContext->RSSetViewports( 1, &m_vp );
	theApp.m_pImmediateContext->VSSetShader( theApp.m_pVertexShaderShift, NULL, 0 );
	theApp.m_pImmediateContext->PSSetShader( m_pPixelShaderMin, NULL, 0 );
	ID3D11Buffer* pBuffers[1] = {m_AnimationBuffer.m_pDeviceBuffer};
	theApp.m_pImmediateContext->VSSetConstantBuffers(0, 1, pBuffers);
	theApp.m_pImmediateContext->Draw( 1, 0 );	// (Vertex Count, Start)
}


void CStimulusPixel::Command(unsigned char message[], DWORD messageLength)
{
}


void CStimulusPixel::Moveto(bool deferred, float x, float y)
{
	float point[2] = {x, y};
	EnterCriticalSection(&g_criticalDrawSection);
	HRESULT hr = m_AnimationBuffer.Update(point, 2);
	LeaveCriticalSection(&g_criticalDrawSection);
}

void CStimulusPixel::GetPos(float pos[2])
{
//	pos[0] =  m_deferableParams.transform._31 - theApp.m_contextTransform._31;
//	pos[1] = -m_deferableParams.transform._32 + theApp.m_contextTransform._32;
	ASSERT(false);	// not implemented
}
/*
bool CStimulusPixel::SetAnimParam(BYTE mode, float value)
{
	COutputList::AddString(L"Currently linear range animations are not implemented for pixel stimuli.");
	return false;
}
*/