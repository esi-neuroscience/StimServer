
#pragma once

/////////////////////////////////////////////////////////////////////////////
// COutputList window

//class COutputList : public CListBox
class COutputList : public CEdit
{
// Construction
public:
	COutputList();

// Implementation
public:
	static void AddString(CString);
	virtual ~COutputList();
private:
	static CEdit* m_pEditWnd;

protected:
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnEditCopy();
	afx_msg void OnEditClear();
	afx_msg void OnViewOutput();

	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnSequenceErr(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnErrorString(WPARAM wParam, LPARAM lParam);
};

class COutputWnd : public CDockablePane
{
// Construction
public:
	COutputWnd();

//	void UpdateFonts();

// Attributes
public:
//	CMFCTabCtrl	m_wndTabs;

	COutputList m_wndOutputBuild;
//	COutputList m_wndOutputDebug;
//	COutputList m_wndOutputFind;

protected:
	void FillBuildWindow();
	void FillDebugWindow();
	void FillFindWindow();

	void AdjustHorzScroll(CListBox& wndListBox);

// Implementation
public:
	virtual ~COutputWnd();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()
};

