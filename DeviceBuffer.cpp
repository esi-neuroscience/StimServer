#include "StdAfx.h"
#include "DeviceBuffer.h"
#include "StimServer.h"

CDeviceBuffer::CDeviceBuffer(void)
	: m_pDeviceBuffer(NULL)
{
//	TRACE("CDeviceBuffer Konstruktor\n");
	ZeroMemory(&m_BuffDesc, sizeof(D3D11_BUFFER_DESC));
}


CDeviceBuffer::~CDeviceBuffer(void)
{
	if (m_pDeviceBuffer) m_pDeviceBuffer->Release();	// in case Init fails
//	m_pDeviceBuffer = NULL;
	TRACE("CDeviceBuffer Destruktor\n");
}


HRESULT CDeviceBuffer::Init(void* pBufferData, UINT sizeOfBuffer)
{
	m_BuffDesc.ByteWidth = sizeOfBuffer;
	TRACE("Byte Width of Buffer: %u\n", m_BuffDesc.ByteWidth);

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory( &InitData, sizeof(InitData) );
	InitData.pSysMem = pBufferData;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	HRESULT hr = theApp.m_pD3Ddevice->CreateBuffer(&m_BuffDesc, &InitData, &m_pDeviceBuffer);
	ASSERT(hr == S_OK);
	return hr;
}



CDeviceConstBuffer::CDeviceConstBuffer(void)
{
	this->CDeviceBuffer::CDeviceBuffer();
	m_BuffDesc.Usage = D3D11_USAGE_DYNAMIC;
	m_BuffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	m_BuffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
}

/*
CDeviceConstBuffer::~CDeviceConstBuffer(void)
{
	if (m_pAnimationBuffer) m_pAnimationBuffer->Release();
	m_pAnimationBuffer = NULL;
	TRACE("CAnimationBuffer Destruktor\n");
}
*/


HRESULT CDeviceConstBuffer::Update(float animationPar[], unsigned char parSize)
{
	D3D11_MAPPED_SUBRESOURCE mappedConstBuffer = {0};

	HRESULT hr = theApp.m_pImmediateContext->Map(m_pDeviceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedConstBuffer);
	ASSERT(hr == S_OK);

	float* pData = (float*) mappedConstBuffer.pData;
	for (unsigned char i=0; i<parSize; i++) *pData++ = animationPar[i];

	theApp.m_pImmediateContext->Unmap(m_pDeviceBuffer, 0);
	return hr;
}



CDeviceVertexBuffer::CDeviceVertexBuffer(void)
{
//	CDeviceBuffer::CDeviceBuffer();
	this->CDeviceBuffer::CDeviceBuffer();
	m_BuffDesc.Usage = D3D11_USAGE_DEFAULT;
	m_BuffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_BuffDesc.CPUAccessFlags = 0;
}

/*
CDeviceVertexBuffer::~CDeviceVertexBuffer(void)
{
	CDeviceBuffer::~CDeviceBuffer();
}
*/