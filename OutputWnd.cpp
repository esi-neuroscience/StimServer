
#include "stdafx.h"

#include "OutputWnd.h"
#include "Resource.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "StimServer.h"
//extern CStimServerApp theApp;

/////////////////////////////////////////////////////////////////////////////
// COutputBar

COutputWnd::COutputWnd()
{
}

COutputWnd::~COutputWnd()
{
}

BEGIN_MESSAGE_MAP(COutputWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
//	ON_MESSAGE(WM_SEQUENCE_ERR, &COutputWnd::OnSequenceErr)
END_MESSAGE_MAP()

int COutputWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();
/*
	// Create tabs window:
	if (!m_wndTabs.Create(CMFCTabCtrl::STYLE_FLAT, rectDummy, this, 1))
	{
		TRACE0("Failed to create output tab window\n");
		return -1;      // fail to create
	}

	// Create output panes:
	const DWORD dwStyle = LBS_NOINTEGRALHEIGHT | WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | ES_MULTILINE | ES_READONLY;

	if (!m_wndOutputBuild.Create(dwStyle, rectDummy, &m_wndTabs, 2) ||
		!m_wndOutputDebug.Create(dwStyle, rectDummy, &m_wndTabs, 3) ||
		!m_wndOutputFind.Create(dwStyle, rectDummy, &m_wndTabs, 4))
*/
	const DWORD dwStyle = LBS_NOINTEGRALHEIGHT | WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | ES_MULTILINE | ES_READONLY;
	if (!m_wndOutputBuild.Create(dwStyle, rectDummy, this, 1))
	{
		TRACE0("Failed to create output windows\n");
		return -1;      // fail to create
	}

//	UpdateFonts();

	CString strTabName;
//	BOOL bNameValid;
/*
	// Attach list windows to tab:
	bNameValid = strTabName.LoadString(IDS_BUILD_TAB);
	ASSERT(bNameValid);
	m_wndTabs.AddTab(&m_wndOutputBuild, strTabName, (UINT)0);
	bNameValid = strTabName.LoadString(IDS_DEBUG_TAB);
	ASSERT(bNameValid);
	m_wndTabs.AddTab(&m_wndOutputDebug, strTabName, (UINT)1);
	bNameValid = strTabName.LoadString(IDS_FIND_TAB);
	ASSERT(bNameValid);
	m_wndTabs.AddTab(&m_wndOutputFind, strTabName, (UINT)2);
*/
	// Fill output tabs with some dummy text (nothing magic here)
//	FillBuildWindow();
//	FillDebugWindow();
//	FillFindWindow();

	return 0;
}

void COutputWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	// Tab control should cover the whole client area:
//	m_wndTabs.SetWindowPos (NULL, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
	TRACE("width: %u, height: %u\n", cx, cy);
	m_wndOutputBuild.SetWindowPos (NULL, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
}

void COutputWnd::AdjustHorzScroll(CListBox& wndListBox)
{
	CClientDC dc(this);
	CFont* pOldFont = dc.SelectObject(&afxGlobalData.fontRegular);

	int cxExtentMax = 0;

	for (int i = 0; i < wndListBox.GetCount(); i ++)
	{
		CString strItem;
		wndListBox.GetText(i, strItem);

		cxExtentMax = max(cxExtentMax, dc.GetTextExtent(strItem).cx);
	}

	wndListBox.SetHorizontalExtent(cxExtentMax);
	dc.SelectObject(pOldFont);
}
/*
void COutputWnd::FillBuildWindow()
{
	m_wndOutputBuild.AddString(_T("Build output is being displayed here."));
	m_wndOutputBuild.AddString(_T("The output is being displayed in rows of a list view"));
	m_wndOutputBuild.AddString(_T("but you can change the way it is displayed as you wish..."));
}
*/
/*
void COutputWnd::FillDebugWindow()
{
	m_wndOutputDebug.AddString(_T("Debug output is being displayed here."));
	m_wndOutputDebug.AddString(_T("The output is being displayed in rows of a list view"));
	m_wndOutputDebug.AddString(_T("but you can change the way it is displayed as you wish..."));
}

void COutputWnd::FillFindWindow()
{
	m_wndOutputFind.AddString(_T("Find output is being displayed here."));
	m_wndOutputFind.AddString(_T("The output is being displayed in rows of a list view"));
	m_wndOutputFind.AddString(_T("but you can change the way it is displayed as you wish..."));
}
*/
/*
void COutputWnd::UpdateFonts()
{
	m_wndOutputBuild.SetFont(&afxGlobalData.fontRegular);
	m_wndOutputDebug.SetFont(&afxGlobalData.fontRegular);
	m_wndOutputFind.SetFont(&afxGlobalData.fontRegular);
}
*/
/////////////////////////////////////////////////////////////////////////////
// COutputList1

	CEdit* COutputList::m_pEditWnd;

COutputList::COutputList()
{
	m_pEditWnd = this;
}

void COutputList::AddString(CString string)
{
	CString strTemp;
	m_pEditWnd->GetWindowTextW(strTemp);
	strTemp += CTime::GetCurrentTime().Format(_T("\r\n%H:%M:%S ")) + string;
	m_pEditWnd->SetWindowTextW(strTemp);
	m_pEditWnd->LineScroll(m_pEditWnd->GetLineCount(), 0);
	theApp.m_pMainWnd->ShowWindow(SW_SHOWNORMAL);
}

COutputList::~COutputList()
{
}

BEGIN_MESSAGE_MAP(COutputList, CListBox)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	ON_COMMAND(ID_VIEW_OUTPUTWND, OnViewOutput)
	ON_WM_WINDOWPOSCHANGING()
	ON_MESSAGE(WM_SEQUENCE_ERR, &COutputList::OnSequenceErr)
	ON_MESSAGE(WM_ERROR_STRING, &COutputList::OnErrorString)
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// COutputList message handlers

void COutputList::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	CMenu menu;
	menu.LoadMenu(IDR_OUTPUT_POPUP);

	CMenu* pSumMenu = menu.GetSubMenu(0);

	if (AfxGetMainWnd()->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
	{
		CMFCPopupMenu* pPopupMenu = new CMFCPopupMenu;

		if (!pPopupMenu->Create(this, point.x, point.y, (HMENU)pSumMenu->m_hMenu, FALSE, TRUE))
			return;

		((CMDIFrameWndEx*)AfxGetMainWnd())->OnShowPopupMenu(pPopupMenu);
		UpdateDialogControls(this, FALSE);
	}

	SetFocus();
}

void COutputList::OnEditCopy()
{
	MessageBox(_T("Copy output"));
}

void COutputList::OnEditClear()
{
	MessageBox(_T("Clear output"));
}

void COutputList::OnViewOutput()
{
	CDockablePane* pParentBar = DYNAMIC_DOWNCAST(CDockablePane, GetOwner());
	CMDIFrameWndEx* pMainFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, GetTopLevelFrame());

	if (pMainFrame != NULL && pParentBar != NULL)
	{
		pMainFrame->SetFocus();
		pMainFrame->ShowPane(pParentBar, FALSE, FALSE, FALSE);
		pMainFrame->RecalcLayout();

	}
}


afx_msg LRESULT COutputList::OnSequenceErr(WPARAM wParam, LPARAM lParam)
{
	CString errString;
	DXGI_FRAME_STATISTICS* pFrameStatsBuffer = (DXGI_FRAME_STATISTICS*) wParam;

	errString.Format(_T("Sequence Error: present count %u --> %u, vSync %u --> %u"), (*pFrameStatsBuffer).PresentCount, (*(pFrameStatsBuffer+1)).PresentCount,
		(*pFrameStatsBuffer).SyncRefreshCount, (*(pFrameStatsBuffer+1)).SyncRefreshCount);
	AddString(errString);
	delete (DXGI_FRAME_STATISTICS*) wParam;
	return 0;
}


afx_msg LRESULT COutputList::OnErrorString(WPARAM wParam, LPARAM lParam)
{
//	CString errString;

//	errString.Format(_T("Sequence Error: present count %u --> %u, vSync %u --> %u"), (*pFrameStatsBuffer).PresentCount, (*(pFrameStatsBuffer+1)).PresentCount,
//		(*pFrameStatsBuffer).SyncRefreshCount, (*(pFrameStatsBuffer+1)).SyncRefreshCount);
	AddString(*(CString*) wParam);
	delete (CString*) wParam;
	return 0;
}
