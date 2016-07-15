
// stdafx.cpp : 僅包含標準 Include 檔的原始程式檔
// CommandTest.pch 會成為先行編譯標頭檔
// stdafx.obj 會包含先行編譯類型資訊

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

