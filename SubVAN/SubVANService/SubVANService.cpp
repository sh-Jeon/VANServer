// SubVANService.cpp : WinMain�� �����Դϴ�.


#include "stdafx.h"
#include "resource.h"
#include "SubVANService_i.h"
#include "ManagerMonitor.h"

#include <stdio.h>

class CSubVANServiceModule : public CAtlServiceModuleT< CSubVANServiceModule, IDS_SERVICENAME >
{
public :
	DECLARE_LIBID(LIBID_SubVANServiceLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_SUBVANSERVICE, "{E559178B-AF5A-4B37-82B3-7616F6D35CAA}")

	bool ParseCommandLine ( LPCTSTR lpCmdLine, HRESULT* pnRetCode ) throw();	
	HRESULT PostMessageLoop() throw();
	HRESULT PreMessageLoop ( int nShowCmd ) throw();

	void RunMessageLoop() throw();
	BOOL Install() throw();

	void ServiceMain ( DWORD dwArgc, LPTSTR* lpszArgv ) throw();
	HRESULT Run ( int nShowCmd = SW_HIDE ) throw();
	HRESULT RegisterAppId ( bool bService = false ) throw();

	HRESULT InitializeSecurity() throw()
	{
		// TODO : CoInitializeSecurity�� ȣ���ϰ� ���񽺿� 
		// �ùٸ� ���� ������
		// �����Ͻʽÿ�. PKT ���� ����, 
		// RPC_C_IMP_LEVEL_IDENTIFY ���� ���� ���� 
		// �� NULL�� �ƴ� ������ ���� ������ ���� �����ϸ� �˴ϴ�.

		return S_OK;
	}
};

CSubVANServiceModule _AtlModule;



//
extern "C" int WINAPI _tWinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, 
                                LPTSTR /*lpCmdLine*/, int nShowCmd)
{
    return _AtlModule.WinMain(nShowCmd);
}


void CSubVANServiceModule::ServiceMain ( DWORD dwArgc, LPTSTR* lpszArgv ) throw()
{
	lpszArgv;
	dwArgc;

	// Register the control request handler
	m_status.dwCurrentState = SERVICE_START_PENDING;
	m_hServiceStatus = RegisterServiceCtrlHandler ( m_szServiceName, _Handler );
	
	if ( m_hServiceStatus == NULL )
	{
		LogEvent ( _T ( "Handler not installed" ) );
		return;
	}
	
	SetServiceStatus(SERVICE_START_PENDING);
	
	m_status.dwWin32ExitCode = S_OK;
	m_status.dwCheckPoint = 0;
	m_status.dwWaitHint = 0;
	
	// When the Run function returns, the service has stopped.
	ATLTRACE(L"run service\n");
	m_status.dwWin32ExitCode = Run ( SW_HIDE );
	SetServiceStatus ( SERVICE_STOPPED );
}

HRESULT CSubVANServiceModule::Run ( int nShowCmd ) throw()
{		
	HRESULT hr = S_OK;	
	hr = PreMessageLoop ( nShowCmd );	
	ATLTRACE(L"PreMessageLoop return %d\n", hr);
	if ( hr == S_OK )
	{
		if ( m_bService )
		{
			SetServiceStatus(SERVICE_RUNNING);

			g_ManagerMonitor.init(GetCurrentThreadId());
		}		

		ATLTRACE(L"run msg loop\n");
		RunMessageLoop();
	}
	
	g_ManagerMonitor.Close();

	if ( SUCCEEDED ( hr ) )
	{
		hr = PostMessageLoop();
	}
	ATLTRACE(L"exit run\n");
	
	return hr;
}

void CSubVANServiceModule::RunMessageLoop() throw()
{
	MSG msg;
	
	while ( GetMessage ( &msg, 0, 0, 0 ) > 0 )
	{
		if ( msg.message == WM_APP + 999 )
		{
		}

		if ( msg.message == WM_APP + 998 )
		{
		}
		
		TranslateMessage ( &msg );
		DispatchMessage ( &msg );
	}
}

HRESULT CSubVANServiceModule::RegisterAppId ( bool bService ) throw()
{
	if (!Uninstall())
	{
		return E_FAIL;
	}

	HRESULT hr = UpdateRegistryAppId ( TRUE );
	
	if ( FAILED ( hr ) )
	{
		return hr;
	}

	CRegKey keyAppID;
	LONG lRes = keyAppID.Open ( HKEY_CLASSES_ROOT, _T ( "AppID" ), KEY_WRITE );
	if ( lRes != ERROR_SUCCESS )
	{
		return AtlHresultFromWin32 ( lRes );
	}

	CRegKey key;
	lRes = key.Create ( keyAppID, GetAppIdT() );
	if ( lRes != ERROR_SUCCESS )
	{
		return AtlHresultFromWin32 ( lRes );
	}

	key.DeleteValue ( _T ( "LocalService" ) );
	
	if ( !bService )
	{
		return S_OK;
	}

	key.SetStringValue ( _T ( "LocalService" ), m_szServiceName );
	
	// Create service
	if ( !Install() )
	{
		return E_FAIL;
	}

	return S_OK;
}

BOOL CSubVANServiceModule::Install() throw()
{
	if (IsInstalled())
	{
		SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if ( hSCM )
		{
			SC_HANDLE hService = ::OpenService(hSCM, m_szServiceName, SC_MANAGER_ALL_ACCESS);
			if ( hService != NULL )
			{
				CloseServiceHandle ( hService );
				CloseServiceHandle ( hSCM );		
				return TRUE;
			}			
		}
	}

	// Get the executable file path
	TCHAR szFilePath[MAX_PATH + _ATL_QUOTES_SPACE];
	DWORD dwFLen = ::GetModuleFileName(NULL, szFilePath + 1, MAX_PATH);
	if ( dwFLen == 0 || dwFLen == MAX_PATH )
	{
		return FALSE;
	}

	// Quote the FilePath before calling CreateService
	szFilePath[0] = _T ( '\"' );
	szFilePath[dwFLen + 1] = _T ( '\"' );
	szFilePath[dwFLen + 2] = 0;
	
	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if ( hSCM == NULL )
	{
		return FALSE;
	}
	
	DWORD dwSVCType = SERVICE_AUTO_START;
	SC_HANDLE hService = ::CreateService(
		hSCM, m_szServiceName, m_szServiceName,
		SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
		dwSVCType, SERVICE_ERROR_NORMAL,
		szFilePath, NULL, NULL, L"RPCSS\0", NULL, NULL);

	if ( hService == NULL )
	{
		::CloseServiceHandle(hSCM);
		return FALSE;
	}
	
	::CloseServiceHandle(hService);	
	::CloseServiceHandle(hSCM);

	//_SetServiceDesc();

	return TRUE;
}

HRESULT CSubVANServiceModule::PostMessageLoop() throw()
{
	return S_OK;
}

HRESULT CSubVANServiceModule::PreMessageLoop ( int /*nShowCmd*/ ) throw()
{
	HRESULT hr = S_OK;
	
	if ( m_bService )
	{
		m_dwThreadID = GetCurrentThreadId();
		
		hr = InitializeSecurity();
		
		if ( FAILED ( hr ) )
		{
			return hr;
		}
	}
	return hr;
}

bool CSubVANServiceModule::ParseCommandLine ( LPCTSTR lpCmdLine, HRESULT* pnRetCode ) throw()
{
	*pnRetCode = S_OK;
	ATLTRACE(L"parse command line : %s", lpCmdLine);
	return __super::ParseCommandLine ( lpCmdLine, pnRetCode );
}