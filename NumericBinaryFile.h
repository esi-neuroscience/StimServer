#pragma once
#include "afx.h"
class CNumericBinaryFile
{
public:
	CNumericBinaryFile(LPCWSTR wzFilename);
	~CNumericBinaryFile(void);
	bool valid(void) { return m_file.m_hFile != INVALID_HANDLE_VALUE; }
	void Init(void);
	UINT64 m_dims[2];
private:
	CFile m_file;
	struct {
		unsigned char version;
		unsigned char type;
		unsigned char nDims;
	} m_header;
public:
	bool Read2Dfloat(float* dest);
	bool ReadPoints2f(float* dest);
};

