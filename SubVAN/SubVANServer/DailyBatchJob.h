#pragma once

#include <vector>

typedef struct {
	char RecordType[2];
	char SerialNo[7];
	char TradeType[1];
	char Date[8];
	char TID[40];
	char Barcode[20];
	char PosNO[15];
	char Price[12];
	char Filler[95];
}ST_VAN_BILL_DATA;
typedef std::vector<ST_VAN_BILL_DATA> VEC_ST_VAN_BILL_DATA;

class CDailyBatchJob
{
public:
	CDailyBatchJob(void);
	~CDailyBatchJob(void);
	void ProcessBatchJob(void);
	void ProcessGenerateVanBillList();

	BOOL m_bDoneBatchProcess;
	BOOL m_bDoneBillProcess;

private:
	CString m_strBatchFilePath;
	CString m_strOrgCode;
	CString m_strCouponOrgCode;

	void _WriteBatchRefundLog(HANDLE hFile);
	void _WriteBatchRefundNoReplyLog(HANDLE hFile);
	void _WriteBatchCouponNotReplyLog(HANDLE hFile);
};
