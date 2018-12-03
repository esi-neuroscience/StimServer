#pragma once
#include "StimulusPixel.h"
#include "stimserverdoc.h"

// CBeamerDialog-Dialogfeld

class CBeamerDialog : public CDialog
{
	DECLARE_DYNAMIC(CBeamerDialog)

public:
	CBeamerDialog(CWnd* pParent = NULL);   // Standardkonstruktor
	virtual ~CBeamerDialog();

// Dialogfelddaten
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClickedButton1();
	CButton m_OnOffButton;
private:
	bool m_enable;
public:
//	CStimulusPixel* m_pStim;
//	CStimulusSymbol* m_pStim;
	CStimulus* m_pStim;
	afx_msg void OnClickedPosition();
private:
	char m_i;
	char m_j;
public:
	CComboBox m_modeBox;
	BOOL OnInitDialog();
	afx_msg void OnSelchangeCombo1();
	CStimServerDoc* m_pDoc;
	WORD m_key;
};
