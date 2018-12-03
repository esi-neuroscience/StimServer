#include "StdAfx.h"
#include "NumericBinaryFile.h"
#include "OutputWnd.h"


CNumericBinaryFile::CNumericBinaryFile(LPCWSTR wzFilename)
{
	CFileException e;
	if (!m_file.Open(wzFilename, 
		CFile::modeRead | CFile::shareExclusive | CFile::osSequentialScan | CFile::modeNoInherit,
		&e))
	{
		CString errString = _T("Error reading numeric binary file: \r\n ");
		TCHAR   szCause[255];
		e.GetErrorMessage(szCause, 255);
		errString += szCause;
		COutputList::AddString(errString);
	}
}


CNumericBinaryFile::~CNumericBinaryFile(void)
{
	if (m_file.m_hFile != INVALID_HANDLE_VALUE)	m_file.Close();
	TRACE("Close Numeric Binary File\n");
}


void CNumericBinaryFile::Init(void)
{
	UINT nRead;
	UINT nToRead = sizeof(m_header);
	nRead = m_file.Read(&m_header, nToRead);
	ASSERT(nRead == nToRead);
	TRACE("Version: %u, Type: %u, Number of dimensions: %u\n", m_header.version, m_header.type, m_header.nDims);
	ASSERT(m_header.type == 9);
	ASSERT(m_header.nDims == 2);
	nRead = m_file.Read(&m_dims, sizeof(m_dims));
//	ASSERT(m_dims[0] == 2);
}


bool CNumericBinaryFile::Read2Dfloat(float* dest)
{
	UINT nToRead = (UINT) m_dims[0] * sizeof(float);
	UINT nRead = m_file.Read(dest, nToRead);
	ASSERT(nRead == nToRead);
	return nRead == nToRead;
}


//bool CNumericBinaryFile::ReadPoints2f(D2D1_POINT_2F* dest)
bool CNumericBinaryFile::ReadPoints2f(float* dest)
{
	UINT nToRead = (UINT) m_dims[0] * (UINT) m_dims[1] * sizeof(float);
	UINT nRead = m_file.Read(dest, nToRead);
	ASSERT(nRead == nToRead);
	return nRead == nToRead;
}
