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
	//���� ���α׷��� ��ť��Ʈ�� �̿��Ͽ� ��� ������ �ʱ�ȭ
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

// non-blocking ���·� ���� �� ȣ��ȴ�.
void CClientSocket::OnReceive(int nErrorCode) 
{
	// TODO: Add your specialized code here and/or call the base class
	int byteRecv = Receive(m_szReceiveBuffer, MAX_RECEIVE); 
	m_pDoc->ReceiveString(m_szReceiveBuffer, byteRecv);	
	
	CSocket::OnReceive(nErrorCode);
}

// non-blocking ���·� ����� ��쿡 ȣ��ȴ�. 
void CClientSocket::OnClose(int nErrorCode) 
{
	// TODO: Add your specialized code here and/or call the base class
	// ������ Socket�� Close�ϴ� ����� �����ϴ� �Լ��� ��ť��Ʈ���� �ϴ� ������ 
	// ��ť��Ʈ�� ����� ����Ǿ� �ִ� CClientSocket���� ��� ������ Close()�� 
	// ���ÿ� delete ��Ű�� �ٽ� NULL�� �ʱ�ȭ ��Ű�� ���ؼ��̴�. 
	AfxMessageBox("�����κ��� ������ �������ϴ�.");
	m_pDoc->CloseClientSocket(); 
	
	CSocket::OnClose(nErrorCode);
}

