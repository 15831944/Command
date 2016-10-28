#pragma once


// CNumEdit

class CNumEdit : public CEdit
{
	DECLARE_DYNAMIC(CNumEdit)

public:
	CNumEdit();
	virtual ~CNumEdit();

protected:
	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    BOOL CheckUnique(char nChar);
};


