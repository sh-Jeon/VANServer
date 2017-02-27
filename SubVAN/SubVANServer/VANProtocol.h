#pragma once

#include <vector>
#include "telpacket.h"
#include "strsafe.h"

typedef enum {
	RES_SUCCESS = 0,
	RES_TIMEOUT,
	RES_ERR_LEN,
	RES_ERR_NET,
	RES_ERR_SYSTEM,
	RES_ERR_DB_CONN,
	RES_ERR_DB_DUPL,
	RES_ERR_DB_TRANSACTION,
	RES_ERR_DB_QUERY,
	RES_ERR_PHURCAHSE,
	RES_ERR_INVALID_FORMAT,
	RES_FAIL
}RES_CODE;

class CVANProtocol
{
public:
	CVANProtocol(void);
	~CVANProtocol(void);

	static PG_ERROR_CODE CheckHeader(int nLenReceived, char *pRecvBuffer) {
		int nSizeHeader = sizeof(ST_PKT_HEADER);
		if (nLenReceived < nSizeHeader) {
			return ERROR_WAIT;
		}

		// Header Size : 7 byte (len : 4byte, 'VAN')
		ST_PKT_HEADER stRecvHeader;
		ZeroMemory(&stRecvHeader, nSizeHeader);
		memcpy(&stRecvHeader, pRecvBuffer, nSizeHeader);

		char strBodylen[4];
		memcpy(strBodylen, stRecvHeader.lenData, 4);

		int nBodyLen = atol(strBodylen);
		if (nLenReceived < 4 + nBodyLen)
		{
			return ERROR_WAIT;  //body 도착하지 않음.
		}

		if (nBodyLen < LEN_TEL - 4) {
			return ERROR_INVALID_LENGTH;
		}

		return ERROR_OK;
	}

	
	//FORMAT1. 구매(0200/000030) (0210/000030), 시스템환불(0420/000030), 
	//FORMAT2. 쿠폰승인(0200/000300) (0210/000300)
	//FORMAT3. 환불(0200/200030), 환불내역확인(0500/810030)
	//FORMAT4. 쿠폰상태확인(0200/000310)
	static TR_TYPE checkTRFromat(char *telCode)
	{
		char telFormat[7];
		ZeroMemory(telFormat, 7);
		StringCbPrintfA(telFormat, _countof(telFormat), "%s", telCode);

		if (0 == strcmp(telFormat, "000030")) {
			return CMD_PURCHASE;
		}

		if (0 == strcmp(telFormat, "000300")) {
			return CMD_COUPON;
		}

		if (0 == strcmp(telFormat, "200030") ||
			0 == strcmp(telFormat, "810030")) {
			return CMD_REFUND;
		}

		if (0 == strcmp(telFormat, "000310")) {
			return CMD_COUPON_STAUTS;
		}

		return CMD_INVALID;
	}

	static void setResponseErrorCode(char *pData, RES_CODE resCode)
	{
		// error code setting
		switch (resCode) {
			case RES_TIMEOUT :
				memcpy(pData, "S11", 3);
				break;
			case RES_ERR_LEN :
				memcpy(pData, "S12", 3);
				break;
			case RES_ERR_NET :
				memcpy(pData, "S41", 3);
				break;
			case RES_ERR_SYSTEM :
				memcpy(pData, "S42", 3);
				break;
			case RES_ERR_DB_CONN :
				memcpy(pData, "S80", 3);
				break;
			case RES_ERR_DB_DUPL :
				memcpy(pData, "S81", 3);
				break;
			case RES_ERR_DB_TRANSACTION :
				memcpy(pData, "S82", 3);
				break;
			case RES_ERR_DB_QUERY :
				memcpy(pData, "S89", 3);
				break;
			case RES_ERR_PHURCAHSE :
				memcpy(pData, "S95", 3);
				break;
			case RES_ERR_INVALID_FORMAT :
				memcpy(pData, "S98", 3);
				break;
			case RES_FAIL :
				memcpy(pData, "S99", 3);
				break;
			default : break;
		}
	}
		
	PG_ERROR_CODE CheckPacketLength(char *pData, int nLen);
	PG_ERROR_CODE SetRequestData(char *pData, int nLen);
	PG_ERROR_CODE SetResponseData(char *pData, int nLen);

	void CopyResponseData(CTELPacket *pData);

	DWORD m_dwRequestListRow;
	DWORD m_dwResponseListRow;

	CTELPacket *m_pRequest;
	CTELPacket *m_pResponse;
	DWORD m_PGTraceNO;

	CString m_strClientIP;

protected:
	void _assignFieldValue(int *pos, char* pdata, char *value, int nLen);
};
