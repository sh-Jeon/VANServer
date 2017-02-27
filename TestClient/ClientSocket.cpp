// ClientSocket.cpp : implementation file
//

#include "stdafx.h"
#include "ClientSocket.h"
#include "TestClientDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CClientSocket

CClientSocket::CClientSocket(CTestClientDlg* pDoc)
{
	//메인 프로그램의 도큐먼트를 이용하여 멤버 변수를 초기화
	m_pDoc = pDoc; 
}

CClientSocket::~CClientSocket()
{
	m_pDoc = NULL;
}


// Do not edit the following lines, which are needed by ClassWizard.
#if 0
BEGIN_MESSAGE_MAP(CClientSocket, CSocket)
	//{{AFX_MSG_MAP(CClientSocket)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
#endif	// 0

/////////////////////////////////////////////////////////////////////////////
// CClientSocket member functions

// non-blocking 상태로 받을 때 호출된다.
void CClientSocket::OnReceive(int nErrorCode) 
{
	// TODO: Add your specialized code here and/or call the base class
	int byteRecv = Receive(m_szReceiveBuffer, MAX_RECEIVE); 
	m_pDoc->ReceiveString(m_szReceiveBuffer, byteRecv);	
	
	CSocket::OnReceive(nErrorCode);
}

// non-blocking 상태로 끊기는 경우에 호출된다. 
void CClientSocket::OnClose(int nErrorCode) 
{
	// TODO: Add your specialized code here and/or call the base class
	// 실제로 Socket을 Close하는 기능을 수행하는 함수로 도큐먼트에서 하는 이유는 
	// 도큐먼트의 멤버로 선언되어 있는 CClientSocket형의 멤버 변수를 Close()와 
	// 동시에 delete 시키고 다시 NULL로 초기화 시키기 위해서이다. 
	AfxMessageBox("서버로부터 접속이 끊겼읍니다.");
	m_pDoc->CloseClientSocket(); 
	
	CSocket::OnClose(nErrorCode);
}

