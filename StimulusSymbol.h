#pragma once
#include "stimulus.h"


class CStimulusSymbol :
	public CStimulus
{
public:
	CStimulusSymbol(void);
	~CStimulusSymbol(void);
//	bool Init(void);
	virtual void Draw(void) = 0;
	virtual void Command(bool deferred, unsigned char message[], DWORD messageLength);
//	virtual void moveto(float point[2]) = 0;
//	static CDeviceVertexBuffer m_VertexBuffer;
//    static D3D11_VIEWPORT m_vp;
protected:
//	struct VS_ANIMATION {
//		D2D1_VECTOR_2F shift;
//		float dummy[2];
//	} m_VSanimationData;
private:
	CSymbol* m_symbol;
};

/*
class CStimulusGeometrySymbol :
	public CStimulusSymbol
{
public:
	CStimulusGeometrySymbol(void);
	CStimulusGeometrySymbol(unsigned short size[2]);
	~CStimulusGeometrySymbol(void);
	void Draw(void);
	void moveto(float point[2]);
private:
    D3D11_VIEWPORT m_vp;
	ID3D11VertexShader* m_pVertexShader;
	static ID3D11GeometryShader* m_pGeometryShader;
	ID3D11PixelShader* m_pPixelShaderMin;
};


class CStimulusSymbolRect :
	public CStimulusGeometrySymbol
{
public:
	CStimulusSymbolRect(void);
	CStimulusSymbolRect(unsigned short size[2]);
	~CStimulusSymbolRect(void);
};



class CStimulusTextureSymbol :
	public CStimulusGeometrySymbol
{
public:
	CStimulusTextureSymbol(void);
	CStimulusTextureSymbol(unsigned short size[2]);
	~CStimulusTextureSymbol(void);
};


class CStimulusDisk :
	public CStimulusTextureSymbol
{
public:
	CStimulusDisk(void);
	CStimulusDisk(unsigned short size[2]);
	~CStimulusDisk(void);
};

*/