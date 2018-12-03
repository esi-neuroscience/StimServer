
// StimServerView.cpp : implementation of the CStimServerView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "StimServer.h"
#endif

#include "StimServerDoc.h"
#include "StimServerView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CStimServerView

IMPLEMENT_DYNCREATE(CStimServerView, CView)

BEGIN_MESSAGE_MAP(CStimServerView, CView)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
//	ON_REGISTERED_MESSAGE(AFX_WM_DRAW2D, &CStimServerView::OnDraw2d)
END_MESSAGE_MAP()

// CStimServerView construction/destruction

CStimServerView::CStimServerView()
{
	// TODO: add construction code here

}

CStimServerView::~CStimServerView()
{
}

BOOL CStimServerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
//	EnableD2DSupport();
	return CView::PreCreateWindow(cs);
}

// CStimServerView drawing

void CStimServerView::OnDraw(CDC* /*pDC*/)
{
	CStimServerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}

void CStimServerView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CStimServerView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CStimServerView diagnostics

#ifdef _DEBUG
void CStimServerView::AssertValid() const
{
	CView::AssertValid();
}

void CStimServerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CStimServerDoc* CStimServerView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CStimServerDoc)));
	return (CStimServerDoc*)m_pDocument;
}
#endif //_DEBUG
