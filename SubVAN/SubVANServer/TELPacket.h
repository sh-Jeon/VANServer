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

//FORMAT1. ����(0200/000030), �ý���ȯ��(0420/000030), 
//FORMAT2. ��������(0200/000300)
//FORMAT3. ȯ��(0200/200030), ȯ�ҳ���Ȯ��(0500/810030)
//FORMAT4. ��������Ȯ��(0200/000310)
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

	//////////// ��������
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

		//////////  ���� ���� -  ȯ��/ȯ�ҳ��� Ȯ��
		memcpy(mOrgPurchaseDate, newObj.mOrgPurchaseDate, 8);
		memcpy(mOrgPurchasePrice, newObj.mOrgPurchasePrice, 12);
		memcpy(mOrgTID, newObj.mOrgTID, 40);

		//////////  ���� ���� -  ���� ���� Ȯ��
		memcpy(mAcceptDateTime, newObj.mAcceptDateTime, 14);
		mCouponStatus = newObj.mCouponStatus;
	}

	char mSystemType[3];    //�ý��� ���� �ڵ� "SMW"
	char mVANCode[3];       //VAN�� �����ڵ� "201"

	char mTelReqType[4];    //����TYPE
	char mTelCode[6];		//���� �����ڵ� (�ŷ������ڵ�)
	char mSendFlag;         //�ۼ��� flag
	char mStatus[3];		//status code - filler
	char mTelRespType[3];	//�����ڵ�

	char mSendDate[8];
	char mSendTime[6];

	char mTraceNo[8];
	//filler 15

	//////////  ���� ���� ����
	char mPurchaseDate[8];
	char mPurchaseTime[6];
	char mPosNo[15];
	char mPosName[50];

	//////////  ���� ���� -  ����/�ý���ȯ��
	char mBarcode[20];
	char mPrice[12];
	char mTID[40];
	char mAcceptNo[13];
	char mResponseMessage[256];
	//filler 70

	//////////  ���� ���� -  ���� ����
	char mProductCode[20];

	//////////  ���� ���� -  ȯ��/ȯ�ҳ��� Ȯ��
	char mOrgPurchaseDate[8];
	char mOrgPurchasePrice[12];
	char mOrgTID[40];

	//////////  ���� ���� -  ���� ���� Ȯ��
	char mAcceptDateTime[14];
	char mCouponStatus;

private:
	void _assignFieldValue(int *pos, char* value, char *pdata, int nLen);
};
