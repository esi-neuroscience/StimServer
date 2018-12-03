#pragma once


// CHardwareDlg-Dialogfeld

class CHardwareDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CHardwareDlg)

public:
	CHardwareDlg(CWnd* pParent = NULL);   // Standardkonstruktor
	virtual ~CHardwareDlg();

// Dialogfelddaten
	enum { IDD = IDD_ABOUTHW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung

	DECLARE_MESSAGE_MAP()
public:
//	afx_msg void OnHelpAbouthardware();
	CString m_featureLevelString;
	CString m_displaySizeString;
	CString m_frameRateString;
	CString m_adapterString;
};
