// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"


#include <windows.h>
#include <Winwlx.h>
#include <tchar.h>
#include <processthreadsapi.h>


wchar_t wszCommand[] = L"TrayIcon.exe";
PROCESS_INFORMATION pi = { 0 };
STARTUPINFO si = { sizeof(STARTUPINFO) };


void SendNotification()
{

    HANDLE hPipe;
    DWORD dwWritten;


    hPipe = CreateFile(TEXT("\\\\.\\pipe\\Pipe"),
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);
    if (hPipe != INVALID_HANDLE_VALUE)
    {
        WriteFile(hPipe,
            "Hello Pipe\n",
            12,   // = length of string + terminating '\0' !!!
            &dwWritten,
            NULL);

        CloseHandle(hPipe);
    }

}


//Entrance function for the DLL
BOOL WINAPI LibMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
    {
        DisableThreadLibraryCalls(hInstance);
    }
    break;
    }
    return TRUE;
}

//Logon
VOID APIENTRY StartProcessAtWinLogon(PWLX_NOTIFICATION_INFO pInfo)
{/*
    STARTUPINFO si;
    si.cb = sizeof(STARTUPINFO);
    si.lpReserved = NULL;
    si.lpTitle = NULL;
    si.lpDesktop = 0;//("WinSta0\\Default");
    si.dwX = si.dwY = si.dwXSize = si.dwYSize = 0L;
    si.dwFlags = 0;;
    si.wShowWindow = SW_SHOW;
    si.lpReserved2 = NULL;
    si.cbReserved2 = 0;

    CreateProcess(NULL, g_szPath, NULL, NULL, FALSE, CREATE_NEW_CONSOLE,
        NULL, NULL, &si, &g_pi); */
}

//Logoff
VOID APIENTRY StopProcessAtWinLogoff(PWLX_NOTIFICATION_INFO pInfo)
{
    //terminates the process
   // TerminateProcess(pi.hProcess, 0xDEADBEEF);
    //  SafeTerminateProcess(pi.hProcess, 0xDEADBEEF);
}



//StartShell
VOID APIENTRY StartProcessAtStartShell(PWLX_NOTIFICATION_INFO pInfo)
{
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOW;

    //https://docs.microsoft.com/en-us/windows/win32/secauthn/event-handler-function-prototype
    //https://wiki.sei.cmu.edu/confluence/display/c/WIN02-C.+Restrict+privileges+when+spawning+child+processes
  

 ///   if (!CreateProcessAsUser(pInfo->hToken, NULL, wszCommand, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
 ///   {

  ///  }
    SendNotification();
    
}

/* 
//This function safely terminates a process, allowing
//it to do cleanup (ie. DLL detach)
//It can be found at the Windows Developer's Journal
//SafeTerminateProcess(HANDLE hProcess, UINT uExitCode);


 //http://hyacinth.byus.net/moniwiki/wiki.php/C%2B%2B/SafeTerminateProcess
BOOL SafeTerminateProcess(HANDLE hProcess, UINT uExitCode) {
    DWORD tid, dwCode, dwErr = 0;
    HANDLE hProcessDup = INVALID_HANDLE_VALUE;
    HANDLE hRemoteThread = NULL;
    HINSTANCE hKernel = GetModuleHandleA("Kernel32");

    BOOL bSuccess = FALSE;
    BOOL bDup = DuplicateHandle(GetCurrentProcess(), hProcess, GetCurrentProcess(), &hProcessDup, PROCESS_ALL_ACCESS, FALSE, 0);

    if (GetExitCodeProcess((bDup) ? hProcessDup : hProcess, &dwCode) && (dwCode == STILL_ACTIVE)) {
        FARPROC pfnExitProc = GetProcAddress(hKernel, "ExitProcess");
        hRemoteThread = CreateRemoteThread((bDup) ? hProcessDup : hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pfnExitProc, (PVOID)uExitCode, 0, &tid);
        if (hRemoteThread == NULL)
            dwErr = GetLastError();
    }
    else {
        dwErr = ERROR_PROCESS_ABORTED;
    }

    if (hRemoteThread) {
        WaitForSingleObject((bDup) ? hProcessDup : hProcess, INFINITE);
        CloseHandle(hRemoteThread);
        bSuccess = TRUE;
    }

    if (bDup)
        CloseHandle(hProcessDup);

    if (!bSuccess)
        SetLastError(dwErr);

    return bSuccess;
}

*/