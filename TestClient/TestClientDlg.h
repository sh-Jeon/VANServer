// TestClientDlg.h : header file
//

#if !defined(AFX_TESTCLIENTDLG_H__4054A81F_0973_4266_9E72_FE671C1040E6__INCLUDED_)
#define AFX_TESTCLIENTDLG_H__4054A81F_0973_4266_9E72_FE671C1040E6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"
#include "ClientSocket.h"

/////////////////////////////////////////////////////////////////////////////
// CTestClientDlg dialog
class CClientSocket;

class CTestClientDlg : public CDialog
{
// Construction
public:
	CString UserId;
	CString m_strIPAddress;			// 연결할 서버의 IP Address
	UINT m_nPort;					// 연결할 서버의 Port 번호
	CString m_strID;					// 사용자의 ID
	CClientSocket *m_pClientSocket;	// 서버와 연결될 client socket을 생성 

	void CloseClientSocket();
	void ReceiveString(char* szReceive, int byteRecv);
	CTestClientDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CTestClientDlg)
	enum { IDD = IDD_TESTCLIENT_DIALOG };
	CRichEditCtrl	m_edtRecv;
	CEdit	m_UserId;
	CButton	m_btnConnect;
	CButton	m_btnSend;
	CEdit	m_edtSend;
	CEdit	m_strPort;
	CIPAddressCtrl	m_IpAddress;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestClientDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CTestClientDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnConnect();
	afx_msg void OnSend();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTCLIENTDLG_H__4054A81F_0973_4266_9E72_FE671C1040E6__INCLUDED_)
