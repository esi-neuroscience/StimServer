#pragma once
class CPerfStat
{
public:
	CPerfStat(WORD n);
	~CPerfStat(void);
private:
	void Init();
	LARGE_INTEGER m_preCount;
	LARGE_INTEGER m_performanceFrequency;
	LARGE_INTEGER m_sum;
	LARGE_INTEGER m_min;
	LARGE_INTEGER m_max;
	WORD m_n;
	WORD m_count;
public:
	void Pre(void);
	void Post(void);
private:
	float m_ms;
};

