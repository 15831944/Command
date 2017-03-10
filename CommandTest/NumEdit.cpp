// NumEdit.cpp : 實作檔
//

#include "stdafx.h"
#include "CommandTest.h"
#include "NumEdit.h"


// CNumEdit

IMPLEMENT_DYNAMIC(CNumEdit, CEdit)

CNumEdit::CNumEdit()
{

}

CNumEdit::~CNumEdit()
{
}

BEGIN_MESSAGE_MAP(CNumEdit, CEdit)
END_MESSAGE_MAP()

// CNumEdit 訊息處理常式
BOOL CNumEdit::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_CHAR)
    {
        //允許退格建
        if ((8 == pMsg->wParam) || (127 == pMsg->wParam))
        {
            return FALSE;
        }
        //只允許一個小數點
        if (('.' == pMsg->wParam) && !CheckUnique('.') && (GetSel() == 0))
        {        
            SetWindowText(L"0.");
            SetSel(3, 2);
            return TRUE;
        }
        else if (('.' == pMsg->wParam) && CheckUnique('.'))
            return TRUE;        
        else if (('.' == pMsg->wParam))
        {
            CString str;
            GetWindowText(str);
            if (str == L"-")//判斷目前是否只有一個-號
            {
                SetWindowText(L"-0.");
                SetSel(4, 3);
                return TRUE;
            }
            return CEdit::PreTranslateMessage(pMsg);
        }    
        //當前字符是"-"及當前光標在第一個 及當前沒有"-"
        if (('-' == pMsg->wParam) && (GetSel() == 0) && !CheckUnique('-'))
            return CEdit::PreTranslateMessage(pMsg);
        else if(('-' == pMsg->wParam))
            return TRUE;
        //只允許輸入點和數字
        if (pMsg->wParam >= 48 && pMsg->wParam <= 57)
        {
            return CEdit::PreTranslateMessage(pMsg);
        }
        return TRUE;
    }
    return CEdit::PreTranslateMessage(pMsg);
}
//判斷是否只有一個小數點
BOOL CNumEdit::CheckUnique(char nChar)
{
    CString str;
    int nPos = 0;
    GetWindowText(str);
    nPos = str.Find(nChar);
    return (nPos >= 0) ? TRUE : FALSE;
}