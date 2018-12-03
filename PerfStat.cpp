#include "StdAfx.h"
#include "PerfStat.h"


CPerfStat::CPerfStat(WORD n)
{
	m_n = n;
	ASSERT(QueryPerformanceFrequency(&m_performanceFrequency));
	m_ms = 1000.0f / m_performanceFrequency.QuadPart;
	Init();
}


CPerfStat::~CPerfStat(void)
{
}


void CPerfStat::Init(void)
{
	m_count = 0;
	m_sum.QuadPart = 0;
	m_min.HighPart = 32765;
	m_max.QuadPart = 0;
}


void CPerfStat::Pre(void)
{
	ASSERT(QueryPerformanceCounter(&m_preCount));
}


void CPerfStat::Post(void)
{
	LARGE_INTEGER postCount;
	LARGE_INTEGER duration;
	ASSERT(QueryPerformanceCounter(&postCount));
	duration.QuadPart = postCount.QuadPart - m_preCount.QuadPart;
	if (duration.QuadPart < m_min.QuadPart) m_min.QuadPart = duration.QuadPart;
	if (duration.QuadPart > m_max.QuadPart) m_max.QuadPart = duration.QuadPart;
	m_sum.QuadPart += duration.QuadPart;
	m_count++;
	if (m_count == m_n)
	{
		TRACE("%f %f %f\n",
			((float) m_sum.QuadPart)/m_n*m_ms,
			m_min.QuadPart*m_ms,
			m_max.QuadPart*m_ms);
		Init();
	}
}
