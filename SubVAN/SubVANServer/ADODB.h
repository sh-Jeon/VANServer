#pragma once

#include <comdef.h>
#include <atlcomtime.h>
#include <vector>
#include "DailyBatchJob.h"

// ADO를 쓰기 위하여 dll을 import 시켜줌 
#import ".\msado60.tlb" no_namespace rename ("EOF", "EndOfFile")

typedef struct _SysRefund_
{
	CString strType;
	CString strSerialNum;
	CString strTradeDate;
	CString strFranchiseNum;
	CString strBarcodeNum;
	CString strTradeAmt;
	CString strTid;
	CString strFiller;
}ST_REFUND_ITEM;
typedef std::vector<ST_REFUND_ITEM> VEC_REFUND_LIST;

typedef struct _SysRefund_NoReply
{
	CString strType;
	CString strSerialNum;
	CString strTradeDate;
	CString strTid;
	CString strFranchiseNum;
	CString strBarcodeNum;
	CString strTradeAmt;
	CString strOriTradeDate;
	CString strOriTid;
	CString strFiller;
}ST_REFUND_NO_REPLY_ITEM;
typedef std::vector<ST_REFUND_NO_REPLY_ITEM> VEC_REFUND_NO_REPLY_LIST;

typedef struct _SysCouponNotReply
{
	CString strJobType;
	CString strDataType;
	CString strSerialNum;
	CString strTradeType;
	CString strReturnCode;
	CString strTradeTime;
	CString strTradeUniqueNum;
	CString strBarcodeNum;
	CString strTid;
	CString strFranchiseName;
	CString strTradeAmt;
	CString strApproveNum;
	CString strProductCode;
	CString strFiller;
}ST_COUPON_NOT_REPLY_ITEM;
typedef std::vector<ST_COUPON_NOT_REPLY_ITEM> VEC_COUPON_NOT_REPLY_LIST;

class CVANProtocol;
class CADODB
{
public:
	CADODB(void);
	~CADODB(void);

	// Connection개체 포인터  
	_ConnectionPtr	m_pConn;
	_RecordsetPtr	m_pRS;
 
	BOOL Open();
	void Close();

	CString ErrMsg();
	int IsConnect(void);

	CString			m_strDBIP;
	CString			m_strDBName;
	CString			m_strDBID;
	CString			m_strDBPass;
	CString			m_strErrorMessage;		// 에러 관련 문자열을 담을 변수 

	CString Call_BarCodePurchaseRequest(CVANProtocol *pClient, BOOL bRequest);
	CString Call_BarCodeRefundRequest(CVANProtocol *pClient, BOOL bRequest);
	CString Call_CouponApproveRequest(CVANProtocol *pClient, BOOL bRequest);
	CString Call_CouponStatusRequest(CVANProtocol *pClient, BOOL bRequest);

	void GetSysRefundList(VEC_REFUND_LIST *pRefundList);
	void GetSysRefundNoReplyList(VEC_REFUND_NO_REPLY_LIST *pRefundList);
	void GetCouponNotReplyList(VEC_COUPON_NOT_REPLY_LIST *pCouponList);

	CString Call_GenerateVanBillList(LPCTSTR fileName, LPCTSTR orgCode, VEC_ST_VAN_BILL_DATA *pVecBillData);

private:
	void AddLog(LPCTSTR logString);
};

inline CString getYesterDay() 
{
	CTime time = CTime::GetCurrentTime();
    CTimeSpan span(-1, 0, 0, 0);
    time += span;

	CString strYesterDay;
	strYesterDay.Format(L"%4d%02d%02d", time.GetYear(), time.GetMonth(), time.GetDay());

	return strYesterDay;
}

inline CString getYesterDayMMDD() 
{
	CTime time = CTime::GetCurrentTime();
    CTimeSpan span(-1, 0, 0, 0);
    time += span;

	CString strYesterDay;
	strYesterDay.Format(L"%02d%02d", time.GetMonth(), time.GetDay());

	return strYesterDay;
}