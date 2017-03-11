#include "StdAfx.h"
#include "TELPacket.h"
#include <strsafe.h>
#include "VANProtocol.h"

/*
��ü���� 557bytes�̾�� �մϴ�.
*/

// ����������ȣ : 8�ڸ�,  POS-->PG : POS��,���ں��� ����ũ(POS+pos��ȣ+�Ϸù�ȣ - P0100001)
//                        PG-->VAN : (BB######)�� ����ũ�� �ѹ� ���� �� ���� (�Ϸù�ȣ index �����ؼ� ����) - ����(FILER)�� �� 8�ڸ��� �־ response to POS.
// POS --> PG : tcp/ip header  : ����4�ڸ� + SYM���� �ö��.
CTELPacket::CTELPacket(CTELPacket *pTelData)
{
	*this = *pTelData;
}

CTELPacket::~CTELPacket(void)
{
}

CTELPacket::CTELPacket(void)
{
	memcpy(m_pktHeader.lenData, "0553", 4);
	memcpy(m_pktHeader.VAN, "VAN", 3);

	memcpy(mSystemType, "SMW", 3);
	memcpy(mVANCode, "201", 3);  //VAN�� �����ڵ� "201"

	ZeroMemory(mSystemType, 3);
	ZeroMemory(mVANCode, 3);
	ZeroMemory(mTelReqType, 4);
	ZeroMemory(mTelCode, 6);
	mSendFlag = ' ';
	ZeroMemory(mStatus, 3);
	ZeroMemory(mTelRespType, 3);
	ZeroMemory(mSendDate, 8);
	ZeroMemory(mSendTime, 6);
	ZeroMemory(mTraceNo, 8);
	ZeroMemory(mPurchaseDate, 8);
	ZeroMemory(mPurchaseTime, 6);
	ZeroMemory(mPosNo, 15);
	ZeroMemory(mPosName, 50);
	ZeroMemory(mBarcode, 20);
	ZeroMemory(mPrice, 12);
	ZeroMemory(mTID, 40);
	ZeroMemory(mAcceptNo, 13);
	ZeroMemory(mProductCode, 20);
	ZeroMemory(mResponseMessage, 256);

	//////////  ���� ���� -  ȯ��/ȯ�ҳ��� Ȯ��
	ZeroMemory(mOrgPurchaseDate, 8);
	ZeroMemory(mOrgPurchasePrice, 12);
	ZeroMemory(mOrgTID, 40);

	//////////  ���� ���� -  ���� ���� Ȯ��
	mCouponStatus = ' ';
}

void CTELPacket::_assignFieldValue(int *pos, char* value, char *pdata, int nLen)
{
	if (value == NULL) { //filler
		for (int idx=0; idx<nLen; idx++) {
			*(pdata + *pos + idx) = ' ';
		}
		*pos += nLen;
		return;
	}

	memcpy(value, pdata + *pos, nLen);
	*pos += nLen;
}

void CTELPacket::_setPurchaseData(int nPtIdx, char *pData)
{
	_assignFieldValue(&nPtIdx, mPurchaseDate, pData, 8);
	_assignFieldValue(&nPtIdx, mPurchaseTime, pData, 6);
	_assignFieldValue(&nPtIdx, mPosNo, pData, 15);
	_assignFieldValue(&nPtIdx, mPosName, pData, 50);
	_assignFieldValue(&nPtIdx, mBarcode, pData, 20);
	_assignFieldValue(&nPtIdx, mPrice, pData, 12);
	_assignFieldValue(&nPtIdx, mTID, pData, 40);
	_assignFieldValue(&nPtIdx, mAcceptNo, pData, 13);
	_assignFieldValue(&nPtIdx, mResponseMessage, pData, 256);
	_assignFieldValue(&nPtIdx, NULL, pData, 70);
}

void CTELPacket::_setCouponPurchaseData(int nPtIdx, char *pData)
{
	_assignFieldValue(&nPtIdx, mPurchaseDate, pData, 8);
	_assignFieldValue(&nPtIdx, mPurchaseTime, pData, 6);
	_assignFieldValue(&nPtIdx, mPosNo, pData, 15);
	_assignFieldValue(&nPtIdx, mPosName, pData, 50);
	_assignFieldValue(&nPtIdx, mBarcode, pData, 20);
	_assignFieldValue(&nPtIdx, mPrice, pData, 12);
	_assignFieldValue(&nPtIdx, mTID, pData, 40);
	_assignFieldValue(&nPtIdx, mAcceptNo, pData, 13);
	_assignFieldValue(&nPtIdx, mResponseMessage, pData, 256);
	_assignFieldValue(&nPtIdx, mProductCode, pData, 20);
	_assignFieldValue(&nPtIdx, NULL, pData, 50);
}

void CTELPacket::_setRefundData(int nPtIdx, char *pData)
{
	_assignFieldValue(&nPtIdx, mPurchaseDate, pData, 8);
	_assignFieldValue(&nPtIdx, mPurchaseTime, pData, 6);
	_assignFieldValue(&nPtIdx, mPosNo, pData, 15);
	_assignFieldValue(&nPtIdx, mPosName, pData, 50);
	_assignFieldValue(&nPtIdx, mTID, pData, 40);
	_assignFieldValue(&nPtIdx, mOrgPurchaseDate, pData, 8);
	_assignFieldValue(&nPtIdx, mOrgPurchasePrice, pData, 12);		
	_assignFieldValue(&nPtIdx, mOrgTID, pData, 40);
	_assignFieldValue(&nPtIdx, mAcceptNo, pData, 13);
	_assignFieldValue(&nPtIdx, mResponseMessage, pData, 256);
	_assignFieldValue(&nPtIdx, NULL, pData, 42);
}

void CTELPacket::_setCouponData(int nPtIdx, char *pData)
{
	_assignFieldValue(&nPtIdx, mPurchaseDate, pData, 8);
	_assignFieldValue(&nPtIdx, mPurchaseTime, pData, 6);
	_assignFieldValue(&nPtIdx, &mCouponStatus, pData, 1);
	_assignFieldValue(&nPtIdx, mPosNo, pData, 15);
	_assignFieldValue(&nPtIdx, mPosName, pData, 50);
	_assignFieldValue(&nPtIdx, mBarcode, pData, 20);
	_assignFieldValue(&nPtIdx, mPrice, pData, 12);
	_assignFieldValue(&nPtIdx, mTID, pData, 40);
	_assignFieldValue(&nPtIdx, mAcceptNo, pData, 13);
	_assignFieldValue(&nPtIdx, mAcceptDateTime, pData, 14);
	_assignFieldValue(&nPtIdx, mResponseMessage, pData, 256);
	_assignFieldValue(&nPtIdx, mProductCode, pData, 20);
	_assignFieldValue(&nPtIdx, NULL, pData, 35);
}

RES_CODE CTELPacket::setPacketData(char *pData)
{
	memcpy(&m_pktHeader, pData, 7);

	int nPtIdx = 7;   // header 7, smw 3, van�� �����ڵ� 3

	_assignFieldValue(&nPtIdx, mSystemType, pData, 3); //�ý��� ���� �ڵ� "SMW"
	_assignFieldValue(&nPtIdx, mVANCode, pData, 3); //VAN�� �����ڵ� "201"
	_assignFieldValue(&nPtIdx, mTelReqType, pData, 4); //����TYPE
	_assignFieldValue(&nPtIdx, mTelCode, pData, 6); // ���� �����ڵ�
	_assignFieldValue(&nPtIdx, &mSendFlag, pData, 1);
	_assignFieldValue(&nPtIdx, mStatus, pData, 3);
	_assignFieldValue(&nPtIdx, mTelRespType, pData, 3);
	_assignFieldValue(&nPtIdx, mSendDate, pData, 8);
	_assignFieldValue(&nPtIdx, mSendTime, pData, 6);

	// POS --> PG : ������ȣ ���� ���ڸ��� 9, 
	// PG --> VAN : 9�� 0���� ��ü
	_assignFieldValue(&nPtIdx, mTraceNo, pData, 8);
	// filler
	nPtIdx += 15;

	
	//FORMAT1. ����(0200/000030), �ý���ȯ��(0420/000030), 
	//FORMAT2. ��������(0200/000300)
	//FORMAT3. ȯ��(0200/200030), ȯ�ҳ���Ȯ��(0500/810030)
	//FORMAT4. ��������Ȯ��(0200/000310)
	TR_TYPE trType = CVANProtocol::checkTRFromat(mTelCode);
	if (CMD_PURCHASE == trType) {
		_setPurchaseData(nPtIdx, pData);
	}

	if (CMD_COUPON == trType) {
		_setCouponPurchaseData(nPtIdx, pData);
	}
	
	if (CMD_REFUND == trType) {
		_setRefundData(nPtIdx, pData);
	}

	if (CMD_COUPON_STAUTS == trType) {
		_setCouponData(nPtIdx, pData);
	}

	//filler 70
	return RES_SUCCESS;
}
