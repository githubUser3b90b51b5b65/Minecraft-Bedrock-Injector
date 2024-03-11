#include "framework.h"
#pragma once
DWORD GetMCProcID(const wchar_t* processName);
int ExtractResource(const HINSTANCE hInstance, WORD resourceID, LPCSTR szFilename);
BOOL GetProcessNameByPID(DWORD dwPID, TCHAR* szProcessName, DWORD nSize);
wchar_t* OnImportDLL(HWND hwnd);