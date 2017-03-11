#pragma once

#define WM_ADD_REQUEST_ITEM		WM_APP+111
#define WM_ADD_RESPONSE_ITEM	WM_APP+112

class CVANProtocol;

typedef struct
{
	CString index;
	CString strIP;
	CString strDirection;
	CString strTelType;
	CString strPOSTelNO;
	CString strPGTelNO;
	CString strRespCode;
	CString strTel;
}ST_MONITOR_LIST_INFO;

//FORMAT1. 구매(0200/000030), 시스템환불(0420/000030), 
//FORMAT2. 쿠폰승인(0200/000300)
//FORMAT3. 환불(0200/200030), 환불내역확인(0500/810030)
//FORMAT4. 쿠폰상태확인(0200/000310)
typedef enum
{
	CMD_PURCHASE,
	CMD_COUPON,
	CMD_REFUND,
	CMD_COUPON_STAUTS,
	CMD_INVALID
}TR_TYPE;

typedef enum {
	RES_SUCCESS = 0,
	RES_WAIT,
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

#define LEN_TEL 557

typedef struct
{
	char lenData[4];
	char VAN[3];
}ST_PKT_HEADER;

class CTELPacket
{
public:
	CTELPacket(CTELPacket *pTelData);
	CTELPacket(void);
	~CTELPacket(void);

	RES_CODE setPacketData(char *pData);
	void _setPurchaseData(int nPtIdx, char *pData);
	void _setCouponPurchaseData(int nPtIdx, char *pData);
	void _setRefundData(int nPtIdx, char *pData);
	void _setCouponData(int nPtIdx, char *pData);

	//////////// 공통정보
	ST_PKT_HEADER m_pktHeader;

	void CTELPacket::operator =(const CTELPacket &newObj) {
		memcpy(&m_pktHeader, &(newObj.m_pktHeader), sizeof(ST_PKT_HEADER));
		memcpy(mSystemType, newObj.mSystemType, 3);
		memcpy(mVANCode, newObj.mVANCode, 3);
		memcpy(mTelReqType, newObj.mTelReqType, 4);
		memcpy(mTelCode, newObj.mTelCode, 6);
		mSendFlag = newObj.mSendFlag;
		memcpy(mStatus, newObj.mStatus, 3);
		memcpy(mTelRespType, newObj.mTelRespType, 3);
		memcpy(mSendDate, newObj.mSendDate, 8);
		memcpy(mSendTime, newObj.mSendTime, 6);
		memcpy(mTraceNo, newObj.mTraceNo, 8);
		memcpy(mPurchaseDate, newObj.mPurchaseDate, 8);
		memcpy(mPurchaseTime, newObj.mPurchaseTime, 6);
		memcpy(mPosNo, newObj.mPosNo, 15);
		memcpy(mPosName, newObj.mPosName, 50);
		memcpy(mBarcode, newObj.mBarcode, 20);
		memcpy(mPrice, newObj.mPrice, 12);
		memcpy(mTID, newObj.mTID, 40);
		memcpy(mAcceptNo, newObj.mAcceptNo, 13);
		memcpy(mProductCode, newObj.mProductCode, 20);
		memcpy(mResponseMessage, newObj.mResponseMessage, 256);

		//////////  업무 정보 -  환불/환불내역 확인
		memcpy(mOrgPurchaseDate, newObj.mOrgPurchaseDate, 8);
		memcpy(mOrgPurchasePrice, newObj.mOrgPurchasePrice, 12);
		memcpy(mOrgTID, newObj.mOrgTID, 40);

		//////////  업무 정보 -  쿠폰 상태 확인
		memcpy(mAcceptDateTime, newObj.mAcceptDateTime, 14);
		mCouponStatus = newObj.mCouponStatus;
	}

	char mSystemType[3];    //시스템 구분 코드 "SMW"
	char mVANCode[3];       //VAN사 구분코드 "201"

	char mTelReqType[4];    //전문TYPE
	char mTelCode[6];		//전문 종별코드 (거래구분코드)
	char mSendFlag;         //송수신 flag
	char mStatus[3];		//status code - filler
	char mTelRespType[3];	//응답코드

	char mSendDate[8];
	char mSendTime[6];

	char mTraceNo[8];
	//filler 15

	//////////  업무 정보 공통
	char mPurchaseDate[8];
	char mPurchaseTime[6];
	char mPosNo[15];
	char mPosName[50];

	//////////  업무 정보 -  구매/시스템환불
	char mBarcode[20];
	char mPrice[12];
	char mTID[40];
	char mAcceptNo[13];
	char mResponseMessage[256];
	//filler 70

	//////////  업무 정보 -  쿠폰 승인
	char mProductCode[20];

	//////////  업무 정보 -  환불/환불내역 확인
	char mOrgPurchaseDate[8];
	char mOrgPurchasePrice[12];
	char mOrgTID[40];

	//////////  업무 정보 -  쿠폰 상태 확인
	char mAcceptDateTime[14];
	char mCouponStatus;

private:
	void _assignFieldValue(int *pos, char* value, char *pdata, int nLen);
};
