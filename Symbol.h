#pragma once

// CSymbol-Befehlsziel

class CSymbol : public CObject
{
public:
	CSymbol();
//	virtual ~CSymbol();
	~CSymbol();
	virtual void DrawTexture() = 0;
	void DrawBrushedTexture();
	virtual void DrawBrushedTexture(ID2D1SolidColorBrush* pBrush) {};
	ID3D11ShaderResourceView *m_pShaderResView;
	ID3D11SamplerState* m_pSampler;
	void CreateTextureTarget();
//	void CreateTextureTarget(D2D1_SIZE_U pixelSize);
//	unsigned short m_size;
	D2D1_SIZE_U m_size;
protected:
	void Init();
//	void DrawBrushedTexture();
	ID3D11Texture2D *m_pRenderTarget;
//	ID3D11RenderTargetView *m_pRenderTargetView;
	ID2D1RenderTarget* m_pD2DrenderTarget;
private:
	union
	{
		BYTE	m_flags;
		struct
		{
			D2D1_ANTIALIAS_MODE aliased:1;
		}
		m_flag;
	};
};


class CDisc : public CSymbol
{
public:
	CDisc(unsigned short size);
	~CDisc(void);
	void DrawTexture();
	void DrawBrushedTexture(ID2D1SolidColorBrush* pBrush);
};

class CCircle :
	public CSymbol
{
public:
	CCircle(unsigned short size);
	~CCircle(void);
	void DrawTexture();
	void DrawBrushedTexture(ID2D1SolidColorBrush* pBrush);
};

class CPicture :
	public CSymbol
{
public:
	CPicture(ID2D1Bitmap* pBitmap);
	~CPicture(void);
	void DrawTexture();
private:
	ID2D1Bitmap* m_pBitmap;
};


