
// SubVANServer.cpp : ���� ���α׷��� ���� Ŭ���� ������ �����մϴ�.
//

#include "stdafx.h"
#include "SubVANServer.h"
#include "SubVANServerDlg.h"
#include "../define.h"
#include <ATLPath.h>
#include <strsafe.h>
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSubVANServerApp

BEGIN_MESSAGE_MAP(CSubVANServerApp, CWinAppEx)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CSubVANServerApp ����

CSubVANServerApp::CSubVANServerApp()
{
	// TODO: ���⿡ ���� �ڵ带 �߰��մϴ�.
	// InitInstance�� ��� �߿��� �ʱ�ȭ �۾��� ��ġ�մϴ�.
	s_hLogMutex = INVALID_HANDLE_VALUE;
}


// ������ CSubVANServerApp ��ü�Դϴ�.

CSubVANServerApp theApp;
CString g_strRegPath;


// CSubVANServerApp �ʱ�ȭ

BOOL CSubVANServerApp::InitInstance()
{
	// ���� ���α׷� �Ŵ��佺Ʈ�� ComCtl32.dll ���� 6 �̻��� ����Ͽ� ���־� ��Ÿ����
	// ����ϵ��� �����ϴ� ���, Windows XP �󿡼� �ݵ�� InitCommonControlsEx()�� �ʿ��մϴ�.
	// InitCommonControlsEx()�� ������� ������ â�� ���� �� �����ϴ�.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ���� ���α׷����� ����� ��� ���� ��Ʈ�� Ŭ������ �����ϵ���
	// �� �׸��� �����Ͻʽÿ�.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	AfxEnableControlContainer();


	// ǥ�� �ʱ�ȭ
	// �̵� ����� ������� �ʰ� ���� ���� ������ ũ�⸦ ���̷���
	// �Ʒ����� �ʿ� ���� Ư�� �ʱ�ȭ
	// ��ƾ�� �����ؾ� �մϴ�.
	// �ش� ������ ����� ������Ʈ�� Ű�� �����Ͻʽÿ�.
	// TODO: �� ���ڿ��� ȸ�� �Ǵ� ������ �̸��� ����
	// ������ �������� �����ؾ� �մϴ�.
	SetRegistryKey(_T("���� ���� ���α׷� �����翡�� ������ ���� ���α׷�"));

	TCHAR szModulePath[MAX_PATH];
	GetModuleFileName(NULL, szModulePath, MAX_PATH);
	ATLPath::RemoveFileSpec(szModulePath);
	g_strRegPath = szModulePath;
	g_strRegPath += L"\\";
	g_strRegPath += PLIST_INI;

	TCHAR szLogPath[MAX_PATH];
	if (::GetPrivateProfileString(L"LOG", L"LogFilePath", _T(""), szLogPath, sizeof(szLogPath), g_strRegPath))
	{
		m_strLogPath = szLogPath;
	} else {
		m_strLogPath = szModulePath;
	}
	m_strLogPath += L"\\log";

	CreateDirectory(m_strLogPath, 0);

	CString strLauncher;
	strLauncher = szModulePath;
	strLauncher += L"\\SubVANLauncher.exe";

	HANDLE hLauncher = OpenNamedPipeHandle(LAUNCHER_PIPENAME, 10000);
	if (hLauncher == INVALID_HANDLE_VALUE)
	{
		ShellExecute(GetDesktopWindow(), L"open", strLauncher, NULL, NULL, SW_HIDE); 
	}	
	else
	{
		CloseHandle(hLauncher);
	}

	CSubVANServerDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: ���⿡ [Ȯ��]�� Ŭ���Ͽ� ��ȭ ���ڰ� ������ �� ó����
		//  �ڵ带 ��ġ�մϴ�.
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: ���⿡ [���]�� Ŭ���Ͽ� ��ȭ ���ڰ� ������ �� ó����
		//  �ڵ带 ��ġ�մϴ�.
	}

	return FALSE;
}

int GetAnsiByteSize(__in LPCWSTR uniStr)
{
	return ::WideCharToMultiByte(CP_ACP, 0, uniStr, -1, NULL, 0, NULL, NULL);
}

std::string ToAnsiStr(__in LPCWSTR uniStr)
{
	int needLength = GetAnsiByteSize(uniStr);
	std::vector<CHAR> ansiBuf;
	ansiBuf.resize(needLength);
	CHAR* pAnsiStr = &ansiBuf[0];
	ZeroMemory(pAnsiStr, needLength);
	::WideCharToMultiByte(CP_ACP, 0, uniStr, EOF, pAnsiStr, needLength, "?", NULL);

	std::string ansiStrBuf = pAnsiStr;
	return ansiStrBuf;
}

void _WriteLog(LPCTSTR szFilePath, char *strLog)
{
	CSubVANServerDlg *pDlg = (CSubVANServerDlg*)AfxGetApp()->m_pMainWnd;
	DWORD m_dwTraceNO = pDlg->m_PGServer.GetCurrentTraceNO();

	SYSTEMTIME cTime;
	::GetLocalTime(&cTime);
	
	//GetCurrentThreadId()
	char szMsg[4096];
	sprintf_s(szMsg, "[%08X][%02d:%02d:%02d] %s\r\n", m_dwTraceNO, cTime.wHour, cTime.wMinute, cTime.wSecond, strLog);

	HANDLE hFile = INVALID_HANDLE_VALUE;
	if (FALSE == ATLPath::FileExists(szFilePath))
	{
		hFile = ::CreateFile(
			szFilePath,
			GENERIC_WRITE,
			FILE_SHARE_WRITE | FILE_SHARE_READ,
			NULL,
			CREATE_ALWAYS,
			0,
			NULL);
	}
	else
	{
		hFile = ::CreateFile(
				szFilePath,
				GENERIC_WRITE,
				FILE_SHARE_WRITE | FILE_SHARE_READ,
				NULL,
				OPEN_EXISTING,
				0,
				NULL);
		::SetFilePointer(hFile, 0, 0, FILE_END);
	}

	DWORD dwWritten=0;
	//std::string s = ToAnsiStr(strText);
	(VOID)WriteFile(hFile, szMsg, (DWORD)strlen(szMsg), &dwWritten, 0);
	//(VOID)WriteFile(hFile,strText, (DWORD)strText.GetLength(), &dwWritten, 0);
	::FlushFileBuffers(hFile);
		
	CloseHandle(hFile);
}

void _WriteLogW(LPCTSTR szFilePath, LPCTSTR strLog)
{
	SYSTEMTIME cTime;
	::GetLocalTime(&cTime);
	
	CString strLogTime;
	strLogTime.Format(L"[%08X][%02d:%02d:%02d] ", GetCurrentThreadId(), cTime.wHour, cTime.wMinute, cTime.wSecond);

	CString strText = strLogTime;
	strText += strLog;
	strText += L"\r\n";

	HANDLE hFile = INVALID_HANDLE_VALUE;
	if (FALSE == ATLPath::FileExists(szFilePath))
	{
		hFile = ::CreateFile(
			szFilePath,
			GENERIC_WRITE,
			FILE_SHARE_WRITE | FILE_SHARE_READ,
			NULL,
			CREATE_ALWAYS,
			0,
			NULL);
	}
	else
	{
		hFile = ::CreateFile(
				szFilePath,
				GENERIC_WRITE,
				FILE_SHARE_WRITE | FILE_SHARE_READ,
				NULL,
				OPEN_EXISTING,
				0,
				NULL);
		::SetFilePointer(hFile, 0, 0, FILE_END);
	}

	DWORD dwWritten=0;
	std::string s = ToAnsiStr(strText);
	(VOID)WriteFile(hFile, s.c_str(), (DWORD)s.size(), &dwWritten, 0);
	//(VOID)WriteFile(hFile,strText, (DWORD)strText.GetLength(), &dwWritten, 0);
	::FlushFileBuffers(hFile);
		
	CloseHandle(hFile);
}

void CSubVANServerApp::AddLog(char *fmt, ...) 
{
	va_list argList;
	va_start(argList, fmt);
	char szMsg[4096];
	vsprintf_s(szMsg, fmt, argList);
	va_end(argList);

	if (s_hLogMutex == INVALID_HANDLE_VALUE)
	{
		s_hLogMutex = CreateMutexW(NULL, FALSE, L"LogLock");
		if (s_hLogMutex == NULL)
		{
			return;
		}
	}

	DWORD dwRet = WaitForSingleObject(s_hLogMutex, 10000);
	if (dwRet == WAIT_OBJECT_0)
	{
		CString strLogFilePath = m_strLogPath;
		CString strLogFileName;

		SYSTEMTIME cTime;
		::GetLocalTime(&cTime);

		strLogFileName.Format(L"%4d-%02d-%02d_%02d.log", cTime.wYear, cTime.wMonth, cTime.wDay, cTime.wHour);

		strLogFilePath += L"\\";
		strLogFilePath += strLogFileName;

		_WriteLog(strLogFilePath, szMsg);

	}
	ReleaseMutex(s_hLogMutex);
}

void CSubVANServerApp::AddLogW(__format_string LPCWSTR fmt, ...)
{
	va_list argList;
	va_start(argList, fmt);
	WCHAR szMsg[4096];
	StringCchVPrintfW(szMsg, _countof(szMsg), fmt, argList);
	va_end(argList);

	if (s_hLogMutex == INVALID_HANDLE_VALUE)
	{
		s_hLogMutex = CreateMutexW(NULL, FALSE, L"LogLock");
		if (s_hLogMutex == NULL)
		{
			return;
		}
	}

	DWORD dwRet = WaitForSingleObject(s_hLogMutex, 10000);
	if (dwRet == WAIT_OBJECT_0)
	{
		CString strLogFilePath = m_strLogPath;
		CString strLogFileName;

		SYSTEMTIME cTime;
		::GetLocalTime(&cTime);

		strLogFileName.Format(L"%4d-%02d-%02d_%02d.log", cTime.wYear, cTime.wMonth, cTime.wDay, cTime.wHour);

		strLogFilePath += L"\\";
		strLogFilePath += strLogFileName;

		_WriteLogW(strLogFilePath, szMsg);

	}
	ReleaseMutex(s_hLogMutex);
}