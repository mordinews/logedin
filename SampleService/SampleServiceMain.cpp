#include <Windows.h>
#include <tchar.h>

SERVICE_STATUS        g_ServiceStatus = {0};
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE                g_ServiceStopEvent = INVALID_HANDLE_VALUE;

VOID WINAPI ServiceMain (DWORD argc, LPTSTR *argv);

LPHANDLER_FUNCTION_EX LphandlerFunctionEx;
DWORD WINAPI ServiceCtrlHandler(DWORD    dwControl,DWORD    dwEventType,LPVOID   lpEventData,LPVOID   lpContext);

DWORD WINAPI ServiceWorkerThread (LPVOID lpParam);
DWORD WINAPI ServiceRespond(LPVOID lpParam);


#define SERVICE_NAME  _T("MySampleService")

//sc create "MySampleService" binPath=C:\Users\tomer\source\repos\add\Release\SampleService.exe
//sc delete "MySampleService"





int _tmain (int argc, TCHAR *argv[])
{
    OutputDebugString(_T("My Sample Service: Main: Entry"));

    SERVICE_TABLE_ENTRY ServiceTable[] = 
    {
        {SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION) ServiceMain},
        {NULL, NULL}
    };

    if (StartServiceCtrlDispatcher (ServiceTable) == FALSE)
    {
       OutputDebugString(_T("My Sample Service: Main: StartServiceCtrlDispatcher returned error"));
       return GetLastError ();
    }

    OutputDebugString(_T("My Sample Service: Main: Exit"));
    return 0;
}


VOID WINAPI ServiceMain (DWORD argc, LPTSTR *argv)
{
    DWORD Status = E_FAIL;

    OutputDebugString(_T("My Sample Service: ServiceMain: Entry"));

    g_StatusHandle = RegisterServiceCtrlHandlerEx(SERVICE_NAME, ServiceCtrlHandler,0);

    if (g_StatusHandle == NULL) 
    {
        OutputDebugString(_T("My Sample Service: ServiceMain: RegisterServiceCtrlHandler returned error"));
        goto EXIT;
    }

    // Tell the service controller we are starting
    ZeroMemory (&g_ServiceStatus, sizeof (g_ServiceStatus));
    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_ServiceStatus.dwControlsAccepted = 0; 
    g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwServiceSpecificExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;

    if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE) 
    {
        OutputDebugString(_T("My Sample Service: ServiceMain: SetServiceStatus returned error"));
    }

    /* 
     * Perform tasks neccesary to start the service here
     */
    OutputDebugString(_T("My Sample Service: ServiceMain: Performing Service Start Operations"));

    // Create stop event to wait on later.
    g_ServiceStopEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
    if (g_ServiceStopEvent == NULL) 
    {
        OutputDebugString(_T("My Sample Service: ServiceMain: CreateEvent(g_ServiceStopEvent) returned error"));

        g_ServiceStatus.dwControlsAccepted = 0;
        g_ServiceStatus.dwCurrentState = SERVICE_STOPPED| SERVICE_ACCEPT_SESSIONCHANGE;
        g_ServiceStatus.dwWin32ExitCode = GetLastError();
        g_ServiceStatus.dwCheckPoint = 1;

        if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE)
	    {
		    OutputDebugString(_T("My Sample Service: ServiceMain: SetServiceStatus returned error"));
	    }
        goto EXIT; 
    }    

    // Tell the service controller we are started
    g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;

    if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE)
    {
	    OutputDebugString(_T("My Sample Service: ServiceMain: SetServiceStatus returned error"));
    }

    // Start the thread that will perform the main task of the service
    HANDLE hThread = CreateThread (NULL, 0, ServiceWorkerThread, NULL, 0, NULL);

    HANDLE hThread2 = CreateThread(NULL, 0, ServiceRespond, NULL, 0, NULL);

    OutputDebugString(_T("My Sample Service: ServiceMain: Waiting for Worker Thread to complete"));

    // Wait until our worker thread exits effectively signaling that the service needs to stop
    WaitForSingleObject (hThread, INFINITE);
    
    OutputDebugString(_T("My Sample Service: ServiceMain: Worker Thread Stop Event signaled"));
    
    
    /* 
     * Perform any cleanup tasks
     */
    OutputDebugString(_T("My Sample Service: ServiceMain: Performing Cleanup Operations"));

    CloseHandle (g_ServiceStopEvent);

    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 3;

    if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE)
    {
	    OutputDebugString(_T("My Sample Service: ServiceMain: SetServiceStatus returned error"));
    }
    
    EXIT:
    OutputDebugString(_T("My Sample Service: ServiceMain: Exit"));

    return;
}


DWORD WINAPI ServiceCtrlHandler(DWORD    dwControl, DWORD    dwEventType, LPVOID   lpEventData, LPVOID   lpContext)
{
    OutputDebugString(_T("My Sample Service: ServiceCtrlHandler: Entry"));

    switch (dwControl)
	{
     case SERVICE_CONTROL_STOP :

        OutputDebugString(_T("My Sample Service: ServiceCtrlHandler: SERVICE_CONTROL_STOP Request"));

        if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
           break;

        /* 
         * Perform tasks neccesary to stop the service here 
         */
        
        g_ServiceStatus.dwControlsAccepted = 0;
        g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        g_ServiceStatus.dwWin32ExitCode = 0;
        g_ServiceStatus.dwCheckPoint = 4;

        if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE)
		{
			OutputDebugString(_T("My Sample Service: ServiceCtrlHandler: SetServiceStatus returned error"));
		}

        // This will signal the worker thread to start shutting down
        SetEvent (g_ServiceStopEvent);

        break;

     case SERVICE_CONTROL_SESSIONCHANGE: //tomer
         //writeEventLog(GetDefaultTitle(), L"Service=Session=Changed");
        // applicationLogger.LogWarning("Session changed");
         if (dwEventType == WTS_SESSION_LOGOFF)
         {
             WTSSESSION_NOTIFICATION* pSessionNotification = static_cast<WTSSESSION_NOTIFICATION*>(lpEventData);
           
             OutputDebugString(_T("logoff"));
         }
         else if (dwEventType == WTS_SESSION_LOGON)
         {
             WTSSESSION_NOTIFICATION* pSessionNotification = static_cast<WTSSESSION_NOTIFICATION*>(lpEventData);
             OutputDebugString(_T("login"));
             // ...
         }
         break;

     default:
         break;
    }

    OutputDebugString(_T("My Sample Service: ServiceCtrlHandler: Exit"));
    return 0;
}


DWORD WINAPI ServiceWorkerThread (LPVOID lpParam)
{
    OutputDebugString(_T("My Sample Service: ServiceWorkerThread: Entry"));

    //  Periodically check if the service has been requested to stop
    while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0)
    {        
        /* 
         * Perform main service function here
         */

        //  Simulate some work by sleeping
        Sleep(3000);
    }

    OutputDebugString(_T("My Sample Service: ServiceWorkerThread: Exit"));

    return ERROR_SUCCESS;
}


DWORD WINAPI ServiceRespond(LPVOID lpParam)
{
    OutputDebugString(_T("ServiceRespond ServiceWorkerThread: Exit"));

    HANDLE hPipe;
    DWORD dwWritten;
    /*
    HANDLE  hConsumerCloseEvent = CreateEvent(
        NULL,               // default security attributes
        TRUE,               // manual-reset event
        FALSE,              // initial state is nonsignaled
        TEXT("Global\\ConsumerCloseEvent")  // object name
    );
  
    SECURITY_DESCRIPTOR sd;
    InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
  
    SECURITY_ATTRIBUTES sa = { 0 };
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = FALSE;
    sa.lpSecurityDescriptor = &sd;

 
    HANDLE  hConsumerCloseEvent = CreateEvent(
        &sa,               // default security attributes
        TRUE,               // manual-reset event
        FALSE,              // initial state is nonsignaled
        TEXT("Global\\ConsumerCloseEvent")  // object name
    );
    */
    hPipe = CreateNamedPipe(TEXT("\\\\.\\pipe\\Pipe"),
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,   // FILE_FLAG_FIRST_PIPE_INSTANCE is not needed but forces CreateNamedPipe(..) to fail if the pipe already exists...
        1,
        1024 * 16,
        1024 * 16,
        NMPWAIT_USE_DEFAULT_WAIT,
        NULL);

    //wait for signal from tray icone 
    //WaitForSingleObject(hConsumerCloseEvent, INFINITE);
    OutputDebugString(_T("ServiceRespond  1 ServiceWorkerThread: Exit"));



    OutputDebugString(_T("ServiceRespond  2 ServiceWorkerThread: Exit"));
    if (hPipe != INVALID_HANDLE_VALUE)
    {

        OutputDebugString(_T("ServiceRespond  3 ServiceWorkerThread: Exit"));
        if (ConnectNamedPipe(hPipe, NULL) != FALSE) {
            WriteFile(hPipe,
                "Hello Pipe user loggedin\n",
                26,   // = length of string + terminating '\0' !!!
                &dwWritten,
                NULL);
        }

        CloseHandle(hPipe);
    }

    return ERROR_SUCCESS;
}

