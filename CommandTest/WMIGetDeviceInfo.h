//#pragma warning(disable : 4995)
#include <comutil.h>
#include <Wbemidl.h>

#pragma comment (lib, "comsuppw.lib")
#pragma comment(lib, "wbemuuid.lib")
struct HDInformation {
	CString DeviceNumber;
};
class CWMIGetDeviceInfo
{
	
public:
	CWMIGetDeviceInfo();
	~CWMIGetDeviceInfo();
private:
	HRESULT hres;
	IWbemLocator *pLoc = NULL;
	IWbemServices *pSvc = NULL;
	IEnumWbemClassObject* pEnumerator = NULL;
	IWbemClassObject *pclsObj = NULL;
	
public:
	BOOL WMI_Initialization();
	HDInformation WMI_GetHDInformation();
	BOOL WMI_Free();
	void DoWithHDSerialNumber(TCHAR *SerialNumber, UINT uSize);
};

