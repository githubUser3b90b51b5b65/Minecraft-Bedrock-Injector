#pragma once
#include "framework.h"
int performInjection(DWORD procId, const char* dllPath);
void SetAccessControl(std::wstring& ExecutableName, const wchar_t* AccessString);