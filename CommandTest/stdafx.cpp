
// stdafx.cpp : �ȥ]�t�з� Include �ɪ���l�{����
// CommandTest.pch �|��������sĶ���Y��
// stdafx.obj �|�]�t����sĶ������T

#include "stdafx.h"
#ifdef PRINTF
void InitConsoleWindow()
{
    /*AllocConsole();
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    int hCrt = _open_osfhandle((long)handle, _O_TEXT);
    FILE * hf = _fdopen(hCrt, "w");
    *stdout = *hf;*/

    int nCrt = 0;
    FILE* fp;
    AllocConsole();
    nCrt = _open_osfhandle((long)GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT);
    fp = _fdopen(nCrt, "w");
    *stdout = *fp;
    setvbuf(stdout, NULL, _IONBF, 0);
}
#endif
CString GetCurrentPath(CString Folder)
{
    CString path;
    GetModuleFileName(NULL, path.GetBufferSetLength(MAX_PATH + 1), MAX_PATH);
    path.ReleaseBuffer();
    int pos = path.ReverseFind('\\');
    path = path.Left(pos) + Folder;
    LPTSTR lpszText = new TCHAR[path.GetLength() + 1];
    lstrcpy(lpszText, path);
    return path;
}

