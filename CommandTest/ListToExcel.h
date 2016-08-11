#pragma once
CString		GetExcelDriver();
BOOL		MakeSurePathExists(CString &Path, bool FilenameIncluded = true);
BOOL        GetDefaultXlsFileName(CString& sExcelFile);
void		ExportListToExcel(CListCtrl* pList, CString strTitle);