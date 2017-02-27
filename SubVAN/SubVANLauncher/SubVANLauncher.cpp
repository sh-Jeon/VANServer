// SubVANLauncher.cpp : WinMain의 구현입니다.


#include "stdafx.h"
#include "resource.h"
#include "SubVANLauncher_i.h"

#include <Sddl.h>
#include "../Define.h"

VOID BuildSecurityAttributes(PSECURITY_ATTRIBUTES SecurityAttributes)
{
	LPTSTR sd = L"D:P(A;;GA;;;SY)(A;;GRGWGX;;;BA)(A;;GRGW;;;WD)(A;;GR;;;RC)";

	ZeroMemory(SecurityAttributes, sizeof(SECURITY_ATTRIBUTES));

	ConvertStringSecurityDescriptorToSecurityDescriptor(
		sd,
		SDDL_REVISION_1,
		&SecurityAttributes->lpSecurityDescriptor,
		NULL);

	SecurityAttributes->nLength = sizeof(SECURITY_ATTRIBUTES);
	SecurityAttributes->bInheritHandle = TRUE;
}


class CSubVANLauncherModule : public CAtlExeModuleT< CSubVANLauncherModule >
{
public :
	//DECLARE_LIBID(LIBID_SubVANLauncherLib)
	//DECLARE_REGISTRY_APPID_RESOURCEID(IDR_SUBVANLAUNCHER, "{1729C8C4-BAB6-4259-A1E1-2F8F18C71858}")
};

CSubVANLauncherModule _AtlModule;



//
extern "C" int WINAPI _tWinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, 
                                LPTSTR /*lpCmdLine*/, int nShowCmd)
{
	SECURITY_ATTRIBUTES sa;
	BuildSecurityAttributes(&sa);

	HANDLE hPipe = CreateNamedPipe(
		LAUNCHER_PIPENAME,      // pipe name
		PIPE_ACCESS_DUPLEX,       // read/write access
		PIPE_TYPE_MESSAGE |       // message type pipe
		PIPE_READMODE_MESSAGE |   // message-read mode
		PIPE_WAIT,                // blocking mode
		PIPE_UNLIMITED_INSTANCES,
		1024*3,            // output buffer size
		1024*3,            // input buffer size
		NMPWAIT_USE_DEFAULT_WAIT, //client time-out
		&sa);

	ATLTRACE(L"Create Named Pipe : %s\n", LAUNCHER_PIPENAME);

	for ( ; ; )
	{
		BOOL fConnected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);	
		if (!fConnected)
		{
			if (INVALID_HANDLE_VALUE != hPipe) CloseHandle(hPipe);

			hPipe = CreateNamedPipe(
				LAUNCHER_PIPENAME,      // pipe name
				PIPE_ACCESS_DUPLEX,       // read/write access
				PIPE_TYPE_MESSAGE |       // message type pipe
				PIPE_READMODE_MESSAGE |   // message-read mode
				PIPE_WAIT,                // blocking mode
				PIPE_UNLIMITED_INSTANCES,
				1024*3,            // output buffer size
				1024*3,            // input buffer size
				NMPWAIT_USE_DEFAULT_WAIT, //client time-out
				&sa );
			continue;
		}

		ATLTRACE(L"launch cadrmanager\n");
	
		BYTE buf[1024*3];
		DWORD dwRead;

		if (!ReadFile(hPipe, buf, 1024*3, &dwRead, NULL))
		{
			CloseHandle(hPipe);
			hPipe = INVALID_HANDLE_VALUE;
			continue;
		}

		SVC_CMD* pCmd = (SVC_CMD *)buf;
		if (pCmd->Cmd == CMD_CLOSE_PIPE)
		{
			DisconnectNamedPipe(hPipe);
			break;
		}
	
		DWORD bytesWritten = 0;
		switch (pCmd->Cmd)
		{
			case CMD_SET_PID:
			{
				DWORD dwPID = pCmd->dwPid;
				ATLTRACE(L"execute pid : %d\n", dwPID);

				TCHAR szManagerPath[MAX_PATH];
				ZeroMemory(szManagerPath, MAX_PATH);
				memcpy(szManagerPath, pCmd->szManagerPath, MAX_PATH);
				ATLTRACE(L"execute - set path : %s\n", szManagerPath);

				pCmd->Cmd = CMD_SET_PID;
				WriteFile(hPipe, pCmd, sizeof(*pCmd), &bytesWritten, NULL);

				ShellExecute(GetDesktopWindow(),L"open",szManagerPath,NULL,NULL,SW_SHOWNORMAL);   

				break;
			}	

			default:
				break;
		}
	
		FlushFileBuffers(hPipe);
		DisconnectNamedPipe(hPipe);
	}
	CloseHandle(hPipe);

    return _AtlModule.WinMain(nShowCmd);
}

