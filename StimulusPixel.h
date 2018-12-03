#pragma once
#include "stimulus.h"
//#include "StimulusSymbol.h"
#include "StimServer.h"

class CStimulusPixel :
	public CStimulus
//	public CStimulusSymbol
{
public:
	CStimulusPixel(void);
	~CStimulusPixel(void);
	void Draw(void);
	void Command(unsigned char message[], DWORD messageLength);
//	void moveto(float point[2]);
	void Moveto(bool deferred, float x, float y);
	void GetPos(float pos[2]);
//	bool SetAnimParam(BYTE mode, float value);
	static CDeviceVertexBuffer m_VertexBuffer;
    static D3D11_VIEWPORT m_vp;
private:
	struct VS_ANIMATION {
		D2D1_VECTOR_2F shift;
		float dummy[2];
	} m_VSanimationData;
	ID3D11PixelShader* m_pPixelShaderMin;
	CDeviceConstBuffer m_AnimationBuffer;
};

