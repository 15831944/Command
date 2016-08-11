#include "stdafx.h"
#include "ExcelToList.h"
#include "ListToExcel.h"
#include <afxdb.h>   
#include <odbcinst.h>  
//獲得默認文件名
BOOL LoadGetDefaultXlsFileName(CString& sExcelFile)
{
    CString timeStr;
    //跳出存檔對話框
    TCHAR szFilters[] = _T("Excle(*.xls)|*.xls|所有檔案(*.*)|*.*||");
    CFileDialog dlgFile(TRUE, L".xls", L"", OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, szFilters);

    CString title;
    CString strFilter;

    title = L"導入Excel檔案";
    dlgFile.m_ofn.lpstrTitle = title;//設定對話框標題

    if (dlgFile.DoModal() == IDCANCEL)
        return FALSE; // open cancelled
    else
        sExcelFile = dlgFile.GetPathName();
    sExcelFile.ReleaseBuffer();
    return TRUE;
}
///////////////////////////////////////////////////////////////////////////////
//	void GetExcelDriver(CListCtrl* pList, CString strTitle)
//	參數：
//		pList		需要導出的List控件指针
//		strTitle	導出的數據表標題
//	說明:\
//      導出CListCtrl控件的全部數據到Excel文件。Excel文件名由用戶通過"另存為"
//      對話框輸入指定。創建名為strTitle的工作表,將List控件內的所有數據(包括列
//      名和數據項)以文本的形式保存到Excel工作表中。保持行列關係。
///////////////////////////////////////////////////////////////////////////////
void ExportExcelToList(CListCtrl* pList, CString strTitle)
{
    CString warningStr;
    CDatabase database;
    CString sDriver;
    CString sExcelFile;
    CString sSql;
    CString tableName = strTitle;

    // 檢查是否有安裝Excel驅動 "Microsoft Excel Driver (*.xls)" 
    sDriver = GetExcelDriver();
    if (sDriver.IsEmpty())
    {
        //沒有發現Excel驅動
        AfxMessageBox(L"沒有安装Excel!\n請先安裝Excel軟件才能使用Excel導出功能!!");
        return;
    }

    //默認文件名(獲得文件名)
    if (!LoadGetDefaultXlsFileName(sExcelFile))
        return;

    //創建進行存取的字符串
    sSql.Format(L"ODBC;DRIVER={%s};DSN='''';DBQ=%s",sDriver, sExcelFile);

    // 創建數據庫 (既Excel表格文件)
    if (database.Open(NULL,false,false,sSql)) //開啟資料庫
    {
        //創建結構表
        CString sItem;
        CString columnName;
        LVCOLUMN columnData;
        columnData.mask = LVCF_TEXT;
        columnData.cchTextMax = 100;
        columnData.pszText = columnName.GetBuffer(100);
        int ListCount = 0;
        CRecordset recset(&database);
        /*****************修改SQL語法在這**************************/
        sSql = L"";
        for (int j = 0; pList->GetColumn(j, &columnData); j++)
        {
            if (j != 0)
            {
                sSql = sSql + L", ";
            }
            sSql = sSql + columnData.pszText;
        }
        columnName.ReleaseBuffer();
        
        sSql = L"SELECT " + sSql + L" FROM " + tableName;
        AfxMessageBox(sSql);
        recset.Open(CRecordset::forwardOnly, sSql, CRecordset::readOnly);
        while (!recset.IsEOF())
        {
            pList->InsertItem(ListCount, NULL);
            for (int i = 0; pList->GetColumn(i, &columnData); i++)
            {
                pList->GetColumn(i, &columnData);
                recset.GetFieldValue(columnData.pszText, sItem);
                LONG Value = _ttol(sItem);
                sItem.Format(L"%d", Value);
                pList->SetItemText(ListCount, i, sItem);
            }
            //next
            ListCount++;
            recset.MoveNext();
        }
    }
    //關閉數據庫
    database.Close();
    warningStr.Format(L"%s檔案導入成功!", sExcelFile);
    AfxMessageBox(warningStr);
}