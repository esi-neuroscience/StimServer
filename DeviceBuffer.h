#pragma once
#include <d3d11_1.h>

class CDeviceBuffer
{
public:
	CDeviceBuffer(void);
	virtual ~CDeviceBuffer(void);
	virtual HRESULT Init(void* pData, UINT dataSize);
	ID3D11Buffer* m_pDeviceBuffer;
//protected:
	D3D11_BUFFER_DESC m_BuffDesc;
};


class CDeviceConstBuffer :
	public CDeviceBuffer
{
public:
	CDeviceConstBuffer(void);
//	~CDeviceConstBuffer(void);
	HRESULT Update(float animationPar[], unsigned char parSize);
};


class CDeviceVertexBuffer :
	public CDeviceBuffer
{
public:
	CDeviceVertexBuffer(void);
//	~CDeviceVertexBuffer(void);
};

