#include "StdAfx.h"
#include "VANClient.h"
#include "SubVANServer.h"
#include "posclient.h"
#include "telpacket.h"
#include <strsafe.h>
#include <atlpath.h>
#include "SubVANServerDlg.h"

// PosClient로부터 전송받은 패킷 파싱 & VAN으로 변경
CVANClient::CVANClient(void)
{
	m_dwRequestListRow = -1;
	m_dwResponseListRow = -1;

	TCHAR szServerIP[15];
	DWORD dwRet = GetPrivateProfileStringW(L"VAN_INFO", L"ServerIP", _T(""), szServerIP, sizeof(szServerIP), g_strRegPath);
	if (dwRet) m_strServerIP = szServerIP;
	m_strClientIP = szServerIP;  // response의 db 기록용

	TCHAR szPort[10];
	dwRet = GetPrivateProfileStringW(L"VAN_INFO", L"Port", _T(""), szPort, sizeof(szPort), g_strRegPath);
	if (dwRet) m_nSvrPort = _tstol(szPort);	

	m_hWaitVANProcess = CreateEvent(NULL, TRUE, FALSE, NULL);
}

// POS --> PG : 전문번호 제일 앞자리는 9, 
// PG --> VAN : 9를 0으로 대체
// 전문추적번호 : 8자리,  POS-->PG : POS별,일자별로 유니크(POS+pos번호+일련번호 - P0100001)
//                        PG-->VAN : (BB######)로 유니크한 넘버 생성 후 전송 (일련번호 index 생성해서 전송) - 응답(FILER)에 앞 8자리에 넣어서 response to POS.
// POS --> PG : tcp/ip header  : 길이4자리 + SYM으로 올라옴.
PG_ERROR_CODE CVANClient::MakeVANRequestData(CTELPacket *packet)
{
	if (NULL == m_pRequest) {
		m_pRequest = new CTELPacket;
	}
	*m_pRequest = *packet; // POS에서 올라온 data를 일단 복사함.

	memcpy(m_pRequest->m_pktHeader.VAN, "VAN", 3);

	memcpy(m_pRequest->mSystemType, "SMW", 3);
	memcpy(m_pRequest->mVANCode, "201", 3);  //VAN사 구분코드 "201"
	
	m_pRequest->mTelReqType[0] = '0';   // PG --> VAN : 9를 0으로 대체


	// POS Data가공 후 VAN으로 전송 함.
	char TID[19];
	_snprintf_s(TID, 18, "%018d", m_PGTraceNO);
	
	memcpy(m_pRequest->mTID, m_pRequest->mPurchaseDate, 8);
	memcpy(m_pRequest->mTID + 8, m_pRequest->mPurchaseTime, 6);
	memcpy(m_pRequest->mTID, m_pRequest->mTraceNo, 8);
	memcpy(m_pRequest->mTID, TID, 18);

	char traceNO[9];
	_snprintf_s(traceNO, 8, "BB%06d", m_PGTraceNO);
	memcpy(m_pRequest->mTraceNo, traceNO, 8);

	ST_MONITOR_LIST_INFO *pListInfo = new ST_MONITOR_LIST_INFO;
	pListInfo->strIP = m_strServerIP;
	pListInfo->index.Format(L"%08d", m_PGTraceNO);
	pListInfo->strDirection = L"PG->VAN";

	CSubVANServerDlg *pMonitorDlg = (CSubVANServerDlg*)AfxGetApp()->m_pMainWnd;
	m_dwRequestListRow = pMonitorDlg->m_monList.GetItemCount();

	AfxGetApp()->m_pMainWnd->PostMessage(WM_ADD_REQUEST_ITEM, (WPARAM)-1, (LPARAM)pListInfo);

	return ERROR_OK;
}

CVANClient::~CVANClient(void)
{
}

void CVANClient::_assignFieldValue(int *pos, char* pdata, char *value, int nLen)
{
	if (value == NULL) { //filler
		for (int idx=0; idx<nLen; idx++) {
			*(pdata + *pos + idx) = ' ';
		}
		*pos += nLen;
		return;
	}

	memcpy(pdata + *pos, value, nLen);
	*pos += nLen;
}

void CVANClient::_MakeCmdReqPacket(char *pData)
{	
	int pos=0;
	_assignFieldValue(&pos, pData, m_pRequest->m_pktHeader.lenData, 4);
	_assignFieldValue(&pos, pData, m_pRequest->m_pktHeader.VAN, 3);
	//_assignFieldValue(&pos, pData, m_pRequest->mSystemType, 3);
	_assignFieldValue(&pos, pData, "SMW", 3);
	_assignFieldValue(&pos, pData, m_pRequest->mVANCode, 3);
	_assignFieldValue(&pos, pData, m_pRequest->mTelReqType, 4);
	_assignFieldValue(&pos, pData, m_pRequest->mTelCode, 6);
	_assignFieldValue(&pos, pData, &m_pRequest->mSendFlag, 1);
	_assignFieldValue(&pos, pData, m_pRequest->mStatus, 3);
	_assignFieldValue(&pos, pData, m_pRequest->mTelRespType, 3);
	_assignFieldValue(&pos, pData, m_pRequest->mSendDate, 8);
	_assignFieldValue(&pos, pData, m_pRequest->mSendTime, 6);
	_assignFieldValue(&pos, pData, m_pRequest->mTraceNo, 8);
	_assignFieldValue(&pos, pData, NULL, 15);


	TR_TYPE trType = CVANProtocol::checkTRFromat(m_pRequest->mTelCode);
	if (CMD_PURCHASE == trType) {
		_assignFieldValue(&pos, pData, m_pRequest->mPurchaseDate, 8);
		_assignFieldValue(&pos, pData, m_pRequest->mPurchaseTime, 6);
		_assignFieldValue(&pos, pData, m_pRequest->mPosNo, 15);
		_assignFieldValue(&pos, pData, m_pRequest->mPosName, 50);
		_assignFieldValue(&pos, pData, m_pRequest->mBarcode, 20);
		_assignFieldValue(&pos, pData, m_pRequest->mPrice, 12);
		_assignFieldValue(&pos, pData, m_pRequest->mTID, 40);
		_assignFieldValue(&pos, pData, m_pRequest->mAcceptNo, 13);
		_assignFieldValue(&pos, pData, m_pRequest->mResponseMessage, 256);
		_assignFieldValue(&pos, pData, NULL, 70);
	}

	if (CMD_COUPON == trType) {
		_assignFieldValue(&pos, pData, m_pRequest->mPurchaseDate, 8);
		_assignFieldValue(&pos, pData, m_pRequest->mPurchaseTime, 6);
		_assignFieldValue(&pos, pData, m_pRequest->mPosNo, 15);
		_assignFieldValue(&pos, pData, m_pRequest->mPosName, 50);
		_assignFieldValue(&pos, pData, m_pRequest->mBarcode, 20);
		_assignFieldValue(&pos, pData, m_pRequest->mPrice, 12);
		_assignFieldValue(&pos, pData, m_pRequest->mTID, 40);
		_assignFieldValue(&pos, pData, m_pRequest->mAcceptNo, 13);
		_assignFieldValue(&pos, pData, m_pRequest->mResponseMessage, 256);
		_assignFieldValue(&pos, pData, m_pRequest->mProductCode, 20);
		_assignFieldValue(&pos, pData, NULL, 50);
	}
	
	if (CMD_REFUND == trType) {
		_assignFieldValue(&pos, pData, m_pRequest->mPurchaseDate, 8);
		_assignFieldValue(&pos, pData, m_pRequest->mPurchaseTime, 6);
		_assignFieldValue(&pos, pData, m_pRequest->mPosNo, 15);
		_assignFieldValue(&pos, pData, m_pRequest->mPosName, 50);
		_assignFieldValue(&pos, pData, m_pRequest->mTID, 40);
		_assignFieldValue(&pos, pData, m_pRequest->mOrgPurchaseDate, 8);
		_assignFieldValue(&pos, pData, m_pRequest->mOrgPurchasePrice, 12);		
		_assignFieldValue(&pos, pData, m_pRequest->mOrgTID, 40);
		_assignFieldValue(&pos, pData, m_pRequest->mAcceptNo, 13);
		_assignFieldValue(&pos, pData, m_pRequest->mResponseMessage, 256);
		_assignFieldValue(&pos, pData, NULL, 42);
	}

	if (CMD_COUPON_STAUTS == trType) {
		_assignFieldValue(&pos, pData, m_pRequest->mPurchaseDate, 8);
		_assignFieldValue(&pos, pData, m_pRequest->mPurchaseTime, 6);
		_assignFieldValue(&pos, pData, &m_pRequest->mCouponStatus, 1);
		_assignFieldValue(&pos, pData, m_pRequest->mPosNo, 15);
		_assignFieldValue(&pos, pData, m_pRequest->mPosName, 50);
		_assignFieldValue(&pos, pData, m_pRequest->mBarcode, 20);
		_assignFieldValue(&pos, pData, m_pRequest->mPrice, 12);
		_assignFieldValue(&pos, pData, m_pRequest->mTID, 40);
		_assignFieldValue(&pos, pData, m_pRequest->mAcceptNo, 13);
		_assignFieldValue(&pos, pData, m_pRequest->mAcceptDateTime, 14);
		_assignFieldValue(&pos, pData, m_pRequest->mResponseMessage, 256);
		_assignFieldValue(&pos, pData, m_pRequest->mProductCode, 20);
		_assignFieldValue(&pos, pData, NULL, 35);
	}
}

BOOL CVANClient::SendRequestToVAN(CPosClient *pClient)
{
	USES_CONVERSION;

	char *pSendData = new char[LEN_TEL+1];
	ZeroMemory(pSendData, LEN_TEL+1);
	_MakeCmdReqPacket(pSendData);
	theApp.AddLog("PG->VAN:%s", pSendData);

	int nTotalSent = 0;
	while (nTotalSent < LEN_TEL) {
		int nSent = send(m_hSock, pSendData + nTotalSent, LEN_TEL - nTotalSent, 0);
		nTotalSent += nSent;
		if (nSent == SOCKET_ERROR) break;
	}

	ST_MONITOR_LIST_INFO *pListInfo = new ST_MONITOR_LIST_INFO;
	char szListData[20];

	ZeroMemory(szListData, 20);
	memcpy(szListData, m_pRequest->mPosNo, 15);
	pListInfo->strPOSTelNO = A2W(szListData);

	ZeroMemory(szListData, 20);
	memcpy(szListData, m_pRequest->mTraceNo, 8);
	pListInfo->strPGTelNO = A2W(szListData);

	ZeroMemory(szListData, 20);
	memcpy(szListData, m_pRequest->mTelReqType, 4);
	pListInfo->strTelType = A2W(szListData);

	ZeroMemory(szListData, 20);
	memcpy(szListData, m_pRequest->mTelCode, 6);
	pListInfo->strTelType += L"/";
	pListInfo->strTelType += A2W(szListData);
	
	pListInfo->strTel =	A2W(pSendData);

	AfxGetApp()->m_pMainWnd->PostMessage(WM_ADD_REQUEST_ITEM, (WPARAM)m_dwRequestListRow, (LPARAM)pListInfo);

	delete pSendData;

	return TRUE;
}

BOOL CVANClient::OnReceive(int nErrorCode)
{
	USES_CONVERSION;

	char buf[LEN_TEL+1];
	ZeroMemory(buf, LEN_TEL+1);
	int nRecvTotal = 0;
	while(1)
	{
		int nRecv = recv(m_hSock, (char*)buf, sizeof(buf), 0);
		nRecvTotal += nRecv;
		if (FALSE == m_bConnected) return FALSE;
		if( nRecv == 0 )
		{
			break;
		}

		if (nRecv == SOCKET_ERROR)
		{
			if (GetLastError() != WSAEWOULDBLOCK)
			{
				break;
			}
		}

		PG_ERROR_CODE result = CVANProtocol::CheckHeader(nRecvTotal, buf);
		if (ERROR_OK == result)
		{
			ST_MONITOR_LIST_INFO *pListInfo = new ST_MONITOR_LIST_INFO;
			pListInfo->strIP = m_strServerIP;
			pListInfo->index.Format(L"%08d", m_PGTraceNO);
			pListInfo->strDirection = L"VAN->PG";

			CSubVANServerDlg *pMonitorDlg = (CSubVANServerDlg*)AfxGetApp()->m_pMainWnd;
			m_dwResponseListRow = pMonitorDlg->m_monList.GetItemCount();
			AfxGetApp()->m_pMainWnd->PostMessage(WM_ADD_RESPONSE_ITEM, (WPARAM)-1, (LPARAM)pListInfo);

			result = SetResponseData(buf, nRecvTotal);
			if (ERROR_OK == result) {			
			
			} else {
					// invalid format.
			}

			break;
		}
	}

	ST_MONITOR_LIST_INFO *pListInfo = new ST_MONITOR_LIST_INFO;

	char szListData[20];

	ZeroMemory(szListData, 20);
	memcpy(szListData, m_pResponse->mPosNo, 15);
	pListInfo->strPOSTelNO = A2W(szListData);

	ZeroMemory(szListData, 20);
	memcpy(szListData, m_pResponse->mTraceNo, 8);
	pListInfo->strPGTelNO = A2W(szListData);

	ZeroMemory(szListData, 20);
	memcpy(szListData, m_pResponse->mTelReqType, 4);
	pListInfo->strTelType = A2W(szListData);

	ZeroMemory(szListData, 20);
	memcpy(szListData, m_pResponse->mTelCode, 6);
	pListInfo->strTelType += L"/";
	pListInfo->strTelType += A2W(szListData);

	ZeroMemory(szListData, 20);
	memcpy(szListData, m_pResponse->mTelRespType, 3);
	pListInfo->strRespCode = A2W(szListData);

	if (0 == strncmp(m_pResponse->mTelCode, "000310", 6)) {
		pListInfo->strRespCode += L"/";
		pListInfo->strRespCode += L"[";
		pListInfo->strRespCode += m_pResponse->mCouponStatus;
		pListInfo->strRespCode += L"]";
	}
	
	pListInfo->strTel =	A2W(buf);
	AfxGetApp()->m_pMainWnd->PostMessage(WM_ADD_RESPONSE_ITEM, (WPARAM)m_dwResponseListRow, (LPARAM)pListInfo);

	Close();
	theApp.AddLog("VAN->PG:%s", buf);
	SetEvent(m_hWaitVANProcess);

	return TRUE;
}

void CVANClient::OnClose(int nErrorCode)
{
	SetEvent(m_hWaitVANProcess);
}
