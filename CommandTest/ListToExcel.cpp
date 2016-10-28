#include "stdafx.h"
#include "ListToExcel.h"
#include <io.h>
#include <odbcinst.h>
#include <afxdb.h>
/////////////////////////////////////////////////////////////////////////////////////////
//解決error LNK2001: 無法解析的外部符號 __imp___vsnprintf問題
//把相依性加入C:\Program Files(x86)\Windows Kits\8.0\Lib\win8\um\x86\odbccp32.lib
/////////////////////////////////////////////////////////////////////////////////////////

//名稱:GetExcelDriver
//功能:獲取ODBC中Excel驅動
CString GetExcelDriver()
{
    TCHAR szBuf[2001];
    WORD cbBufMax = 2000;
    WORD cbBufOut;
    TCHAR *pszBuf = szBuf;
    CString sDriver;
    //獲取已安裝驅動的名稱(函數在odbcinst.h裡)
    if (!SQLGetInstalledDrivers(szBuf, cbBufMax, &cbBufOut))
        return L"";
    //檢查已安裝的驅動是否有Excel...
    do
    {
        if (wcsstr(pszBuf, L"Excel") != 0)
        {
            //找到!
            sDriver = CString(pszBuf);
            break;
        }
        pszBuf = wcschr(pszBuf, '\0') + 1;
    } while (pszBuf[1] != '\0');
    return sDriver;
}
//名稱:MakeSurePathExists
//參數：
//	Path				路徑
//	FilenameIncluded	路徑是否包含文件名
//返回值:
//	文件是否存在
//說明:
//	判断Path文件(FilenameIncluded=true)是否存在,存在返回TURE，不存在返回FALSE
//	自動創建目錄
BOOL MakeSurePathExists(CString &Path,bool FilenameIncluded)
{
    int Pos = 0;
    while ((Pos = Path.Find('\\', Pos + 1)) != -1)
        CreateDirectory(Path.Left(Pos), NULL);
    if (!FilenameIncluded)
        CreateDirectory(Path, NULL);
    //	return ((!FilenameIncluded)?!_access(Path,0):
    //	!_access(Path.Left(Path.ReverseFind('\\')),0));

    return !_waccess(Path, 0);
}
//獲得默認文件名
BOOL GetDefaultXlsFileName(CString& sExcelFile)
{
    //默認文件名：yyyymmddhhmmss.xls
    CString timeStr;
    //CTime day;
    //day = CTime::GetCurrentTime();
    //int filenameday, filenamemonth, filenameyear, filehour, filemin, filesec;
    //filenameday = day.GetDay();//dd
    //filenamemonth = day.GetMonth();//mm月份
    //filenameyear = day.GetYear();//yyyy
    //filehour = day.GetHour();//hh
    //filemin = day.GetMinute();//mm分钟
    //filesec = day.GetSecond();//ss
    //timeStr.Format(L"%04d%02d%02d%02d%02d%02d", filenameyear, filenamemonth, filenameday, filehour, filemin, filesec);
    //sExcelFile = timeStr + L".xls";
    
    // prompt the user (with all document templates)
    //跳出存檔對話框
    TCHAR szFilters[] = _T("Excle(*.xls)|*.xls|所有檔案(*.*)|*.*||");
    CFileDialog dlgFile(FALSE, L".xls", L"", OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, szFilters);
    
    CString title;
    CString strFilter;

    title = L"導出Excel檔案";
    strFilter = L"Excel文件(*.xls)";
    strFilter += (TCHAR)'\0';   // next string please
    strFilter += L"*.xls";
    strFilter += (TCHAR)'\0';   // last string
    dlgFile.m_ofn.nMaxCustFilter++;
    dlgFile.m_ofn.nFilterIndex = 1;
    // append the "*.*" all files filter
    CString allFilter;
    VERIFY(allFilter.LoadString(AFX_IDS_ALLFILTER));
    strFilter += allFilter;
    strFilter += (TCHAR)'\0';   // next string please
    strFilter += L"*.*";
    strFilter += (TCHAR)'\0';   // last string
    dlgFile.m_ofn.nMaxCustFilter++;

    dlgFile.m_ofn.lpstrFilter = strFilter;//設定檔案過濾類型
    dlgFile.m_ofn.lpstrTitle = title;//設定對話框標題

    if (dlgFile.DoModal() == IDCANCEL)
        return FALSE; // open cancelled
    else
        sExcelFile = dlgFile.GetPathName();
    AfxMessageBox(sExcelFile);
    sExcelFile.ReleaseBuffer();

    if (MakeSurePathExists(sExcelFile, true)) 
    {//確定路徑是否存在
        if (!DeleteFile(sExcelFile)) 
        {    // delete the file
            AfxMessageBox(L"覆蓋文件時出錯!");
            return FALSE;
        }
    }
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
void ExportListToExcel(CListCtrl* pList, CString strTitle)
{
    CString warningStr;
    if (pList->GetItemCount()>0) {
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
        if (!GetDefaultXlsFileName(sExcelFile))
            return;

        //創建進行存取的字符串
        sSql.Format(L"DRIVER={%s};DSN='';FIRSTROWHASNAMES=1;READONLY=FALSE;CREATE_DB=\"%s\";DBQ=%s", sDriver, sExcelFile, sExcelFile);

        // 創建數據庫 (既Excel表格文件)
        if (database.OpenEx(sSql, CDatabase::noOdbcDialog)) //開啟資料庫
        {
            //創建結構表
            int i;
            LVCOLUMN columnData;
            CString columnName;
            int columnNum = 0;
            CString strH;
            CString strV;

            sSql = L"";
            strH = L"";
            columnData.mask = LVCF_TEXT;
            columnData.cchTextMax = 100;
            columnData.pszText = columnName.GetBuffer(100);
            for (i = 0; pList->GetColumn(i, &columnData); i++)
            {
                if (i != 0)
                {
                    sSql = sSql + L", ";
                    strH = strH + L", ";
                }
                sSql = sSql + L" " + columnData.pszText + L" INT";
                strH = strH + L" " + columnData.pszText + L" ";
            }
            columnName.ReleaseBuffer();
            columnNum = i;

            sSql = L"CREATE TABLE " + tableName + L" ( " + sSql + L" ) ";
            database.ExecuteSQL(sSql);

            //插入數據項
            int nItemIndex;
            for (nItemIndex = 0; nItemIndex<pList->GetItemCount(); nItemIndex++) {
                strV = L"";
                for (i = 0; i<columnNum; i++)
                {
                    if (i != 0)
                    {
                        strV = strV + L", ";
                    }
                    strV = strV + L" '" + pList->GetItemText(nItemIndex, i) +L"' ";
                }

                sSql = L"INSERT INTO " + tableName
                    + L" (" + strH + ")"
                    + L" VALUES(" + strV + L")";
                database.ExecuteSQL(sSql);
            }

        }

        //關閉數據庫
        database.Close();

        warningStr.Format(L"導出文件保存於:%s!", sExcelFile);
        AfxMessageBox(warningStr);
    }
}