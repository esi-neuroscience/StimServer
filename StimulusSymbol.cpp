#include "StdAfx.h"
#include "StimulusSymbol.h"
#include "StimServer.h"

//extern CStimServerApp theApp;

// ID3D11GeometryShader* CStimulusGeometrySymbol::m_pGeometryShader = NULL;

CStimulusSymbol::CStimulusSymbol(void)
{
}


CStimulusSymbol::~CStimulusSymbol(void)
{
}

/*
bool CStimulusSymbol::Init(void)
{
	return false;
}
*/

void CStimulusSymbol::Draw(void)
{
	if (theApp.m_drawMode) theApp.EndDraw();

	TRACE("CStimulusSymbol::Draw(void)\n");
    // Set vertex buffer
//    UINT stride = sizeof( XMFLOAT3 );
    UINT stride = 12;
    UINT offset = 0;
//	theApp.m_pImmediateContext->IASetVertexBuffers( 0, 1, &m_VertexBuffer.m_pDeviceBuffer, &stride, &offset );
    // Set primitive topology
    theApp.m_pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_POINTLIST );
	theApp.m_pImmediateContext->VSSetShader( theApp.m_pVertexShader, NULL, 0 );
	theApp.m_pImmediateContext->Draw( 1, 0 );	// (Vertex Count, Start)
}


void CStimulusSymbol::Command(bool deferred, unsigned char message[], DWORD messageLength)
{
}

#ifdef undef

CStimulusGeometrySymbol::CStimulusGeometrySymbol(void)
{
}


CStimulusGeometrySymbol::CStimulusGeometrySymbol(unsigned short size[2])
{
	TRACE("CStimulusGeometrySymbol::CStimulusGeometrySymbol(unsigned short size[2])\n");
    // Setup the viewport
	m_vp.Width =  size[0];
	m_vp.Height = size[1];
    m_vp.MinDepth = 0.0f;
	m_vp.MaxDepth = 1.0f;

	HRESULT hr;
	/*
	float vertices[3] = {0.0f, 0.0f, 0.0f};
	m_VertexBuffer = CDeviceVertexBuffer();
	HRESULT hr = m_VertexBuffer.Init(vertices, sizeof( float ) * 3);
	ASSERT(hr == S_OK);
	*/
//	delete vertices;

    // Compile the vertex shader
	const char vertexShader[] = 
//"cbuffer VS_PARAMS : register (b0)\n"
//"{\n"
//"    float4  color : packoffset(c0.x);\n"
//"    float   size  : packoffset(c1.x);\n"
//"};\n"
"struct OUTPUT {\n"
"    float4 Position   : SV_POSITION;\n"
"    float4 Color      : COLOR;\n"
"	 float	Size	   : PSIZE;\n"
"};\n"
"OUTPUT VSmin( float4 Pos : POSITION )\n"
"{\n"
"	 OUTPUT output;\n"
"	 output.Position = Pos;\n"
//"	 output.Color = color;\n"
//"	 output.Size = size;\n"
"	 output.Color = float4(1.0f, 1.0f, 1.0f, 1.0f);\n"
"	 output.Size = float(2.0f);\n"
"    return output;\n"
"}\n"
;
    ID3DBlob* pVSBlob = NULL;
	theApp.CompileShader(vertexShader, sizeof(vertexShader), "VSmin", "vs_4_0", &pVSBlob);

	// Create the vertex shader
//	ID3D11VertexShader* pVertexShader;
	hr = theApp.m_pD3Ddevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &m_pVertexShader );
	if( FAILED( hr ) )
	{	
		pVSBlob->Release();
        ASSERT(false);
	}
	TRACE("pVertex Shader: %p\n", m_pVertexShader);

	if (m_pGeometryShader == NULL) {
		// Compile the geometry shader
		const char geometryShader[] = 
//			"cbuffer GS_PARAMS : register (b0)\n"
//			"{\n"
//			"    float2 pixelSize : packoffset(c0.x);\n"
//			"};\n"
			"static float2 newPos[4] =\n"
			"{\n"
			"	float2(-1,-1),\n"
			"	float2(-1, 1),\n"
			"	float2( 1,-1),\n"
			"	float2( 1, 1) \n"
			"};\n"
			"struct INPUT {\n"
			"    float4 Position   : SV_POSITION;\n"
			"    float4 Color      : COLOR;\n"
			"	 float	Size	   : PSIZE;\n"
			"};\n"
			"struct OUTPUT {\n"
			"    float4 Position   : SV_POSITION;\n"
			"    float4 Color      : COLOR;\n"
			"};\n"
			"[maxvertexcount(4)]\n"
			"void GSmin( point INPUT particle[1], inout TriangleStream<OUTPUT> triangleStream )\n"
			"{\n"
			"	 OUTPUT output;\n"
			"	 for (int i=0; i<4; i++)\n"
			"	 {\n"
			"		output.Position = particle[0].Position;\n"
//			"		output.Position.xy += newPos[i] * (float2) particle[0].Size * pixelSize;\n"
			"		output.Position.xy += newPos[i] * (float2) particle[0].Size;\n"
			"		output.Color = particle[0].Color;\n"
			"		triangleStream.Append(output);\n"
			"	 }\n"
			"}\n"
			;
		ID3DBlob* pGSBlob = NULL;
		hr = CStimServerApp::CompileShader(geometryShader, sizeof(geometryShader), "GSmin", "gs_4_0", &pGSBlob);
		if( FAILED( hr ) )
		{	
			if (pGSBlob) pGSBlob->Release();
			m_pGeometryShader = NULL;
			ASSERT(false);
		}

		// Create the geometry shader
		hr = theApp.m_pD3Ddevice->CreateGeometryShader( pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize(), NULL, &m_pGeometryShader );
		if( FAILED( hr ) )
		{	
			pGSBlob->Release();
			m_pGeometryShader = NULL;
			ASSERT(false);
		}
	}
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
	TRACE("pVertex Shader: %p\n", m_pVertexShader);
}


CStimulusGeometrySymbol::~CStimulusGeometrySymbol(void)
{
}

void CStimulusGeometrySymbol::Draw(void)
{
	if (theApp.m_drawMode) theApp.EndDraw();

    // Set vertex buffer
//    UINT stride = sizeof( XMFLOAT3 );
    UINT stride = 12;
    UINT offset = 0;
//	theApp.m_pImmediateContext->IASetVertexBuffers( 0, 1, &m_VertexBuffer.m_pDeviceBuffer, &stride, &offset );
    // Set primitive topology
    theApp.m_pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_POINTLIST );
	TRACE("pVertex Shader: %p\n", m_pVertexShader);
	theApp.m_pImmediateContext->VSSetShader( m_pVertexShader, NULL, 0 );
	theApp.m_pImmediateContext->GSSetShader( m_pGeometryShader, NULL, 0 );
//	theApp.m_pImmediateContext->PSSetShader( theApp.m_pVertexShader, NULL, 0 );
	theApp.m_pImmediateContext->Draw( 1, 0 );	// (Vertex Count, Start)
}

void CStimulusGeometrySymbol::moveto(float point[2])
{
//	HRESULT hr = m_AnimationBuffer.Update((float*) point, 2);
}


CStimulusSymbolRect::CStimulusSymbolRect(void)
{
}


CStimulusSymbolRect::CStimulusSymbolRect(unsigned short size[2])
{
	CStimulusGeometrySymbol::CStimulusGeometrySymbol(size);
}


CStimulusSymbolRect::~CStimulusSymbolRect(void)
{
}

#endif

