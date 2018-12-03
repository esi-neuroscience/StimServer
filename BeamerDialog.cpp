// BeamerDialog.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "StimServer.h"
#include "BeamerDialog.h"
#include "afxdialogex.h"


// CBeamerDialog-Dialogfeld

IMPLEMENT_DYNAMIC(CBeamerDialog, CDialog)

CBeamerDialog::CBeamerDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CBeamerDialog::IDD, pParent)
	, m_enable(true)
	, m_i(0)
	, m_j(0)
{
	m_pStim = (CStimulusPixel*) new CStimulusPixel();
	m_pStim->m_enabled = true;
}

CBeamerDialog::~CBeamerDialog()
{
}

void CBeamerDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON1, m_OnOffButton);
	DDX_Control(pDX, IDC_COMBO1, m_modeBox);
}


BEGIN_MESSAGE_MAP(CBeamerDialog, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &CBeamerDialog::OnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CBeamerDialog::OnClickedPosition)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CBeamerDialog::OnSelchangeCombo1)
END_MESSAGE_MAP()


// CBeamerDialog-Meldungshandler


void CBeamerDialog::OnClickedButton1()
{
	m_enable = !m_enable;
	m_OnOffButton.SetWindowTextW(m_enable ? L"Off" : L"On");
	m_pStim->m_enabled = m_enable;
}


void CBeamerDialog::OnClickedPosition()
{
//	float point[2] = {(m_i-1)*(2.0f/3.0f), (m_j-1)*(2.0f/3.0f)};
	m_pStim->Moveto(false, (m_i-1)*(2.0f/3.0f), (m_j-1)*(2.0f/3.0f));
	m_i += 1;
	if (m_i == 3)
	{
		m_i = 0;
		m_j += 1;
		if (m_j == 3) m_j = 0;
	}
}


BOOL CBeamerDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
//	TRACE("CBeamerDialog::OnInitDialog A\n");
	m_modeBox.AddString(L"1 Pixel");
	m_modeBox.AddString(L"4 Pixels");
	m_modeBox.SetCurSel(0);

//	TRACE("CBeamerDialog::OnInitDialog O\n");
	return TRUE;  // return TRUE unless you set the focus to a control
	// AUSNAHME: OCX-Eigenschaftenseite muss FALSE zurückgeben.
}


void CBeamerDialog::OnSelchangeCombo1()
{
	switch (m_modeBox.GetCurSel())
	{
	case 0:	// 1 Pixel --> CStimulusPixel
		{
			CStimulusPixel* pStim = new CStimulusPixel();
			WORD key = m_pDoc->ReplaceStimulusObject(pStim, &m_key);
			m_key = key;
			m_pStim = pStim;
		}
		break;
	case 1: // 4 Pixel --> CStimulusSymbolRect
		{
			unsigned short size[2] = {2, 2}; 
//			CStimulusSymbolRect* pStim = new CStimulusSymbolRect(size);
//			WORD key = m_pDoc->ReplaceStimulusObject(pStim, &m_key);
//			m_key = key;
//			m_pStim = pStim;
		}
	}
	m_pStim->m_enabled = m_enable;
}
