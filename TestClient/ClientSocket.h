#if !defined(AFX_CLIENTSOCKET_H__CE4E8278_8EA7_11D2_8D1C_00001C1BC187__INCLUDED_)
#define AFX_CLIENTSOCKET_H__CE4E8278_8EA7_11D2_8D1C_00001C1BC187__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ClientSocket.h : header file
//

#include "TestClientDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CClientSocket command target

#define MAX_RECEIVE 4096		// �����κ��� ���� �� �ִ� �������� �ִ� ũ�� 

class CTestClientDlg;

// ���� ���α׷��� Document�� Ŭ������ ��� ������ �����ϱ� ���� ���

class CClientSocket : public CSocket
{
// Attributes
public:

// Operations
public:
	CClientSocket(CTestClientDlg* m_pDoc);
	virtual ~CClientSocket();

	void operator =(const CClientSocket &CCliSrc);

public:
	// ���� ���α׷��� ��ť��Ʈ�� ���� �����͸� ��� ������ ���� 
	CTestClientDlg* m_pDoc;

// Overrides
public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CClientSocket)
	public:
	virtual void OnReceive(int nErrorCode);
	virtual void OnClose(int nErrorCode);
	//}}AFX_VIRTUAL

	// Generated message map functions
	//{{AFX_MSG(CClientSocket)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

// Implementation
protected:

private:
	// �����κ��� ���� �ڷḦ ������ �� ������ ���� 
	char m_szReceiveBuffer[MAX_RECEIVE];
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CLIENTSOCKET_H__CE4E8278_8EA7_11D2_8D1C_00001C1BC187__INCLUDED_)
