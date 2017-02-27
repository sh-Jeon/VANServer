
// SubVANServer.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.
#include "ADODB.h"

// CSubVANServerApp:
// �� Ŭ������ ������ ���ؼ��� SubVANServer.cpp�� �����Ͻʽÿ�.
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
	
	// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.
	HANDLE s_hLogMutex;
	CString m_strLogPath;

	DECLARE_MESSAGE_MAP()


private:
	CADODB m_dbManager;
};

extern CSubVANServerApp theApp;
extern CString g_strRegPath;