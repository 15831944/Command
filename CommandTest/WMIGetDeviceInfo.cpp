#include "stdafx.h"
#include "WMIGetDeviceInfo.h"
#include <strsafe.h>
//#include <algorithm>
CWMIGetDeviceInfo::CWMIGetDeviceInfo()
{
	//初始化參數
	pLoc = NULL;
	pSvc = NULL;
	pEnumerator = NULL;
	pclsObj = NULL;
}


CWMIGetDeviceInfo::~CWMIGetDeviceInfo()
{
}

BOOL CWMIGetDeviceInfo::WMI_Initialization()
{
	// Step 1: --------------------------------------------------
	// Initialize COM. ------------------------------------------

	hres = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (FAILED(hres))
	{
#ifdef PRINTF
		_cwprintf(L"Failed to initialize COM library.\n");
#endif
		//cout << "Failed to initialize COM library. Error code = 0x"
		//	<< hex << hres << endl;
		return 1;                  // Program has failed.
	}

	// Step 2: --------------------------------------------------
	// Set general COM security levels --------------------------

	hres = CoInitializeSecurity(
		NULL,
		-1,                          // COM authentication
		NULL,                        // Authentication services
		NULL,                        // Reserved
		RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
		RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
		NULL,                        // Authentication info
		EOAC_NONE,                   // Additional capabilities 
		NULL                         // Reserved
	);

	if (FAILED(hres))
	{
#ifdef PRINTF
		_cwprintf(L"Failed to initialize security.\n");
#endif
		//cout << "Failed to initialize security. Error code = 0x"
		//	<< hex << hres << endl;
		CoUninitialize();
		return 1;                    // Program has failed.
	}

	// Step 3: ---------------------------------------------------
	// Obtain the initial locator to WMI -------------------------

	hres = CoCreateInstance(
		CLSID_WbemLocator,
		0,
		CLSCTX_INPROC_SERVER,
		IID_IWbemLocator, (LPVOID *)&pLoc);

	if (FAILED(hres))
	{
#ifdef PRINTF
		_cwprintf(L"Failed to create IWbemLocator object.\n");
#endif
		//cout << "Failed to create IWbemLocator object."
		//	<< " Err code = 0x"
		//	<< hex << hres << endl;
		CoUninitialize();//Close the COM library.
		return 1;                 // Program has failed.
	}

	// Step 4: -----------------------------------------------------
	// Connect to WMI through the IWbemLocator::ConnectServer method

	// Connect to the root\cimv2 namespace with
	// the current user and obtain pointer pSvc
	// to make IWbemServices calls.
	hres = pLoc->ConnectServer(
		_bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
		NULL,                    // User name. NULL = current user
		NULL,                    // User password. NULL = current
		0,                       // Locale. NULL indicates current
		NULL,                    // Security flags.
		0,                       // Authority (for example, Kerberos)
		0,                       // Context object 
		&pSvc                    // pointer to IWbemServices proxy
	);

	if (FAILED(hres))
	{
#ifdef PRINTF
		_cwprintf(L"Could not connect.");
#endif
		//cout << "Could not connect. Error code = 0x"
		//	<< hex << hres << endl;
		pLoc->Release();
		CoUninitialize();//Close the COM library.
		return 1;                // Program has failed.
	}

#ifdef PRINTF
	_cwprintf(L"Connected to ROOT\\CIMV2 WMI namespace.\n");
#endif
	//cout << "Connected to ROOT\\CIMV2 WMI namespace" << endl;

	// Step 5: --------------------------------------------------
	// Set security levels on the proxy -------------------------

	hres = CoSetProxyBlanket(
		pSvc,                        // Indicates the proxy to set
		RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
		RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
		NULL,                        // Server principal name 
		RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
		RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
		NULL,                        // client identity
		EOAC_NONE                    // proxy capabilities 
	);

	if (FAILED(hres))
	{
#ifdef PRINTF
		_cwprintf(L"Could not set proxy blanket.\n");
#endif
		//cout << "Could not set proxy blanket. Error code = 0x"
		//	<< hex << hres << endl;
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();//Close the COM library.
		return 1;               // Program has failed.
	}

	// Step 6: --------------------------------------------------
	// Use the IWbemServices pointer to make requests of WMI ----

	// For example, get the name of the operating system
	
	hres = pSvc->ExecQuery(
		bstr_t("WQL"),
		bstr_t("SELECT * FROM Win32_DiskDrive WHERE (SerialNumber IS NOT NULL) AND (MediaType LIKE 'Fixed hard disk%')"),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		NULL,
		&pEnumerator);

	if (FAILED(hres))
	{
		//cout << "Query for operating system name failed."
		//	<< " Error code = 0x"
		//	<< hex << hres << endl;
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return 1;               // Program has failed.
	}

	return 0;
}

HDInformation CWMIGetDeviceInfo::WMI_GetHDInformation()
{
	// Step 7: -------------------------------------------------
	// Get the data from the query in step 6 -------------------
	HDInformation Result;

	ULONG uReturn = 0;
	TCHAR strResult[128] = L"";

	while (pEnumerator)
	{
		HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
			&pclsObj, &uReturn);

		if (0 == uReturn)
		{
			break;
		}

		VARIANT vtProp;

		// Get the value of the Name property
		hr = pclsObj->Get(L"SerialNumber", 0, &vtProp, 0, 0);
		StringCchCopy(strResult, 128, W2T(vtProp.bstrVal));
		DoWithHDSerialNumber(strResult, 128);
		VariantClear(&vtProp);

		pclsObj->Release();
		pclsObj = NULL;
	}

	Result.DeviceNumber.Format(L"%s", strResult);
	
	return Result;
}
//WMI 佇列釋放
BOOL CWMIGetDeviceInfo::WMI_Free()
{
	// Cleanup
	// ========
	if(pSvc != NULL)
		pSvc->Release();
	if (pLoc != NULL)
		pLoc->Release();
	if (pEnumerator != NULL)
		pEnumerator->Release();
	pLoc = NULL;
	pSvc = NULL;
	pEnumerator = NULL;
	CoUninitialize();
	return 0;
}
//HD序號編碼轉換
void CWMIGetDeviceInfo::DoWithHDSerialNumber(TCHAR * SerialNumber, UINT uSize)
{
	UINT	iLen;
	UINT	i;

	iLen = _tcslen(SerialNumber);
	if (iLen == 40)	// InterfaceType = "IDE"
	{	// 需要将16进制编码串转换为字符串
		TCHAR ch, szBuf[32];
		BYTE b;

		for (i = 0; i < 20; i++)
		{	// 将16进制字符转换为高4位
			ch = SerialNumber[i * 2];
			if ((ch >= '0') && (ch <= '9'))
			{
				b = ch - '0';
			}
			else if ((ch >= 'A') && (ch <= 'F'))
			{
				b = ch - 'A' + 10;
			}
			else if ((ch >= 'a') && (ch <= 'f'))
			{
				b = ch - 'a' + 10;
			}
			else
			{	// 非法字符
				break;
			}

			b <<= 4;

			// 将16进制字符转换为低4位
			ch = SerialNumber[i * 2 + 1];
			if ((ch >= '0') && (ch <= '9'))
			{
				b += ch - '0';
			}
			else if ((ch >= 'A') && (ch <= 'F'))
			{
				b += ch - 'A' + 10;
			}
			else if ((ch >= 'a') && (ch <= 'f'))
			{
				b += ch - 'a' + 10;
			}
			else
			{	// 非法字符
				break;
			}

			szBuf[i] = b;
		}

		if (i == 20)
		{	// 转换成功
			szBuf[i] = L'\0';
			StringCchCopy(SerialNumber, uSize, szBuf);
			iLen = _tcslen(SerialNumber);
		}
	}

	// 每2个字符互换位置
	for (i = 0; i < iLen; i += 2)
	{
		//std::swap(SerialNumber[i], SerialNumber[i + 1]);
	}

	// 去掉空格
	//std::remove(SerialNumber, SerialNumber + _tcslen(SerialNumber) + 1, L' ');
}
