
// StimServerView.h : interface of the CStimServerView class
//

#pragma once


class CStimServerView : public CView
{
protected: // create from serialization only
	CStimServerView();
	DECLARE_DYNCREATE(CStimServerView)

// Attributes
public:
	CStimServerDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementation
public:
	virtual ~CStimServerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
//	afx_msg LRESULT OnDraw2d(WPARAM wParam, LPARAM lParam);
};

#ifndef _DEBUG  // debug version in StimServerView.cpp
inline CStimServerDoc* CStimServerView::GetDocument() const
   { return reinterpret_cast<CStimServerDoc*>(m_pDocument); }
#endif

