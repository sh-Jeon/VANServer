#include "StdAfx.h"
#include "VANProtocol.h"
#include "PosClient.h"
#include "SubVANServer.h"

CVANProtocol::CVANProtocol(void)
{
	m_pRequest = NULL;
	m_pResponse = NULL;
}

CVANProtocol::~CVANProtocol(void)
{
	if (m_pRequest) {
		delete m_pRequest;
		m_pRequest = NULL;
	}
	if (m_pResponse) {
		delete m_pResponse;
		m_pResponse = NULL;
	}
}

void CVANProtocol::CopyResponseData(CTELPacket *pData)
{
	if (pData) {
		(m_pResponse == NULL) ? m_pResponse = new CTELPacket(pData) : m_pResponse = pData;
	}
}

void CVANProtocol::_assignFieldValue(int *pos, char* pdata, char *value, int nLen)
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

PG_ERROR_CODE CVANProtocol::CheckPacketLength(char *pData, int nLen)
{
	if (nLen < sizeof(ST_PKT_HEADER)) {
		return ERROR_WAIT;
	}

	ST_PKT_HEADER stRecvHeader;   
	memcpy(&stRecvHeader, pData, sizeof(ST_PKT_HEADER));

	char strBodylen[4];
	memcpy(strBodylen, stRecvHeader.lenData, 4);

    int nBodyLen = atol(strBodylen);
	if (nLen < 4 + nBodyLen)
	{
		return ERROR_WAIT;  //body 도착하지 않음.
	}

	return ERROR_OK;
}

PG_ERROR_CODE CVANProtocol::SetRequestData(char *pData, int nLen)
{
	USES_CONVERSION;

	PG_ERROR_CODE chkLen = CheckPacketLength(pData, nLen);
	if (ERROR_OK != chkLen) {
		return chkLen;
	}

	if (m_pRequest == NULL) {
		m_pRequest = new CTELPacket();
	}

	m_pRequest->setPacketData(pData);

	ST_MONITOR_LIST_INFO *pListInfo = new ST_MONITOR_LIST_INFO;
	pListInfo->strPOSTelNO.Append(A2W(m_pRequest->mPosNo), 15);
	pListInfo->strPGTelNO.Append(A2W(m_pRequest->mTraceNo), 8);
	pListInfo->strTelType.Append(A2W(m_pRequest->mTelReqType), 4);
	pListInfo->strTelType += L"/";
	pListInfo->strTelType.Append(A2W(m_pRequest->mTelCode), 6);	
	pListInfo->strTel =	A2W(pData);

	AfxGetApp()->m_pMainWnd->PostMessage(WM_ADD_REQUEST_ITEM, (WPARAM)m_dwRequestListRow, (LPARAM)pListInfo);
	
	return ERROR_OK;
}

PG_ERROR_CODE CVANProtocol::SetResponseData(char *pData, int nLen) 
{
	USES_CONVERSION;

	PG_ERROR_CODE chkLen = CheckPacketLength(pData, nLen);
	if (ERROR_OK != chkLen) {
		return chkLen;
	}

	if (m_pResponse == NULL) {
		m_pResponse = new CTELPacket();
	}

	ST_PKT_HEADER *pstRecvHeader = &(m_pResponse->m_pktHeader);
	memcpy(&pstRecvHeader, pData, sizeof(ST_PKT_HEADER));

	m_pResponse->setPacketData(pData);

	return ERROR_OK;
}