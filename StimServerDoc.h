
// StimServerDoc.h : interface of the CStimServerDoc class
//


#pragma once
#include "afxcoll.h"
//#include "Stimulus.h"
#include "GeometryStimuli.h"
#include "StimulusPixel.h"
#include "Anim.h"
#include "OutputWnd.h"
#include <map>

using namespace std;

class CStimServerDoc : public CDocument
{
protected: // create from serialization only
	CStimServerDoc();
	DECLARE_DYNCREATE(CStimServerDoc)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~CStimServerDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
private:
	map<WORD, CStimulus*> m_stimuli;
	CMapWordToOb m_animations;
	virtual short AddStimulusObject(CStimulus* pObject);
	virtual short AddStimulusObject(CLoadedStimulus* pObject, unsigned char filename[]);
	virtual short ReplaceStimulusObject(CLoadedStimulus* pObject, unsigned char filename[]);
	virtual short AddAnimationObject(CAnim* pObject);
	virtual short AddAnimationObject(CLoadedAnim* pObject, unsigned char filename[]);
//	CPhotoDiode* m_pPhotoDiode;
	D2D1_COLOR_F m_backgroundColor = D2D1::ColorF(.5f, .5f, .5f, 1.f);
	bool m_photoDiodeEnabledCopy;
	D2D1_COLOR_F m_backgroundColorCopy = D2D1::ColorF(.5f, .5f, .5f, 1.f);
	PDCSR m_PDCSRcopy = { 0,0 };
	bool m_deferredUpdate;
	HANDLE m_hDeferredUpdateDone;
	HANDLE m_hEndDeferredMode;
	void UnprotectAllStimuli(void);
	void RemoveAllStimuli(void);
public:
//	bool Draw();
	void Draw();
	short Command(WORD key, unsigned char message[], DWORD messageLength);
	virtual short ReplaceStimulusObject(CStimulus* pObject, WORD* pKey);
//	void AddPhotoDiode(D2D1_RECT_F PDrect);
	virtual void OnCloseDocument();
	HANDLE m_hCloseDocument;
	afx_msg void OnAppsBeamertest();
	void EndDeferredMode(void);
	static bool m_valid;
};
