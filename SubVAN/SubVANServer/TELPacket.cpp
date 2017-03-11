#include "StdAfx.h"
#include "TELPacket.h"
#include <strsafe.h>
#include "VANProtocol.h"

/*
전체전문 557bytes이어야 합니다.
*/

// 전문추적번호 : 8자리,  POS-->PG : POS별,일자별로 유니크(POS+pos번호+일련번호 - P0100001)
//                        PG-->VAN : (BB######)로 유니크한 넘버 생성 후 전송 (일련번호 index 생성해서 전송) - 응답(FILER)에 앞 8자리에 넣어서 response to POS.
// POS --> PG : tcp/ip header  : 길이4자리 + SYM으로 올라옴.
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
	memcpy(mVANCode, "201", 3);  //VAN사 구분코드 "201"

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

	//////////  업무 정보 -  환불/환불내역 확인
	ZeroMemory(mOrgPurchaseDate, 8);
	ZeroMemory(mOrgPurchasePrice, 12);
	ZeroMemory(mOrgTID, 40);

	//////////  업무 정보 -  쿠폰 상태 확인
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

	int nPtIdx = 7;   // header 7, smw 3, van사 구분코드 3

	_assignFieldValue(&nPtIdx, mSystemType, pData, 3); //시스템 구분 코드 "SMW"
	_assignFieldValue(&nPtIdx, mVANCode, pData, 3); //VAN사 구분코드 "201"
	_assignFieldValue(&nPtIdx, mTelReqType, pData, 4); //전문TYPE
	_assignFieldValue(&nPtIdx, mTelCode, pData, 6); // 전문 종별코드
	_assignFieldValue(&nPtIdx, &mSendFlag, pData, 1);
	_assignFieldValue(&nPtIdx, mStatus, pData, 3);
	_assignFieldValue(&nPtIdx, mTelRespType, pData, 3);
	_assignFieldValue(&nPtIdx, mSendDate, pData, 8);
	_assignFieldValue(&nPtIdx, mSendTime, pData, 6);

	// POS --> PG : 전문번호 제일 앞자리는 9, 
	// PG --> VAN : 9를 0으로 대체
	_assignFieldValue(&nPtIdx, mTraceNo, pData, 8);
	// filler
	nPtIdx += 15;

	
	//FORMAT1. 구매(0200/000030), 시스템환불(0420/000030), 
	//FORMAT2. 쿠폰승인(0200/000300)
	//FORMAT3. 환불(0200/200030), 환불내역확인(0500/810030)
	//FORMAT4. 쿠폰상태확인(0200/000310)
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
