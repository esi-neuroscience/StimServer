#pragma once

#define NPARAMS 8

//#include "stdafx.h"
#include "StimServer.h"
#include "DeviceBuffer.h"
#include "symbol.h"
//#include "Anim.h"
#include <d2d1_1.h>
#include "D2DBitmapFile.h"


HRESULT CompileShaderFromFile(
	LPCWSTR wzFilename, 
	LPCSTR	pEntrypoint,
	LPCSTR	pTarget,
	ID3DBlob** pSourceBlob);

static ID3D11GeometryShader* g_GSparticle;


class CAnim;

// CStimulus-Befehlsziel

class CStimulus : public CObject
{
public:
	CStimulus();
	virtual ~CStimulus();
//	DECLARE_SERIAL(CStimulus)
	virtual void Draw() = 0;
	virtual void Command(unsigned char message[], DWORD messageLength) = 0;
	virtual void makeCopy(void) {m_enabledCopy = m_enabled;};
	virtual void getCopy(void)  {m_enabled = m_enabledCopy;};
	virtual void Moveto(bool deferred, float x, float y) {};
	virtual void GetPos(float pos[2]) = 0;	// abstract <-- ARCADE wants to use it
	// define a default for all stimuli in case a "linear change animation" is assigned.
	virtual bool SetAnimParam(BYTE mode, float value);

	bool m_enabled;
	bool m_enabledCopy;
	bool m_supressed;
	bool m_protected;
	static CDeviceVertexBuffer VertexBufferUnitQuad;
	static ID3D11PixelShader* pPixelShaderTex;
	static ID3D11VertexShader* m_pVertexShaderCol;
	CAnim* m_animation;
protected:
//	bool CheckCommandLength(DWORD commandLength, DWORD requiredLength, LPCWSTR command);
	void InvalidCommand(BYTE code);
	static bool CreatePixelShaderTex(void);
	WORD m_errorCode;
	LPCTSTR m_typeName = _T("");
};


class CD2DStimulus :
	public CStimulus
{
public:
	CD2DStimulus(void);
	~CD2DStimulus(void);
	virtual void Draw() { if (!theApp.m_drawMode) theApp.BeginDraw(); };
	virtual void Command(unsigned char message[], DWORD messageLength) {};
	virtual void makeCopy(void);
	virtual void getCopy(void);
	void Moveto(bool deferred, float x, float y);
protected:
	bool ShapeCommand(unsigned char message[], DWORD messageLength);
	bool ShapeSetSize(unsigned char message[], DWORD messageLength, float* w2, float* h2);
	void GetPos(float pos[2]);
	void SetOrientation(float angle);
	D2D1_MATRIX_3X2_F m_transform, m_transformCopy;
	ID2D1SolidColorBrush* m_pBrush = NULL;
	ID2D1SolidColorBrush* m_pOutlineBrush = NULL;
	struct
	{
		D2D1_COLOR_F color;
		D2D1_COLOR_F outlineColor;
		float strokeWidth;
		BYTE drawMode;
	} m_deferableParams, m_deferableParamsCopy = { 0 };
	union
	{
		unsigned char all;
		struct
		{
			unsigned char color : 1;
			unsigned char outlineColor : 1;
			unsigned char unused : 6;
		};
	} m_updateFlags;
};



class CLoadedStimulus :
	public CStimulus
{
public:
	CLoadedStimulus(void);
	~CLoadedStimulus(void);
	virtual bool Init(LPCWSTR wzFilename) = 0;
};


class C3DStimulus //:
//	public CStimulus
{
public:
	C3DStimulus(void);
	virtual ~C3DStimulus(void);
protected:
	void Moveto(bool deferred, float x, float y);
	void GetPos(float pos[2]);
    D3D11_VIEWPORT m_vp, m_vpCopy;
};


class CStimulusPic : public CLoadedStimulus
{
	DECLARE_DYNAMIC( CStimulusPic )
public:
	CStimulusPic();
	bool Init(LPCWSTR wzFilename);
	~CStimulusPic();
	void Draw();
	void Command(unsigned char message[], DWORD messageLength);
	void Rotate(bool deferred, float phi);
	void makeCopy(void);
	void getCopy(void);
	void Moveto(bool deferred, float x, float y);
	void GetPos(float pos[2]);
	bool SetAnimParam(BYTE mode, float value);
	ID2D1Bitmap* m_pBitmap;
	struct
	{
		D2D1_MATRIX_4X4_F transform;
		float	alpha;
		char	phiInc;
	} m_deferableParams, m_deferableParamsCopy;
protected:
	D2D1_RECT_F m_destRect;
	short m_iPhi;
};

// Base class for Stimuli generated by (animated) Pixel Shaders
class CPixelShader : public CLoadedStimulus, public C3DStimulus
{
public:
	CPixelShader();
	~CPixelShader();
	ID3DBlob* CreateShader(LPCWSTR wzFilename);
	virtual void Draw();
	void SetParam(BYTE index, float value);
	virtual void Command(unsigned char message[], DWORD messageLength);
	void makeCopy(void);
	virtual void getCopy(void);
	bool SetAnimParam(BYTE mode, float value);
protected:
	ID3D11PixelShader* m_pPixelShader;
	float m_phiInc, m_phiIncCopy;
	float m_phaseCopy;
	struct PS_CONSTANT_BUFFER {
		float center[2];
		float width;
		float height;
		float params[NPARAMS];
	} m_PSconstData;
	struct PS_ANIMATION {
		float phase;
		float dummy[3];
	} m_PSanimationData;
	CDeviceConstBuffer m_ConstantBuffer;
	CDeviceConstBuffer m_AnimationBuffer;
	void Moveto(bool deferred, float x, float y);
	void GetPos(float pos[2]) {
		C3DStimulus::GetPos(pos);
	};
	void UpdateConstantBuffer(void) {m_ConstantBuffer.Update(&m_PSconstData.center[0], NPARAMS+4);};
//	void UpdateColorBuffer(void) {m_ColorBuffer.Update(&m_PScolorData.components[0], 16);};
	union {
		unsigned char all;
		struct {
			unsigned char constantBuffer:1;
			unsigned char colorBuffer:1;	// not used by PSpic
			unsigned char animatedValue:1;
			unsigned char unused:5;
		};
	} m_updateFlags;
};

// Stimulus generated by a (animated) Pixel Shader
class CStimulusPS : public CPixelShader
{
public:
	CStimulusPS();
	bool Init(LPCWSTR wzFilename);
//	~CStimulusPS();
	void Draw();
	void Command(unsigned char message[], DWORD messageLength);
	void getCopy(void);
private:
	union PS_COLOR_BUFFER {
		float components[16];
		D3DCOLORVALUE color[4];
	} m_PScolorData;
	CDeviceConstBuffer m_ColorBuffer;
	void UpdateColorBuffer(void) {m_ColorBuffer.Update(&m_PScolorData.components[0], 16);};
	bool m_legacy;
};

struct PDCSR {
	unsigned char lit : 1;
	unsigned char flicker : 1;
	unsigned char unused : 6;
};

class CPhotoDiode : public CObject
{
public:
	static void Init(void);
	static void Cleanup(void);
//	CPhotoDiode(D2D1_RECT_F PDrect);
//	virtual ~CPhotoDiode(void);
	static void SetPosition(BYTE position);
private:
	static D2D1_RECT_F m_rect;
	static ID2D1SolidColorBrush* m_pBlackBrush;
public:
	static void Draw(void);
	static bool m_enabled;
	static PDCSR CSR;
};

class CStimulusMov : public CLoadedStimulus
{
public:
	CStimulusMov(void);
	~CStimulusMov(void);
	bool Init(LPCWSTR wzFilename);
	void Draw(void);
	void Command(unsigned char message[], DWORD messageLength);
	void makeCopy(void);
	void GetPos(float pos[2]);
//	bool SetAnimParam(BYTE mode, float value);
private:
	ID2D1Bitmap1* m_pBitmap;
};

class CStimulusSymbol :
	public CStimulus , public C3DStimulus
{
public:
//	CStimulusSymbol(void);
	CStimulusSymbol(BYTE type, unsigned short size);
//	~CStimulusSymbol(void);
//	bool Init(void);
	void Draw(void);
	void Command(unsigned char message[], DWORD messageLength);
	void makeCopy(void);
	void getCopy(void);
private:
	
	void Moveto(bool deferred, float x, float y) {C3DStimulus::Moveto(deferred, x, y);};
	void GetPos(float pos[2]) {C3DStimulus::GetPos(pos);};
	struct VS_PARAMS
	{
		D3DCOLORVALUE vertexColor;
	} m_VSparams;
	union
	{
		unsigned char all;
		struct
		{
			unsigned char vertexColor:1;
			unsigned char particleSize:1;
			unsigned char unused:6;
		};
	} m_updateFlags;
	CSymbol* m_symbol;
	CDeviceConstBuffer m_ParamsBuffer;
	void UpdateParticleSize(void);
};


class CStimulusPart : public CLoadedStimulus, public C3DStimulus
{
public:
	CStimulusPart(unsigned short size[2]);
	~CStimulusPart(void);
	bool Init(LPCWSTR wzFilename);
	void Draw();
	void Command(unsigned char message[], DWORD messageLength);
	void makeCopy(void);
	void getCopy(void);
//	bool SetAnimParam(BYTE mode, float value);
	struct VS_ANIMATION {
		float shift;
		float dummy[3];
	} m_VSanimationData;
private:
	struct VS_PARAMS
	{
		D3DCOLORVALUE vertexColor;
		float size;
		float angle;
		float dummy[2];
	} m_VSparams;
	void Moveto(bool deferred, float x, float y);
//	void GetPos(float pos[2]);
	void GetPos(float pos[2]) {
		C3DStimulus::GetPos(pos);
	};
	CDeviceVertexBuffer m_VertexBuffer;
	unsigned short m_nParticles;
	float m_velocity, m_velocityCopy;
	float m_shift;
	float m_shiftCopy;
	CDeviceConstBuffer m_AnimationBuffer;
	CDeviceConstBuffer m_ParamsBuffer;
	CDeviceConstBuffer m_GSparamsBuffer;
	CSymbol* m_symbol;
	struct GS_PARAMS {
		float pixelSize[2];
		float patchRadius;
		float gaussRadius;
	} m_GSparams;
	union {
		unsigned char all;
		struct {
			unsigned char vertexColor:1;
			unsigned char patchRadius:1;
			unsigned char particleSize:1;
			unsigned char angle:1;
			unsigned char shift:1;
			unsigned char gaussRadius:1;
			unsigned char flags:1;
			unsigned char unused:1;
		};
	} m_updateFlags;
	void UpdateParticleSize(void);
	BYTE m_flags;
};


class CStimBmpBrush :
	public CStimulusPic
{
public:
	CStimBmpBrush(void);
	~CStimBmpBrush(void);
	bool Init(LPCWSTR wzFilename);
	void Draw();
	void Moveto(bool deferred, float x, float y);
	void GetPos(float pos[2]);
	void Command(unsigned char message[], DWORD messageLength);
	void makeCopy(void);
	void getCopy(void);
private:
	ID2D1BitmapBrush* m_pBitmapBrush;
	ID2D1Bitmap* m_pOpacityMask;
	D2D1_RECT_F m_destRectCopy;
	D2D1_SIZE_U m_size;
	D2D1_VECTOR_2F m_shift, m_shiftCopy;
;	float m_xShift;
;	float m_yShift;
	bool m_maskEnabled;
	bool m_maskEnabledCopy;
};


// Stimulus generated by a (animated) Pixel Shader
class CStimPSpic : public CPixelShader
{
public:
	CStimPSpic();
	bool Init(LPCWSTR wzFilename);
//	virtual ~CStimPSpic();
	void Draw();
	void assignPic(CStimulus* pPic);
	void Command(unsigned char message[], DWORD messageLength);
	void getCopy(void);
private:
	CPicture* m_pPicture;	// Symbol
};

class CStimulusRect :
	public CD2DStimulus
{
public:
	CStimulusRect(void);
//	~CStimulusRect(void);
	void makeCopy(void);
	void getCopy(void);
	void Draw(void);
	void Command(unsigned char message[], DWORD messageLength);
//	bool SetAnimParam(BYTE mode, float value);
private:
//	void Moveto(bool deferred, float x, float y);
	/*
	union {
		unsigned char all;
		struct {
			unsigned char color:1;
			unsigned char unused:7;
		};
	} m_updateFlags;
	*/
	D2D1_RECT_F m_rect, m_rectCopy;
};


class CStimulusParticle : public CLoadedStimulus, public C3DStimulus
{
public:
	CStimulusParticle(unsigned short size[2]);
	~CStimulusParticle(void);
	bool Init(LPCWSTR wzFilename);

	void Draw();
	void Command(unsigned char message[], DWORD messageLength);
//	void makeCopy(void);
//	void getCopy(void);
//	bool SetAnimParam(BYTE mode, float value);
	struct VS_ANIMATION {
		float shift;
		float dummy[3];
	} m_VSanimationData;
private:
	struct VS_PARAMS
	{
		D3DCOLORVALUE vertexColor;
		float size;
		float angle;
		float dummy[2];
	} m_VSparams;
//	void Moveto(bool deferred, float x, float y);
//	void GetPos(float pos[2]);
	void GetPos(float pos[2]) {
		C3DStimulus::GetPos(pos);
	};
	ID3D11VertexShader* m_pVertexShader;
	CDeviceVertexBuffer m_VertexBuffer;
	unsigned short m_nParticles;
	float m_velocity, m_velocityCopy;
	float m_shift;
	float m_shiftCopy;
	CDeviceConstBuffer m_AnimationBuffer;
	CDeviceConstBuffer m_ParamsBuffer;
	CDeviceConstBuffer m_GSparamsBuffer;
	CSymbol* m_symbol;
	struct GS_PARAMS {
		float pixelSize[2];
		float patchRadius;
		float gaussRadius;
	} m_GSparams;
	union {
		unsigned char all;
		struct {
			unsigned char vertexColor:1;
			unsigned char patchRadius:1;
			unsigned char particleSize:1;
			unsigned char angle:1;
			unsigned char shift:1;
			unsigned char gaussRadius:1;
			unsigned char flags:1;
			unsigned char unused:1;
		};
	} m_updateFlags;
	void UpdateParticleSize(void);
	BYTE m_flags;
};


class CStimulusPics : public CStimulusPic
{
	DECLARE_DYNAMIC( CStimulusPics )
public:
	CStimulusPics();
	bool Init(LPCWSTR wzFilename);
	~CStimulusPics();
	void Draw();
	void Command(unsigned char message[], DWORD messageLength);
	ID2D1Bitmap** m_pBitmaps;
private:
	UINT32 m_nFrames;
	UINT32 m_currentFrame;
	WORD m_rateFrac;
	WORD m_fracCount;
	HANDLE m_hEndEvent;
};


class CEllipse :
	public CD2DStimulus
{
public:
	CEllipse(void);
	//	~CStimulusRect(void);
	void makeCopy(void);
	void getCopy(void);
	void Draw(void);
	void Command(unsigned char message[], DWORD messageLength);
	//	bool SetAnimParam(BYTE mode, float value);
private:
	D2D1_ELLIPSE m_ellipse, m_ellipseCopy;
};


