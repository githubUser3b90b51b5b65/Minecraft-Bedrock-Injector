#include "framework.h"
#include "getrelate.h"
#pragma once
extern const wchar_t* processName;
// 获取进程ID
DWORD GetMCProcID(const wchar_t* processName) {
    DWORD processId = 0; // 定义用于存储进程ID的变量，默认为0
    // 创建系统快照，包括所有当前运行的进程
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnapshot != INVALID_HANDLE_VALUE) { // 非空句柄（检查快照句柄是否有效）
        PROCESSENTRY32 pe; // 定义进程条目结构体
        pe.dwSize = sizeof(PROCESSENTRY32); // 设置结构体大小

        if (Process32First(hSnapshot, &pe)) { // 获取快照中的第一个进程
            do { // 使用_wcsicmp比较进程名称，不区分大小写
                if (_wcsicmp(pe.szExeFile, processName) == 0) {
                    processId = pe.th32ProcessID; // 若进程名匹配则赋值ProcID
                    break;
                }
            } while (Process32Next(hSnapshot, &pe)); // 继续获取下个进程
        }

        CloseHandle(hSnapshot); // 关闭快照句柄
    }

    return processId;
}
// 导出DLL资源文件
BOOL ExtractResource(const HINSTANCE hInstance, WORD resourceID, LPCSTR szFilename)
{
    BOOL bSuccess = FALSE;
    HRSRC hResource = FindResource(hInstance, MAKEINTRESOURCE(resourceID), _T("BINARY"));
    if (hResource)
    {
        HGLOBAL hLoadedResource = LoadResource(hInstance, hResource);
        if (hLoadedResource)
        {
            // 获取资源大小和数据指针
            DWORD dwSize = SizeofResource(hInstance, hResource);
            LPVOID lpResourceData = LockResource(hLoadedResource);
            if (lpResourceData)
            {
                // 创建输出文件并写入数据
                std::ofstream outputFile(szFilename, std::ios::binary);
                if (outputFile.is_open())
                {
                    outputFile.write((const char*)lpResourceData, dwSize);
                    outputFile.close();
                    bSuccess = TRUE;
                }
            }
            FreeResource(hLoadedResource);
        }
    }
    return bSuccess;
}
// 通过 PID 查找进程名
BOOL GetProcessNameByPID(DWORD dwPID, TCHAR* szProcessName, DWORD nSize)
{

    BOOL bSuccess = FALSE;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(hSnapshot, &pe32))
        {
            do
            {
                if (pe32.th32ProcessID == dwPID)
                {
                    _tcsncpy_s(szProcessName, nSize, pe32.szExeFile, _TRUNCATE);
                    bSuccess = TRUE;
                    break;
                }
            } while (Process32Next(hSnapshot, &pe32));
        }
        CloseHandle(hSnapshot);
    }
    return bSuccess;
}
// 导入DLL
wchar_t* OnImportDLL(HWND hWnd) {
    OPENFILENAME ofn;
    wchar_t szFile[260] = { 0 };
    wchar_t* filePath = NULL;

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t); // 注意这里要除以wchar_t的大小
    ofn.lpstrFilter = L"Dynamic Link Library (*.dll)\0*.dll\0";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn)) {
        size_t len = wcslen(szFile) + 1; // '\0'
        filePath = (wchar_t*)malloc(len * sizeof(wchar_t));
        if (filePath != NULL) {
            errno_t err = wcscpy_s(filePath, len, szFile);
            if (err != 0) {
                // 处理错误，例如释放分配的内存并将filePath设置为none
                free(filePath);
                filePath = NULL;
            }
        }
    }
    return filePath; // 返回动态分配的内存指针
}