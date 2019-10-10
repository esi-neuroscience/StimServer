#include "StdAfx.h"
#include "Anim.h"
#include "OutputWnd.h"
#include "Stimulus.h"
#define _USE_MATH_DEFINES true
#include <math.h>
#include "NumericBinaryFile.h"
#include "StimServerDoc.h"

BYTE CAnim::m_defaultFinalActionMask = 0;
HANDLE CAnim::m_hfinalEvent = NULL;

extern float g_frameRate;
extern CStimServerDoc* g_pDoc;

CAnim::CAnim(void)
	: m_pStimulus(NULL)
	, m_errorCode(0)
{
	m_finalAction.mask = m_defaultFinalActionMask;
	TRACE("Animation Constructor Final Action: %u\n", m_finalAction.mask);
}


CAnim::~CAnim(void)
{
	Deassign();
	TRACE("Animation Destructor\n");
}


void CAnim::Deassign(void)
{
	if (m_pStimulus)
	{
		TRACE("Animation Deassignment\n");
		m_pStimulus->m_supressed = false;
		m_pStimulus->m_animation = NULL;
		m_pStimulus = NULL;
	}
}


void CAnim::Finalize(void)
{
	TRACE("Animation Finalize\n");
	if (m_finalAction.disable)
	{
		m_pStimulus->m_enabled = false;
		m_pStimulus->m_enabledCopy = false;
	}
	if (m_finalAction.togglePD) CPhotoDiode::CSR.lit ^= 1;
	TRACE("Final Action: %u, Restart: %u\n", m_finalAction.mask, m_finalAction.restart);
	if (!m_finalAction.restart) Deassign();
	if (m_finalAction.endDeferredMode && theApp.m_deferredMode) g_pDoc->EndDeferredMode();
	if (m_finalAction.signalEvent) SetEvent(m_hfinalEvent);
}



CLoadedAnim::CLoadedAnim(void)
{
}


CLoadedAnim::~CLoadedAnim(void)
{
}



CAnimationPath::CAnimationPath(void)
	: m_pPathCoords(NULL)
	, m_nPathCoords(0)
	, m_index(0)
{
}


CAnimationPath::~CAnimationPath(void)
{
	delete m_pPathCoords;
}


bool CAnimationPath::Init(LPCWSTR wzFilename)
{
	TRACE("Animation Init: %S\n", wzFilename);

	CNumericBinaryFile animFile(wzFilename);
	if (!animFile.valid()) return false;
	animFile.Init();
	UINT nEntries = (UINT) animFile.m_dims[1];

	m_pPathCoords = new D2D1_POINT_2F[nEntries];
	if (!animFile.ReadPoints2f((float*) m_pPathCoords)) return false;
	m_nPathCoords = nEntries;
	TRACE("Motion path created\n");
	return true;
}


void CAnimationPath::Command(unsigned char message[], DWORD messageLength)
{
	TRACE("message[0]: %u, length: %u\n", message[0], messageLength);
	switch (message[0])
	{
	case 0:
		switch (messageLength)
		{
		case 4:	// assign / unassign
			break;
		}
	break;
	}
}


void CAnimationPath::Advance(void)
{
	m_pStimulus->Moveto(false, m_pPathCoords[m_index].x, m_pPathCoords[m_index].y);
	if (theApp.m_deferredMode) m_pStimulus->Moveto(true, m_pPathCoords[m_index].x, m_pPathCoords[m_index].y);
	if (++m_index == m_nPathCoords)
	{	// final action
		m_index = 0;
		CAnim::Finalize();
	}
}



CAnimLineSegPath::CAnimLineSegPath(WORD speed)
	: m_pPathVertices(NULL)
	, m_nPathVertices(0)
	, m_index(0)
	, m_iSegment(0)
	, m_vx(0)
	, m_vy(0)
	, m_x0(0)
	, m_y0(0)
	, m_nThisSegment(0)
{
	m_speed = speed;
	TRACE("Speed: %u\n", m_speed);
}


CAnimLineSegPath::~CAnimLineSegPath(void)
{
	if (m_pPathVertices) delete m_pPathVertices;
}


void CAnimLineSegPath::Command(unsigned char message[], DWORD messageLength)
{
	TRACE("message[0]: %u, length: %u\n", message[0], messageLength);
	switch (message[0])
	{
	case 11:
		{
			WORD nCurrent = m_nPathVertices;
			m_nPathVertices = ((WORD) messageLength-1) / 4;
			ASSERT(m_nPathVertices * 4 == (messageLength-1));
			if (m_pPathVertices && (m_nPathVertices != nCurrent)) delete m_pPathVertices;
			if (m_nPathVertices != nCurrent) m_pPathVertices = new short[m_nPathVertices*2];
			short* pMess = (short*) &message[1];
			short* pDest = m_pPathVertices;
			for (BYTE i=0; i < m_nPathVertices; i++)
			{
				*pDest++ = *pMess++;	// x
				*pDest++ = *pMess++;	// y
//				TRACE("%i %i\n", m_pPathVertices[2*i], m_pPathVertices[2*i+1]);
			}
			m_index = 0;
			m_iSegment = 0;
		}
		break;
	default:
		ASSERT(false);
	}
}


void CAnimLineSegPath::Advance(void)
{
	if ((!m_pStimulus) || (!m_nPathVertices)) return;
	if (m_index == 0)
	{
		m_x0 = m_pPathVertices[2*m_iSegment];
		m_y0 = m_pPathVertices[2*m_iSegment+1];
//		TRACE("x0: %i, y0: %i\n", m_x0, m_y0); 
		float dx = (float) (m_pPathVertices[2*(m_iSegment+1)] - m_x0);
		float dy = (float) (m_pPathVertices[2*(m_iSegment+1)+1] - m_y0);
		float segmentLength = sqrtf(dx*dx + dy*dy);
//		TRACE("dx: %f, dy: %f, Length: %f\n", dx, dy, segmentLength); 
		m_nThisSegment = (long) (g_frameRate * segmentLength / (float) m_speed);
		if (m_iSegment == m_nPathVertices-2) m_nThisSegment++;	// last segment includes end point
		if (m_nThisSegment == 0)
		{
			if (++m_iSegment == m_nPathVertices-1) Finalize();
			return;
		}
		m_vx = dx / (float) (m_nThisSegment - 1);
		m_vy = dy / (float) (m_nThisSegment - 1);
	}
//	TRACE("%f %f\n", (float) m_x0 + (float) m_index * m_vx, 
//		(float) m_y0 + (float) m_index * m_vy);
	m_pStimulus->Moveto(false,
		(float) m_x0 + (float) m_index * m_vx, 
		(float) m_y0 + (float) m_index * m_vy);
	if (theApp.m_deferredMode) m_pStimulus->Moveto(true,
		(float) m_x0 + (float) m_index * m_vx, 
		(float) m_y0 + (float) m_index * m_vy);
	if (++m_index == m_nThisSegment)
	{
		m_index = 0;
		if (++m_iSegment == m_nPathVertices-1) Finalize();
	}
}


void CAnimLineSegPath::Finalize(void)
{
	m_index = 0;
	m_iSegment = 0;
	CAnim::Finalize();
}

///////////////////////////////////////////////////////////////////////////////////////////

bool CAnim::CreateEvent(void)
{
	m_hfinalEvent = ::CreateEvent(NULL, FALSE, FALSE, _T("StimServerAnimationDone"));
	ASSERT(m_hfinalEvent != NULL);
	return m_hfinalEvent != NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////

CAnimHarmonic::CAnimHarmonic(WORD nQuaters)
	: m_A(0)
	, m_phi(0)
	, m_phiIncr(0)
	, m_direction(0)
{
	m_nQuaters = nQuaters;
}


CAnimHarmonic::~CAnimHarmonic(void)
{
}

void CAnimHarmonic::Command(unsigned char message[], DWORD messageLength)
{
	TRACE("message[0]: %u, length: %u\n", message[0], messageLength);
	switch (message[0])
	{
	case 1:
		switch (message[1])
		{
		case 1:
			m_A = *(float*) &message[2];
			break;
		default:
			ASSERT(false);
		}
		break;
	case 2:
		m_phiIncr = *(float*) &message[1];
		break;
	case 4:
		m_direction = (*(float*) &message[1])/180.0f * (float) M_PI;
		break;
	case 6:
		m_phi = (*(float*) &message[1])/180.0f * (float) M_PI;
		break;
	default:
		ASSERT(false);
	}
}


void CAnimHarmonic::Advance(void)
{
	float r = m_A*sinf(m_phi);
	m_phi += m_phiIncr;
	float x = r*cosf(m_direction);
	float y = r*sinf(m_direction);
	m_pStimulus->Moveto(false, x, y);
	if (theApp.m_deferredMode) m_pStimulus->Moveto(true, x, y);
}

//*****************************************************************************************
CAnimLinearRange::CAnimLinearRange(
	float startValue,
	float endValue,
	float duration,
	BYTE mode)
{
	TRACE("Start: %f, End: %f, Dauer: %f, Modus: %u\n", startValue, endValue, duration, mode);
	m_startValue = startValue;
	m_endValue = endValue;
	m_currentValue = m_startValue;
	m_increment = (m_endValue-m_startValue)/g_frameRate/duration;
	TRACE("Start: %f, End: %f, Current: %f,Increment: %f\n", m_startValue, m_endValue, m_currentValue, m_increment);
	m_mode = mode;
}


CAnimLinearRange::~CAnimLinearRange(void)
{
}


void CAnimLinearRange::Command(unsigned char message[], DWORD messageLength)
{
}


void CAnimLinearRange::Advance(void)
{
	m_currentValue += m_increment;
	bool inRange = m_increment > 0 ?
		m_currentValue <= m_endValue : m_currentValue >= m_endValue;
	if (inRange)
	{
		if (!m_pStimulus->SetAnimParam(m_mode, m_currentValue))
		{
			Deassign();
			return;
		}
	}
	else
	{
		m_currentValue = m_startValue;
		CAnim::Finalize();
	}
}

//*****************************************************************************************

CAnimFlash::CAnimFlash(WORD nFrames)
{
	m_nFrames = nFrames;
	m_frameCounter = nFrames;
}


CAnimFlash::~CAnimFlash(void)
{
}


void CAnimFlash::Command(unsigned char message[], DWORD messageLength)
{
	TRACE("message[0]: %u, length: %u\n", message[0], messageLength);
	switch (message[0])
	{
	case 2:
		CHECK_COMMAND_LENGTH(3, _T("Set Flash Duration"));
		m_nFrames = *(WORD*) &message[1];
		if (m_pStimulus == NULL) m_frameCounter = m_nFrames;
		break;
	default:
		ASSERT(false);
	}
}


void CAnimFlash::Advance(void)
{
	if (--m_frameCounter == 0)
	{
		CAnim::Finalize();
		m_frameCounter = m_nFrames;
	}
}

//*****************************************************************************************

CAnimFlicker::CAnimFlicker(WORD nOnFrames, WORD nOffFrames)
{
	m_nOnFrames = nOnFrames;
	m_nOffFrames = nOffFrames;
	m_frameCounter = nOnFrames;
}


CAnimFlicker::~CAnimFlicker(void)
{
}


void CAnimFlicker::Command(unsigned char message[], DWORD messageLength)
{
	TRACE("message[0]: %u, length: %u\n", message[0], messageLength);
	switch (message[0])
	{
	case 2:
		CHECK_COMMAND_LENGTH(5, _T("Set Flicker Duration"));
		m_nOnFrames = *(WORD*) &message[1];
		m_nOffFrames = *(WORD*) &message[3];
		if (m_pStimulus == NULL) m_frameCounter = m_nOnFrames;
		break;
	default:
		ASSERT(false);
	}
}


void CAnimFlicker::Advance(void)
{
	if (--m_frameCounter == 0)
	{
		if (m_pStimulus->m_supressed)
		{
			m_frameCounter = m_nOnFrames;
		}
		else
		{
			m_frameCounter = m_nOffFrames;
		}
		m_pStimulus->m_supressed = !m_pStimulus->m_supressed;
	}
}

//*****************************************************************************************

CAnimExternalPositionControl::CAnimExternalPositionControl()
{
	m_hMemMap = NULL;
	m_pMemMap = NULL;
	m_xOffset = 0.0f;
	m_yOffset = 0.0f;
}


bool CAnimExternalPositionControl::Init(LPCTSTR memMapName)
{
	m_hMemMap = OpenFileMapping(FILE_MAP_READ, FALSE, memMapName);
	if (m_hMemMap == NULL)
	{
		CString errMsg;
		errMsg.Format(_T("Could not create External Position Control animation."));
		COutputList::AddString(errMsg);
		DWORD error = GetLastError();
		switch (error)
		{
		case ERROR_FILE_NOT_FOUND:
			errMsg.Format(_T(" Shared memory named '%s' not found."), memMapName);
			break;
		default:
			errMsg.Format(_T(" Open Error: %u (shared memory name: '%s')."), error, memMapName);
		}
		COutputList::AddString(errMsg);
//		TRACE("Error: %u\n", error);
		return false;
	}
	m_pMemMap = (float*) MapViewOfFile(m_hMemMap, FILE_MAP_READ, 0, 0, 0);
	if (m_pMemMap == NULL)
	{
		CString errMsg;
		errMsg.Format(_T("Could not create External Position Control animation."));
		COutputList::AddString(errMsg);
		DWORD error = GetLastError();
		errMsg.Format(_T(" Map Error: %u (shared memory name: '%s')."), error, memMapName);
		COutputList::AddString(errMsg);
		CloseHandle(m_hMemMap);
		return false;
	}
	return true;
}


CAnimExternalPositionControl::~CAnimExternalPositionControl(void)
{
	if (m_pMemMap)
	{
		UnmapViewOfFile(m_pMemMap);
		m_pMemMap = NULL;
	}
	if (m_hMemMap)
	{
		CloseHandle(m_hMemMap);
		m_hMemMap = NULL;
	}
}


void CAnimExternalPositionControl::Command(unsigned char message[], DWORD messageLength)
{
	TRACE("message[0]: %u, length: %u\n", message[0], messageLength);
	switch (message[0])
	{
	case 3:
		CHECK_COMMAND_LENGTH(9, _T("Set Position Offset"));
		m_xOffset = *(float*) &message[1];
		m_yOffset = *(float*) &message[5];
		break;
	default:
		CString errMsg;
		errMsg.Format(_T("Invalid command (code=%u) for External Position Control animation."), message[0]);
		COutputList::AddString(errMsg);
	}
}


void CAnimExternalPositionControl::Advance(void)
{
	m_pStimulus->Moveto( false, m_pMemMap[0] + m_xOffset, m_pMemMap[1] + m_yOffset);
//	TRACE("Pos: %f, %f\n", m_pMemMap[0], m_pMemMap[1]);
}


//*****************************************************************************************
CAnimIntegerRange::CAnimIntegerRange(
	UINT32 startValue,
	UINT32 endValue,
	INT16  increment)
{
	TRACE("Start: %u, End: %u, Increment: %i\n", startValue, endValue, increment);
	m_startValue = startValue;
	m_endValue = endValue;
	m_currentValue = m_startValue;
	m_increment = increment;
//	m_mode = mode;
}


CAnimIntegerRange::~CAnimIntegerRange(void)
{
}


void CAnimIntegerRange::Command(unsigned char message[], DWORD messageLength)
{
}


void CAnimIntegerRange::Advance(void)
{
	m_currentValue += m_increment;
	bool inRange = m_increment > 0 ?
		m_currentValue <= m_endValue : m_currentValue >= m_endValue;
	if (!inRange)
	{
		m_currentValue = m_startValue;
		CAnim::Finalize();
	}
}
