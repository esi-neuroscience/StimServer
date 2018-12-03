// HardwareDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "StimServer.h"
#include "HardwareDlg.h"
#include "afxdialogex.h"

extern D2D1_SIZE_F g_ScreenSize;
extern float g_frameRate;
extern CString g_adapterName;

// CHardwareDlg-Dialogfeld

IMPLEMENT_DYNAMIC(CHardwareDlg, CDialogEx)

CHardwareDlg::CHardwareDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CHardwareDlg::IDD, pParent)
{
	m_featureLevelString = theApp.m_featureLevelString;
	m_displaySizeString.Format(_T("Display Size: %ux%u"),
		(WORD) g_ScreenSize.width, (WORD) g_ScreenSize.height);
	m_frameRateString.Format(_T("Frame Rate: %f Hz"), g_frameRate);
	m_adapterString = g_adapterName;
}

CHardwareDlg::~CHardwareDlg()
{
}

void CHardwareDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_featureLevelString);
	DDV_MaxChars(pDX, m_featureLevelString, 7);
	DDX_Text(pDX, IDC_EDIT_DISPLAY_SIZE, m_displaySizeString);
	DDV_MaxChars(pDX, m_displaySizeString, 24);
	DDX_Text(pDX, IDC_EDIT_FRAME_RATE, m_frameRateString);
	DDX_Text(pDX, IDC_EDIT_ADAPTER, m_adapterString);
}


BEGIN_MESSAGE_MAP(CHardwareDlg, CDialogEx)
END_MESSAGE_MAP()


// CHardwareDlg-Meldungshandler
