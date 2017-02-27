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

#define MAX_RECEIVE 4096		// 서버로부터 받을 수 있는 데이터의 최대 크기 

class CTestClientDlg;

// 메인 프로그램의 Document를 클래스의 멤버 변수로 선언하기 위해 사용

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
	// 메인 프로그램의 도큐먼트에 대한 포인터를 멤버 변수로 선언 
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
	// 서버로부터 받은 자료를 저장해 둘 공간을 설정 
	char m_szReceiveBuffer[MAX_RECEIVE];
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CLIENTSOCKET_H__CE4E8278_8EA7_11D2_8D1C_00001C1BC187__INCLUDED_)
