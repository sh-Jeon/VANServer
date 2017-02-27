
// SubVANServerDlg.cpp : ���� ����
//

#include "stdafx.h"
#include "SubVANServer.h"
#include "SubVANServerDlg.h"
#include "VANProtocol.h"
#include "../define.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#define TIMER_ICON_DISPLAY	100
// ���� ���α׷� ������ ���Ǵ� CAboutDlg ��ȭ �����Դϴ�.

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

// �����Դϴ�.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CSubVANServerDlg ��ȭ ����

CSubVANServerDlg::CSubVANServerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSubVANServerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSubVANServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MON_LIST, m_monList);
	DDX_Control(pDX, IDC_INFO, m_infoLabel);
	DDX_Control(pDX, IDC_EDIT1, m_edtRawData);
}

BEGIN_MESSAGE_MAP(CSubVANServerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDCANCEL, &CSubVANServerDlg::OnBnClickedCancel)
	ON_WM_SIZE()
	ON_WM_MENUSELECT()
	ON_COMMAND(ID_TRAY_OPENWINDOW, &CSubVANServerDlg::OnTrayOpenwindow)
	ON_COMMAND(ID_TRAY_EXIT, &CSubVANServerDlg::OnTrayExit)

	ON_MESSAGE(WM_TRAYICON, OnTrayIcon)
	ON_MESSAGE(WM_APP+101, OnNewSerialNo)
	ON_MESSAGE(WM_ADD_REQUEST_ITEM, OnAddRequestListItem)
	ON_MESSAGE(WM_ADD_RESPONSE_ITEM, OnAddResponseListItem)

	ON_MESSAGE(WM_CONNECT_DB, OnTryToConnectDB)

	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_NOTIFY(NM_DBLCLK, IDC_MON_LIST, &CSubVANServerDlg::OnNMDblclkMonList)
	ON_BN_CLICKED(IDC_BUTTON2, &CSubVANServerDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BILL_TEST, &CSubVANServerDlg::OnBnClickedBillTest)
END_MESSAGE_MAP()

void CSubVANServerDlg::OnOK(){ return; }
void CSubVANServerDlg::OnCancel(){ return; }

LRESULT CSubVANServerDlg::OnTrayIcon(WPARAM wParam, LPARAM lParam)
{
	if (lParam == WM_RBUTTONDOWN)
	{
		OnTrayRButtonUp();
	}

	return 0;
}

LRESULT CSubVANServerDlg::OnNewSerialNo(WPARAM wParam, LPARAM lParam)
{
	m_infoLabel.SetWindowText(getInfoLabel());

	// ��¥�� �ٲ� ��쿡�� ����Ʈ�� ��� ����
	if (1 == wParam) {
		m_monList.DeleteAllItems();
	}

	return 0;
}

void CSubVANServerDlg::_UpdateItem(DWORD nRow, ST_MONITOR_LIST_INFO *pListItem)
{
	CString strItemText = m_monList.GetItemText(nRow, 2);
	if (pListItem->strIP.GetLength() && pListItem->strIP != strItemText) 
		m_monList.SetItemText(nRow, 2, pListItem->strIP);

	strItemText = m_monList.GetItemText(nRow, 3);
	if (pListItem->strDirection.GetLength() && pListItem->strDirection != strItemText) 
		m_monList.SetItemText(nRow, 3, pListItem->strDirection);

	strItemText = m_monList.GetItemText(nRow, 4);
	if (pListItem->strTelType.GetLength() && pListItem->strTelType != strItemText) 
		m_monList.SetItemText(nRow, 4, pListItem->strTelType);

	strItemText = m_monList.GetItemText(nRow, 5);
	if (pListItem->strPOSTelNO.GetLength() && pListItem->strPOSTelNO != strItemText) 
		m_monList.SetItemText(nRow, 5, pListItem->strPOSTelNO);

	strItemText = m_monList.GetItemText(nRow, 6);
	if (pListItem->strPGTelNO.GetLength() && pListItem->strPGTelNO != strItemText) 
		m_monList.SetItemText(nRow, 6, pListItem->strPGTelNO);

	strItemText = m_monList.GetItemText(nRow, 7);
	if (pListItem->strRespCode.GetLength() && pListItem->strRespCode != strItemText) 
		m_monList.SetItemText(nRow, 7, pListItem->strRespCode);

	strItemText = m_monList.GetItemText(nRow, 8);
	if (pListItem->strTel.GetLength() && pListItem->strTel != strItemText) 
		m_monList.SetItemText(nRow, 8, pListItem->strTel);
}

LRESULT CSubVANServerDlg::OnAddRequestListItem(WPARAM wParam, LPARAM lParam)
{
	ST_MONITOR_LIST_INFO *pListItem = (ST_MONITOR_LIST_INFO *)lParam;
	DWORD dwRow = (DWORD)wParam;
	if (pListItem) {
		if (-1 == dwRow) {
			SYSTEMTIME cTime;
			::GetLocalTime(&cTime);

			CString strCurrentTime;
			strCurrentTime.Format(L"%2d:%02d:%02d %03d", cTime.wHour, cTime.wMinute, cTime.wSecond, cTime.wMilliseconds);
			
			dwRow = m_monList.InsertItem(m_monList.GetItemCount(), pListItem->index);
			m_monList.SetItemText(dwRow, 1, strCurrentTime);
		} 
		
		_UpdateItem(dwRow, pListItem);

		delete pListItem;
	}

	return 0;
}

LRESULT CSubVANServerDlg::OnAddResponseListItem(WPARAM wParam, LPARAM lParam)
{
	ST_MONITOR_LIST_INFO *pListItem = (ST_MONITOR_LIST_INFO *)lParam;

	DWORD dwRow = (DWORD)wParam;

	if (pListItem) {
		if (-1 == dwRow) { 
			SYSTEMTIME cTime;
			::GetLocalTime(&cTime);

			CString strCurrentTime;
			strCurrentTime.Format(L"%2d:%02d:%02d %03d", cTime.wHour, cTime.wMinute, cTime.wSecond, cTime.wMilliseconds);
			
			dwRow = m_monList.InsertItem(m_monList.GetItemCount(), pListItem->index);
			m_monList.SetItemText(dwRow, 1, strCurrentTime);
			m_monList.SetItemText(dwRow, 2, pListItem->strIP);
		}

		_UpdateItem(dwRow, pListItem);

		delete pListItem;
	}

	return 0;
}

LRESULT CSubVANServerDlg::OnTryToConnectDB(WPARAM wParam, LPARAM lParam)
{
	if (FALSE == theApp.GetDBManager()->Open()) {
		::MessageBox(NULL, L"DB ������ ������ �� �����ϴ�.", L"Error", MB_OK);
#ifndef _DEBUG
		PostMessage(WM_QUIT, 0, 0);
#endif
	}

	SetTimer(TIMER_ICON_DISPLAY, 500, NULL);

	return 0;
}

void CSubVANServerDlg::SetProcessIDToMonitorService()
{
	//Named pipe�� ���� process id�� �����Ͽ� ������ �� �ְ� �Ѵ�.
	HANDLE hSVC = OpenNamedPipeHandle(MONITOR_SVC_PIPENAME, 10000);
	if (hSVC == INVALID_HANDLE_VALUE)
	{
		return;
	}

	DWORD dwBytesWritten;
	SVC_CMD svcCmd;
	svcCmd.Cmd = CMD_SET_PID;
	svcCmd.dwPid = _getpid();

	TCHAR szModulePath[MAX_PATH];
	GetModuleFileName(NULL, szModulePath, MAX_PATH);

	StringCbCopy(svcCmd.szManagerPath, sizeof(svcCmd.szManagerPath), szModulePath);
	BOOL bWrite = WriteFile(hSVC, &svcCmd, sizeof(svcCmd), &dwBytesWritten, 0);
	if (!bWrite)
	{
		CloseHandle(hSVC);
		return;
	}

	FlushFileBuffers(hSVC);

	DWORD dwRead;
	ZeroMemory(&svcCmd, sizeof(SVC_CMD));
	if (!ReadFile(hSVC, &svcCmd, sizeof(SVC_CMD), &dwRead, 0))
	{
		CloseHandle(hSVC);
		return;
	}

	CloseHandle(hSVC);
}

BOOL CSubVANServerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// IDM_ABOUTBOX�� �ý��� ��� ������ �־�� �մϴ�.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// �� ��ȭ ������ �������� �����մϴ�. ���� ���α׷��� �� â�� ��ȭ ���ڰ� �ƴ� ��쿡��
	//  �����ӿ�ũ�� �� �۾��� �ڵ����� �����մϴ�.
	SetIcon(m_hIcon, TRUE);			// ū �������� �����մϴ�.
	SetIcon(m_hIcon, FALSE);		// ���� �������� �����մϴ�.

	SetWindowText(L"��ũ�Ӵ� �߰輭��");
	TCHAR szTitle[255];
	if (::GetPrivateProfileString(L"GROUP_INFO", L"AppTitle", _T(""), szTitle, sizeof(szTitle), g_strRegPath))
	{
		SetWindowText(szTitle);
	}	

	SetWindowPos(NULL, 0, 0, 800, 450, SWP_NOACTIVATE);
	CenterWindow();

	m_infoLabel.SetWindowText(getInfoLabel());

	for (int i=0;i<4;i++)
	{
		m_hStIcon[i] = (HICON)LoadImage(theApp.m_hInstance, MAKEINTRESOURCE(IDI_ICON1 + i),
										IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	}

	CString strTooltip;
	strTooltip.Format(L"BANK MONEY ���� ��");

	NOTIFYICONDATA nid;
	
	ZeroMemory (&nid, sizeof(NOTIFYICONDATA));
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = this->m_hWnd;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.uCallbackMessage = WM_TRAYICON;
	lstrcpy (nid.szTip, strTooltip);
	nid.uID = 0;
	nid.hIcon = m_hStIcon[0];
	Shell_NotifyIcon (NIM_ADD, &nid);
	m_nIconIdx = 1;

	CenterWindow();

	TCHAR* pszColumnName[] = { L"Index", L"Time", L"IP", L"Direction", L"TYPE", L"������ ��ȣ", L"����������ȣ", L"Response", L"RAW Data" };
	for (int i=0; i<_countof(pszColumnName); i++)
	{
		LVCOLUMN lvc;
		lvc.mask	= LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvc.fmt		= LVCFMT_LEFT;
		lvc.pszText = pszColumnName[i];
		lvc.cx = 100;

		if (i==0) lvc.cx = 80;
		else if (i==3) lvc.cx = 70;
		else if (i==4) lvc.cx = 100;
		else if (i==5) lvc.cx = 130;
		else if (i==6) lvc.cx = 90;
		else if (i==7) lvc.cx = 70;
		else if (i==8) lvc.cx = 70;
		lvc.iSubItem = i;
		m_monList.InsertColumn(i, &lvc);
	}
	m_monList.SetExtendedStyle(m_monList.GetExtendedStyle() | LVS_EX_FULLROWSELECT);	
	m_monList.SetFocus();

	theApp.AddLog("Server is Starting...");

	m_PGServer.InitServer();

	SetProcessIDToMonitorService();

	PostMessage(WM_CONNECT_DB, 0, 0);

	return TRUE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
}


void CSubVANServerDlg::OnTrayRButtonUp()
{
	// Get the menu position
	CPoint pos;
	GetCursorPos(&pos);

	// Load the menu
	CMenu oMenu;
	if ( FALSE == oMenu.LoadMenu(IDR_MENU_TRAY))
	{
		return;
	}

	MENUITEMINFO minfo = {0};
	minfo.cbSize = sizeof(minfo);
	minfo.fMask = MIIM_FTYPE | MIIM_ID;

	this->SetForegroundWindow();

	// Track
	oMenu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN, pos.x, pos.y, this);

	// BUGFIX: See "PRB: Menus for Notification Icons Don't Work Correctly"
	if (::IsWindow(m_hWnd)) PostMessage(WM_NULL);

	// Done
	oMenu.DestroyMenu();
}


// ��ȭ ���ڿ� �ּ�ȭ ���߸� �߰��� ��� �������� �׸�����
//  �Ʒ� �ڵ尡 �ʿ��մϴ�. ����/�� ���� ����ϴ� MFC ���� ���α׷��� ��쿡��
//  �����ӿ�ũ���� �� �۾��� �ڵ����� �����մϴ�.

void CSubVANServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // �׸��⸦ ���� ����̽� ���ؽ�Ʈ

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Ŭ���̾�Ʈ �簢������ �������� ����� ����ϴ�.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �������� �׸��ϴ�.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// ����ڰ� �ּ�ȭ�� â�� ���� ���ȿ� Ŀ���� ǥ�õǵ��� �ý��ۿ���
//  �� �Լ��� ȣ���մϴ�.
HCURSOR CSubVANServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CSubVANServerDlg::OnBnClickedCancel()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	ShowWindow(SW_HIDE);
}

void CSubVANServerDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	if (::IsWindow(m_monList)) {
		m_monList.SetWindowPos(NULL, 0, 30, cx, cy-130, SWP_NOACTIVATE);

		//RECT rcOwner;
		//this->GetWindowRect(&rcOwner); 
		//m_monList.SetWindowPos(NULL, 0, 30, rcOwner.right, rcOwner.bottom - 130, SWP_NOACTIVATE);

		RECT rcList;
		m_monList.GetWindowRect(&rcList);
		m_edtRawData.SetWindowPos(NULL, 0, cy - 100, cx, 100, SWP_NOACTIVATE);
	}
}

CString CSubVANServerDlg::getInfoLabel(void)
{
	SYSTEMTIME cTime;
	::GetLocalTime(&cTime);
	
	CString strTraceDate;
	strTraceDate.Format(L"%02d%02d%02d : %06u", cTime.wYear, cTime.wMonth, cTime.wDay, m_PGServer.GetCurrentTraceNO());

	return strTraceDate;
}

void CSubVANServerDlg::OnTrayOpenwindow()
{
	ShowWindow(SW_SHOW);
	SetForegroundWindow();
}

void CSubVANServerDlg::OnTrayExit()
{
	if (IDYES == ::MessageBox(NULL, L"�ݰ�� ��ũ�Ӵ� �߰輭�� ���� ���α׷��� �����Ͻðڽ��ϱ�?", L"����", MB_YESNO))
	{
		PostQuitMessage(0);
	}
}

void CSubVANServerDlg::OnDestroy()
{
	CDialog::OnDestroy();

	KillTimer(TIMER_ICON_DISPLAY);

	NOTIFYICONDATA nid;
	
	ZeroMemory (&nid, sizeof(NOTIFYICONDATA));
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = this->m_hWnd;;
	nid.uFlags = NULL;
	::Shell_NotifyIcon (NIM_DELETE, &nid);

	HANDLE hSVC = OpenNamedPipeHandle(MONITOR_SVC_PIPENAME, 10000);
	if (hSVC == INVALID_HANDLE_VALUE)
	{
		return;
	}

	DWORD dwBytesWritten;
	SVC_CMD svcCmd;
	svcCmd.Cmd = CMD_END_PROCESS;
	svcCmd.dwPid = _getpid();
	svcCmd.szManagerPath[0] = L'\0';

	BOOL bWrite = WriteFile(hSVC, &svcCmd, sizeof(svcCmd), &dwBytesWritten, 0);
	if (!bWrite)
	{
		CloseHandle(hSVC);
		return;
	}

	FlushFileBuffers(hSVC);

	DWORD dwRead;
	ZeroMemory(&svcCmd, sizeof(SVC_CMD));
	if (!ReadFile(hSVC, &svcCmd, sizeof(SVC_CMD), &dwRead, 0))
	{
		CloseHandle(hSVC);
		return;
	}

	CloseHandle(hSVC);

	theApp.GetDBManager()->Close();
}


void CSubVANServerDlg::SendAliveToService()
{
	static DWORD lastTick = GetTickCount();
	if (GetTickCount() - lastTick < 3000){
		return;
	}

	lastTick = GetTickCount();
	//Named pipe�� ���� process id�� �����Ͽ� ������ �� �ְ� �Ѵ�.
	HANDLE hSVC = OpenNamedPipeHandle(MONITOR_SVC_PIPENAME, 10000);
	if (hSVC == INVALID_HANDLE_VALUE)
	{
		return;
	}

	DWORD dwBytesWritten;
	SVC_CMD svcCmd;
	svcCmd.Cmd = CMD_ALIVE;
	svcCmd.dwPid = _getpid();
	svcCmd.szManagerPath[0] = L'\0';
	BOOL bWrite = WriteFile(hSVC, &svcCmd, sizeof(svcCmd), &dwBytesWritten, 0);
	if (!bWrite)
	{
		CloseHandle(hSVC);
		return;
	}

	FlushFileBuffers(hSVC);

	DWORD dwRead;
	ZeroMemory(&svcCmd, sizeof(SVC_CMD));
	if (!ReadFile(hSVC, &svcCmd, sizeof(SVC_CMD), &dwRead, 0))
	{
		CloseHandle(hSVC);
		return;
	}

	CloseHandle(hSVC);
}

void CSubVANServerDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == TIMER_ICON_DISPLAY)
	{
		NOTIFYICONDATA nid;
		ZeroMemory (&nid, sizeof(NOTIFYICONDATA));
		nid.cbSize = sizeof(NOTIFYICONDATA);
		nid.hWnd = this->m_hWnd;;
		nid.hIcon	= m_hStIcon[m_nIconIdx++];
		nid.uFlags = NIF_ICON;
		::Shell_NotifyIcon(NIM_MODIFY, &nid);
		if (m_nIconIdx == 4)
		{
			m_nIconIdx = 0;
		}

		SendAliveToService();
		CheckDoBatchJob();
	}	

	CDialog::OnTimer(nIDEvent);
}

void CSubVANServerDlg::OnNMDblclkMonList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	if (pNMItemActivate->iItem != -1)
    {
        CString str = m_monList.GetItemText(pNMItemActivate->iItem, 8);
        m_edtRawData.SetWindowTextW(str);
    }

	*pResult = 0;
}

void CSubVANServerDlg::CheckDoBatchJob()
{
	static DWORD lastTick = GetTickCount();

#ifdef _DEBUG
	if (GetTickCount() - lastTick > 15000) {
#else
	if (GetTickCount() - lastTick > 60000 * 5) {
#endif
		lastTick = GetTickCount();

		SYSTEMTIME cTime;
		::GetLocalTime(&cTime);

		if (cTime.wHour == 0 && cTime.wMinute > 30 && FALSE == m_DailyBatch.m_bDoneBatchProcess) {
			m_DailyBatch.ProcessBatchJob();
		}

		if (cTime.wHour == 1 && TRUE == m_DailyBatch.m_bDoneBatchProcess) {
			m_DailyBatch.m_bDoneBatchProcess = FALSE;
		}

		if (cTime.wHour == 6 && cTime.wMinute >= 0 && FALSE == m_DailyBatch.m_bDoneBillProcess) {
			m_DailyBatch.ProcessGenerateVanBillList();
		}

		if (cTime.wHour == 7 && TRUE == m_DailyBatch.m_bDoneBillProcess) {
			m_DailyBatch.m_bDoneBillProcess = FALSE;
		}
	}
}

void CSubVANServerDlg::OnBnClickedButton2()
{
	m_DailyBatch.ProcessBatchJob();
}

void CSubVANServerDlg::OnBnClickedBillTest()
{
	m_DailyBatch.ProcessGenerateVanBillList();
}
