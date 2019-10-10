#pragma once
#include "afx.h"

class CStimulus;

class CAnim :
	public CObject
{
public:
	CAnim(void);
	~CAnim(void);
	void Deassign(void);
	virtual void Command(unsigned char message[], DWORD messageLength) = 0;
	virtual void Advance(void) = 0;
	CStimulus* m_pStimulus;
	static BYTE m_defaultFinalActionMask;
	union
	{
		BYTE mask;
		struct
		{
			BYTE disable:1;
			BYTE unused:1;
			BYTE togglePD:1;
			BYTE signalEvent:1;
			BYTE restart:1;
			BYTE reverse:1;
			BYTE initialState:1;
			BYTE endDeferredMode:1;
		};
	} m_finalAction;
protected:
	void Finalize(void);
	WORD m_errorCode;
private:
	static HANDLE m_hfinalEvent;
public:
	static bool CreateEvent(void);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////

class CLoadedAnim :
	public CAnim
{
public:
	CLoadedAnim(void);
	~CLoadedAnim(void);
	virtual bool Init(LPCWSTR wzFilename) = 0;
};


class CAnimationPath :
	public CLoadedAnim
{
public:
	CAnimationPath(void);
	~CAnimationPath(void);
	bool Init(LPCWSTR wzFilename);
	void Advance(void);
private:
	void Command(unsigned char message[], DWORD messageLength);
	D2D1_POINT_2F* m_pPathCoords;
	unsigned short m_nPathCoords;
	unsigned long m_index;
};

class CAnimLineSegPath :
	public CAnim
{
public:
	CAnimLineSegPath(WORD speed);
	~CAnimLineSegPath(void);
	void Advance(void);
private:
	void Command(unsigned char message[], DWORD messageLength);
	WORD m_speed;
	short* m_pPathVertices;
	unsigned short m_nPathVertices;
	unsigned long m_index;
	BYTE m_iSegment;
	float m_vx;
	float m_vy;
	short m_x0;
	short m_y0;
	unsigned long m_nThisSegment;
	void Finalize(void);
};

class CAnimHarmonic :
	public CAnim
{
public:
	CAnimHarmonic(WORD nQuaters);
	~CAnimHarmonic(void);
	void Advance(void);
private:
	void Command(unsigned char message[], DWORD messageLength);
	WORD m_nQuaters;
	float m_A;
	float m_phi;
	float m_phiIncr;
	float m_direction;
};

class CAnimLinearRange :
	public CAnim
{
public:
	CAnimLinearRange(float startValue, float endValue, float duration, BYTE mode);
	~CAnimLinearRange(void);
	void Advance(void);
private:
	void Command(unsigned char message[], DWORD messageLength);
	float m_startValue;
	float m_endValue;
	float m_increment;
	float m_currentValue;
	BYTE  m_mode;
};

class CAnimFlash :
	public CAnim
{
public:
	CAnimFlash(WORD nFrames);
	~CAnimFlash(void);
	void Advance(void);
private:
	void Command(unsigned char message[], DWORD messageLength);
	WORD m_nFrames;
	WORD m_frameCounter;
};

class CAnimFlicker :
	public CAnim
{
public:
	CAnimFlicker(WORD nOnFrames, WORD nOffFrames);
	~CAnimFlicker(void);
	void Advance(void);
private:
	void Command(unsigned char message[], DWORD messageLength);
	WORD m_nOnFrames;
	WORD m_nOffFrames;
	WORD m_frameCounter;
};

class CAnimExternalPositionControl :
	public CLoadedAnim
{
public:
	CAnimExternalPositionControl();
	bool Init(LPCTSTR memMapName);
	~CAnimExternalPositionControl(void);
	void Advance(void);
private:
	void Command(unsigned char message[], DWORD messageLength);
	HANDLE m_hMemMap;
	float* m_pMemMap;
	float m_xOffset;
	float m_yOffset;
};

class CAnimIntegerRange :
	public CAnim
{
public:
	CAnimIntegerRange(UINT32 startValue, UINT32 endValue, INT16 increment);
	~CAnimIntegerRange(void);
	void Advance(void);
private:
	void Command(unsigned char message[], DWORD messageLength);
	UINT32 m_startValue;
	UINT32 m_endValue;
	UINT32 m_currentValue;
	INT16  m_increment;
};

