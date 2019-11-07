
// StimServerDoc.cpp : implementation of the CStimServerDoc class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "StimServer.h"
#endif

#include "StimServerDoc.h"
#include "PipeProcedure.h"

#include <propkey.h>

//#include <atlbase.h>
//#include <atlconv.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern float g_frameRate;
extern CRITICAL_SECTION g_criticalMapSection;
extern CRITICAL_SECTION g_criticalDrawSection;

// CStimServerDoc

IMPLEMENT_DYNCREATE(CStimServerDoc, CDocument)

BEGIN_MESSAGE_MAP(CStimServerDoc, CDocument)
	ON_COMMAND(ID_APPS_BEAMERTEST, &CStimServerDoc::OnAppsBeamertest)
END_MESSAGE_MAP()

bool CPhotoDiode::m_enabled = true;

// CStimServerDoc construction/destruction

CStimServerDoc::CStimServerDoc()
//	: m_deferredMode(false)
	: m_photoDiodeEnabledCopy(false)
	, m_deferredUpdate(false)
{
	// TODO: add one-time construction code here
	m_backgroundColor = D2D1::ColorF(.5f, .5f, .5f, 1.f);
	// creste a manual reset event in signaled state
	m_hDeferredUpdateDone = CreateEvent(NULL, true, true, NULL);
	ASSERT(m_hDeferredUpdateDone != NULL);
}

CStimServerDoc::~CStimServerDoc()
{
	CloseHandle(m_hDeferredUpdateDone);
	TRACE("Destroy Document\n");
}


BOOL CStimServerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)
	return TRUE;
}




// CStimServerDoc serialization

void CStimServerDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CStimServerDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CStimServerDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data. 
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CStimServerDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CStimServerDoc diagnostics

#ifdef _DEBUG
void CStimServerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CStimServerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CStimServerDoc commands

//bool CStimServerDoc::Draw()
void CStimServerDoc::Draw()
{
//	POSITION pos;
	map <WORD, CStimulus*> :: const_iterator cIter;
//	WORD key;
	CStimulus* pStimulus;

//	if (m_pPhotoDiode == NULL) return false;
	if (m_deferredUpdate) {
		TRACE("Deferred Update\n");
		CPhotoDiode::m_enabled = m_photoDiodeEnabledCopy;
		m_backgroundColor = m_backgroundColorCopy;
		CPhotoDiode::CSR = m_PDCSRcopy;
		EnterCriticalSection(&g_criticalMapSection);
		for (cIter = m_stimuli.cbegin(); cIter != m_stimuli.cend(); cIter++)
			cIter->second->getCopy();
		LeaveCriticalSection(&g_criticalMapSection);
		m_deferredUpdate = false;
		theApp.m_deferredMode = false;
		VERIFY(SetEvent(m_hDeferredUpdateDone));
	}
	EnterCriticalSection(&g_criticalDrawSection);
	theApp.BeginDraw();
	theApp.m_pContext->Clear(m_backgroundColor);
	EnterCriticalSection(&g_criticalMapSection);
	for (cIter = m_stimuli.cbegin(); cIter != m_stimuli.cend(); cIter++)
	{
		pStimulus = cIter->second;
		if (pStimulus->m_enabled)
		{
			if (pStimulus->m_animation != NULL) pStimulus->m_animation->Advance();
			if (!pStimulus->m_supressed) pStimulus->Draw();
		}
	}
	LeaveCriticalSection(&g_criticalMapSection);
	if (CPhotoDiode::m_enabled) {
//		if (m_pPhotoDiode == NULL) return false;
		if (!theApp.m_drawMode) theApp.BeginDraw();
//		m_pPhotoDiode->Draw();
		CPhotoDiode::Draw();
	}
	if (theApp.m_drawMode) theApp.EndDraw();
	LeaveCriticalSection(&g_criticalDrawSection);
//	return true;
}

short CStimServerDoc::Command(WORD key, unsigned char message[], DWORD messageLength)
{
//	POSITION pos;
	map <WORD, CStimulus*> :: const_iterator cIter;
//	WORD stimKey;
	CStimulus* pStimulus;
	if (key == 0) {
		DWORD status;
		TRACE("Code: %u\n", message[0]);
		switch (message[0]) {
		case 0:		// [00 0 ...
			switch (messageLength) {
			case 1:		// [00 0] delete all stimuli
				RemoveAllStimuli();
				break;
			case 2: {	// [00 0 e] enable/disable photo diode signal
				bool* pPDe = theApp.m_deferredMode ? &m_photoDiodeEnabledCopy : &CPhotoDiode::m_enabled;
				*pPDe = message[1] != 0;
				}
				break;
			case 3:		// [00 0 * x]
				EnterCriticalSection(&g_criticalMapSection);
				for (cIter = m_stimuli.cbegin(); cIter != m_stimuli.cend(); cIter++)
				{
					pStimulus = cIter->second;
					switch (message[1])
					{
					case 0:	// [00 0 0 e] enable/disable all stimuli
						if (pStimulus->m_protected) break;
//						bool* pEnabled = theApp.m_deferredMode ? &pStimulus->m_enabledCopy : &pStimulus->m_enabled;
//						*pEnabled = message[2] != 0;
						*(theApp.m_deferredMode ? &pStimulus->m_enabledCopy : &pStimulus->m_enabled) =
							message[2] != 0;
						break;
					case 1: // [00 0 1 p] protect/unprotect all stimuli
						pStimulus->m_protected = message[2] != 0;
						break;
					default:
						ASSERT(false);
					}
				}
				LeaveCriticalSection(&g_criticalMapSection);
				break;
			case 4: {	// [00 0 r g b] set screen background
				D2D1_COLOR_F* pbc = theApp.m_deferredMode ? &m_backgroundColorCopy : & m_backgroundColor;
				pbc->r = message[1]/255.f;
				pbc->g = message[2]/255.f;
				pbc->b = message[3]/255.f;
				TRACE("Change background color: %f %f %f\n", pbc->r, pbc->g, pbc->b);
				}
			}
			break;
		case 1:		// [00 1 ...
			switch (message[1])
			{
			case 0:	// [00 1 0] end deferred mode
			case 1:	// [00 1 1] start deferred mode
				status = WaitForSingleObject(m_hDeferredUpdateDone, INFINITE);
				ASSERT(status == WAIT_OBJECT_0);
				if (message[1] != 0) {	// start deferred mode
					theApp.m_deferredMode = true;
					m_photoDiodeEnabledCopy = CPhotoDiode::m_enabled;
					m_backgroundColorCopy = m_backgroundColor;
					m_PDCSRcopy = CPhotoDiode::CSR;
					for (cIter = m_stimuli.cbegin(); cIter != m_stimuli.cend(); cIter++)
					{
						cIter->second->makeCopy();
					}
				} else {			// end deferred mode
					m_deferredUpdate = true;
					VERIFY(ResetEvent(m_hDeferredUpdateDone));
				}
				break;
			case 2:	// [00 1 2] query performance counter
				LARGE_INTEGER performanceCount;
				VERIFY(QueryPerformanceCounter(&performanceCount));
				WritePipe(&performanceCount, sizeof(performanceCount));
				return -1;
				break;
			case 3:	// [00 1 3 a] set default final action for animations
			{
				BYTE temp = CAnim::m_defaultFinalActionMask;
				CAnim::m_defaultFinalActionMask = message[2];
				TRACE("Set Default from %u to %u\n", temp, CAnim::m_defaultFinalActionMask);
				return -1;
			}
			break;
			case 4: // [00 1 4] query and reset global error mask
				short errorMask;
				errorMask = theApp.m_errorMask;
				theApp.m_errorMask = 0;
				return errorMask;
				break;
			case 5: // [00 1 5 r g b a] set default draw color
				theApp.m_defaultDrawColor = D2D1::ColorF(
					message[2]/255.f, message[3]/255.f, message[4]/255.f, message[5]/255.f);
				break;
			case 6:	// [00 1 6] query performance frequency
				LARGE_INTEGER performanceFrequency;
				VERIFY(QueryPerformanceFrequency(&performanceFrequency));
				WritePipe(&performanceFrequency, sizeof(performanceFrequency));
				return -1;
				break;
			case 7: // [00 1 7] query and reset global server error code
				short errorCode;
				errorCode = theApp.m_errorCode;
				theApp.m_errorCode = 0;
				return errorCode;
				break;
			case 8:	// [00 1 8] query physical frame rate
				WritePipe(&g_frameRate, sizeof(g_frameRate));
				return -1;
				break;
			case 9: // [00 1 9 r g b a] set default outline color
				theApp.m_defaultOutlineColor = D2D1::ColorF(
					message[2]/255.f, message[3]/255.f, message[4]/255.f, message[5]/255.f);
				break;
			}
			break;
			/*
		case 2: {
			// load bitmap file
			CStimulusPic* pStim = new CStimulusPic();
			return AddStimulusObject(pStim, &message[1]);
			}
			break;
			*/
		case 2:
		case 3: {
			// load or replace bitmap file
			CStimulusPic* pStim = new CStimulusPic();
			return message[0] == 2  ? AddStimulusObject(pStim, &message[1])
									: ReplaceStimulusObject(pStim, &message[1]);
			}
			break;
			/*
		case 4: {
			// load Pixel Shader
			CStimulusPS* pStim = new CStimulusPS();
			return AddStimulusObject(pStim, &message[1]);
			}
			break;
			*/
		case 4:
		case 5: 
			{
			// load or replace Pixel Shader
			CStimulusPS* pStim = new CStimulusPS();
//			return ReplaceStimulusObject(pStim, &message[1]);
			return message[0] == 4  ? AddStimulusObject(pStim, &message[1])
									: ReplaceStimulusObject(pStim, &message[1]);
			}
			break;
		case 6: {
			// load movie file
			CStimulusMov* pStim = new CStimulusMov();
			return AddStimulusObject(pStim, &message[1]);
			}
			break;
			/*
		case 7: {
			// load or replace movie file
			CStimulusMov* pStim = new CStimulusMov();
			return ReplaceStimulusObject(pStim, &message[1]);
			}
			break;
			*/
		case 8: {
			// load particle file
			TRACE("Load Particle\n");
			unsigned short width = *((unsigned short*) &message[1]);
			unsigned short height = *((unsigned short*) &message[3]);
			if (width*height == 0)
			{
				CString ErrorString = _T("Width and height of particle stimuli must be non zero.");
				COutputList::AddString(ErrorString);
				theApp.m_errorCode = 6;
				theApp.m_errorMask |= 1;
				return 0;
			}
			CStimulusPart* pStim = new CStimulusPart((unsigned short *) &message[1]);
			return AddStimulusObject(pStim, &message[5]);
			}
			break;
		case 9: {
			// load or replace particle file
			unsigned short width = *((unsigned short*) &message[1]);
			unsigned short height = *((unsigned short*) &message[3]);
			if (width*height == 0)
			{
				CString ErrorString = _T("Width and height of particle stimuli must be non zero.");
				COutputList::AddString(ErrorString);
				theApp.m_errorCode = 6;
				theApp.m_errorMask |= 1;
				return 0;
			}
			CStimulusPart* pStim = new CStimulusPart((unsigned short *) &message[1]);
			return ReplaceStimulusObject(pStim, &message[5]);
			}
			break;
		case 10: {
			// create pixel stimulus
			TRACE("Pixel\n");
			CStimulusPixel* pStim = new CStimulusPixel();
			return AddStimulusObject(pStim);
			}
			break;
		case 11: {
			// create or replace pixel stimulus
			CStimulusPixel* pStim = new CStimulusPixel();
			return ReplaceStimulusObject(pStim, (WORD*) &message[1]);
			}
			break;
		case 12:
			// create symbol stimulus
			{
			TRACE("Symbol\n");
			unsigned short size = *((unsigned short*) &message[2]);
			if (size == 0)
			{
				CString ErrorString = _T("Symbol of size 0 not created.");
				COutputList::AddString(ErrorString);
				theApp.m_errorCode = 5;
				theApp.m_errorMask |= 1;
				return 0;
			}
			CStimulusSymbol* pStim = new CStimulusSymbol(message[1], *((unsigned short*) &message[2]));
			return AddStimulusObject(pStim);
			}
			break;
		case 13:
			// create or replace symbol stimulus
			{
			unsigned short size = *((unsigned short*) &message[2]);
			if (size == 0)
			{
				CString ErrorString = _T("Symbol of size 0 not created.");
				COutputList::AddString(ErrorString);
				theApp.m_errorCode = 5;
				theApp.m_errorMask |= 1;
				return 0;
			}
			CStimulusSymbol* pStim = new CStimulusSymbol(message[1], *((unsigned short*) &message[2]));
			TRACE(_T("Key of symbol to replace: %u\n"), *(WORD*) &message[4]);
			WORD key = ReplaceStimulusObject(pStim, (WORD*) &message[4]);
			TRACE(_T("Key returned from replace: %u\n"), key);
//			return ReplaceStimulusObject(pStim, (WORD*) &message[4]);
			return key;
			}
			break;
		case 14: {
			// load bitmap brush file
			CStimBmpBrush* pStim = new CStimBmpBrush();
			return AddStimulusObject(pStim, &message[1]);
			}
			break;
		case 15:
			// load or replace bitmap brush file
			CStimBmpBrush* pStim;
			pStim = new CStimBmpBrush();
			return ReplaceStimulusObject(pStim, &message[1]);
			break;
		case 16:
			switch(messageLength)
			{
			case 2:
				PDCSR* pPDCSR;
				pPDCSR = theApp.m_deferredMode ? &m_PDCSRcopy : &CPhotoDiode::CSR;
				pPDCSR->flicker = message[1] == 3;
				if (message[1] == 2)
					pPDCSR->lit ^= 1;
				else
					pPDCSR->lit = message[1] & 1;
				break;
			case 3:
				ASSERT(message[1] == 3);
				CPhotoDiode::SetPosition(message[2] & 1);
				break;
			}
			break;
		case 18:
			{
				// load pixel shader file for picture object
				CStimPSpic* pStim = new CStimPSpic();
				WORD sKey = *((WORD*) &message[1]);
				cIter = m_stimuli.find(sKey);
				if (cIter == m_stimuli.cend())
				{
					CString ErrorString;
					ErrorString.Format(_T("Trying to assign non existing picture object (key=%u) to pixel shader."), sKey);
					COutputList::AddString(ErrorString);
					delete pStim;
					theApp.m_errorCode = 3;
					theApp.m_errorMask |= 1;
					//					return -1;
					return 0;
				}
				pStim->assignPic(cIter->second);
				return AddStimulusObject(pStim, &message[3]);
			}
			break;
		case 20:	// rectangle
			{
				CStimulusRect* pStim = new CStimulusRect();
				return AddStimulusObject(pStim);
			}
			break;
		case 22:	// particle 
			{
				// load Vertex Shader for particle stimulus
				unsigned short width = *((unsigned short*) &message[1]);
				unsigned short height = *((unsigned short*) &message[3]);
				if (width*height == 0)
				{
					CString ErrorString = _T("Width and height of particle stimuli must be non zero.");
					COutputList::AddString(ErrorString);
					theApp.m_errorCode = 6;
					theApp.m_errorMask |= 1;
					return 0;
				}
				CStimulusParticle* pStim = new CStimulusParticle((unsigned short *) &message[1]);
				return AddStimulusObject(pStim, &message[5]);
			}
			break;
		case 24:
		case 25: {
			// load or replace Motion Picture file
			CStimulusPics* pStim = new CStimulusPics();
			return message[0] == 24  ? AddStimulusObject(pStim, &message[1])
									 : ReplaceStimulusObject(pStim, &message[1]);
			}
			break;
		case 26:	// Petal
		case 27: {
			CPetal* pStim = new CPetal();
			return message[0] == 26 ? AddStimulusObject(pStim)
				: ReplaceStimulusObject(pStim, (WORD*)&message[1]);
		}
		break;
		case 28:	// Ellipse
		case 29:
		{
			CEllipse* pStim = new CEllipse();
			return message[0] == 28 ? AddStimulusObject(pStim)
				: ReplaceStimulusObject(pStim, (WORD*)&message[1]);
		}
		break;
		case 30:	// Wedge
		case 31:
		{
			CWedge* pStim = new CWedge();
			return message[0] == 30 ? AddStimulusObject(pStim)
				: ReplaceStimulusObject(pStim, (WORD*)&message[1]);
		}
		break;
		case 130:
			{
				// load animation path
				TRACE("Load animation path\n");
				CAnimationPath* pAnim = new CAnimationPath();
				return AddAnimationObject(pAnim, &message[1]);
			}
			break;
		case 132:
			{
				// create straight line segment animation path
				TRACE("Straight line segment animation path\n");
				CAnimLineSegPath* pAnim = new CAnimLineSegPath(*((WORD*) &message[1]));
				return AddAnimationObject(pAnim);
			}
			break;
		case 134:
			{
				// create harmonic oscillation motion path
				TRACE("Harmonic oscillation motion path\n");
				CAnimHarmonic* pAnim = new CAnimHarmonic(*((WORD*) &message[1]));
				return AddAnimationObject(pAnim);
			}
			break;
		case 136:
			{
				// create linear range animation
				TRACE("Linear range animation\n");
				CAnimLinearRange* pAnim = new CAnimLinearRange(
					*((float*) &message[1]),
					*((float*) &message[5]),
					*((float*) &message[9]),
					*((BYTE*)  &message[13]));
				return AddAnimationObject(pAnim);
			}
			break;
		case 138:
			switch(messageLength)
			{
			case 3:
				// create flash animation
				{
				TRACE("Flash animation\n");
				CAnimFlash* pAnim = new CAnimFlash(
					*((WORD*) &message[1]));
				return AddAnimationObject(pAnim);
				}
			case 5:
				// create flicker animation
				{
				TRACE("Flicker animation\n");
				CAnimFlicker* pAnim = new CAnimFlicker(
					*((WORD*) &message[1]), *((WORD*) &message[3]));
				return AddAnimationObject(pAnim);
				}
			default:
				;
			}
			break;
		case 140:	// External Position Control
			{
				CAnimExternalPositionControl* pAnim;
				pAnim = new CAnimExternalPositionControl();
				return AddAnimationObject(pAnim, &message[1]);
			}
			break;
		case 142:	// create integer range animation
			{
				CAnimIntegerRange* pAnim = new CAnimIntegerRange(
					*((UINT32*) &message[1]),
					*((UINT32*) &message[5]),
					*((INT16*) &message[9]));
				return AddAnimationObject(pAnim);
			}
			break;
		default:
			CString ErrorString;
			ErrorString.Format(_T("Invalid command. Key=%u, Code=%u"), key, message[0]);
			COutputList::AddString(ErrorString);
			return 0;
		}
	} else {	// key not 0 --> address stimulus or animation object
		if ((key & 0x8000) != 0)
		// animation command
		{
			CAnim* pAnim;
			if (!m_animations.Lookup(key, (CObject*&) pAnim)) {
				CString ErrorString;
				ErrorString.Format(_T("Command for non existing animation object (key=%i, length=%u)"), key, messageLength);
				COutputList::AddString(ErrorString);
				return -1;
			}
			switch (message[0]) {
			case 0:
				switch (messageLength) {
				case 1:	// ... BREAK ANY ASSIGNMENTS TO STIMULI
					if (pAnim->m_pStimulus) pAnim->m_pStimulus->m_animation = NULL;
					EnterCriticalSection(&g_criticalMapSection);
					m_animations.RemoveKey(key);
					LeaveCriticalSection(&g_criticalMapSection);
					delete pAnim;
					break;
				case 2:	// [ka 0 a] set final action
					{
						BYTE temp = pAnim->m_finalAction.mask;
					pAnim->m_finalAction.mask = message[1];
				TRACE("Set Final Action from %u to %u\n", temp, pAnim->m_finalAction.mask);
					}
					break;
				case 4:	// assign / deassign stimulus
					{
						CStimulus* pStim;
						WORD sKey = *((WORD*) &message[2]);
						cIter = m_stimuli.find(sKey);
						if (cIter == m_stimuli.cend())
						{
							CString ErrorString;
							ErrorString.Format(_T("Trying to %Sassign animation (key=%u) %S non existing stimulus object (key=%u)"),
								message[1] ? "" : "de", key, message[1] ? "to" : "from", sKey);
							COutputList::AddString(ErrorString);
							return -1;
						}
						pStim = cIter->second;
						if (message[1])
						{ // assign
							pStim->m_animation = pAnim;
							pAnim->m_pStimulus = pStim;
						} else
						{ // deassign
							pAnim->Deassign();
						}
					}
					break;
				default:
					ASSERT(pAnim);
					pAnim->Command(&message[0], messageLength);
				}
				break;
			default:
				ASSERT(pAnim);
				pAnim->Command(&message[0], messageLength);
			}
		} else
		// stimulus command
		{
			CStimulus* pStim;
			cIter = m_stimuli.find(key);
			if (cIter == m_stimuli.cend())
			{
				CString ErrorString;
				ErrorString.Format(_T("Command for non existing stimulus object (key=%i, length=%u)"), key, messageLength);
				COutputList::AddString(ErrorString);
				theApp.m_errorCode = 2;
				theApp.m_errorMask |= 1;
				return -1;
			}
			pStim = cIter->second;
			//		TRACE("Key: %u, Pointer: %p\n", key, pStim);
			switch (message[0]) {
			case 0:
				if (messageLength == 1) { // remove
					EnterCriticalSection(&g_criticalMapSection);
					VERIFY(m_stimuli.erase(key) == 1);
					LeaveCriticalSection(&g_criticalMapSection);
					EnterCriticalSection(&g_criticalDrawSection);
					delete pStim;
					LeaveCriticalSection(&g_criticalDrawSection);
				} else {
					ASSERT(pStim);
//					*(theApp.m_deferredMode ? &(pStim->m_enabledCopy) : &(pStim->m_enabled)) = message[1] != 0;
					(theApp.m_deferredMode ? pStim->m_enabledCopy : pStim->m_enabled) = message[1] != 0;
//					pStim->Command(&message[0], messageLength);
				}
				break;
			case 3:
				if (messageLength == 2)
				{	// [kk 3 p]
					pStim->m_protected = message[1] != 0;
				}
				else	// moveto
				{
					pStim->Moveto(theApp.m_deferredMode, (*(float*) &message[1]), (*(float*) &message[5]));
				}
				break;
			case 8:
				float buffer[2];
				pStim->GetPos(&buffer[0]);
				WritePipe(&buffer, sizeof(buffer));
//				TRACE("Position: %f, %f, Size: %u\n", buffer[0], buffer[1], sizeof(buffer));
				break;
			case 14:	// stimulus order
				EnterCriticalSection(&g_criticalMapSection);
				switch (messageLength)
				{
				case 1:	// move to front
					{
					map <WORD, CStimulus*> ::const_reverse_iterator cNewIter;
					m_stimuli.erase(cIter);	// remove stimulus from old position
					cNewIter = m_stimuli.crbegin();
					WORD newKey;
					newKey = cNewIter->first + 1;	// ... check for bit 15!!!
					m_stimuli[newKey] = pStim;
					WritePipe(&newKey, sizeof(newKey));
					}
					break;
				case 3:	// swap stimuli
					{
					CStimulus* p2ndStim;
					WORD key2 = *((WORD*)&message[1]);
					map <WORD, CStimulus*> ::const_iterator c2ndIter = m_stimuli.find(key2);
					if (c2ndIter == m_stimuli.cend())
					{
						LeaveCriticalSection(&g_criticalMapSection);
						CString ErrorString;
						ErrorString.Format(_T("Unexisting swap partner (key=%i) for stimulus object (key=%i)."), key2, key);
						COutputList::AddString(ErrorString);
						theApp.m_errorCode = 2;
						theApp.m_errorMask |= 1;
						return -1;
					}
					p2ndStim = c2ndIter->second;
					m_stimuli[key] = p2ndStim;
					m_stimuli[key2] = pStim;
					}
					break;
				default:
					LeaveCriticalSection(&g_criticalMapSection);
					CString ErrorString;
					ErrorString.Format(_T("Invalid length (%u) of stimuli order command."), messageLength);
					COutputList::AddString(ErrorString);
					theApp.m_errorCode = 7;
					theApp.m_errorMask |= 1;
					return -1;
				}
				LeaveCriticalSection(&g_criticalMapSection);
				break;
			default:
				ASSERT(pStim);
				pStim->Command(&message[0], messageLength);
			}
		}
	}
	return -1;
}


short CStimServerDoc::AddStimulusObject(CStimulus* pObject)
{
	if (theApp.m_deferredMode) pObject->makeCopy();
	WORD key;
	for (key = 1; m_stimuli.find(key) != m_stimuli.end(); key++) ;
	EnterCriticalSection(&g_criticalMapSection);
	m_stimuli[key] = pObject;
	LeaveCriticalSection(&g_criticalMapSection);
	TRACE("Doc::AddObj: done, Key=%u\n", key);
	return key;
}


short CStimServerDoc::ReplaceStimulusObject(CStimulus* pNewStim, WORD* pKey)
{
	if (theApp.m_deferredMode) pNewStim->makeCopy();
	CStimulus* pStim;
	WORD key = *pKey;
	TRACE("Key to replace: %u\n", key);
	EnterCriticalSection(&g_criticalMapSection);
	if (m_stimuli.find(key) != m_stimuli.end())
	{
		pStim = m_stimuli.at(key);
		delete pStim;
		TRACE("Replace: deleted stimulus with key %u\n", key);
	}
	if (key <= 0)
	{
		for (key = 1; m_stimuli.find(key) != m_stimuli.end(); key++) ;
	}
	m_stimuli[key] = pNewStim;
	LeaveCriticalSection(&g_criticalMapSection);
	TRACE("Replace: inserted stimulus at key %u\n", key);
	TRACE("Doc::Replace Object: done\n");
	return key;
}


short CStimServerDoc::AddStimulusObject(CLoadedStimulus* pObject, unsigned char filename[])
{
	if (!pObject->Init(CString(filename))) {
		TRACE("Pre delete object\n");
		delete pObject;
		TRACE("Post delete object\n");
		theApp.m_errorCode = 1;
		theApp.m_errorMask |= 1;
		return 0;
	}
	return AddStimulusObject(pObject);
}


short CStimServerDoc::ReplaceStimulusObject(CLoadedStimulus* pNewStim, unsigned char filename[])
{
	if (!pNewStim->Init(CString(&filename[2]))) {
		delete pNewStim;
		return 0;
	}
	return ReplaceStimulusObject(pNewStim, (WORD*) &filename[0]);
}


short CStimServerDoc::AddAnimationObject(CAnim* pObject)
{
	WORD key;
	CAnim* pAnim;
	for (key = 32769; NULL != m_animations.Lookup(key, (CObject*&) pAnim); key++) ;
	m_animations[key] = pObject;
	TRACE("Doc::AddAnimation: done\n");
	return key;
}


short CStimServerDoc::AddAnimationObject(CLoadedAnim* pObject, unsigned char filename[])
{
	if (!pObject->Init(CString(filename))) {
		delete pObject;
		return 0;
	}
	return AddAnimationObject(pObject);
}

/*
void CStimServerDoc::AddPhotoDiode(D2D1_RECT_F PDrect)
{
	m_pPhotoDiode = new CPhotoDiode(PDrect);
//	D2D1_RECT_F PhotoDiodeRect = D2D1::RectF(contextSize.width/-2.f, contextSize.height/-2.f, contextSize.width/-2.f+15.f, contextSize.height/-2.f+15.f);
//	bool PhotoDiode = false;
}
*/

void CStimServerDoc::OnCloseDocument()
{
	TRACE("Start of close document\n");
	m_hCloseDocument = CreateEvent(NULL, false, false, NULL);
	ASSERT(m_hCloseDocument != NULL);
	m_valid = false;
//	delete m_pPhotoDiode;
//	m_pPhotoDiode = NULL;
	WaitForSingleObject(m_hCloseDocument, INFINITE);
	CloseHandle(m_hCloseDocument);
	CPhotoDiode::Cleanup();
	UnprotectAllStimuli();
	RemoveAllStimuli();
	TRACE("End of close document\n");

	CDocument::OnCloseDocument();
}


void CStimServerDoc::UnprotectAllStimuli(void)
{
	map <WORD, CStimulus*> :: const_iterator cIter;
	CStimulus* pStimulus;

	EnterCriticalSection(&g_criticalMapSection);
	for (cIter = m_stimuli.cbegin(); cIter != m_stimuli.cend(); cIter++)
	{
		pStimulus = cIter->second;
		pStimulus->m_protected = false;
	}
	LeaveCriticalSection(&g_criticalMapSection);
}


void CStimServerDoc::RemoveAllStimuli(void)
{
//	POSITION pos;
	map <WORD, CStimulus*> :: const_iterator cIter;
//	WORD key;
	CStimulus* pStimulus;

	EnterCriticalSection(&g_criticalMapSection);
	WORD nProtected = 0;
	for (cIter = m_stimuli.cbegin(); cIter != m_stimuli.cend(); cIter++)
	{
		pStimulus = cIter->second;
		if (pStimulus->m_protected)
		{
			nProtected++;
		}
		else
		{
			delete pStimulus;
		}
	}
	if (nProtected == 0) m_stimuli.clear();
	LeaveCriticalSection(&g_criticalMapSection);
}

#include "BeamerDialog.h"

void CStimServerDoc::OnAppsBeamertest()
{
	m_backgroundColor.r = 0.0f;
	m_backgroundColor.g = 0.0f;
	m_backgroundColor.b = 0.0f;
	TRACE("Beamer Test 1\n");
	CBeamerDialog* pBeamerDialog = new CBeamerDialog();
	TRACE("Beamer Test 2\n");
	pBeamerDialog->m_pDoc = this;
	pBeamerDialog->m_key = AddStimulusObject(pBeamerDialog->m_pStim);
	TRACE("Beamer Test 3\n");
	VERIFY(pBeamerDialog->Create(IDD_DIALOG1));
}


void CStimServerDoc::EndDeferredMode(void)
{
//	status = WaitForSingleObject(m_hDeferredUpdateDone, INFINITE);
//	ASSERT(status == WAIT_OBJECT_0);
	theApp.m_deferredMode = false;
	m_deferredUpdate = true;
//	VERIFY(ResetEvent(m_hDeferredUpdateDone));
}
