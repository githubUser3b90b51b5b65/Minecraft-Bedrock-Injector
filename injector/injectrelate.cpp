#include "injectrelate.h"
#include "framework.h"
#pragma once
int performInjection(DWORD procId, const char* dllPath) {
    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, 0, procId);

    if (hProc && hProc != INVALID_HANDLE_VALUE)
    {
        void* loc = VirtualAllocEx(hProc, 0, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

        WriteProcessMemory(hProc, loc, dllPath, strlen(dllPath) + 1, 0);

        HANDLE hThread = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, loc, 0, 0);

        if (hThread)
        {
            CloseHandle(hThread);
        }
    }
    if (hProc)
    {
        CloseHandle(hProc);
    }
    return 0;
}

void SetAccessControl(std::wstring& ExecutableName, const wchar_t* AccessString)
{
    PSECURITY_DESCRIPTOR SecurityDescriptor = nullptr;
    EXPLICIT_ACCESSW ExplicitAccess = { 0 };

    ACL* AccessControlCurrent = nullptr;
    ACL* AccessControlNew = nullptr;

    SECURITY_INFORMATION SecurityInfo = DACL_SECURITY_INFORMATION;
    PSID SecurityIdentifier = nullptr;

    if (
        GetNamedSecurityInfoW(
            ExecutableName.c_str(),
            SE_FILE_OBJECT,
            DACL_SECURITY_INFORMATION,
            nullptr,
            nullptr,
            &AccessControlCurrent,
            nullptr,
            &SecurityDescriptor
        ) == ERROR_SUCCESS
        )
    {
        ConvertStringSidToSidW(AccessString, &SecurityIdentifier);
        if (SecurityIdentifier != nullptr)
        {
            ExplicitAccess.grfAccessPermissions = GENERIC_READ | GENERIC_EXECUTE;
            ExplicitAccess.grfAccessMode = SET_ACCESS;
            ExplicitAccess.grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
            ExplicitAccess.Trustee.TrusteeForm = TRUSTEE_IS_SID;
            ExplicitAccess.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
            ExplicitAccess.Trustee.ptstrName = reinterpret_cast<wchar_t*>(SecurityIdentifier);

            if (
                SetEntriesInAclW(
                    1,
                    &ExplicitAccess,
                    AccessControlCurrent,
                    &AccessControlNew
                ) == ERROR_SUCCESS
                )
            {
                SetNamedSecurityInfoW(
                    const_cast<wchar_t*>(ExecutableName.c_str()),
                    SE_FILE_OBJECT,
                    SecurityInfo,
                    nullptr,
                    nullptr,
                    AccessControlNew,
                    nullptr
                );
            }
        }
    }
    if (SecurityDescriptor)
    {
        LocalFree(reinterpret_cast<HLOCAL>(SecurityDescriptor));
    }
    if (AccessControlNew)
    {
        LocalFree(reinterpret_cast<HLOCAL>(AccessControlNew));
    }
}