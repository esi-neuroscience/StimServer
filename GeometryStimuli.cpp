#include "stdafx.h"
#include "GeometryStimuli.h"
#include "OutputWnd.h"
#define _USE_MATH_DEFINES TRUE
#include <math.h>

extern D2D1_SIZE_F g_ScreenSize;

CGeometryStimulus::CGeometryStimulus(void)
{
	TRACE("CGeometryStimulus::Konstruktor\n");
	m_reconstructFlag = false;
	InitializeCriticalSection(&m_CriticalSection);

}

void CGeometryStimulus::Init(void)
{
	TRACE("CGeometryStimulus::Init\n");

	EnterCriticalSection(&m_CriticalSection);
	Construct();
	LeaveCriticalSection(&m_CriticalSection);
}

CGeometryStimulus::~CGeometryStimulus(void)
{
	m_pPathGeometry->Release();
	DeleteCriticalSection(&m_CriticalSection);
}

void CGeometryStimulus::Reconstruct(void)
{
	EnterCriticalSection(&m_CriticalSection);
	m_pPathGeometry->Release();
	Construct();
	LeaveCriticalSection(&m_CriticalSection);
}

void CGeometryStimulus::Draw(void)
{
	CD2DStimulus::Draw();
	//	if (!theApp.m_drawMode) theApp.BeginDraw();
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


CPetal::CPetal(void)
{
	m_typeName = _T("petal");
	m_petalParams.m_r = 25.0f;
	m_petalParams.m_R = 100.0f;
	m_petalParams.m_d = 250.0f;
	m_petalParams.m_q = 0.3819660113f;	// (golden ratio)
	Init();		// initial Construct
}

CPetal::~CPetal(void)
{
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
	phi = acosf(m_petalParams.m_r / bs.point1.x);
	//	TRACE("phi: %f\n", phi*180.0f/3.141592654f);
	startPoint.x = m_petalParams.m_r*cosf(phi);
	startPoint.y = m_petalParams.m_r*sinf(phi);
	endPoint.x = startPoint.x;
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
	phi = acosf(m_petalParams.m_R / (m_petalParams.m_d - bs.point1.x));
	bs.point2 = D2D1::Point2F(
		m_petalParams.m_d - (m_petalParams.m_R*cosf(phi)),
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

void CPetal::Command(unsigned char message[], DWORD messageLength)
{
	//	TRACE("CPetal::Command\n");
	CString errString;
	PETAL_PARAMS* pDP = theApp.m_deferredMode ?
		&m_petalParamsCopy : &m_petalParams;
	bool isShapeCommand = ShapeCommand(message, messageLength);
	if (!isShapeCommand) switch (message[0])
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
			float radius = *(float*)&message[2];
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
				else m_reconstructFlag = true;
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
			float radius = *(float*)&message[2];
			float maxRadius = m_petalParams.m_d*(1.0f - m_petalParams.m_q);
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
				else m_reconstructFlag = true;
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
			float d = *(float*)&message[2];
			UINT16 mind = (UINT16)(1.0f + max(
				m_petalParams.m_r / m_petalParams.m_q,
				m_petalParams.m_R / (1.0f - m_petalParams.m_q)));
			if (!theApp.m_deferredMode && (d < (float)mind))
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
				else m_reconstructFlag = true;
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
			float q = *(float*)&message[2];
			float minq = m_petalParams.m_r / m_petalParams.m_d;
			float maxq = 1.0f - m_petalParams.m_R / m_petalParams.m_d;
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
				else m_reconstructFlag = true;
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
	default:
		InvalidCommand(message[0]);
		return;
	}
}

void CPetal::makeCopy(void)
{
	CD2DStimulus::makeCopy();
	m_petalParamsCopy = m_petalParams;
}

void CPetal::getCopy(void)
{
	CD2DStimulus::getCopy();
	if (m_reconstructFlag)
	{
		if ((m_petalParamsCopy.m_q*m_petalParamsCopy.m_d >= m_petalParamsCopy.m_r) &&
			((1.0f - m_petalParamsCopy.m_q)*m_petalParamsCopy.m_d >= m_petalParamsCopy.m_R))
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
		m_reconstructFlag = false;
	}
}


CWedge::CWedge(void)
{
	m_typeName = _T("wedge");
	m_wedgeParams.gamma = 9.0f;
	TRACE("CWedge Konstruktor\n");
	Init();		// initial Construct
}

CWedge::~CWedge(void)
{
}

void CWedge::Construct(void)
{
	TRACE("CWedge::Construct\n");
	HRESULT hr;
	hr = theApp.m_d2dFactory->CreatePathGeometry(&m_pPathGeometry);
	ASSERT(hr == S_OK);
	ID2D1GeometrySink *pSink = NULL;
	hr = m_pPathGeometry->Open(&pSink);
	ASSERT(hr == S_OK);
	D2D1_POINT_2F startPoint = { 0.0f, 0.0f };
	float h = sqrtf(g_ScreenSize.width*g_ScreenSize.width + g_ScreenSize.height*g_ScreenSize.height);
	float x = h * tanf((m_wedgeParams.gamma / 2.0f) * (float) (M_PI / 180.0f));
	D2D1_POINT_2F points[2] = { {h, -x}, {h,x} };
	pSink->BeginFigure(startPoint, D2D1_FIGURE_BEGIN_FILLED);
	pSink->AddLines(points, 2);
	pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
	hr = pSink->Close();
	ASSERT(hr == S_OK);
}

void CWedge::Command(unsigned char message[], DWORD messageLength)
{
	CString errString;
	WEDGE_PARAMS* pDP = theApp.m_deferredMode ?
		&m_wedgeParamsCopy : &m_wedgeParams;
	bool isShapeCommand = ShapeCommand(message, messageLength);
	if (!isShapeCommand) switch (message[0])
	{
	case 1:		// set parameter
		switch (message[1])
		{
		case 1:	// phi
		{
			if (!theApp.CheckCommandLength(messageLength, 6, _T("Set Wedge Angle")))
			{
				m_errorCode = 2;
				theApp.m_errorMask |= 2;
				return;
			}
			float phi = *(float*)&message[2];
			if (phi <= 0.0f)
			{
				errString.Format(_T("Non positive angle (%f) is invalid for wedge -- ignored."),
					phi);
				COutputList::AddString(errString);
			}
			else if (phi > 90.0f)
			{
				errString.Format(_T("Wedge angle must be less than 90 (requested was %f) -- ignored."),
					phi);
				COutputList::AddString(errString);
				//					m_r = maxRadius;
				//					Reconstruct();
			}
			else
			{
				//				m_wedgeParams.gamma = phi;
				pDP->gamma = phi;
				if (!theApp.m_deferredMode) Reconstruct();
				else m_reconstructFlag = true;
			}
		}
		break;
		default:
			m_errorCode = 3;
			theApp.m_errorMask |= 2;
			errString.Format(_T("Invalid wedge command. Trace: %u %u %u %u %u %u."),
				message[0], message[1], message[2], message[3], message[4], message[5]);
			COutputList::AddString(errString);
			return;
		}
		break;
	default:
		InvalidCommand(message[0]);
		return;
	}
}

void CWedge::makeCopy(void)
{
	CD2DStimulus::makeCopy();
	m_wedgeParamsCopy = m_wedgeParams;
}

void CWedge::getCopy(void)
{
	CD2DStimulus::getCopy();

	if (m_reconstructFlag)
	{
		Reconstruct();
		m_reconstructFlag = false;
	}
}

bool CWedge::SetAnimParam(BYTE mode, float value)
{
	if (mode != 1)
	{
		CString errString;
		errString.Format(_T("Mode %u is invalid for Linear Range Animation of Wedge stimulus."), mode);
		COutputList::AddString(errString);
		return false;
	}
	// change wedge orientation
	SetOrientation(value);
	return true;
}
