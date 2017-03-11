#include "StdAfx.h"
#include "PosClient.h"
#include "SubVANServer.h"

CPosClient::CPosClient(SOCKET clSock, CPGServer *pServer)
{
	m_sock = clSock;
	m_pPGServer = pServer;

	m_dwRequestListRow = -1;
	m_dwResponseListRow = -1;
}

CPosClient::~CPosClient(void)
{

}

// Response for POS
TR_TYPE CPosClient::_MakeCmdResponsePacket(char *pData)
{
	int pos = 0;
	_assignFieldValue(&pos, pData, m_pResponse->m_pktHeader.lenData, 4);
	_assignFieldValue(&pos, pData, m_pRequest->m_pktHeader.VAN, 3);
	_assignFieldValue(&pos, pData, m_pRequest->mSystemType, 3);
	_assignFieldValue(&pos, pData, m_pResponse->mVANCode, 3);

	m_pResponse->mTelReqType[0] = '9';
	_assignFieldValue(&pos, pData, m_pResponse->mTelReqType, 4);   //9200
	
	
	// POS에서 올라온 원래 전문번호는 그대로 사용 (P000###)
	_assignFieldValue(&pos, pData, m_pRequest->mTelCode, 6);       //000030
	_assignFieldValue(&pos, pData, &m_pResponse->mSendFlag, 1);    //R
	_assignFieldValue(&pos, pData, m_pResponse->mStatus, 3);
	_assignFieldValue(&pos, pData, m_pResponse->mTelRespType, 3);
	_assignFieldValue(&pos, pData, m_pResponse->mSendDate, 8);
	_assignFieldValue(&pos, pData, m_pResponse->mSendTime, 6);
	_assignFieldValue(&pos, pData, m_pRequest->mTraceNo, 8);

	// 공통부 filler 15자리 중 앞 8자리에 PG가 생성한 전문추적 번호를 넣음.
	char PGTranceNO[16];
	_snprintf_s(PGTranceNO, sizeof(PGTranceNO), "BB%06d0000000", m_PGTraceNO);
	_assignFieldValue(&pos, pData, PGTranceNO, 15);

	TR_TYPE trType = CVANProtocol::checkTRFromat(m_pResponse->mTelCode);
	if (CMD_PURCHASE == trType) {
		_assignFieldValue(&pos, pData, m_pResponse->mPurchaseDate, 8);
		_assignFieldValue(&pos, pData, m_pResponse->mPurchaseTime, 6);
		_assignFieldValue(&pos, pData, m_pResponse->mPosNo, 15);
		_assignFieldValue(&pos, pData, m_pResponse->mPosName, 50);
		_assignFieldValue(&pos, pData, m_pResponse->mBarcode, 20);
		_assignFieldValue(&pos, pData, m_pResponse->mPrice, 12);
		_assignFieldValue(&pos, pData, m_pResponse->mTID, 40);
		_assignFieldValue(&pos, pData, m_pResponse->mAcceptNo, 13);
		_assignFieldValue(&pos, pData, m_pResponse->mResponseMessage, 256);
		_assignFieldValue(&pos, pData, NULL, 70);
	}

	if (CMD_COUPON == trType) {
		_assignFieldValue(&pos, pData, m_pResponse->mPurchaseDate, 8);
		_assignFieldValue(&pos, pData, m_pResponse->mPurchaseTime, 6);
		_assignFieldValue(&pos, pData, m_pResponse->mPosNo, 15);
		_assignFieldValue(&pos, pData, m_pResponse->mPosName, 50);
		_assignFieldValue(&pos, pData, m_pResponse->mBarcode, 20);
		_assignFieldValue(&pos, pData, m_pResponse->mPrice, 12);
		_assignFieldValue(&pos, pData, m_pResponse->mTID, 40);
		_assignFieldValue(&pos, pData, m_pResponse->mAcceptNo, 13);
		_assignFieldValue(&pos, pData, m_pResponse->mResponseMessage, 256);
		_assignFieldValue(&pos, pData, m_pResponse->mProductCode, 20);
		_assignFieldValue(&pos, pData, NULL, 50);
	}
	
	if (CMD_REFUND == trType) {
		_assignFieldValue(&pos, pData, m_pResponse->mPurchaseDate, 8);
		_assignFieldValue(&pos, pData, m_pResponse->mPurchaseTime, 6);
		_assignFieldValue(&pos, pData, m_pResponse->mPosNo, 15);
		_assignFieldValue(&pos, pData, m_pResponse->mPosName, 50);
		_assignFieldValue(&pos, pData, m_pResponse->mTID, 40);
		_assignFieldValue(&pos, pData, m_pResponse->mOrgPurchaseDate, 8);
		_assignFieldValue(&pos, pData, m_pResponse->mOrgPurchasePrice, 12);		
		_assignFieldValue(&pos, pData, m_pResponse->mOrgTID, 40);
		_assignFieldValue(&pos, pData, m_pResponse->mAcceptNo, 13);
		_assignFieldValue(&pos, pData, m_pResponse->mResponseMessage, 256);
		_assignFieldValue(&pos, pData, NULL, 42);
	}

	if (CMD_COUPON_STAUTS == trType) {
		_assignFieldValue(&pos, pData, m_pResponse->mPurchaseDate, 8);
		_assignFieldValue(&pos, pData, m_pResponse->mPurchaseTime, 6);
		_assignFieldValue(&pos, pData, &m_pResponse->mCouponStatus, 1);
		_assignFieldValue(&pos, pData, m_pResponse->mPosNo, 15);
		_assignFieldValue(&pos, pData, m_pResponse->mPosName, 50);
		_assignFieldValue(&pos, pData, m_pResponse->mBarcode, 20);
		_assignFieldValue(&pos, pData, m_pResponse->mPrice, 12);
		_assignFieldValue(&pos, pData, m_pResponse->mTID, 40);
		_assignFieldValue(&pos, pData, m_pResponse->mAcceptNo, 13);

		_assignFieldValue(&pos, pData, m_pResponse->mAcceptDateTime, 14);
		_assignFieldValue(&pos, pData, m_pResponse->mResponseMessage, 256);

		_assignFieldValue(&pos, pData, m_pResponse->mProductCode, 20);
		_assignFieldValue(&pos, pData, NULL, 35);
	}

	return trType;
}

/*
void CPosClient::SendResultToPOS(PG_ERROR_CODE errCode)
{
	char *pSendData = new char[LEN_TEL+1];
	ZeroMemory(pSendData, LEN_TEL+1);

	// error code setting
	switch (errCode) {
		case ERROR_VAN_NORESPONSE :
			memcpy(m_pResponse->mTelRespType, "S11", 3);
			break;
		case ERROR_INVALID_LENGTH :
			memcpy(m_pResponse->mTelRespType, "S12", 3);
			break;
		case ERROR_WAIT :
			memcpy(m_pResponse->mTelRespType, "S43", 3);
			break;
		case ERROR_ETC :
			memcpy(m_pResponse->mTelRespType, "S99", 3);
			break;
	}
	
	TR_TYPE trType = _MakeCmdResponsePacket(pSendData);
	theApp.AddLog("PG->POS:%s", pSendData);

	int nTotalSent = 0;
	while (nTotalSent < LEN_TEL) {
		int nSent = send(m_sock, pSendData + nTotalSent, LEN_TEL - nTotalSent, 0);
		nTotalSent += nSent;
		if (nSent == SOCKET_ERROR) break;
	}

	delete pSendData;
}
*/
void CPosClient::SendResultToPOS(RES_CODE resCode)
{
	USES_CONVERSION;

	char *pSendData = new char[LEN_TEL+1];
	ZeroMemory(pSendData, LEN_TEL+1);

	CVANProtocol::setResponseErrorCode(m_pResponse->mTelRespType, resCode);

	TR_TYPE trType = _MakeCmdResponsePacket(pSendData);
	theApp.AddLog("PG->POS:%s", pSendData);

	ST_MONITOR_LIST_INFO *pListInfo = new ST_MONITOR_LIST_INFO;
	pListInfo->strIP = m_strClientIP;
	pListInfo->index.Format(L"%08d", m_PGTraceNO);
	pListInfo->strDirection = L"PG->POS";

	char szListData[20];
	ZeroMemory(szListData, 20);
	memcpy(szListData, m_pRequest->mPosNo, 15);
	pListInfo->strPOSTelNO = A2W(szListData);

	ZeroMemory(szListData, 20);
	memcpy(szListData, m_pRequest->mTraceNo, 8);
	pListInfo->strPGTelNO = A2W(szListData);

	////////////////////////////////////////////////////////////////////////////////////////////////
	// PG->POS DB 기록값을 위해 response에 원래의 값으로 복원
	memcpy(m_pResponse->m_pktHeader.VAN, m_pRequest->m_pktHeader.VAN, 3);
	memcpy(m_pResponse->mTraceNo, m_pRequest->mTraceNo, 8);
	///

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
	pListInfo->strTel =	A2W(pSendData);

	AfxGetApp()->m_pMainWnd->PostMessage(WM_ADD_RESPONSE_ITEM, (WPARAM)m_dwResponseListRow, (LPARAM)pListInfo);

	int nTotalSent = 0;
	while (nTotalSent < LEN_TEL) {
		int nSent = send(m_sock, pSendData + nTotalSent, LEN_TEL - nTotalSent, 0);
		nTotalSent += nSent;
		if (nSent == SOCKET_ERROR) break;
	}

	delete pSendData;
}

void CPosClient::CloseSocket() {
	if (m_sock != INVALID_SOCKET)
	{		
		ATLTRACE(L"close socket\n");
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
	}
}