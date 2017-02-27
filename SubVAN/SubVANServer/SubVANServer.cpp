
// SubVANServer.cpp : 응용 프로그램에 대한 클래스 동작을 정의합니다.
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


// CSubVANServerApp 생성

CSubVANServerApp::CSubVANServerApp()
{
	// TODO: 여기에 생성 코드를 추가합니다.
	// InitInstance에 모든 중요한 초기화 작업을 배치합니다.
	s_hLogMutex = INVALID_HANDLE_VALUE;
}


// 유일한 CSubVANServerApp 개체입니다.

CSubVANServerApp theApp;
CString g_strRegPath;


// CSubVANServerApp 초기화

BOOL CSubVANServerApp::InitInstance()
{
	// 응용 프로그램 매니페스트가 ComCtl32.dll 버전 6 이상을 사용하여 비주얼 스타일을
	// 사용하도록 지정하는 경우, Windows XP 상에서 반드시 InitCommonControlsEx()가 필요합니다.
	// InitCommonControlsEx()를 사용하지 않으면 창을 만들 수 없습니다.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 응용 프로그램에서 사용할 모든 공용 컨트롤 클래스를 포함하도록
	// 이 항목을 설정하십시오.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	AfxEnableControlContainer();


	// 표준 초기화
	// 이들 기능을 사용하지 않고 최종 실행 파일의 크기를 줄이려면
	// 아래에서 필요 없는 특정 초기화
	// 루틴을 제거해야 합니다.
	// 해당 설정이 저장된 레지스트리 키를 변경하십시오.
	// TODO: 이 문자열을 회사 또는 조직의 이름과 같은
	// 적절한 내용으로 수정해야 합니다.
	SetRegistryKey(_T("로컬 응용 프로그램 마법사에서 생성된 응용 프로그램"));

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
		// TODO: 여기에 [확인]을 클릭하여 대화 상자가 없어질 때 처리할
		//  코드를 배치합니다.
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: 여기에 [취소]를 클릭하여 대화 상자가 없어질 때 처리할
		//  코드를 배치합니다.
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