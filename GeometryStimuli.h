#pragma once

#include "Stimulus.h"

class CGeometryStimulus :
	public CD2DStimulus
{
protected:
	CGeometryStimulus(void);
	~CGeometryStimulus(void);
	void Init(void);
	virtual void Construct(void) = 0;
	void Reconstruct(void);
	void Draw(void);

	bool m_reconstructFlag;
	CRITICAL_SECTION m_CriticalSection;
	ID2D1PathGeometry1* m_pPathGeometry;
};

class CPetal :
	public CGeometryStimulus
{
public:
	CPetal(void);
	~CPetal(void);
	void makeCopy(void);
	void getCopy(void);
	void Command(unsigned char message[], DWORD messageLength);
private:
	void Construct(void);

	// The following parameters change the shape of the petal.
	// If they are modified, the petal has to be reconstructed.
	struct PETAL_PARAMS
	{
		float m_r;
		float m_R;
		float m_d;
		float m_q;
	} m_petalParams, m_petalParamsCopy;
};


class CWedge :
	public CGeometryStimulus
{
public:
	CWedge(void);
	~CWedge(void);
	void makeCopy(void);
	void getCopy(void);
	void Command(unsigned char message[], DWORD messageLength);
	bool SetAnimParam(BYTE mode, float value);
	void Construct(void);
private:

	// The following parameters change the shape of the wedge.
	// If they are modified, the wedge has to be reconstructed.
	struct WEDGE_PARAMS
	{
		float gamma;
	} m_wedgeParams, m_wedgeParamsCopy;
};
