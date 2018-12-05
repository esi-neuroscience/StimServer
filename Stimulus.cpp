// Stimulus.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "StimServer.h"
#include "Stimulus.h"
#include "OutputWnd.h"
#include "LoadD2DBitmap.h"
#define _USE_MATH_DEFINES TRUE
#include <math.h>
#include <d3d11_1.h>
//#include <d3dcompiler.inl>
#include <d3dcompiler.h>
//#include <d3d11shader.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <Mferror.h>
#include <DirectXMath.h>	// XMFLOAT3
#include <D2d1_1helper.h>
#include "NumericBinaryFile.h"
#include "PipeProcedure.h"
#include "Anim.h"

using namespace DirectX;
using namespace D2D1;

#ifdef _DEBUG
extern HRESULT LogMediaType(IMFMediaType *pType);
#endif

extern D2D1_SIZE_F g_ScreenSize;
extern CRITICAL_SECTION g_criticalMapSection;
extern CRITICAL_SECTION g_criticalDrawSection;

D2D1_RECT_F CPhotoDiode::m_rect;
PDCSR CPhotoDiode::CSR = {0, 0};
ID2D1SolidColorBrush* CPhotoDiode::m_pBlackBrush;
// bool  CPhotoDiode::m_enabled = true;

//ID3D11GeometryShader* CStimulusPart::m_pGeometryShader;
//ID3D11GeometryShader* CStimulusParticle::m_pGeometryShader;
//ID3D11PixelShader* CStimulusPart::m_pPixelShader;
ID3D11PixelShader* CStimulus::pPixelShaderTex = NULL;
//ID3D11InputLayout* CStimulus::pVertexLayout = NULL;
//ID3D11InputLayout* CStimulusSymbol::pVertexTexLayout = NULL;

//DEFINE_GUID(BITSPERPIXEL, 0xC496F370, 0x2F8B, 0x4F51, 0xAE, 0x46, 0x9C, 0xFC, 0x1B, 0xC8, 0x2A, 0x47);


HRESULT CompileShaderFromFile(
	LPCWSTR wzFilename,
	LPCSTR	pEntrypoint,
	LPCSTR	pTarget,
	ID3DBlob** pSourceBlob)
{
	// Compile the pixel shader
    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_WARNINGS_ARE_ERRORS;
#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	HRESULT hr;
    ID3DBlob* pErrorBlob = NULL;
	hr = D3DCompileFromFile(
		wzFilename,
		NULL,
		NULL,
		pEntrypoint,
		pTarget,	// ps_5_0
        dwShaderFlags,
		0,
		pSourceBlob,
		&pErrorBlob
		);
    if( FAILED(hr) )
    {
        if( pErrorBlob != NULL ) {
            OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
            COutputList::AddString( CString((char*)pErrorBlob->GetBufferPointer() ));
		}
		TRACE("HRESULT: %X\n", hr);
		CString errString = _T("Error reading shader source: ");
		errString += wzFilename;
		switch (hr) {
		case 0x80070003:
			errString += "\r\n Cannot find the path specified.";
			break;
		case 0x8007052E:
			errString += "\r\n Logon Failure: unknown user name or bad password.";
			break;
		case 0x80004005:	// E_FAIL unspecified failure
			break;
		default:
			errString.AppendFormat(_T("\r\nError: 0x%X"), hr);
		}
		COutputList::AddString(errString);
 		theApp.m_errorCode = 4;
		theApp.m_errorMask |= 1;
   }
    if( pErrorBlob ) pErrorBlob->Release();
	return hr;
}


// CStimulus

//IMPLEMENT_SERIAL(CStimulus, CObject, 0)

CStimulus::CStimulus()
	: m_enabledCopy(false)
	, m_supressed(false)
	, m_animation(NULL)
	, m_protected(false)
	, m_errorCode(0)
{
	m_enabled = false;
}

CStimulus::~CStimulus()
{
	m_enabled = false;
	if (m_animation)
	{
		m_animation->m_pStimulus = NULL;
		CString errString;
		errString.Format(_T("Stimulus destruction: implizit deassignment of animation!"));
		COutputList::AddString(errString);
	}
}

bool CStimulus::SetAnimParam(BYTE mode, float value)
{
	CString temp;
	temp.Format(_T("Currently linear range animations are not implemented for %s stimuli."), m_typeName);
	COutputList::AddString(temp);
	return false;
}

CD2DStimulus::CD2DStimulus(void)
{
	m_transform = theApp.m_contextTransform;
}

void CD2DStimulus::GetPos(float pos[2])
{
	pos[0] =  m_transform._31 - theApp.m_contextTransform._31;
	pos[1] = -m_transform._32 + theApp.m_contextTransform._32;
}

void CD2DStimulus::makeCopy(void)
{
	CStimulus::makeCopy();
	m_transformCopy = m_transform;
}

void CD2DStimulus::getCopy(void)
{
	CStimulus::getCopy();
	m_transform = m_transformCopy;
}

void CD2DStimulus::Moveto(bool deferred, float x, float y)
{
	D2D1_MATRIX_3X2_F* pTransform;
	pTransform = &(deferred ? m_transformCopy : m_transform);
//	TRACE("Translation: %f, %f\n", pTransform->_31, pTransform->_32);
	pTransform->_31 = theApp.m_contextTransform._31 + x;
	pTransform->_32 = theApp.m_contextTransform._32 - y;
}

CLoadedStimulus::CLoadedStimulus(void)
{
}


CLoadedStimulus::~CLoadedStimulus(void)
{
}


C3DStimulus::C3DStimulus(void)
{
}


C3DStimulus::~C3DStimulus(void)
{
}


// This is the default implementation which simply moves the viewport of the stimulus
void C3DStimulus::Moveto(bool deferred, float x, float y)
{
	D3D11_VIEWPORT* pVP = deferred ? &m_vpCopy : &m_vp;
    // Update the viewport
	pVP->TopLeftX = (g_ScreenSize.width-pVP->Width)/2.0f + x;
	pVP->TopLeftY = (g_ScreenSize.height-pVP->Height)/2.0f - y;
}


void C3DStimulus::GetPos(float pos[2])
{
	pos[0] = m_vp.TopLeftX - (g_ScreenSize.width-m_vp.Width)/2.0f;
	pos[1] = (g_ScreenSize.height-m_vp.Height)/2.0f - m_vp.TopLeftY;
}


bool CStimulus::CreatePixelShaderTex(void)
{
	HRESULT hr;
	// Compile the pixel shader
	const char pixelShader[] = 
		"Texture2D<float4> Texture	: register(t0);\n"
		"sampler TexSampler : register(s0);\n"
		"struct INPUT {\n"
		"    float4 Pos        : SV_POSITION;\n"
		"    float4 Color      : COLOR;\n"
		"	 float2 TexCoord   : TEXCOORD;\n"
		"};\n"
		//			"float4 PSmin( float4 Pos : SV_POSITION, float4 Color : COLOR, float2 TexCoord : TEXCOORD) : SV_Target\n"
		"float4 PStex( INPUT input) : SV_Target\n"
		"{\n"
		"    return input.Color * Texture.Sample(TexSampler, input.TexCoord);\n"
		"}\n"
		;
	ID3DBlob* pPSBlob = NULL;
	hr = theApp.CompileShader(pixelShader, sizeof(pixelShader), "PStex", "ps_4_0", &pPSBlob);
	if( FAILED( hr ) )
	{	
		if (pPSBlob) pPSBlob->Release();
		pPixelShaderTex = NULL;
		return false;
	}
	TRACE("Texture Pixel Shader compiled\n");
	// Create the pixel shader
	hr = theApp.m_pD3Ddevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &pPixelShaderTex );
	if( FAILED( hr ) )
	{	
		pPSBlob->Release();
		pPixelShaderTex = NULL;
		return false;
	}
	TRACE("Texture Pixel Shader created\n");
	return true;
}


// CStimulusPic

IMPLEMENT_DYNAMIC( CStimulusPic, CLoadedStimulus)

CStimulusPic::CStimulusPic()
	: m_pBitmap(NULL)
	, m_iPhi(0)
{
	m_deferableParams.alpha = 1.0f;
	m_deferableParams.phiInc = 0;
	m_typeName = _T("picture");
}

bool CStimulusPic::Init(LPCWSTR wzFilename)
{
	TRACE("Pic Init: %S\n", wzFilename);

	if (!LoadD2DBitmap(wzFilename, &m_pBitmap)) return false;
	D2D1_SIZE_F sizeF = m_pBitmap->GetSize();
	TRACE(" SizeX: %f, SizeY: %f\n", sizeF.width, sizeF.height);
	m_destRect = D2D1::RectF(sizeF.width/-2.f, sizeF.height/-2.f, sizeF.width/2.f, sizeF.height/2.f);
	m_deferableParams.transform = D2D1::Matrix4x4F(
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 0.f, 0.f,
		0.f, 0.f, 0.f, 1.f);
	TRACE("End of Init\n");
	return true;
}

CStimulusPic::~CStimulusPic()
{
	if (m_pBitmap) {
		m_pBitmap->Release();
		TRACE("Bitmap Released\n");
		m_pBitmap = NULL;
	}
//	CStimulus::~CStimulus();
}


// CStimulusPic-Memberfunktionen

void CStimulusPic::Draw()
{
//	if (immediate) return;
//	TRACE("CStimulusPic::Draw\n");
	if (m_deferableParams.phiInc != 0) {
		m_iPhi = (m_iPhi + m_deferableParams.phiInc) % 7200;
		Rotate(false, m_iPhi * (0.05f*(float)M_PI/180.f));
	}
	if (!theApp.m_drawMode) theApp.BeginDraw();
	theApp.m_pContext->DrawBitmap(
		m_pBitmap,
		m_destRect,
		m_deferableParams.alpha,
		D2D1_INTERPOLATION_MODE_LINEAR,
		NULL,
		&m_deferableParams.transform);
}


void CStimulusPic::Command(unsigned char message[], DWORD messageLength)
{
//	TRACE("CStimulusPic::Command\n");
	switch (message[0]) {
	case 1:
		(theApp.m_deferredMode ? m_deferableParamsCopy : m_deferableParams).alpha = message[1]/255.f;
		break;
	case 2:
		(theApp.m_deferredMode ? m_deferableParamsCopy : m_deferableParams).phiInc = message[1];
		break;
	case 4:
		float *pPhi;
		pPhi = (float*) &message[1];
		TRACE("Picture orientation: %f\n", *pPhi);
		Rotate(theApp.m_deferredMode, *pPhi*(float)M_PI/180.f);
		break;
	case 7:
		WritePipe(&m_errorCode, sizeof(m_errorCode));
		m_errorCode = 0;
		break;
	}
}

void CStimulusPic::Rotate(bool deferred, float phi)
{
	D2D1_MATRIX_4X4_F* pTransform = &(deferred ? m_deferableParamsCopy : m_deferableParams).transform;
	pTransform->_21 = sin(phi);
	pTransform->_12 = -pTransform->_21;
	pTransform->_11 = cos(phi);
	pTransform->_22 =  pTransform->_11;
}

void CStimulusPic::Moveto(bool deferred, float x, float y)
{
	D2D1_MATRIX_4X4_F* pTransform = &(deferred ? m_deferableParamsCopy : m_deferableParams).transform;
	pTransform->_41 =  x;
	pTransform->_42 = -y;
}

void CStimulusPic::makeCopy(void)
{
	CStimulus::makeCopy();
	m_deferableParamsCopy = m_deferableParams;
}

void CStimulusPic::getCopy(void)
{
	CStimulus::getCopy();
	TRACE("CStimulusPic::getCopy\n");
	m_deferableParams = m_deferableParamsCopy;
}

void CStimulusPic::GetPos(float pos[2])
{
	pos[0] =  m_deferableParams.transform._41;
	pos[1] = -m_deferableParams.transform._42;
}

bool CStimulusPic::SetAnimParam(BYTE mode, float value)
{
	if (mode != 1)
	{
		CString errString;
		errString.Format(_T("Mode %u is invalid for Linear Range Animation of picture stimulus."), mode);
		COutputList::AddString(errString);
		return false;
	}
	// change global transparency
	m_deferableParams.alpha = value;
	if (theApp.m_deferredMode)
		m_deferableParamsCopy.alpha = value;
	return true;
}

// CPixelShader

CPixelShader::CPixelShader()
	: m_phiInc(0.0)
	, m_pPixelShader(NULL)
{
	m_updateFlags.all = 0;
	TRACE("Pixel Shader Kontruktor\n");
//	m_typeName = _T("pixel shader");
}

CPixelShader::~CPixelShader()
{
	if (m_pPixelShader) m_pPixelShader->Release();	// if Init fails then there is no valid shader
	TRACE("Pixel Shader Released\n");
}

// CPixelShader-Memberfunktionen

// Return ID3DBlob pointer for the reflector to support the old shader format
ID3DBlob* CPixelShader::CreateShader(LPCWSTR wzFilename)
{
	TRACE("Pixel Shader Init: %S\n", wzFilename);
	/*
	// Compile the pixel shader
    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_WARNINGS_ARE_ERRORS;
#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif
	*/
	HRESULT hr;
	ID3DBlob* pPSBlob = NULL;
	hr = CompileShaderFromFile(wzFilename, "PSmain", "ps_4_0", &pPSBlob);
	if (FAILED(hr)) return NULL;

	EnterCriticalSection(&g_criticalDrawSection);

	// Create the pixel shader
	hr = theApp.m_pD3Ddevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_pPixelShader );
	ASSERT(hr == S_OK);
	return pPSBlob;
}


void CPixelShader::Draw()
{
	if (theApp.m_drawMode) theApp.EndDraw();

    // Set vertex buffer
    UINT stride = sizeof( XMFLOAT3 );
    UINT offset = 0;
	theApp.m_pImmediateContext->IASetVertexBuffers( 0, 1, &VertexBufferUnitQuad.m_pDeviceBuffer, &stride, &offset );

    // Set primitive topology
    theApp.m_pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

	theApp.m_pImmediateContext->RSSetViewports( 1, &m_vp );

	theApp.m_pImmediateContext->GSSetShader( NULL, NULL, 0 );
	theApp.m_pImmediateContext->PSSetShader( m_pPixelShader, NULL, 0 );
	bool updateNow = false;
	if ((!theApp.m_deferredMode) && m_updateFlags.animatedValue)
	{
		m_updateFlags.animatedValue = 0;
		updateNow = true;
	}
	if (m_phiInc != 0.0f) {
		m_PSanimationData.phase += m_phiInc;
		updateNow = true;
	}
	if (updateNow)
	{
		HRESULT hr = m_AnimationBuffer.Update(&m_PSanimationData.phase, 1);
		ASSERT(hr == S_OK);
	}
	if ((!theApp.m_deferredMode) && m_updateFlags.constantBuffer)
	{
		UpdateConstantBuffer();
		m_updateFlags.constantBuffer = 0;
	}
}

void CPixelShader::SetParam(BYTE index, float value)
{
	TRACE("Param: %u, Value: %f\n", index, value);
	m_PSconstData.params[index] = value;
	m_updateFlags.constantBuffer = 1;
}

void CPixelShader::Command(unsigned char message[], DWORD messageLength)
{
//	TRACE("CStimulusPS::Command\n");
	switch (message[0])
	{
	case 2:
		{
		float* pPhiInc = theApp.m_deferredMode ? &m_phiIncCopy : &m_phiInc;
		float *dphi;
		dphi = (float*) &message[1];
		*pPhiInc = *dphi;
		}
		break;
	case 6:		// set animated value
		(theApp.m_deferredMode ? m_phaseCopy : m_PSanimationData.phase) = *((float*) &message[1]);
		m_updateFlags.animatedValue = 1;
		break;
	case 7:
		WritePipe(&m_errorCode, sizeof(m_errorCode));
		m_errorCode = 0;
		break;
	default:
		CString errString;
		errString.Format(_T("Invalid command code (%u) for pixel shader."), message[0]);
		COutputList::AddString(errString);
	}
}

void CPixelShader::makeCopy(void)
{
	CStimulus::makeCopy();
	m_phiIncCopy = m_phiInc;
	m_vpCopy = m_vp;
}

void CPixelShader::getCopy(void)
{
	CStimulus::getCopy();
	m_phiInc = m_phiIncCopy;
	m_vp = m_vpCopy;
	if (m_updateFlags.constantBuffer) UpdateConstantBuffer();
	if (m_updateFlags.animatedValue) m_PSanimationData.phase = m_phaseCopy-m_phiInc;
}

void CPixelShader::Moveto(bool deferred, float x, float y)
{
	C3DStimulus::Moveto(deferred, x, y);	// move the viewport
	m_PSconstData.center[0] = g_ScreenSize.width/2.0f + x;
	m_PSconstData.center[1] = g_ScreenSize.height/2.0f - y;

	m_updateFlags.constantBuffer = 1;
}

void CPixelShader::GetPos(float pos[2])
{
	C3DStimulus::GetPos(pos);
}

bool CPixelShader::SetAnimParam(BYTE mode, float value)
{
//	TRACE("Param: %u, Value: %f\n", mode, value);
	SetParam(mode-1, value);
	return true;
}


// CStimulusPS

CStimulusPS::CStimulusPS()
	: m_legacy(false)
{
//	m_updateFlags.all = 0;
	m_typeName = _T("pixel shader");
}

/*
CStimulusPS::~CStimulusPS()
{
//	CPixelShader::~CPixelShader();
}
*/

// CStimulusPS-Memberfunktionen

bool CStimulusPS::Init(LPCWSTR wzFilename)
{
	//ID3DBlob* pPSBlob = NULL;
	//pPSBlob = CreateShader(wzFilename);
	//if (pPSBlob == NULL) return false;
	HRESULT hr;
	//ID3D11ShaderReflection* pReflector = NULL; 
	//hr = D3D11Reflect( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), 
 //           &pReflector);
	//ASSERT(hr == S_OK);
//	pPSBlob->Release();

	//ID3D11ShaderReflectionVariable* pShaderVar;
	//D3D11_SHADER_VARIABLE_DESC pShaderVarDesc = {0};
	//pShaderVar = pReflector->GetVariableByName("width");
	//pShaderVar->GetDesc(&pShaderVarDesc);
	//TRACE("Shader Size: %u\n", (UINT) pShaderVarDesc.Size);
	//float* pWidth = (float*) pShaderVarDesc.DefaultValue;
	//m_legacy = (pWidth != NULL);
	float width = 0.0f;
	float height = 0.0f;
	//if (m_legacy) {
	//	width  = *pWidth;
	//	TRACE("Shader Width: %f\n", width);
	//	pShaderVar = pReflector->GetVariableByName("height");
	//	pShaderVar->GetDesc(&pShaderVarDesc);
	//	height  = *((float*) pShaderVarDesc.DefaultValue);
	//}
	//pReflector->Release();

    // Setup the viewport
	m_vp.Width =  width;
	m_vp.Height = height;
    m_vp.MinDepth = 0.0f;
	m_vp.MaxDepth = 1.0f;
	// TopLeft is set in Moveto
	C3DStimulus::Moveto(false, 0.0f, 0.0f);	// move the viewport
	m_PSconstData.center[0] = g_ScreenSize.width/2.0f;
	m_PSconstData.center[1] = g_ScreenSize.height/2.0f;
	m_PSconstData.width = 0.0f;
	m_PSconstData.height = 0.0f;

	for (unsigned int i = 0; i < NPARAMS; i++) m_PSconstData.params[i] = 0.0f;

	hr = m_ConstantBuffer.Init(&m_PSconstData, sizeof(m_PSconstData));
	ASSERT(hr == S_OK);
	
	TRACE("Constant Buffer Initialized\n");

	m_PSanimationData.phase = 0.0f;
	hr = m_AnimationBuffer.Init(&m_PSanimationData, sizeof(m_PSanimationData));
	ASSERT(hr == S_OK);

	TRACE("Animation Buffer Initialized\n");

	for (unsigned int i = 0; i < 4; i++)
	{
		float gray = i/3.0f;
		m_PScolorData.color[i] = D2D1::ColorF(gray, gray, gray, 1.0f);
	}
	hr = m_ColorBuffer.Init(&m_PScolorData, sizeof(m_PScolorData));
	ASSERT(hr == S_OK);
	LeaveCriticalSection(&g_criticalDrawSection);
	TRACE("End of Init\n");
	return true;
}

void CStimulusPS::Draw()
{
	CPixelShader::Draw();

	theApp.m_pImmediateContext->VSSetShader( theApp.m_pVertexShader, NULL, 0 );
	if ((!theApp.m_deferredMode) && m_updateFlags.colorBuffer)
	{
		UpdateColorBuffer();
		m_updateFlags.colorBuffer = 0;
	}

	ID3D11Buffer* pBuffers[3] = {
		m_ConstantBuffer.m_pDeviceBuffer,
		m_AnimationBuffer.m_pDeviceBuffer,
		m_ColorBuffer.m_pDeviceBuffer};
	theApp.m_pImmediateContext->PSSetConstantBuffers(0, 3, pBuffers);
	theApp.m_pImmediateContext->Draw( 4, 0 );	// (Vertex Count, Start)
}


void CStimulusPS::Command(unsigned char message[], DWORD messageLength)
{
//	TRACE("CStimulusPS::Command\n");
	switch (message[0])
	{
	float *param;
	case 1:		// set value of parameter
		param = (float*) &message[2];
		if (m_legacy)
		{
			switch (message[1])
			{
			case 1:
				m_PSconstData.width = *param;
				break;
			case 2:
				m_PSconstData.height = *param;
				break;
			default:
				m_PSconstData.params[message[1]-3] = *param;
			}
			m_updateFlags.constantBuffer = 1;
		}
		else
		{
			SetParam(message[1]-1, *param);
		}
		break;
	case 5:		// set a color
		m_PScolorData.color[message[1]-1] = D2D1::ColorF(message[2]/255.0f, message[3]/255.0f, message[4]/255.0f, message[5]/255.0f);
		m_updateFlags.colorBuffer=1;
		break;
	case 9:		// set the size of the shader
		{
			D3D11_VIEWPORT* pVP = theApp.m_deferredMode ? &m_vpCopy : &m_vp;
			float width = (float) *((WORD*) &message[1]);
			float height = (float) *((WORD*) &message[3]);
			pVP->TopLeftX += (pVP->Width-width)/2.0f;
			pVP->TopLeftY += (pVP->Height-height)/2.0f;
			pVP->Width = width;
			pVP->Height = height;
			m_PSconstData.width = width;
			m_PSconstData.height = height;

			m_updateFlags.constantBuffer = 1;
		}
		break;
	default:
		CPixelShader::Command(message, messageLength);
	}
}

void CStimulusPS::getCopy(void)
{
	CPixelShader::getCopy();
	if (m_updateFlags.colorBuffer) UpdateColorBuffer();
	TRACE("Constant Flag: %u, ColorFlag: %u\n", m_updateFlags.constantBuffer, m_updateFlags.colorBuffer);
	m_updateFlags.all = 0;
}


void CPhotoDiode::Init()
{
	SetPosition(0);
	HRESULT hr;
	hr = theApp.m_pContext->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::Black),
		&m_pBlackBrush
		);
	ASSERT(hr == S_OK);
}

void CPhotoDiode::Cleanup()
{
	m_enabled = false;
	m_pBlackBrush->Release();
}

/*
CPhotoDiode::CPhotoDiode(D2D1_RECT_F PDrect)
{
	m_rect = PDrect;
	m_enabled = true;
	CSR.flicker = 0;
	CSR.lit = 0;
	HRESULT hr;
	hr = theApp.m_pContext->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::Black),
		&m_pBlackBrush
		);
	ASSERT(hr == S_OK);
}
*/
/*
CPhotoDiode::~CPhotoDiode(void)
{
	m_enabled = false;
	m_pBlackBrush->Release();
	TRACE("Photo Diode Released\n");
}
*/
void CPhotoDiode::SetPosition(BYTE position)
{
	float y = position & 1 ? g_ScreenSize.height/2.f-15.f : g_ScreenSize.height/-2.f;
	m_rect = D2D1::RectF(g_ScreenSize.width/-2.f, y, g_ScreenSize.width/-2.f+15.f, y+15.f);
}

void CPhotoDiode::Draw(void)
{
	theApp.m_pContext->FillRectangle(
			&m_rect,
			CSR.lit ? theApp.m_pWhiteBrush : m_pBlackBrush);
	if (CSR.flicker) CSR.lit ^= 1;
}



CStimulusMov::CStimulusMov(void)
{
	m_typeName = _T("movie");
}


CStimulusMov::~CStimulusMov(void)
{
//	CStimulus::~CStimulus();
}


// CStimulusMov-Memberfunktionen

bool CStimulusMov::Init(LPCWSTR wzFilename)
{
	TRACE("Movie Init: %S\n", wzFilename);

	HRESULT hr = S_OK;

	hr = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
	
	IMFAttributes *pReaderAttributes = NULL;
	hr = MFCreateAttributes(&pReaderAttributes, 1);
	pReaderAttributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, 1);

	IMFSourceReader *sourceReader;
	if (SUCCEEDED(hr)) hr = MFCreateSourceReaderFromURL(wzFilename, pReaderAttributes, &sourceReader); 
//	if (SUCCEEDED(hr)) hr = MFCreateSourceReaderFromURL(wzFilename, NULL, &sourceReader); 
	if (SUCCEEDED(hr)) {

		IMFMediaType *pCurrentMediaType = NULL;
		hr = sourceReader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, &pCurrentMediaType);
		ASSERT(SUCCEEDED(hr));
////		hr = LogMediaType(pCurrentMediaType);
		ASSERT(SUCCEEDED(hr));
		BOOL selected;
		hr = sourceReader->GetStreamSelection(MF_SOURCE_READER_FIRST_VIDEO_STREAM, &selected);
		TRACE("First Video Stream Selected: %u\n", selected);
		ASSERT(SUCCEEDED(hr));


		union {
			UINT64 combined;
			struct {
				UINT32 height;
				UINT32 width;
			};
		} frameSize;
		IMFMediaType *pNativeMediaType = NULL;
		hr = sourceReader->GetNativeMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &pNativeMediaType);
		if (SUCCEEDED(hr)) {
			GUID guidMajorType;
			hr = pNativeMediaType->GetMajorType(&guidMajorType);
			ASSERT(SUCCEEDED(hr));
			ASSERT(guidMajorType == MFMediaType_Video);
			hr = pNativeMediaType->GetUINT64(MF_MT_FRAME_SIZE, &frameSize.combined);
			ASSERT(SUCCEEDED(hr));
			TRACE("Width: %u, Height:%u\n", frameSize.width, frameSize.height);

			IMFMediaType *pMediaType = NULL;
			hr = MFCreateMediaType(&pMediaType);
			ASSERT(SUCCEEDED(hr));
			hr = pMediaType->SetGUID(MF_MT_MAJOR_TYPE, guidMajorType);
			ASSERT(SUCCEEDED(hr));
			hr = pMediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB24);
//			hr = pMediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_YUY2);
			ASSERT(SUCCEEDED(hr));

			GUID guidNativeSubtype;	// DEBUG
			hr = pNativeMediaType->GetGUID(MF_MT_SUBTYPE, &guidNativeSubtype);
			ASSERT(SUCCEEDED(hr));
#ifdef _DEBUG
			TRACE("Native Subtype: %x %x %x %x\n", guidNativeSubtype.Data1, guidNativeSubtype.Data2, guidNativeSubtype.Data3, guidNativeSubtype.Data4);
			hr = LogMediaType(pNativeMediaType);
			hr = LogMediaType(pMediaType);
#endif

			hr = sourceReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, NULL, pNativeMediaType);
			if (hr == MF_E_INVALIDMEDIATYPE) {
				MFT_REGISTER_TYPE_INFO inputTypeInfo = {guidMajorType, guidNativeSubtype};
				GUID guidSubtype;
				hr = pMediaType->GetGUID(MF_MT_SUBTYPE, &guidSubtype);
				ASSERT(SUCCEEDED(hr));
//				MFT_REGISTER_TYPE_INFO outputTypeInfo = {guidMajorType, guidSubtype};
				IMFActivate **ppMFTActivate;
				UINT32 numMFTActivate;
				hr = MFTEnumEx(MFT_CATEGORY_VIDEO_DECODER,
					MFT_ENUM_FLAG_SYNCMFT | MFT_ENUM_FLAG_TRANSCODE_ONLY | MFT_ENUM_FLAG_SORTANDFILTER,
					&inputTypeInfo,
//					&outputTypeInfo,
					NULL,
					&ppMFTActivate,
					&numMFTActivate);
				TRACE("Number of Decoders: %u\n", numMFTActivate);
				ASSERT(SUCCEEDED(hr));
				IMFTransform *pDecoder = NULL;      // Pointer to the decoder.
				DWORD nInputStreams;
				DWORD nOutputStreams;
				for (unsigned int i = 0; i < numMFTActivate; i++) {
					ppMFTActivate[i]->ActivateObject(IID_PPV_ARGS(&pDecoder));
					hr = pDecoder->GetStreamCount(&nInputStreams, &nOutputStreams);
					TRACE("Input Streams: %u, OutputStreams: %u\n", nInputStreams, nOutputStreams);
					ASSERT(SUCCEEDED(hr));
					ASSERT((nInputStreams == 1) && (nOutputStreams == 1));
					/*
					IMFAttributes *decoderAttributes;
					hr = pDecoder->GetAttributes(&decoderAttributes);
					ASSERT(SUCCEEDED(hr));
					*/
					DWORD inputID = 0;
					DWORD outputID = 0;
					hr = pDecoder->GetStreamIDs(1, &inputID, 1, &outputID);
					if (hr ==  E_NOTIMPL) hr = S_OK;	// the defaults of 0 are initialized
					ASSERT(SUCCEEDED(hr));

					IMFMediaType *pDecoderInputType = NULL;
					hr = pDecoder->GetInputAvailableType(inputID, 0, &pDecoderInputType);
					ASSERT(SUCCEEDED(hr));
////					hr = LogMediaType(pDecoderInputType);

					IMFMediaType *pNativeMediaTypeMin = NULL;
					hr = MFCreateMediaType(&pNativeMediaTypeMin);
					ASSERT(SUCCEEDED(hr));

					UINT32 nItems;
					hr = pNativeMediaType->GetCount(&nItems);
					TRACE("Number of Items: %u\n", nItems);
					ASSERT(SUCCEEDED(hr));
					GUID guidKey;
					const GUID bitspersample = {0xC496F370, 0x2F8B, 0x4F51, 0xAE, 0x46, 0x9C, 0xFC, 0x1B, 0xC8, 0x2A, 0x47};
					PROPVARIANT value;
					for (UINT32 index = 0; index < nItems; index++) {
						PropVariantInit(&value);
						hr = pNativeMediaType->GetItemByIndex(index, &guidKey, &value);
						ASSERT(SUCCEEDED(hr));
						if ((guidKey != bitspersample) &&
							(guidKey != MF_MT_SAMPLE_SIZE) &&
//							(guidKey != MF_MT_FRAME_RATE) &&
//							(guidKey != MF_MT_FRAME_SIZE) &&
							(guidKey != MF_MT_PIXEL_ASPECT_RATIO) &&
							(guidKey != MF_MT_ALL_SAMPLES_INDEPENDENT) &&
							(guidKey != MF_MT_INTERLACE_MODE)) {
							hr = pNativeMediaTypeMin->SetItem(guidKey, value);
							ASSERT(SUCCEEDED(hr));
						}
						hr = PropVariantClear(&value);
						ASSERT(SUCCEEDED(hr));
					}
			/*
			hr = pNativeMediaTypeMin->SetGUID(MF_MT_MAJOR_TYPE, guidMajorType);
			ASSERT(SUCCEEDED(hr));
			hr = pNativeMediaTypeMin->SetGUID(MF_MT_SUBTYPE, guidNativeSubtype);
			ASSERT(SUCCEEDED(hr));
			*/
					/*
					hr = pNativeMediaType->CopyAllItems(pNativeMediaTypeMin);
					ASSERT(SUCCEEDED(hr));
					hr = pNativeMediaTypeMin->DeleteItem(MF_MT_SAMPLE_SIZE);
					ASSERT(SUCCEEDED(hr));
					hr = pNativeMediaTypeMin->DeleteItem(MF_MT_PIXEL_ASPECT_RATIO);
					ASSERT(SUCCEEDED(hr));
					hr = pNativeMediaTypeMin->DeleteItem(MF_MT_INTERLACE_MODE);
					ASSERT(SUCCEEDED(hr));
					hr = pNativeMediaTypeMin->DeleteItem(MF_MT_FRAME_RATE);
					ASSERT(SUCCEEDED(hr));
					hr = pNativeMediaTypeMin->DeleteItem(MF_MT_ALL_SAMPLES_INDEPENDENT);
					ASSERT(SUCCEEDED(hr));
//					hr = pNativeMediaTypeMin->DeleteItem(BITSPERPIXEL);
//					ASSERT(SUCCEEDED(hr));
					*/
////					hr = LogMediaType(pNativeMediaTypeMin);
					hr = pDecoder->SetInputType(inputID, pNativeMediaTypeMin, 0);
					ASSERT(SUCCEEDED(hr));
//					pDecoder->GetOutputAvailableType(
					pDecoder->Release();
					ppMFTActivate[i]->ShutdownObject();
					ppMFTActivate[i]->Release();
					pNativeMediaTypeMin->Release();
				}
				CoTaskMemFree(ppMFTActivate);
			}
//			ASSERT(SUCCEEDED(hr));
			pMediaType->Release();
			pNativeMediaType->Release();
		}
		DWORD actualStreamIndex;
		DWORD streamFlags;
		LONGLONG timestamp;
		IMFSample *pSample;
		hr = sourceReader->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &actualStreamIndex, &streamFlags, &timestamp, &pSample);
		ASSERT(SUCCEEDED(hr));
		DWORD bufferCount;
		hr = pSample->GetBufferCount(&bufferCount);
		ASSERT(SUCCEEDED(hr));
		LONGLONG sampleDuration;
		hr = pSample->GetSampleDuration(&sampleDuration);
		ASSERT(SUCCEEDED(hr));
		LONGLONG sampleTime;
		hr = pSample->GetSampleTime(&sampleTime);
		ASSERT(SUCCEEDED(hr));
		TRACE("Buffer Count: %u, Sample Duration: %fms, Sample Time:%fms\n", bufferCount, sampleDuration/10000.0f, sampleTime/10000.0f);
		ASSERT(bufferCount == 1);
		// CHECK IF UNCOMPRESSED
		IMFMediaBuffer *pBuffer;
		hr = pSample->ConvertToContiguousBuffer(&pBuffer);
		IMF2DBuffer *p2Dbuffer;
//		hr = pBuffer->QueryInterface(__uuidof(IMF2DBuffer), (void**)(&p2Dbuffer) );
//		hr = pBuffer->QueryInterface(IID_IMF2DBuffer, (void**)(&p2Dbuffer) );
		hr = pBuffer->QueryInterface(IID_IMF2DBuffer, reinterpret_cast<void**>(&p2Dbuffer) );
		if (hr != E_NOINTERFACE) {
			TRACE("IMF2DBuffer interface to be implemented\n");
			p2Dbuffer->Release();
		} else {
			IMFMediaBuffer *pMediaBuffer;
			hr = pBuffer->QueryInterface(IID_IMFMediaBuffer, reinterpret_cast<void**>(&pMediaBuffer) );
			ASSERT(SUCCEEDED(hr));
			BYTE *pByteBuffer;
			DWORD maxLength;
			DWORD currentLength;
			hr = pMediaBuffer->Lock(&pByteBuffer, &maxLength, &currentLength);
			TRACE("Max. Length: %u, Current Length: %u\n", maxLength, currentLength);
			ASSERT(SUCCEEDED(hr));
			D2D1_PIXEL_FORMAT pixelFormat = {DXGI_FORMAT_R8G8B8A8_UINT, D2D1_ALPHA_MODE_IGNORE};
			D2D1_BITMAP_PROPERTIES1 bitmapProperties;
			bitmapProperties.pixelFormat = pixelFormat;
			bitmapProperties.dpiX = 0;
			bitmapProperties.dpiY = 0;
			bitmapProperties.bitmapOptions = D2D1_BITMAP_OPTIONS_NONE;
			bitmapProperties.colorContext = NULL;
//			theApp.m_pContext->SetTarget(m_pBitmap);
			hr = theApp.m_pContext->CreateBitmap(
				D2D1::SizeU(frameSize.width, frameSize.height),
				pByteBuffer,
				frameSize.width*3,	// pitch!
				bitmapProperties,
				&m_pBitmap);
			ASSERT(SUCCEEDED(hr));
			hr = pMediaBuffer->Unlock();
			ASSERT(SUCCEEDED(hr));
			pMediaBuffer->Release();
		}
		ASSERT(SUCCEEDED(hr));
		if (SUCCEEDED(hr))
		{
			/*
			// Create a Direct2D bitmap.
			hr = theApp.m_pContext->CreateBitmap(D2D1::SizeU(frameSize.width, frameSize.height),

				converter,
				&m_pBitmap);
			ASSERT(hr == S_OK);
			D2D1_SIZE_F sizeF = m_pBitmap->GetSize();
			TRACE(" SizeX: %f, SizeY: %f\n", sizeF.width, sizeF.height);
			m_destRect = D2D1::RectF(sizeF.width/-2.f, sizeF.height/-2.f, sizeF.width/2.f, sizeF.height/2.f);
			m_transform = D2D1::Matrix4x4F(
				1.f, 0.f, 0.f, 0.f,
				0.f, 1.f, 0.f, 0.f,
				0.f, 0.f, 0.f, 0.f,
				0.f, 0.f, 0.f, 1.f);
			*/
		}
		sourceReader->Release();
	}
	/*
	if (SUCCEEDED(hr))
	{
		// Create a Direct2D bitmap from the WIC bitmap.
		hr = theApp.m_pContext->CreateBitmapFromWicBitmap(
			converter,
			&m_pBitmap);
		ASSERT(hr == S_OK);
		converter->Release();
		D2D1_SIZE_F sizeF = m_pBitmap->GetSize();
		TRACE(" SizeX: %f, SizeY: %f\n", sizeF.width, sizeF.height);
		m_destRect = D2D1::RectF(sizeF.width/-2.f, sizeF.height/-2.f, sizeF.width/2.f, sizeF.height/2.f);
		m_transform = D2D1::Matrix4x4F(
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 0.f, 0.f,
		0.f, 0.f, 0.f, 1.f);
	}
	*/
	TRACE("End of Movie Init\n");
	return SUCCEEDED(hr);
}

void CStimulusMov::Draw()
{
	/*
	if (m_phiInc != 0) {
		m_iPhi = (m_iPhi + m_phiInc) % 7200;
		Rotate(false, m_iPhi * (0.05f*(float)M_PI/180.f));
	}
	if (!theApp.m_drawMode) theApp.BeginDraw();
	theApp.m_pContext->DrawBitmap(m_pBitmap, m_destRect, m_alpha, D2D1_INTERPOLATION_MODE_LINEAR, NULL, &m_transform);
	*/
}


void CStimulusMov::Command(unsigned char message[], DWORD messageLength)
{
	/*
//	TRACE("CStimulusPic::Command\n");
	switch (message[0]) {
	case 0: {
		bool* pEnabled = deferred ? &m_enabledCopy : &m_enabled;
		*pEnabled = message[1];
//		m_enabled = message[1];
		}
		break;
	case 1: {
		float* pAlpha = deferred ? &m_alphaCopy : & m_alpha;
		*pAlpha = message[1]/255.f;
//		m_alpha = message[1]/255.f;
		}
		break;
	case 2: {
		char* pPhiInc = deferred ? &m_phiIncCopy : &m_phiInc;
		*pPhiInc = message[1];
//		m_phiInc = message[1];
		}
		break;
	case 3: {
		float *cx, *cy;
		cx = (float*) &message[1];
		cy = (float*) &message[5];
		TRACE("Move Picture to: %f, %f\n", *cx, *cy);
		D2D1_MATRIX_4X4_F* pTransform = deferred ? &m_transformCopy : &m_transform;
		pTransform->_41 =  *cx;
		pTransform->_42 = -*cy;
//		m_transform._41 =   *cx;
//		m_transform._42 = -*cy;
		}
		break;
	case 4:
		float *pPhi;
		pPhi = (float*) &message[1];
		TRACE("Picture orientation: %f\n", *pPhi);
		Rotate(deferred, *pPhi*(float)M_PI/180.f);
		break;
	case 7:
		WritePipe(&m_errorCode, sizeof(m_errorCode));
		m_errorCode = 0;
		break;
	}
	*/
}

void CStimulusMov::makeCopy(void)
{
	/*
	CStimulus::makeCopy();
	m_alphaCopy = m_alpha;
	m_phiIncCopy = m_phiInc;
	m_transformCopy = m_transform;
	*/
}
/*
bool CStimulusMov::SetAnimParam(BYTE mode, float value)
{
		COutputList::AddString(L"Currently linear range animations are not implemented for movie stimuli.");
		return false;
}
*/
void CStimulusMov::GetPos(float pos[2])
{
//	pos[0] =  m_deferableParams.transform._41;
//	pos[1] = -m_deferableParams.transform._42;
}



CStimulusPart::CStimulusPart(unsigned short size[2])
	: m_nParticles(0)
	, m_shift(0.0f)
	, m_velocity(0.0f)
	, m_symbol(NULL)
	, m_flags(0)
{
	m_typeName = _T("particle");
    // Setup the viewport
	m_vp.Width =  size[0];
	m_vp.Height = size[1];
    m_vp.MinDepth = 0.0f;
	m_vp.MaxDepth = 1.0f;

	m_updateFlags.all = 0;
}


CStimulusPart::~CStimulusPart(void)
{
	if (m_symbol)
	{
		delete m_symbol;
		m_symbol = NULL;
	}
//	CStimulus::~CStimulus();
}


// CStimulusPart-Memberfunktionen

bool CStimulusPart::Init(LPCWSTR wzFilename)
{
	TRACE("Particle Init: %S\n", wzFilename);

	CNumericBinaryFile partFile(wzFilename);
	if (!partFile.valid()) return false;
	partFile.Init();
	UINT nParts = (UINT) partFile.m_dims[1];
	XMFLOAT3* vertices = new XMFLOAT3[nParts];
	switch (partFile.m_dims[0])
	{
	case 2:
		float coords[2];
		for (unsigned short i = 0; i < nParts; i++)
		{
			if (!partFile.Read2Dfloat(coords)) return false;
			vertices[i].x = coords[0];
			vertices[i].y = coords[1];
			vertices[i].z = 0.0f;
			TRACE("%f %f %f\n", vertices[i].x, vertices[i].y, vertices[i].z);
		}
		break;
	case 3:
		if (!partFile.ReadPoints2f((float*) vertices)) return false;
		break;
	default:
		CString errString;
		errString.Format(_T("Bad particle file. Dimensions: %ux%u\r\n"),
			partFile.m_dims[0], partFile.m_dims[1]);
		COutputList::AddString(errString);
		return false;
	}
	m_VertexBuffer = CDeviceVertexBuffer();
	HRESULT hr = m_VertexBuffer.Init(vertices, UINT (sizeof( XMFLOAT3 ) * nParts));
    if ( FAILED( hr ) ) return false;
	delete [] vertices;

	m_nParticles = (unsigned short) nParts;
	if (CStimServerApp::m_pVertexShaderPart == NULL) {
		hr = CStimServerApp::createVertexShaderPart();
		ASSERT(SUCCEEDED(hr));
		if( FAILED( hr ) )
			return false;
	}
	if (g_GSparticle == NULL) {
		// Compile the geometry shader
		const char geometryShader[] = 
			"cbuffer GS_PARAMS : register (b0)\n"
			"{\n"
			"    float2 pixelSize : packoffset(c0.x);\n"
			"    float patchRadius : packoffset(c0.z);\n"
			"    float gaussRadius : packoffset(c0.w);\n"
			"};\n"
			"static float2 newPos[4] =\n"
			"{\n"
			"	float2(-1,-1),\n"
			"	float2(-1, 1),\n"
			"	float2( 1,-1),\n"
			"	float2( 1, 1) \n"
			"};\n"
			"static float2 newTex[4] =\n"
			"{\n"
			"	float2(0,1),\n"
			"	float2(0,0),\n"
			"	float2(1,1),\n"
			"	float2(1,0) \n"
			"};\n"
			"struct INPUT {\n"
			"    float4 Position   : SV_POSITION;\n"
			"    float4 Color      : COLOR;\n"
			"	 float	Size	   : PSIZE;\n"
			"};\n"
			"struct OUTPUT {\n"
			"    float4 Position   : SV_POSITION;\n"
			"    float4 Color      : COLOR;\n"
			"	 float2 TexCoord   : TEXCOORD;\n"
			"};\n"
			"[maxvertexcount(4)]\n"
			"void GS( point INPUT particle[1], inout TriangleStream<OUTPUT> triangleStream )\n"
			"{\n"
			"    float Length = length(particle[0].Position.xy);\n"
			"	 if ((patchRadius != 0.0f) && (Length > patchRadius)) return;\n"
			"    float alpha = 1.0;\n"
			"    if (gaussRadius != 0.0f)\n"
			"        alpha = exp(-pow(Length, 2) / (2.0f * pow(gaussRadius, 2)));\n"
			"	 OUTPUT output;\n"
			"	 for (int i=0; i<4; i++)\n"
			"	 {\n"
			"		output.Position = particle[0].Position;\n"
//			"		output.Position.xy += newPos[i] * 0.02f;\n"
			"		output.Position.xy += newPos[i] * (float2) particle[0].Size * pixelSize;\n"
//			"		output.Color = particle[0].Color;\n"
			"		output.Color = particle[0].Color * float4(1.0f, 1.0f, 1.0f, alpha);\n"
			"		output.TexCoord = newTex[i];\n"
			"		triangleStream.Append(output);\n"
			"	 }\n"
//			"	 triangleStream.RestartStrip();\n"
			"}\n"
			;
		ID3DBlob* pGSBlob = NULL;
		hr = CStimServerApp::CompileShader(geometryShader, sizeof(geometryShader), "GS", "gs_4_0", &pGSBlob);
		if( FAILED( hr ) )
		{	
			if (pGSBlob) pGSBlob->Release();
			g_GSparticle = NULL;
			return false;
		}

		// Create the geometry shader
		hr = theApp.m_pD3Ddevice->CreateGeometryShader( pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize(), NULL, &g_GSparticle );
		if( FAILED( hr ) )
		{	
			pGSBlob->Release();
			g_GSparticle = NULL;
			return false;
		}
	}
	if (pPixelShaderTex == NULL) {
		if (!CreatePixelShaderTex()) return false;
	}

//	if (!m_symbol)	m_symbol = (CDisc*) new CDisc(20);

	m_VSanimationData.shift = 0.0f;
	hr = m_AnimationBuffer.Init(&m_VSanimationData, sizeof(m_VSanimationData));
	ASSERT(SUCCEEDED(hr));
	if( FAILED( hr ) )
		return false;

	TRACE("Animation Buffer initialized\n");

	m_VSparams.vertexColor = theApp.m_defaultDrawColor;
	m_VSparams.size = 4.0f;
	m_VSparams.angle = 0.0f;
	hr = m_ParamsBuffer.Init(&m_VSparams, sizeof(m_VSparams));
	ASSERT(SUCCEEDED(hr));
	if( FAILED( hr ) )
		return false;

	m_GSparams.pixelSize[0] = 1.0f / m_vp.Width;
	m_GSparams.pixelSize[1] = 1.0f / m_vp.Height;
	m_GSparams.patchRadius = 0.0f;
	m_GSparams.gaussRadius = 0.0f;
	hr = m_GSparamsBuffer.Init(&m_GSparams, sizeof(m_GSparams));
	ASSERT(SUCCEEDED(hr));
	if( FAILED( hr ) )
		return false;

//	TRACE("Params Buffer initialized\n");

	if (!m_symbol)	m_symbol = (CDisc*) new CDisc((unsigned short) m_VSparams.size);

	TRACE("Disc created\n");

	C3DStimulus::Moveto(false, 0.0f, 0.0f);	// move viewport
	TRACE("TopLeft: %f, %f\n", m_vp.TopLeftX, m_vp.TopLeftY);
	TRACE("End of Particle Init\n");
	return true;
}


void CStimulusPart::Draw()
{
	// TRACE("CStimulusPart::Draw\n");
	if (theApp.m_drawMode) theApp.EndDraw();
	//+
	// do diverse updates if not in deferred mode
	if (!theApp.m_deferredMode)
	{
		if (m_updateFlags.vertexColor || m_updateFlags.particleSize || m_updateFlags.angle)
		{
			HRESULT hr = m_ParamsBuffer.Update((float*) &m_VSparams, 6);	// need to copy entire buffer
			ASSERT(SUCCEEDED(hr));
			//		TRACE("Particle color: %f %f %f %f\n", m_VSparams.vertexColor.r, m_VSparams.vertexColor.g,
			//			m_VSparams.vertexColor.b, m_VSparams.vertexColor.a);
			m_updateFlags.vertexColor = 0;
			m_updateFlags.angle = 0;
			if (m_updateFlags.particleSize)
			{
				UpdateParticleSize();
				m_updateFlags.particleSize = 0;
			}
		}
		if (m_updateFlags.patchRadius || m_updateFlags.gaussRadius)
		{
			HRESULT hr = m_GSparamsBuffer.Update((float*) &m_GSparams, 4);
			ASSERT(SUCCEEDED(hr));
			m_updateFlags.patchRadius = 0;
			m_updateFlags.gaussRadius = 0;
		}
		if (m_updateFlags.shift && (m_velocity == 0.0f))
		{
			HRESULT hr = m_AnimationBuffer.Update((float*) &m_shift, 1);
			ASSERT(SUCCEEDED(hr));
			m_updateFlags.shift = 0;
		}
	}
	//-
    // Set vertex buffer
    UINT stride = sizeof( XMFLOAT3 );
    UINT offset = 0;
	theApp.m_pImmediateContext->IASetVertexBuffers( 0, 1, &m_VertexBuffer.m_pDeviceBuffer, &stride, &offset );

    // Set primitive topology
    theApp.m_pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_POINTLIST );

	theApp.m_pImmediateContext->RSSetViewports( 1, &m_vp );
	theApp.m_pImmediateContext->VSSetShader(theApp.m_pVertexShaderPart, NULL, 0 );
	theApp.m_pImmediateContext->GSSetShader(g_GSparticle, NULL, 0);
	theApp.m_pImmediateContext->PSSetShader(pPixelShaderTex, NULL, 0);
	ID3D11Buffer* pBuffers[2] = {
		m_AnimationBuffer.m_pDeviceBuffer,
		m_ParamsBuffer.m_pDeviceBuffer};
	theApp.m_pImmediateContext->VSSetConstantBuffers(0, 2, pBuffers);
	theApp.m_pImmediateContext->GSSetConstantBuffers(0, 1, &m_GSparamsBuffer.m_pDeviceBuffer);
	theApp.m_pImmediateContext->PSSetShaderResources(0, 1, &m_symbol->m_pShaderResView);
	theApp.m_pImmediateContext->PSSetSamplers(0, 1, &m_symbol->m_pSampler);
	if (m_velocity != 0.0f) {
		m_shift += m_velocity;
		HRESULT hr = m_AnimationBuffer.Update((float*) &m_shift, 1);
		ASSERT(SUCCEEDED(hr));
	}
	theApp.m_pImmediateContext->Draw( m_nParticles, 0 );	// (Vertex Count, Start)
}


void CStimulusPart::Moveto(bool deferred, float x, float y)
{
	C3DStimulus::Moveto(deferred, x, y);
}


void CStimulusPart::GetPos(float pos[2])
{
	C3DStimulus::GetPos(pos);
}


void CStimulusPart::Command(unsigned char message[], DWORD messageLength)
{
//	TRACE("CStimulusPart::Command\n");
	switch (message[0])
	{
	case 1:	// change a param
		switch (message[1])
		{
		case 1:	// particle size
			if (!theApp.CheckCommandLength(messageLength, 4, _T("Set Particle Size")))
			{
				m_errorCode = 2;
				return;
			}
			unsigned short size;
			size = *((unsigned short*) &message[2]);
			if (size == 0)
			{
				m_errorCode = 4;
				theApp.m_errorMask |= 2;
				CString errString;
				errString.Format(_T("Invalid particle size: 0."));
				COutputList::AddString(errString);
				return;
			}
			m_VSparams.size = (float) size;
			m_updateFlags.particleSize = 1;
			break;
		case 2: // patch radius
			if (!theApp.CheckCommandLength(messageLength, 6, _T("Set Particle Patch Radius")))
			{
				m_errorCode = 2;
				return;
			}
			m_GSparams.patchRadius = *((float*) &message[2]);
			m_updateFlags.patchRadius = 1;
			break;
		case 3: // gauss radius
			if (!theApp.CheckCommandLength(messageLength, 6, _T("Set Particle Gauss Patch")))
			{
				m_errorCode = 2;
				return;
			}
			m_GSparams.gaussRadius = *((float*) &message[2]);
			m_updateFlags.gaussRadius = 1;
			break;
		case 4: // flags
			if (!theApp.CheckCommandLength(messageLength, 3, _T("Set Particle Flags")))
			{
				m_errorCode = 2;
				return;
			}
			m_flags = message[2];
			m_updateFlags.flags = 1;
			break;
		default:
			m_errorCode = 5;
			theApp.m_errorMask |= 2;
			CString errString;
			errString.Format(_T("Particle stimulus: invalid parameter code: %u."), message[1]);
			COutputList::AddString(errString);
			return;
		}
		break;
	case 2:
		(theApp.m_deferredMode ? m_velocityCopy : m_velocity) = *(float*) & message[1];
		break;
	case 4:
		m_VSparams.angle = *((float*) &message[1]);
		m_updateFlags.angle = 1;
		break;
	case 5:
		m_VSparams.vertexColor = D2D1::ColorF(message[1]/255.0f, message[2]/255.0f, message[3]/255.0f, message[4]/255.0f);
		m_updateFlags.vertexColor = 1;
		break;
	case 6:		// set animation value (shift)
		if (!theApp.CheckCommandLength(messageLength, 5, _T("Set Particle Animation Value")))
		{
			m_errorCode = 2;
			return;
		}
		(theApp.m_deferredMode ? m_shiftCopy : m_shift) = *(float*) & message[1];
		m_updateFlags.shift = 1;
		break;
	case 7:
		WritePipe(&m_errorCode, sizeof(m_errorCode));
		m_errorCode = 0;
		break;
	}
}

void CStimulusPart::makeCopy(void)
{
	CStimulus::makeCopy();
	m_velocityCopy = m_velocity;
	m_vpCopy = m_vp;
	m_shiftCopy = m_shift;
}

void CStimulusPart::getCopy(void)
{
	CStimulus::getCopy();
//	TRACE("CStimulusPart::getCopy\n");
	m_velocity = m_velocityCopy;
	m_vp = m_vpCopy;
	m_shift = m_shiftCopy;
	if (m_updateFlags.vertexColor || m_updateFlags.angle ||
		m_updateFlags.particleSize) m_ParamsBuffer.Update((float*) &m_VSparams, 6);
	if (m_updateFlags.patchRadius || m_updateFlags.gaussRadius)
		m_GSparamsBuffer.Update((float*) &m_GSparams, 4);
	if (m_updateFlags.particleSize) UpdateParticleSize();
	if (m_updateFlags.shift) m_AnimationBuffer.Update((float*) &m_shift, 1);
//	TRACE("Constant Flag: %u, ColorFlag: %u\n", m_updateFlags.constantBuffer, m_updateFlags.colorBuffer);
	m_updateFlags.all = 0;
}


void CStimulusPart::UpdateParticleSize(void)
{
	m_symbol->m_size = D2D1::SizeU((UINT32) m_VSparams.size, (UINT32) m_VSparams.size);
	m_symbol->CreateTextureTarget();
	m_symbol->DrawTexture();
}

/*
bool CStimulusPart::SetAnimParam(BYTE mode, float value)
{
		COutputList::AddString(L"Currently linear range animations are not implemented for particle stimuli.");
		return false;
}
*/

CStimulusSymbol::CStimulusSymbol(BYTE type, unsigned short size)
{
	m_typeName = _T("symbol");
	m_updateFlags.all = 0;
	m_vp.Width =  (float) size;
	m_vp.Height = (float) size;
    m_vp.MinDepth = 0.0f;
	m_vp.MaxDepth = 1.0f;
	if (m_pVertexShaderCol == NULL) {
		HRESULT hr = CStimServerApp::createVertexShaderCol();
		ASSERT(SUCCEEDED(hr));
		if( FAILED( hr ) )
			ASSERT(false);
	}
	m_VSparams.vertexColor = theApp.m_defaultDrawColor;
	HRESULT hr = m_ParamsBuffer.Init(&m_VSparams, sizeof(m_VSparams));
	ASSERT(SUCCEEDED(hr));
	if( FAILED( hr ) )
		ASSERT(false);
	switch (type)
	{
	case 1:
		m_symbol = new CDisc(size);
		break;
	case 2:
		m_symbol = new CCircle(size);
		break;
	default:
		;
	}
	if (pPixelShaderTex == NULL) {
		if (!CreatePixelShaderTex()) ASSERT(false);
	}
	C3DStimulus::Moveto(false, 0.0f, 0.0f);		// move viewport
}


CStimulusSymbol::~CStimulusSymbol(void)
{
//	CStimulus::~CStimulus();
}


void CStimulusSymbol::makeCopy(void)
{
	CStimulus::makeCopy();
	m_vpCopy = m_vp;
}

void CStimulusSymbol::getCopy(void)
{
	CStimulus::getCopy();
//	TRACE("CStimulusPart::getCopy\n");
	m_vp = m_vpCopy;
	if (m_updateFlags.vertexColor) m_ParamsBuffer.Update((float*) &m_VSparams, 4);
	if (m_updateFlags.particleSize) UpdateParticleSize();
//	TRACE("Constant Flag: %u, ColorFlag: %u\n", m_updateFlags.constantBuffer, m_updateFlags.colorBuffer);
	m_updateFlags.all = 0;
}


void CStimulusSymbol::Moveto(bool deferred, float x, float y)
{
	C3DStimulus::Moveto(deferred, x, y);
//	TRACE("Move Symbol to: %f, %f\n", x, y);
}


void CStimulusSymbol::GetPos(float pos[2])
{
	C3DStimulus::GetPos(pos);
}


void CStimulusSymbol::Draw(void)
{
	if (theApp.m_drawMode) theApp.EndDraw();

	if ((!theApp.m_deferredMode) && m_updateFlags.particleSize)
	{
		UpdateParticleSize();
		m_updateFlags.particleSize = 0;
	}
	if ((!theApp.m_deferredMode) && m_updateFlags.vertexColor)
	{
		HRESULT hr = m_ParamsBuffer.Update((float*) &m_VSparams, 4);
		m_updateFlags.vertexColor = 0;
	}
//	TRACE("CStimulusSymbol::Draw(void)\n");
    // Set vertex buffer
    UINT stride = sizeof( XMFLOAT3 );
    UINT offset = 0;
	theApp.m_pImmediateContext->IASetVertexBuffers( 0, 1, &VertexBufferUnitQuad.m_pDeviceBuffer, &stride, &offset );
    // Set primitive topology
    theApp.m_pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
	theApp.m_pImmediateContext->VSSetShader( m_pVertexShaderCol, NULL, 0 );
	
	theApp.m_pImmediateContext->RSSetViewports( 1, &m_vp );

	theApp.m_pImmediateContext->GSSetShader( NULL, NULL, 0 );
	theApp.m_pImmediateContext->PSSetShader( pPixelShaderTex, NULL, 0 );
	ID3D11Buffer* pBuffers[1] = {
		m_ParamsBuffer.m_pDeviceBuffer};
	theApp.m_pImmediateContext->VSSetConstantBuffers(1, 1, pBuffers);	// 2nd buffer in shader !!
	theApp.m_pImmediateContext->PSSetShaderResources(0, 1, &m_symbol->m_pShaderResView);
	theApp.m_pImmediateContext->PSSetSamplers(0, 1, &m_symbol->m_pSampler);


	theApp.m_pImmediateContext->Draw( 4, 0 );	// (Vertex Count, Start)
}


void CStimulusSymbol::Command(unsigned char message[], DWORD messageLength)
{
//	TRACE("CStimulusSymbol::Command\n");
	switch (message[0]) {
	case 1:	// change a param
		if (!theApp.CheckCommandLength(messageLength, 4, _T("Set Symbol Size")))
		{
			m_errorCode = 2;
			return;
		}
		switch (message[1])
		{
		case 1:	// Symbol size
			unsigned short size;
			size = *((unsigned short*) &message[2]);
			if (size == 0)
			{
				m_errorCode = 4;
				theApp.m_errorMask |= 2;
				CString errString;
				errString.Format(_T("Invalid symbol size: 0."));
				COutputList::AddString(errString);
				return;
			}
			m_symbol->m_size = D2D1::SizeU(size, size);
			m_updateFlags.particleSize = 1;
			break;
		default:
			;
		}
		break;
	case 5:
		if (!theApp.CheckCommandLength(messageLength, 5, _T("Set Symbol Color")))
		{
			m_errorCode = 2;
			return;
		}
		m_VSparams.vertexColor = D2D1::ColorF(message[1]/255.0f, message[2]/255.0f, message[3]/255.0f, message[4]/255.0f);
		m_updateFlags.vertexColor=1;
		break;
	case 7:
		WritePipe(&m_errorCode, sizeof(m_errorCode));
		m_errorCode = 0;
		break;
	}
}

void CStimulusSymbol::UpdateParticleSize(void)
{
	m_symbol->CreateTextureTarget();
	m_symbol->DrawTexture();
	m_vp.TopLeftX += (m_vp.Width-m_symbol->m_size.width)/2.0f;
	m_vp.TopLeftY += (m_vp.Height-m_symbol->m_size.height)/2.0f;
	m_vp.Width = (float) m_symbol->m_size.width;
	m_vp.Height = (float) m_symbol->m_size.height;
}

/*
bool CStimulusSymbol::SetAnimParam(BYTE mode, float value)
{
		COutputList::AddString(L"Currently linear range animations are not implemented for symbol stimuli.");
		return false;
}
*/
CStimBmpBrush::CStimBmpBrush(void)
	: m_pBitmapBrush(NULL)
	, m_pOpacityMask(NULL)
	, m_maskEnabled(false)
	, m_maskEnabledCopy(false)
{
	m_typeName = _T("bitmap brush");
	m_shift.x = 0.0f;
	m_shift.y = 0.0f;
}

CStimBmpBrush::~CStimBmpBrush(void)
{
	EnterCriticalSection(&g_criticalDrawSection);
	if (m_pOpacityMask != NULL) m_pOpacityMask->Release();
	if (m_pBitmapBrush != NULL) m_pBitmapBrush->Release();
	LeaveCriticalSection(&g_criticalDrawSection);
}


bool CStimBmpBrush::Init(LPCWSTR wzFilename)
{
	bool result = CStimulusPic::Init(wzFilename);
	if (result)
	{
		HRESULT hr = theApp.m_pContext->CreateBitmapBrush(m_pBitmap, &m_pBitmapBrush);
		result = SUCCEEDED(hr);
		m_size = m_pBitmap->GetPixelSize();
		m_pBitmap->Release();
		m_pBitmap = NULL;
		TRACE("End of Bitmap Brush Init.\n");
	}
	return result;
}


void CStimBmpBrush::Draw()
{
//	TRACE("CStimulusPic::Draw\n");
	if (!theApp.m_drawMode) theApp.BeginDraw();
	m_pBitmapBrush->SetTransform(
		D2D1::Matrix3x2F::Translation(D2D1::SizeF(m_destRect.left,m_destRect.top)));
	if (m_maskEnabled)
	{
		D2D1_SIZE_U size = m_pOpacityMask->GetPixelSize();
		theApp.m_pContext->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
		if ((size.width < m_size.width) && (size.height < m_size.height))
		{
			D2D1_RECT_F sourceRect;
			sourceRect.left = (m_size.width - size.width) / 2.0f + m_shift.x;
			sourceRect.top = (m_size.height - size.height) / 2.0f - m_shift.y;
			D2D1_RECT_F destRect;
			destRect.left = m_destRect.left + sourceRect.left;
			destRect.right = destRect.left + size.width;
			destRect.top = m_destRect.top + sourceRect.top;
			destRect.bottom = destRect.top + size.height;
			theApp.m_pContext->FillOpacityMask(m_pOpacityMask, m_pBitmapBrush, destRect);
		}
		else if ((size.width < m_size.width) || (size.height < m_size.height))
		{
			theApp.m_pContext->FillOpacityMask(m_pOpacityMask, m_pBitmapBrush, m_destRect);
		}
		else
		{
			D2D1_RECT_F sourceRect;
			sourceRect.left = (size.width - m_size.width) / 2.0f - m_shift.x;
			sourceRect.right = sourceRect.left + m_size.width;
			sourceRect.top = (size.height - m_size.height) / 2.0f + m_shift.y;
			sourceRect.bottom = sourceRect.top + m_size.height;
			theApp.m_pContext->FillOpacityMask(m_pOpacityMask, m_pBitmapBrush, m_destRect, sourceRect);
		}
		theApp.m_pContext->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
	}
	else
	{
		theApp.m_pContext->FillRectangle(m_destRect, m_pBitmapBrush);
	}
}


void CStimBmpBrush::Moveto(bool deferred, float x, float y)
{
	D2D1_RECT_F* pRect = deferred ? &m_destRectCopy : &m_destRect;
	TRACE("Move Bitmap Brush to: %f, %f\n", x, y);
	pRect->left  = ((float) m_size.width) / -2.0f + x;
	pRect->right = ((float) m_size.width) /  2.0f + x;
	pRect->top   = ((float) m_size.height) / -2.0f - y;
	pRect->bottom  = ((float) m_size.height) / 2.0f - y;
}


void CStimBmpBrush::GetPos(float pos[2])
{
	pos[0] =  (m_destRect.left+m_destRect.right) / 2.0f;
	pos[1] = -(m_destRect.bottom+m_destRect.top) / 2.0f;
}


void CStimBmpBrush::Command(unsigned char message[], DWORD messageLength)
{
	switch (message[0])
	{
	case 7:
		WritePipe(&m_errorCode, sizeof(m_errorCode));
		m_errorCode = 0;
		break;
	case 10:
		if (messageLength == 1)	// remove opacity mask
		{
			EnterCriticalSection(&g_criticalDrawSection);
			if (m_pOpacityMask != NULL) m_pOpacityMask->Release();
			m_pOpacityMask = NULL;
			m_maskEnabled = false;
			m_maskEnabledCopy = false;
			LeaveCriticalSection(&g_criticalDrawSection);
		}
		else	// enable / disable opacity mask
		{
			(theApp.m_deferredMode ? m_maskEnabledCopy : m_maskEnabled) = message[1] != 0;
		}
		break;
	case 11:
		EnterCriticalSection(&g_criticalDrawSection);
		if (m_pOpacityMask != NULL) m_pOpacityMask->Release();
		m_pOpacityMask = NULL;
		if (!LoadD2DBitmap(CString(&message[1]), &m_pOpacityMask))
		{
			m_errorCode = 1;
			theApp.m_errorMask |= 2;
		}
		LeaveCriticalSection(&g_criticalDrawSection);
		break;
	case 13:
		(theApp.m_deferredMode ? m_shiftCopy : m_shift) = *(D2D1_VECTOR_2F*) & message[1];
		break;
	default:
		;
	}
}


void CStimBmpBrush::makeCopy(void)
{
	CStimulusPic::makeCopy();
	m_maskEnabledCopy = m_maskEnabled;
	m_shiftCopy = m_shift;
	m_destRectCopy = m_destRect;
}


void CStimBmpBrush::getCopy(void)
{
	CStimulusPic::getCopy();
	m_maskEnabled = m_maskEnabledCopy;
	m_shift = m_shiftCopy;
	m_destRect = m_destRectCopy;
}



CStimPSpic::CStimPSpic(void)
	: m_pPicture(NULL)
{
	m_typeName = _T("pixel shaded picture");
//	m_updateFlags.all = 0;
	if (m_pVertexShaderCol == NULL) {
		HRESULT hr = CStimServerApp::createVertexShaderCol();
		ASSERT(SUCCEEDED(hr));
		if( FAILED( hr ) )
			ASSERT(false);
	}
}

/*
CStimPSpic::~CStimPSpic(void)
{
}
*/

bool CStimPSpic::Init(LPCWSTR wzFilename)
{
	ID3DBlob* pPSBlob = NULL;
	pPSBlob = CreateShader(wzFilename);
	if (pPSBlob == NULL) return false;
	pPSBlob->Release();

    // Setup the viewport
	m_vp.Width =  (float) m_pPicture->m_size.width;
	m_vp.Height = (float) m_pPicture->m_size.height;
    m_vp.MinDepth = 0.0f;
	m_vp.MaxDepth = 1.0f;
	// TopLeft is set in Moveto
	C3DStimulus::Moveto(false, 0.0f, 0.0f);	// move the viewport

	m_PSconstData.center[0] = g_ScreenSize.width/2.0f;
	m_PSconstData.center[1] = g_ScreenSize.height/2.0f;
	m_PSconstData.width = m_vp.Width;
	m_PSconstData.height = m_vp.Height;
	for (unsigned int i = 0; i < ARRAYSIZE(m_PSconstData.params); i++) m_PSconstData.params[i] = 0.0f;

	HRESULT hr;
	hr = m_ConstantBuffer.Init(&m_PSconstData, sizeof(m_PSconstData));
	ASSERT(hr == S_OK);
	
//	TRACE("Constant Buffer Initialized\n");

	m_PSanimationData.phase = 0.0f;
	hr = m_AnimationBuffer.Init(&m_PSanimationData, sizeof(m_PSanimationData));
	ASSERT(hr == S_OK);

	TRACE("Animation Buffer Initialized\n");

	LeaveCriticalSection(&g_criticalDrawSection);
	TRACE("End of Init\n");
	return true;
}


void CStimPSpic::assignPic(CStimulus* pPic)
{
	m_pPicture = new CPicture(((CStimulusPic*)pPic)->m_pBitmap);
}

void CStimPSpic::getCopy(void)
{
	CPixelShader::getCopy();
	m_updateFlags.all = 0;
}

void CStimPSpic::Draw()
{
	CPixelShader::Draw();

	theApp.m_pImmediateContext->VSSetShader( m_pVertexShaderCol, NULL, 0 );
	
	ID3D11Buffer* pBuffers[2] = {
		m_ConstantBuffer.m_pDeviceBuffer,
		m_AnimationBuffer.m_pDeviceBuffer};
	theApp.m_pImmediateContext->PSSetConstantBuffers(0, 2, pBuffers);
	theApp.m_pImmediateContext->PSSetShaderResources(0, 1, &m_pPicture->m_pShaderResView);
	theApp.m_pImmediateContext->PSSetSamplers(0, 1, &m_pPicture->m_pSampler);
	theApp.m_pImmediateContext->Draw( 4, 0 );	// (Vertex Count, Start)
}


void CStimPSpic::Command(unsigned char message[], DWORD messageLength)
{
	switch (message[0])
	{
	float *param;
	case 1:		// set value of parameter
		param = (float*) &message[2];
		SetParam(message[1]-1, *param);
		break;
	default:
		CPixelShader::Command(message, messageLength);
	}
}


CStimulusRect::CStimulusRect(void)
{
	CD2DStimulus::CD2DStimulus();
	m_typeName = _T("rectangle");
	HRESULT hr;
	m_deferableParams.color = theApp.m_defaultDrawColor;
	hr = theApp.m_pContext->CreateSolidColorBrush(
		theApp.m_defaultDrawColor,
		&m_pBrush
		);
	ASSERT(hr == S_OK);
//	m_deferableParams.transform = theApp.m_contextTransform;
//	m_transform = theApp.m_contextTransform;
	m_deferableParams.rect = D2D1::RectF(-5.0f, -10.0f, 5.0f, 10.0f);
	m_updateFlags.all = 0;
}

/*
CStimulusRect::~CStimulusRect(void)
{
//	CStimulus::~CStimulus();
}
*/

void CStimulusRect::Draw(void)
{
	if (!theApp.m_drawMode) theApp.BeginDraw();
//	theApp.m_pContext->SetTransform(m_deferableParams.transform);
	theApp.m_pContext->SetTransform(m_transform);
	theApp.m_pContext->FillRectangle(&m_deferableParams.rect, m_pBrush);
	theApp.m_pContext->SetTransform(theApp.m_contextTransform);
}


void CStimulusRect::Command(unsigned char message[], DWORD messageLength)
{
//	TRACE("CStimulusRect::Command\n");
	switch (message[0]) {
	case 1:		// set size
		if (!theApp.CheckCommandLength(messageLength, 6, _T("Set Rectangle Size")))
		{
			m_errorCode = 2;
			return;
		}
		if (message[1] != 1)
		{
			m_errorCode = 3;
			theApp.m_errorMask |= 2;
			CString errString;
			errString.Format(_T("Invalid Rectangle command (set size?). Trace: %u %u %u %u %u %u."),
				message[0], message[1], message[2], message[3], message[4], message[5]);
			COutputList::AddString(errString);
			return;
		}
		WORD* pSize;
		pSize = (WORD*) &message[2];
		float w2;
		float h2;
		w2 = (*pSize++ - 1) / 2.0f;
		h2 = (*pSize   - 1) / 2.0f;
		(theApp.m_deferredMode ? m_deferableParamsCopy : m_deferableParams).rect = 
			D2D1::RectF(-w2, -h2, w2, h2);
		break;
	case 4:
		if (!theApp.CheckCommandLength(messageLength, 5, _T("Set Rectangle Orientation")))
		{
			m_errorCode = 2;
			return;
		}
		float *pPhi;
		pPhi = (float*) &message[1];
//		TRACE("Rectangle orientation: %f\n", *pPhi);
		float phi;
		phi = *pPhi*(float)M_PI/180.f;
		D2D1_MATRIX_3X2_F* pTransform;
//		pTransform = 
//			&(theApp.m_deferredMode ? m_deferableParamsCopy : m_deferableParams).transform;
		pTransform = &(theApp.m_deferredMode ? m_transformCopy : m_transform);
		pTransform->_21 = sin(phi);
		pTransform->_12 = -pTransform->_21;
		pTransform->_11 = cos(phi);
		pTransform->_22 =  pTransform->_11;
		break;
	case 5:
		if (!theApp.CheckCommandLength(messageLength, 5, _T("Set Rectangle Color")))
		{
			m_errorCode = 2;
			return;
		}
		ASSERT(messageLength == 5);
		if (theApp.m_deferredMode)
		{
			m_deferableParamsCopy.color = D2D1::ColorF(
				message[1]/255.f, message[2]/255.f, message[3]/255.f, message[4]/255.f);
			m_updateFlags.color = true;
		}
		else
		{
			m_deferableParams.color = D2D1::ColorF(
				message[1]/255.f, message[2]/255.f, message[3]/255.f, message[4]/255.f);
			m_pBrush->SetColor(m_deferableParams.color);
		}
		break;
	case 7:
		WritePipe(&m_errorCode, sizeof(m_errorCode));
		m_errorCode = 0;
		break;
	}
}

/*
void CStimulusRect::Moveto(bool deferred, float x, float y)
{
	D2D1_MATRIX_3X2_F* pTransform;
	pTransform = 
		&(deferred ? m_deferableParamsCopy : m_deferableParams).transform;
//	TRACE("Translation: %f, %f\n", pTransform->_31, pTransform->_32);
	pTransform->_31 = theApp.m_contextTransform._31 + x;
	pTransform->_32 = theApp.m_contextTransform._32 - y;
}
*/
/*
void CStimulusRect::GetPos(float pos[2])
{
//	pos[0] =  m_deferableParams.transform._31 - theApp.m_contextTransform._31;
//	pos[1] = -m_deferableParams.transform._32 + theApp.m_contextTransform._32;
	pos[0] =  m_transform._31 - theApp.m_contextTransform._31;
	pos[1] = -m_transform._32 + theApp.m_contextTransform._32;
}
*/

void CStimulusRect::makeCopy(void)
{
	CD2DStimulus::makeCopy();
//	CStimulus::makeCopy();
	m_deferableParamsCopy = m_deferableParams;
}


void CStimulusRect::getCopy(void)
{
	CD2DStimulus::getCopy();
	m_deferableParams = m_deferableParamsCopy;
	if (m_updateFlags.color)
	{
		m_pBrush->SetColor(m_deferableParams.color);
	}
	m_updateFlags.all = 0;
}
/*
bool CStimulusRect::SetAnimParam(BYTE mode, float value)
{
		COutputList::AddString(L"Currently linear range animations are not implemented for rectangle stimuli.");
		return false;
}
*/

CStimulusParticle::CStimulusParticle(unsigned short size[2])
	: m_nParticles(0)
	, m_shift(0.0f)
	, m_velocity(0.0f)
	, m_symbol(NULL)
	, m_flags(0)
{
	m_typeName = _T("particle");
    // Setup the viewport
	m_vp.Width =  size[0];
	m_vp.Height = size[1];
    m_vp.MinDepth = 0.0f;
	m_vp.MaxDepth = 1.0f;

	m_updateFlags.all = 0;
}


CStimulusParticle::~CStimulusParticle(void)
{
	if (m_symbol)
	{
		delete m_symbol;
		m_symbol = NULL;
	}
//	CStimulus::~CStimulus();
}



// CStimulusParticle-Memberfunktionen

bool CStimulusParticle::Init(LPCWSTR wzFilename)
{
	TRACE("Particle Init: %S\n", wzFilename);

	HRESULT hr;
	ID3DBlob* pVSBlob = NULL;
	hr = CompileShaderFromFile(wzFilename,"VSmain", "vs_4_0",  &pVSBlob);
	if (FAILED(hr)) return false;

//!!!	EnterCriticalSection(&g_criticalDrawSection);
/*
	// Create the vertex shader
	hr = theApp.m_pD3Ddevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &m_pVertexShader );
	ASSERT(hr == S_OK);
	pVSBlob->Release();
*/
	// Create the vertex shader
	D3D11_SO_DECLARATION_ENTRY SOdecl[1];
	SOdecl[0].Stream = 0;
	SOdecl[0].SemanticName = "POSITION";
	SOdecl[0].SemanticIndex = 0;
	SOdecl[0].StartComponent = 0;
	SOdecl[0].ComponentCount = 4;
	SOdecl[0].OutputSlot = 0;
	UINT strides[1] = {16};
	hr = theApp.m_pD3Ddevice->CreateGeometryShaderWithStreamOutput(
		pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(),
		&SOdecl[0],
		1,
		&strides[0],
		1,
//		D3D11_SO_NO_RASTERIZED_STREAM,
		0,
		NULL,
		(ID3D11GeometryShader**) &m_pVertexShader );
	ASSERT(hr == S_OK);
	pVSBlob->Release();

	UINT nParts = 1000;
	XMFLOAT3* vertices = new XMFLOAT3[nParts];

	m_VertexBuffer = CDeviceVertexBuffer();
	m_VertexBuffer.m_BuffDesc.BindFlags |= D3D11_BIND_STREAM_OUTPUT;
	hr = m_VertexBuffer.Init(vertices, UINT (sizeof( XMFLOAT3 ) * nParts));
    if ( FAILED( hr ) ) return false;

	UINT offsets[1] = {0};
	ID3D11Buffer* pSOTarget[1] = {m_VertexBuffer.m_pDeviceBuffer};
	theApp.m_pImmediateContext->SOSetTargets(1, &pSOTarget[0], &offsets[0]);

	m_nParticles = (unsigned short) nParts;

	if (g_GSparticle == NULL) {
		// Compile the geometry shader
		const char geometryShader[] = 
			"cbuffer GS_PARAMS : register (b0)\n"
			"{\n"
			"    float2 pixelSize : packoffset(c0.x);\n"
			"    float patchRadius : packoffset(c0.z);\n"
			"    float gaussRadius : packoffset(c0.w);\n"
			"};\n"
			"static float2 newPos[4] =\n"
			"{\n"
			"	float2(-1,-1),\n"
			"	float2(-1, 1),\n"
			"	float2( 1,-1),\n"
			"	float2( 1, 1) \n"
			"};\n"
			"static float2 newTex[4] =\n"
			"{\n"
			"	float2(0,1),\n"
			"	float2(0,0),\n"
			"	float2(1,1),\n"
			"	float2(1,0) \n"
			"};\n"
			"struct INPUT {\n"
			"    float4 Position   : SV_POSITION;\n"
			"    float4 Color      : COLOR;\n"
			"	 float	Size	   : PSIZE;\n"
			"};\n"
			"struct OUTPUT {\n"
			"    float4 Position   : SV_POSITION;\n"
			"    float4 Color      : COLOR;\n"
			"	 float2 TexCoord   : TEXCOORD;\n"
			"};\n"
			"[maxvertexcount(4)]\n"
			"void GS( point INPUT particle[1], inout TriangleStream<OUTPUT> triangleStream )\n"
			"{\n"
			"    float Length = length(particle[0].Position.xy);\n"
			"	 if ((patchRadius != 0.0f) && (Length > patchRadius)) return;\n"
			"    float alpha = 1.0;\n"
			"    if (gaussRadius != 0.0f)\n"
			"        alpha = exp(-pow(Length, 2) / (2.0f * pow(gaussRadius, 2)));\n"
			"	 OUTPUT output;\n"
			"	 for (int i=0; i<4; i++)\n"
			"	 {\n"
			"		output.Position = particle[0].Position;\n"
//			"		output.Position.xy += newPos[i] * 0.02f;\n"
			"		output.Position.xy += newPos[i] * (float2) particle[0].Size * pixelSize;\n"
//			"		output.Color = particle[0].Color;\n"
			"		output.Color = particle[0].Color * float4(1.0f, 1.0f, 1.0f, alpha);\n"
			"		output.TexCoord = newTex[i];\n"
			"		triangleStream.Append(output);\n"
			"	 }\n"
//			"	 triangleStream.RestartStrip();\n"
			"}\n"
			;
		ID3DBlob* pGSBlob = NULL;
		hr = CStimServerApp::CompileShader(geometryShader, sizeof(geometryShader), "GS", "gs_4_0", &pGSBlob);
		if( FAILED( hr ) )
		{	
			if (pGSBlob) pGSBlob->Release();
			g_GSparticle = NULL;
			return false;
		}

		// Create the geometry shader
		hr = theApp.m_pD3Ddevice->CreateGeometryShader( pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize(), NULL, &g_GSparticle );
		if( FAILED( hr ) )
		{	
			pGSBlob->Release();
			g_GSparticle = NULL;
			return false;
		}
	}
	if (pPixelShaderTex == NULL) {
		if (!CreatePixelShaderTex()) return false;
	}

//	if (!m_symbol)	m_symbol = (CDisc*) new CDisc(20);

	m_VSanimationData.shift = 0.0f;
	hr = m_AnimationBuffer.Init(&m_VSanimationData, sizeof(m_VSanimationData));
	ASSERT(SUCCEEDED(hr));
	if( FAILED( hr ) )
		return false;

	TRACE("Animation Buffer initialized\n");

	m_VSparams.vertexColor = theApp.m_defaultDrawColor;
	m_VSparams.size = 4.0f;
	m_VSparams.angle = 0.0f;
	hr = m_ParamsBuffer.Init(&m_VSparams, sizeof(m_VSparams));
	ASSERT(SUCCEEDED(hr));
	if( FAILED( hr ) )
		return false;

	m_GSparams.pixelSize[0] = 1.0f / m_vp.Width;
	m_GSparams.pixelSize[1] = 1.0f / m_vp.Height;
	m_GSparams.patchRadius = 0.0f;
	m_GSparams.gaussRadius = 0.0f;
	hr = m_GSparamsBuffer.Init(&m_GSparams, sizeof(m_GSparams));
	ASSERT(SUCCEEDED(hr));
	if( FAILED( hr ) )
		return false;

//	TRACE("Params Buffer initialized\n");

	if (!m_symbol)	m_symbol = (CDisc*) new CDisc((unsigned short) m_VSparams.size);

	TRACE("Disc created\n");

	C3DStimulus::Moveto(false, 0.0f, 0.0f);	// move viewport
	TRACE("TopLeft: %f, %f\n", m_vp.TopLeftX, m_vp.TopLeftY);
	TRACE("End of Particle Init\n");
	return true;
}

void CStimulusParticle::Draw()
{
	// TRACE("CStimulusPart::Draw\n");
	if (theApp.m_drawMode) theApp.EndDraw();
	//+
	// do diverse updates if not in deferred mode
	if (!theApp.m_deferredMode)
	{
		if (m_updateFlags.vertexColor || m_updateFlags.particleSize || m_updateFlags.angle)
		{
			HRESULT hr = m_ParamsBuffer.Update((float*) &m_VSparams, 6);	// need to copy entire buffer
			ASSERT(SUCCEEDED(hr));
			//		TRACE("Particle color: %f %f %f %f\n", m_VSparams.vertexColor.r, m_VSparams.vertexColor.g,
			//			m_VSparams.vertexColor.b, m_VSparams.vertexColor.a);
			m_updateFlags.vertexColor = 0;
			m_updateFlags.angle = 0;
			if (m_updateFlags.particleSize)
			{
				UpdateParticleSize();
				m_updateFlags.particleSize = 0;
			}
		}
		if (m_updateFlags.patchRadius || m_updateFlags.gaussRadius)
		{
			HRESULT hr = m_GSparamsBuffer.Update((float*) &m_GSparams, 4);
			ASSERT(SUCCEEDED(hr));
			m_updateFlags.patchRadius = 0;
			m_updateFlags.gaussRadius = 0;
		}
		if (m_updateFlags.shift && (m_velocity == 0.0f))
		{
			HRESULT hr = m_AnimationBuffer.Update((float*) &m_shift, 1);
			ASSERT(SUCCEEDED(hr));
			m_updateFlags.shift = 0;
		}
	}
	//-
    // Set vertex buffer
    UINT stride = sizeof( XMFLOAT3 );
    UINT offset = 0;
	theApp.m_pImmediateContext->IASetVertexBuffers( 0, 1, &m_VertexBuffer.m_pDeviceBuffer, &stride, &offset );

    // Set primitive topology
    theApp.m_pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_POINTLIST );

//	theApp.m_pImmediateContext->RSSetViewports( 1, &m_vp );
	theApp.m_pImmediateContext->VSSetShader(m_pVertexShader, NULL, 0 );
//	theApp.m_pImmediateContext->GSSetShader(g_GSparticle, NULL, 0);
	theApp.m_pImmediateContext->PSSetShader(NULL, NULL, 0);
	ID3D11Buffer* pBuffers[2] = {
		m_AnimationBuffer.m_pDeviceBuffer,
		m_ParamsBuffer.m_pDeviceBuffer};
	theApp.m_pImmediateContext->VSSetConstantBuffers(0, 2, pBuffers);
//	theApp.m_pImmediateContext->GSSetConstantBuffers(0, 1, &m_GSparamsBuffer.m_pDeviceBuffer);
//	theApp.m_pImmediateContext->PSSetShaderResources(0, 1, &m_symbol->m_pShaderResView);
//	theApp.m_pImmediateContext->PSSetSamplers(0, 1, &m_symbol->m_pSampler);
	/*
	if (m_velocity != 0.0f)
	{
		m_shift += m_velocity;
		HRESULT hr = m_AnimationBuffer.Update((float*) &m_shift, 1);
		ASSERT(SUCCEEDED(hr));
	}
	*/
	theApp.m_pImmediateContext->Draw( m_nParticles, 0 );	// (Vertex Count, Start)
}

void CStimulusParticle::GetPos(float pos[2])
{
	C3DStimulus::GetPos(pos);
}

void CStimulusParticle::Command(unsigned char message[], DWORD messageLength)
{
//	TRACE("CStimulusPart::Command\n");
	switch (message[0])
	{
	case 1:	// change a param
		switch (message[1])
		{
		case 1:	// particle size
			if (!theApp.CheckCommandLength(messageLength, 4, _T("Set Particle Size")))
			{
				m_errorCode = 2;
				return;
			}
			unsigned short size;
			size = *((unsigned short*) &message[2]);
			if (size == 0)
			{
				m_errorCode = 4;
				theApp.m_errorMask |= 2;
				CString errString;
				errString.Format(_T("Invalid particle size: 0."));
				COutputList::AddString(errString);
				return;
			}
			m_VSparams.size = (float) size;
			m_updateFlags.particleSize = 1;
			break;
		case 2: // patch radius
			if (!theApp.CheckCommandLength(messageLength, 6, _T("Set Particle Patch Radius")))
			{
				m_errorCode = 2;
				return;
			}
			m_GSparams.patchRadius = *((float*) &message[2]);
			m_updateFlags.patchRadius = 1;
			break;
		case 3: // gauss radius
			if (!theApp.CheckCommandLength(messageLength, 6, _T("Set Particle Gauss Patch")))
			{
				m_errorCode = 2;
				return;
			}
			m_GSparams.gaussRadius = *((float*) &message[2]);
			m_updateFlags.gaussRadius = 1;
			break;
		case 4: // flags
			if (!theApp.CheckCommandLength(messageLength, 3, _T("Set Particle Flags")))
			{
				m_errorCode = 2;
				return;
			}
			m_flags = message[2];
			m_updateFlags.flags = 1;
			break;
		default:
			m_errorCode = 5;
			theApp.m_errorMask |= 2;
			CString errString;
			errString.Format(_T("Particle stimulus: invalid parameter code: %u."), message[1]);
			COutputList::AddString(errString);
			return;
		}
		break;
	case 2:
		(theApp.m_deferredMode ? m_velocityCopy : m_velocity) = *(float*) & message[1];
		break;
	case 4:
		m_VSparams.angle = *((float*) &message[1]);
		m_updateFlags.angle = 1;
		break;
	case 5:
		m_VSparams.vertexColor = D2D1::ColorF(message[1]/255.0f, message[2]/255.0f, message[3]/255.0f, message[4]/255.0f);
		m_updateFlags.vertexColor = 1;
		break;
	case 6:		// set animation value (shift)
		if (!theApp.CheckCommandLength(messageLength, 5, _T("Set Particle Animation Value")))
		{
			m_errorCode = 2;
			return;
		}
		(theApp.m_deferredMode ? m_shiftCopy : m_shift) = *(float*) & message[1];
		m_updateFlags.shift = 1;
		break;
	case 7:
		WritePipe(&m_errorCode, sizeof(m_errorCode));
		m_errorCode = 0;
		break;
	}
}
/*
bool CStimulusParticle::SetAnimParam(BYTE mode, float value)
{
		COutputList::AddString(L"Currently linear range animations are not implemented for particle stimuli.");
		return false;
}
*/

void CStimulusParticle::UpdateParticleSize(void)
{
	m_symbol->m_size = D2D1::SizeU((UINT32) m_VSparams.size, (UINT32) m_VSparams.size);
	m_symbol->CreateTextureTarget();
	m_symbol->DrawTexture();
}


// CStimulusPics

IMPLEMENT_DYNAMIC( CStimulusPics, CLoadedStimulus)

CStimulusPics::CStimulusPics()
	: m_pBitmaps(NULL)
	, m_currentFrame(0)
	, m_rateFrac(1)
	, m_fracCount(0)
	, m_hEndEvent(NULL)
{
	m_typeName = _T("motion picture");
}

bool CStimulusPics::Init(LPCWSTR wzFilename)
{
	TRACE("Pics Init: %S\n", wzFilename);
	CD2DBitmapFile D2Dbitmap;
	if (!D2Dbitmap.Init(wzFilename)) return false;

	HRESULT hr = D2Dbitmap.GetFrameCount(&m_nFrames);
	ASSERT(hr == S_OK);
	TRACE("Number of Frames: %u\n", m_nFrames);
	m_pBitmaps = new ID2D1Bitmap*[m_nFrames];
	for (UINT i = 0; i < m_nFrames; i++)
	{
		hr = D2Dbitmap.LoadBitmapFile(i, &m_pBitmaps[i]);
		// handle D2DERR_RECREATE_TARGET
		if (hr != S_OK)
		{
			CString errString;
			switch (hr)
			{
			case E_OUTOFMEMORY:
				errString.Format(_T("GPU memory exhausted. Motion Picture truncated to %u frames (of %u)."), i, m_nFrames);
				COutputList::AddString(errString);
				m_nFrames = i;
				continue;
				break;
			default:
				ASSERT(false);
			}
		}
	}
	D2Dbitmap.Cleanup();
	TRACE("Bitmaps created\n");


//	if (!LoadD2DBitmaps(wzFilename, &m_pBitmap)) return false;
	D2D1_SIZE_F sizeF = m_pBitmaps[0]->GetSize();
	TRACE(" SizeX: %f, SizeY: %f\n", sizeF.width, sizeF.height);
	m_destRect = D2D1::RectF(sizeF.width/-2.f, sizeF.height/-2.f, sizeF.width/2.f, sizeF.height/2.f);
	m_deferableParams.transform = D2D1::Matrix4x4F(
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 0.f, 0.f,
		0.f, 0.f, 0.f, 1.f);
	TRACE("End of Init\n");
	return true;
}

CStimulusPics::~CStimulusPics()
{
	if (m_pBitmaps)
	{
		for (UINT i = 0; i < m_nFrames; i++)
		{
			m_pBitmaps[i]->Release();
		}
		delete m_pBitmaps;
		TRACE("Bitmap Released\n");
		m_pBitmaps = NULL;
	}
	if (m_hEndEvent) CloseHandle(m_hEndEvent);
}


// CStimulusPics-Memberfunktionen

void CStimulusPics::Draw()
{
//	if (immediate) return;
//	TRACE("CStimulusPic::Draw\n");
	if (m_deferableParams.phiInc != 0) {
		m_iPhi = (m_iPhi + m_deferableParams.phiInc) % 7200;
		Rotate(false, m_iPhi * (0.05f*(float)M_PI/180.f));
	}
	if (!theApp.m_drawMode) theApp.BeginDraw();
	theApp.m_pContext->DrawBitmap(
		m_pBitmaps[m_currentFrame],
		m_destRect,
		m_deferableParams.alpha,
		D2D1_INTERPOLATION_MODE_LINEAR,
		NULL,
		&m_deferableParams.transform);
	if (m_rateFrac != 0)
	{
		if (++m_fracCount >= m_rateFrac)	// ">" in case we decreased the rate
		{
			m_fracCount = 0;
			m_currentFrame++;
			if (m_currentFrame == m_nFrames)
			{
				m_currentFrame = 0;
				m_enabled = false;
				if (m_hEndEvent) VERIFY(SetEvent(m_hEndEvent));
			}
		}
	}
}


void CStimulusPics::Command(unsigned char message[], DWORD messageLength)
{
	CString errString;
	switch (message[0])
	{
	case 6:
		{
		UINT32 currentFrame = *((UINT32*) &message[1]);
		if (currentFrame < m_nFrames)
		{
			m_currentFrame = currentFrame;
			TRACE("Frame: %u\n", m_currentFrame);
		}
		else
		{
			errString.Format(_T("Can't set frame # of Motion Picture to %u. Number of frames is %u."), currentFrame, m_nFrames);
			COutputList::AddString(errString);
			m_errorCode = 5;
			theApp.m_errorMask |= 2;
		}
		}
		break;
	case 9:
		m_rateFrac = *((WORD*) &message[1]);
		TRACE("Frame Rate Fraction: %u\n", m_rateFrac);
		break;
	case 10:
		VERIFY(m_hEndEvent = CreateEventA(NULL, FALSE, FALSE, (LPCSTR) &message[1]));
		break;
	default:
		CStimulusPic::Command(message, messageLength);
	}
}


CPetal::CPetal(void)
	/*
	: m_r(10.0f)
	, m_R(50.0f)
	, m_d(100.0f)
	, m_q(0.3f)
	, m_drawMode(1)
	*/
{
	m_typeName = _T("petal");
	m_petalParams.m_r =  25.0f;
	m_petalParams.m_R = 100.0f;
	m_petalParams.m_d = 250.0f;
	m_petalParams.m_q =   0.3819660113f;	// (golden ratio)
	m_deferableParams.strokeWidth = 2.0f;
	m_deferableParams.drawMode = 1;
	HRESULT hr;
	m_deferableParams.color = theApp.m_defaultDrawColor;
	hr = theApp.m_pContext->CreateSolidColorBrush(
		theApp.m_defaultDrawColor,
		&m_pBrush
		);
	ASSERT(hr == S_OK);
	m_deferableParams.outlineColor = theApp.m_defaultOutlineColor;
	hr = theApp.m_pContext->CreateSolidColorBrush(
		theApp.m_defaultOutlineColor,
		&m_pOutlineBrush
		);
	ASSERT(hr == S_OK);
//	m_deferableParams.transform = theApp.m_contextTransform;
	m_transform = theApp.m_contextTransform;
	m_updateFlags.all = 0;
	InitializeCriticalSection(&m_CriticalSection);
	EnterCriticalSection(&m_CriticalSection);
	Construct();
	LeaveCriticalSection(&m_CriticalSection);
}

CPetal::~CPetal(void)
{
	m_pPathGeometry->Release();
	m_pBrush->Release();
}

void CPetal::Construct(void)
{
	HRESULT hr;
	hr = theApp.m_d2dFactory->CreatePathGeometry(&m_pPathGeometry);
	ASSERT(hr == S_OK);
	ID2D1GeometrySink *pSink = NULL;
	hr = m_pPathGeometry->Open(&pSink);
	ASSERT(hr == S_OK);
	D2D1_POINT_2F startPoint, endPoint;
	D2D1_QUADRATIC_BEZIER_SEGMENT bs;
	bs.point1 = D2D1::Point2F(m_petalParams.m_d*m_petalParams.m_q, 0.0f);
//	TRACE("Point1x: %f\n", bs.point1.x);
	float phi;
	phi = acosf(m_petalParams.m_r/bs.point1.x);
//	TRACE("phi: %f\n", phi*180.0f/3.141592654f);
	startPoint.x = m_petalParams.m_r*cosf(phi);
	startPoint.y = m_petalParams.m_r*sinf(phi);
	endPoint.x =  startPoint.x;
	endPoint.y = -startPoint.y;
	TRACE("End point: %f, %f\n", endPoint.x, endPoint.y);
	pSink->BeginFigure(startPoint, D2D1_FIGURE_BEGIN_FILLED);
	D2D1_ARC_SEGMENT arcSegment =
	{
		endPoint,
		D2D1::SizeF(m_petalParams.m_r, m_petalParams.m_r),
		0.0f,
		D2D1_SWEEP_DIRECTION_CLOCKWISE,
		D2D1_ARC_SIZE_LARGE
	};
	pSink->AddArc(arcSegment);
	phi = acosf(m_petalParams.m_R/(m_petalParams.m_d-bs.point1.x));
	bs.point2 = D2D1::Point2F(
		 m_petalParams.m_d-(m_petalParams.m_R*cosf(phi)),
		-m_petalParams.m_R*sinf(phi)); 
	pSink->AddQuadraticBezier(bs);
	arcSegment.point = D2D1::Point2F(bs.point2.x, -bs.point2.y);
	arcSegment.size = D2D1::SizeF(m_petalParams.m_R, m_petalParams.m_R);
	pSink->AddArc(arcSegment);
	bs.point2 = startPoint;
	pSink->AddQuadraticBezier(bs);
	pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
	hr = pSink->Close();
	ASSERT(hr == S_OK);
}

void CPetal::Reconstruct(void)
{
	EnterCriticalSection(&m_CriticalSection);
	m_pPathGeometry->Release();
	Construct();
	LeaveCriticalSection(&m_CriticalSection);
}

void CPetal::Draw(void)
{
	if (!theApp.m_drawMode) theApp.BeginDraw();
	theApp.m_pContext->SetTransform(m_transform);
	EnterCriticalSection(&m_CriticalSection);
	if (m_deferableParams.drawMode & 1)
		theApp.m_pContext->FillGeometry(m_pPathGeometry, m_pBrush);
	if (m_deferableParams.drawMode & 2)
		theApp.m_pContext->DrawGeometry(m_pPathGeometry, m_pOutlineBrush,
		m_deferableParams.strokeWidth);
	LeaveCriticalSection(&m_CriticalSection);
	theApp.m_pContext->SetTransform(theApp.m_contextTransform);
}

void CPetal::Command(unsigned char message[], DWORD messageLength)
{
//	TRACE("CPetal::Command\n");
	CString errString;
	PETAL_PARAMS* pDP = theApp.m_deferredMode ? 
		&m_petalParamsCopy : &m_petalParams;
	switch (message[0])
	{
	case 1:		// set parameter
		switch (message[1])
		{
		case 1:	// r
			{
				if (!theApp.CheckCommandLength(messageLength, 6, _T("Set Petal Radius r")))
				{
					m_errorCode = 2;
					theApp.m_errorMask |= 2;
					return;
				}
				float radius = *(float*) &message[2];
				float maxRadius = m_petalParams.m_d*m_petalParams.m_q;
				if (radius < 0.0f)
				{
					errString.Format(_T("Negative radius (%f) is invalid for petal -- ignored."),
						radius);
					COutputList::AddString(errString);
				}
				else if (!theApp.m_deferredMode && (radius >= maxRadius))
				{
					errString.Format(_T("Petal radius r must be less than %f (requested was %f) -- ignored."),
						maxRadius, radius);
					COutputList::AddString(errString);
//					m_r = maxRadius;
//					Reconstruct();
				}
				else
				{
					pDP->m_r = radius;
					if (!theApp.m_deferredMode) Reconstruct();
					else m_updateFlags.reconstruct = true;
				}
			}
			break;
		case 2:	// R
			{
				if (!theApp.CheckCommandLength(messageLength, 6, _T("Set Petal Radius R")))
				{
					m_errorCode = 2;
					theApp.m_errorMask |= 2;
					return;
				}
				float radius = *(float*) &message[2];
				float maxRadius = m_petalParams.m_d*(1.0f-m_petalParams.m_q);
//				TRACE("MaxRadius = %f\n", maxRadius);
				if (radius < 0.0f)
				{
					errString.Format(_T("Negative radius (%f) is invalid for petal -- ignored."),
						radius);
					COutputList::AddString(errString);
				}
				else if (!theApp.m_deferredMode && (radius >= maxRadius))
				{
					errString.Format(_T("Petal radius R must be less than %f (requested was %f) -- ignored."),
						maxRadius, radius);
					COutputList::AddString(errString);
//					m_R = maxRadius;
//					Reconstruct();
				}
				else
				{
					pDP->m_R = radius;
					if (!theApp.m_deferredMode) Reconstruct();
					else m_updateFlags.reconstruct = true;
				}
			}
			break;
		case 3:	// d
			{
				if (!theApp.CheckCommandLength(messageLength, 6, _T("Set Petal distance d")))
				{
					m_errorCode = 2;
					theApp.m_errorMask |= 2;
					return;
				}
				float d = *(float*) &message[2];
				UINT16 mind = (UINT16) (1.0f + max(
					m_petalParams.m_r/m_petalParams.m_q,
					m_petalParams.m_R/(1.0f-m_petalParams.m_q)));
				if (!theApp.m_deferredMode && (d < (float) mind))
				{
					errString.Format(_T("Petal distance d must be >= %u (requested was %f)."),
						mind, d);
					COutputList::AddString(errString);
//					d = (float) mind;
				}
				else
				{
					pDP->m_d = d;
					if (!theApp.m_deferredMode) Reconstruct();
					else m_updateFlags.reconstruct = true;
				}
			}
			break;
		case 4:	// q
			{
				if (!theApp.CheckCommandLength(messageLength, 6, _T("Set Petal parameter q")))
				{
					m_errorCode = 2;
					theApp.m_errorMask |= 2;
					return;
				}
				float q = *(float*) &message[2];
				float minq = m_petalParams.m_r/m_petalParams.m_d;
				float maxq = 1.0f - m_petalParams.m_R/m_petalParams.m_d;
				if (!theApp.m_deferredMode && ((q <= minq) || (q >= maxq)))
				{
					errString.Format(_T("Petal parameter q must be greater than %f and less than %f (requested was %f)."),
						minq, maxq, q);
					COutputList::AddString(errString);
//					q = minq;
				}
				else
				{
					pDP->m_q = q;
					if (!theApp.m_deferredMode) Reconstruct();
					else m_updateFlags.reconstruct = true;
				}
			}
			break;
		default:
			m_errorCode = 3;
			theApp.m_errorMask |= 2;
			errString.Format(_T("Invalid petal command. Trace: %u %u %u %u %u %u."),
				message[0], message[1], message[2], message[3], message[4], message[5]);
			COutputList::AddString(errString);
			return;
		}
		break;
	case 4:
		if (!theApp.CheckCommandLength(messageLength, 5, _T("Set Petal Orientation")))
		{
			m_errorCode = 2;
			return;
		}
		float *pPhi;
		pPhi = (float*) &message[1];
//		TRACE("Rectangle orientation: %f\n", *pPhi);
		float phi;
		phi = *pPhi*(float)M_PI/180.f;
		D2D1_MATRIX_3X2_F* pTransform;
		pTransform = &(theApp.m_deferredMode ? m_transformCopy : m_transform);
		pTransform->_21 = sin(phi);
		pTransform->_12 = -pTransform->_21;
		pTransform->_11 = cos(phi);
		pTransform->_22 =  pTransform->_11;
		break;
	case 5:
		if (!theApp.CheckCommandLength(messageLength, 5, _T("Set Petal Color")))
		{
			m_errorCode = 2;
			return;
		}
		if (theApp.m_deferredMode)
		{
			m_deferableParamsCopy.color = D2D1::ColorF(
				message[1]/255.f, message[2]/255.f, message[3]/255.f, message[4]/255.f);
			m_updateFlags.color = true;
		}
		else
		{
			m_deferableParams.color = D2D1::ColorF(
				message[1]/255.f, message[2]/255.f, message[3]/255.f, message[4]/255.f);
			m_pBrush->SetColor(m_deferableParams.color);
		}
		break;
	case 6:
		if (!theApp.CheckCommandLength(messageLength, 2, _T("Set Petal Draw Mode")))
		{
			m_errorCode = 2;
			return;
		}
		BYTE* pMode;
		pMode = &(theApp.m_deferredMode ? m_deferableParamsCopy.drawMode : m_deferableParams.drawMode);
		*pMode = message[1];
		break;
	case 7:
		WritePipe(&m_errorCode, sizeof(m_errorCode));
		m_errorCode = 0;
		break;
	case 9:
		if (!theApp.CheckCommandLength(messageLength, 5, _T("Set Petal Outline Color")))
		{
			m_errorCode = 2;
			return;
		}
		if (theApp.m_deferredMode)
		{
			m_deferableParamsCopy.outlineColor = D2D1::ColorF(
				message[1]/255.f, message[2]/255.f, message[3]/255.f, message[4]/255.f);
			m_updateFlags.outlineColor = true;
		}
		else
		{
			m_deferableParams.outlineColor = D2D1::ColorF(
				message[1]/255.f, message[2]/255.f, message[3]/255.f, message[4]/255.f);
			m_pOutlineBrush->SetColor(m_deferableParams.outlineColor);
			TRACE("Outline Color: %f %f %f %f\n",
				m_deferableParams.outlineColor.r,
				m_deferableParams.outlineColor.g,
				m_deferableParams.outlineColor.b,
				m_deferableParams.outlineColor.a);
		}
		break;
	case 10:
		if (!theApp.CheckCommandLength(messageLength, 5, _T("Set Petal Linewidth")))
		{
			m_errorCode = 2;
			return;
		}
		float* pWidth;
		pWidth = &(theApp.m_deferredMode ? m_deferableParamsCopy.strokeWidth : m_deferableParams.strokeWidth);
		*pWidth = *(float*) &message[1];
		break;
	}
}

void CPetal::makeCopy(void)
{
	CD2DStimulus::makeCopy();
	m_deferableParamsCopy = m_deferableParams;
	m_petalParamsCopy = m_petalParams;
}

void CPetal::getCopy(void)
{
	CD2DStimulus::getCopy();
	m_deferableParams = m_deferableParamsCopy;
	if (m_updateFlags.color)
	{
		m_pBrush->SetColor(m_deferableParams.color);
	}
	if (m_updateFlags.outlineColor)
	{
		m_pOutlineBrush->SetColor(m_deferableParams.outlineColor);
	}
	if (m_updateFlags.reconstruct)
	{
		if ((m_petalParamsCopy.m_q*m_petalParamsCopy.m_d >= m_petalParamsCopy.m_r) &&
			((1.0f-m_petalParamsCopy.m_q)*m_petalParamsCopy.m_d >= m_petalParamsCopy.m_R))
		{
			m_petalParams = m_petalParamsCopy;
			Reconstruct();
		}
		else
		{
			CString errString;
			errString.Format(_T("Invalid petal params - changes ignored."));
			COutputList::AddString(errString);
		}
	}
	m_updateFlags.all = 0;
}


CEllipse::CEllipse(void)
{
	CD2DStimulus::CD2DStimulus();
	m_typeName = _T("ellipse");
	HRESULT hr;
	m_deferableParams.strokeWidth = 2.0f;
	m_deferableParams.color = theApp.m_defaultDrawColor;
	hr = theApp.m_pContext->CreateSolidColorBrush(
		theApp.m_defaultDrawColor,
		&m_pBrush
	);
	ASSERT(hr == S_OK);
	m_deferableParams.outlineColor = theApp.m_defaultOutlineColor;
	hr = theApp.m_pContext->CreateSolidColorBrush(
		theApp.m_defaultOutlineColor,
		&m_pOutlineBrush
	);
	ASSERT(hr == S_OK);
	//	m_deferableParams.transform = theApp.m_contextTransform;
	//	m_transform = theApp.m_contextTransform;
	m_deferableParams.ellipse = D2D1::Ellipse(D2D1::Point2F(0.0f, 0.0f), 50.0f, 50.0f);
	m_updateFlags.all = 0;
}

/*
CStimulusRect::~CStimulusRect(void)
{
//	CStimulus::~CStimulus();
}
*/

void CEllipse::Draw(void)
{
	if (!theApp.m_drawMode) theApp.BeginDraw();
	theApp.m_pContext->SetTransform(m_transform);
	if (m_deferableParams.drawMode & 1)
		theApp.m_pContext->FillEllipse(m_deferableParams.ellipse, m_pBrush);
	if (m_deferableParams.drawMode & 2)
		theApp.m_pContext->DrawEllipse(m_deferableParams.ellipse, m_pOutlineBrush,
			m_deferableParams.strokeWidth);
	theApp.m_pContext->SetTransform(theApp.m_contextTransform);
}


void CEllipse::Command(unsigned char message[], DWORD messageLength)
{
	//	TRACE("CStimulusEllipse::Command\n");
	switch (message[0]) {
	case 1:		// set size
		if (!theApp.CheckCommandLength(messageLength, 6, _T("Set Ellipse Size")))
		{
			m_errorCode = 2;
			return;
		}
		if (message[1] != 1)
		{
			m_errorCode = 3;
			theApp.m_errorMask |= 2;
			CString errString;
			errString.Format(_T("Invalid Ellipse command (set size?). Trace: %u %u %u %u %u %u."),
				message[0], message[1], message[2], message[3], message[4], message[5]);
			COutputList::AddString(errString);
			return;
		}
		WORD* pSize;
		pSize = (WORD*)&message[2];
		float w2;
		float h2;
		w2 = (*pSize++ - 1) / 2.0f;
		h2 = (*pSize - 1) / 2.0f;
		(theApp.m_deferredMode ? m_deferableParamsCopy : m_deferableParams).ellipse =
			D2D1::Ellipse(D2D1::Point2F(0.0f, 0.0f), w2, h2);
		break;
	case 4:
		if (!theApp.CheckCommandLength(messageLength, 5, _T("Set Ellipse Orientation")))
		{
			m_errorCode = 2;
			return;
		}
		float *pPhi;
		pPhi = (float*)&message[1];
		//		TRACE("Rectangle orientation: %f\n", *pPhi);
		float phi;
		phi = *pPhi*(float)M_PI / 180.f;
		D2D1_MATRIX_3X2_F* pTransform;
		//		pTransform = 
		//			&(theApp.m_deferredMode ? m_deferableParamsCopy : m_deferableParams).transform;
		pTransform = &(theApp.m_deferredMode ? m_transformCopy : m_transform);
		pTransform->_21 = sin(phi);
		pTransform->_12 = -pTransform->_21;
		pTransform->_11 = cos(phi);
		pTransform->_22 = pTransform->_11;
		break;
	case 5:
		if (!theApp.CheckCommandLength(messageLength, 5, _T("Set Ellipse Color")))
		{
			m_errorCode = 2;
			return;
		}
		ASSERT(messageLength == 5);
		if (theApp.m_deferredMode)
		{
			m_deferableParamsCopy.color = D2D1::ColorF(
				message[1] / 255.f, message[2] / 255.f, message[3] / 255.f, message[4] / 255.f);
			m_updateFlags.color = true;
		}
		else
		{
			m_deferableParams.color = D2D1::ColorF(
				message[1] / 255.f, message[2] / 255.f, message[3] / 255.f, message[4] / 255.f);
			m_pBrush->SetColor(m_deferableParams.color);
		}
		break;
	case 6:
		if (!theApp.CheckCommandLength(messageLength, 2, _T("Set Ellipse Draw Mode")))
		{
			m_errorCode = 2;
			return;
		}
		BYTE* pMode;
		pMode = &(theApp.m_deferredMode ? m_deferableParamsCopy.drawMode : m_deferableParams.drawMode);
		*pMode = message[1];
		break;
	case 7:
		WritePipe(&m_errorCode, sizeof(m_errorCode));
		m_errorCode = 0;
		break;
	case 9:
		if (!theApp.CheckCommandLength(messageLength, 5, _T("Set Ellipse Outline Color")))
		{
			m_errorCode = 2;
			return;
		}
		if (theApp.m_deferredMode)
		{
			m_deferableParamsCopy.outlineColor = D2D1::ColorF(
				message[1] / 255.f, message[2] / 255.f, message[3] / 255.f, message[4] / 255.f);
			m_updateFlags.outlineColor = true;
		}
		else
		{
			m_deferableParams.outlineColor = D2D1::ColorF(
				message[1] / 255.f, message[2] / 255.f, message[3] / 255.f, message[4] / 255.f);
			m_pOutlineBrush->SetColor(m_deferableParams.outlineColor);
			TRACE("Outline Color: %f %f %f %f\n",
				m_deferableParams.outlineColor.r,
				m_deferableParams.outlineColor.g,
				m_deferableParams.outlineColor.b,
				m_deferableParams.outlineColor.a);
		}
		break;
	case 10:
		if (!theApp.CheckCommandLength(messageLength, 5, _T("Set Ellipse Linewidth")))
		{
			m_errorCode = 2;
			return;
		}
		float* pWidth;
		pWidth = &(theApp.m_deferredMode ? m_deferableParamsCopy.strokeWidth : m_deferableParams.strokeWidth);
		*pWidth = *(float*)&message[1];
		break;
	}
}

/*
void CStimulusRect::Moveto(bool deferred, float x, float y)
{
	D2D1_MATRIX_3X2_F* pTransform;
	pTransform =
		&(deferred ? m_deferableParamsCopy : m_deferableParams).transform;
//	TRACE("Translation: %f, %f\n", pTransform->_31, pTransform->_32);
	pTransform->_31 = theApp.m_contextTransform._31 + x;
	pTransform->_32 = theApp.m_contextTransform._32 - y;
}
*/
/*
void CStimulusRect::GetPos(float pos[2])
{
//	pos[0] =  m_deferableParams.transform._31 - theApp.m_contextTransform._31;
//	pos[1] = -m_deferableParams.transform._32 + theApp.m_contextTransform._32;
	pos[0] =  m_transform._31 - theApp.m_contextTransform._31;
	pos[1] = -m_transform._32 + theApp.m_contextTransform._32;
}
*/

void CEllipse::makeCopy(void)
{
	CD2DStimulus::makeCopy();
	//	CStimulus::makeCopy();
	m_deferableParamsCopy = m_deferableParams;
}


void CEllipse::getCopy(void)
{
	CD2DStimulus::getCopy();
	m_deferableParams = m_deferableParamsCopy;
	if (m_updateFlags.color)
	{
		m_pBrush->SetColor(m_deferableParams.color);
	}
	if (m_updateFlags.outlineColor)
	{
		m_pOutlineBrush->SetColor(m_deferableParams.outlineColor);
	}
	m_updateFlags.all = 0;
}
/*
bool CStimulusRect::SetAnimParam(BYTE mode, float value)
{
		COutputList::AddString(L"Currently linear range animations are not implemented for rectangle stimuli.");
		return false;
}
*/
