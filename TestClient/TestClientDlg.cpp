// TestClientDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TestClient.h"
#include "TestClientDlg.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



typedef std::vector<BYTE> VEC_SEND_BUF;
/////////////////////////////////////////////////////////////////////////////
// CTestClientDlg dialog

CTestClientDlg::CTestClientDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTestClientDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTestClientDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pClientSocket=NULL;
	AfxInitRichEdit();
}

void CTestClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTestClientDlg)
	DDX_Control(pDX, IDC_RECV, m_edtRecv);
	DDX_Control(pDX, IDC_UID, m_UserId);
	DDX_Control(pDX, ID_CONNECT, m_btnConnect);
	DDX_Control(pDX, IDSEND, m_btnSend);
	DDX_Control(pDX, IDC_SEND, m_edtSend);
	DDX_Control(pDX, IDC_PORT, m_strPort);
	DDX_Control(pDX, IDC_IPADDRESS, m_IpAddress);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTestClientDlg, CDialog)
	//{{AFX_MSG_MAP(CTestClientDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(ID_CONNECT, OnConnect)
	ON_BN_CLICKED(IDSEND, OnSend)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTestClientDlg message handlers

BOOL CTestClientDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTestClientDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CTestClientDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CTestClientDlg::OnConnect() 
{
	// TODO: Add your control notification handler code here
	BYTE addr[4];
	CString strip, strport;
	int nport;
	BOOL bFlag;

	m_IpAddress.GetAddress(addr[0], addr[1], addr[2], addr[3]);
	strip.Format("%u.%u.%u.%u", addr[0], addr[1], addr[2], addr[3]);
	m_strPort.GetWindowText(strport);
	nport = atoi(strport);


	// 접속할 socket을 생성. 현재 도큐먼트에 대한 포인터를 인자로 받는다. 
	m_pClientSocket = new CClientSocket(this); 
	
	// Create 시도 
	bFlag = m_pClientSocket->Create();
	if (!bFlag) {	// Socket 생성 실패 
		AfxMessageBox("Create() Failure !!!"); 
		delete m_pClientSocket;		// delete시키고 다시 초기화 
		m_pClientSocket = NULL;
		return;
	}
	
	// Connect 시도 
	bFlag = m_pClientSocket->Connect(strip, nport);
	if (!bFlag) {	// Connect 시도 실패 
		AfxMessageBox("Connect() Failure !!!");
		delete m_pClientSocket;		// delete시키고 다시 초기화 
		m_pClientSocket = NULL;
	} 	

	m_UserId.GetWindowText(UserId);
}

void CTestClientDlg::ReceiveString(char *szReceive, int byteRecv)
{
	// 멤버 변수에 서버로부터 받은 문자열을 저장. CClientSocket에서 호출하는 함수
	char msg[2048+1];

	ZeroMemory(msg, 2048+1);
	strncpy(msg,szReceive, byteRecv);	
	strcat(msg, "\r\n");
	
	int nLeft = m_edtRecv.GetTextLength();	
	int nRight = nLeft + strlen(msg);	
	m_edtRecv.SetSel(nLeft, nLeft);
	m_edtRecv.ReplaceSel(msg);

	CRect rc;
	m_edtRecv.GetRect(&rc);
	CPoint pt = m_edtRecv.GetCharPos(1000000);
	if (!rc.PtInRect(pt))
	{
		int nLine = (pt.y - rc.bottom) / 16 + 1;
		m_edtRecv.LineScroll(nLine);
	}
}

void CTestClientDlg::CloseClientSocket()
{
	if (m_pClientSocket){
		m_pClientSocket->Close();	// Socket을 Close 하고
		delete m_pClientSocket;		// 멤버 변수인 m_pClientSocket을 delete 하고
		m_pClientSocket = NULL;		// 다시 NULL로 초기화 
	}
}

/*
void _MakeCmdReqPacket(VEC_SEND_BUF *bufSend, int nTerminalID, LPCTSTR pData, int nLenData)
{

	BYTE *pData = new BYTE[550];
	DWORD len = 543;
	memcpy(pData, &(len), 4);
		
	char pVAN[] = "VAN";
	memcpy(pData + 4, pVAN, 3);

	char pSMW[] = "SMW";
	memcpy(pData + 7, pSMW, 3);

	char pTest[] = "test";
	memcpy(pData + 10, pTest, 4);


	BYTE STX = 0x02, ETX = 0x03;  //(1byte)

	BYTE* transByteData = new BYTE[nLenData];
	int transDataLen = 0;
	for (int i=0; i<nLenData; i++)
	{
		if (i % 2 == 0){
			TCHAR transData[2];
			
			transData[0] = *(pData + i);
			transData[1] = *(pData + i + 1);

			TCHAR *stop;
			DWORD hexa = _tcstol(transData, &stop, 16);
			transByteData[transDataLen++] = hexa;
		}
	}

	char LRC = MakeLRC((char*)transByteData, transDataLen);

	bufSend->push_back(STX);
	for (int i=0; i<transDataLen; i++)
	{
		bufSend->push_back(*(transByteData + i));
	}
	bufSend->push_back(LRC);
	bufSend->push_back(ETX);

	int nSendDataLen = bufSend->size();
	int *sendlog = new int[nSendDataLen];

	CString strDexStream;
	for (int i=0;i<nSendDataLen; i++)
	{
		CString strTemp;
		strTemp.Format(L"%d", (*bufSend)[i]);
		strDexStream += strTemp;
	}

	delete transByteData;
}
*/

void _MakeCmdReqPacket(VEC_SEND_BUF *bufSend)
{
	BYTE *pData = new BYTE[560];

	CString strDataLen;
	strDataLen.Format("0553");

	//TCHAR *stop;
	//DWORD dec = _tcstoul(strDataLen, &stop, 16);
	//memcpy(pData, &(dec), 2);

	//strDataLen.Format("43");	
	//dec = _tcstoul(strDataLen, &stop, 16);
	//memcpy(pData + 2, &(dec), 2);
	
	memcpy(pData, strDataLen.GetBuffer(0), 4);

	char pVAN[] = "VAN";
	memcpy(pData + 4, pVAN, 3);

	char pSMW[] = "SMW";
	memcpy(pData + 7, pSMW, 3);

    char pTEL[] = "2019200000030R000   2017020812014046061722               20170208120140000000000000048서울대중계　　　　　　　　　　　　　　　　　　　　72020402477230009942000000007500                                                                                                                                                                                                                                                                                                                                                                                          ";
	memcpy(pData + 10, pTEL, sizeof(pTEL));

	for (int i=0; i<557; i++)
	{
		bufSend->push_back(*(pData + i));
	}
;
	delete pData;
}

void CTestClientDlg::OnSend() 
{
	// TODO: Add your control notification handler code here
	// Server에 보낼 문자열을 입력받는다. 
	// 입력한 글자보다 한 글자 큰 크기로 데이터를 전송 (NULL 문자 때문)
	CString msg;
	m_edtSend.GetWindowText(msg);

	UpdateData(FALSE);
	//CString strSendString;
	//strSendString.Format("%-8s : %s", UserId, msg);

	// TODO: Add extra initialization here
	//m_edtSend.SetWindowText(lpstrSample);

	VEC_SEND_BUF bufSend;
	_MakeCmdReqPacket(&bufSend);

	int nLen = sizeof(bufSend.size());
	if (m_pClientSocket) m_pClientSocket->Send(&(bufSend[0]), bufSend.size());
	else{
		AfxMessageBox("연결되지 않은 상태임");
	}

	m_edtSend.SetWindowText("");
	m_edtSend.SetFocus();
}

void CTestClientDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	CloseClientSocket();
}
