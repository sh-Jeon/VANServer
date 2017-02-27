
// SubVANServer.h : PROJECT_NAME 응용 프로그램에 대한 주 헤더 파일입니다.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH에 대해 이 파일을 포함하기 전에 'stdafx.h'를 포함합니다."
#endif

#include "resource.h"		// 주 기호입니다.
#include "ADODB.h"

// CSubVANServerApp:
// 이 클래스의 구현에 대해서는 SubVANServer.cpp을 참조하십시오.
//

class CSubVANServerApp : public CWinAppEx
{
public:
	CSubVANServerApp();

	void AddLogW(__format_string LPCWSTR fmt, ...);	
	void AddLog(char *fmt, ...);


	CADODB *GetDBManager() {
		return &m_dbManager;
	}
	
	// 재정의입니다.
public:
	virtual BOOL InitInstance();

// 구현입니다.
	HANDLE s_hLogMutex;
	CString m_strLogPath;

	DECLARE_MESSAGE_MAP()


private:
	CADODB m_dbManager;
};

extern CSubVANServerApp theApp;
extern CString g_strRegPath;