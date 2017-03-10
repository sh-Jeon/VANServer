
// SubVANServerDlg.h : ��� ����
//

#pragma once

#include "PGServer.h"
#include "afxcmn.h"
#include "afxwin.h"

#include "DailyBatchJob.h"

#define WM_TRAYICON				WM_APP + 110	// Ʈ���� ������ �޽���
#define WM_CONNECT_DB			WM_APP + 500

// CSubVANServerDlg ��ȭ ����
class CSubVANServerDlg : public CDialog
{
// �����Դϴ�.
public:
	CSubVANServerDlg(CWnd* pParent = NULL);	// ǥ�� �������Դϴ�.

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_SUBVANSERVER_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �����Դϴ�.

// �����Դϴ�.
protected:
	HICON m_hIcon;

	// ������ �޽��� �� �Լ�
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedCancel();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	LRESULT OnTrayIcon(WPARAM wParam, LPARAM lParam);
	LRESULT OnNewSerialNo(WPARAM wParam, LPARAM lParam);
	LRESULT OnAddRequestListItem(WPARAM wParam, LPARAM lParam);
	LRESULT OnAddResponseListItem(WPARAM wParam, LPARAM lParam);
	LRESULT OnTryToConnectDB(WPARAM wParam, LPARAM lParam);

	CListCtrl m_monList;
	CStatic m_infoLabel;
	CString getInfoLabel(void);

	CPGServer m_PGServer;

	virtual void OnOK();
	virtual void OnCancel();


	virtual void OnTrayRButtonUp();

private:
	HICON m_hStIcon[4];
	int m_nIconIdx;

	CDailyBatchJob m_DailyBatch;

	void _UpdateItem(DWORD nRow, ST_MONITOR_LIST_INFO *pListItem);
	void SendAliveToService();
	void SetProcessIDToMonitorService();

	afx_msg void OnTrayOpenwindow();
	afx_msg void OnTrayExit();
public:
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CEdit m_edtRawData;
	afx_msg void OnNMDblclkMonList(NMHDR *pNMHDR, LRESULT *pResult);
	void CheckDoBatchJob();
	afx_msg void OnBnClickedBatchTest();
	afx_msg void OnBnClickedBillTest();
};