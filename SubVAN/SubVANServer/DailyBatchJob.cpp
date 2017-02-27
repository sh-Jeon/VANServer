#include "StdAfx.h"
#include "DailyBatchJob.h"
#include "SubVANServer.h"
#include "ADODB.h"
#include <ATLPath.h>

typedef struct {
	char RecordType[2];
	char No[7];
	char Date[8];
	char Code[7];
	char ServiceType[4];
	char Filler[172];
}ST_REFUND_LOG_HEADER;

typedef struct {
	char RecordType[2];
	char No[7];
	char Count[7];
	char Filler[184];
}ST_REFUND_LOG_FOOTER;


typedef struct {
	char JobType[6];
	char DataType[2];
	char SerialNo[7];
	char OrgCode[3];
	char Count[3];
	char Date[8];
	char Filler[267];
}ST_COUPON_LOG_HEADER;

typedef struct {
	char JobType[6];
	char DataType[2];
	char SerialNo[7];
	char OrgCode[3];
	char Count[3];
	char Filler[275];
}ST_COUPON_LOG_FOOTER;

CDailyBatchJob::CDailyBatchJob(void)
{
	m_bDoneBatchProcess = FALSE;
	m_bDoneBillProcess = FALSE;

	TCHAR szCode[255];
	m_strOrgCode = L"0000010";
	if (::GetPrivateProfileString(L"BATCH", L"BarcodeOrgCode", _T(""), szCode, sizeof(szCode), g_strRegPath))
	{
		m_strOrgCode = szCode;
	}

	m_strCouponOrgCode = L"000";
	if (::GetPrivateProfileString(L"BATCH", L"CouponOrgCode", _T(""), szCode, sizeof(szCode), g_strRegPath))
	{
		m_strCouponOrgCode = szCode;
	}
}

CDailyBatchJob::~CDailyBatchJob(void)
{

}

HANDLE _CreateBatchFile(LPCTSTR filePath)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	if (ATLPath::FileExists(filePath))
	{
		SYSTEMTIME cTime;
		::GetLocalTime(&cTime);

		CString strCurrentTime;
		strCurrentTime.Format(L"%2d%02d%02d_%03d", cTime.wHour, cTime.wMinute, cTime.wSecond, cTime.wMilliseconds);

		CString newFilePath;
		newFilePath.Format(L"%s_%s", filePath, strCurrentTime);
		::MoveFile(filePath, newFilePath);
	}

	hFile = ::CreateFile(
			filePath,
			GENERIC_WRITE,
			FILE_SHARE_WRITE | FILE_SHARE_READ,
			NULL,
			CREATE_ALWAYS,
			0,
			NULL);

	return hFile;
}

void CDailyBatchJob::ProcessBatchJob(void)
{
	USES_CONVERSION;

	theApp.AddLog("Process BatchJob------");

	TCHAR szCode[255];
	m_strOrgCode = L"0000010";
	if (::GetPrivateProfileString(L"BATCH", L"BarcodeOrgCode", _T(""), szCode, sizeof(szCode), g_strRegPath))
	{
		m_strOrgCode = szCode;
	}

	m_strCouponOrgCode = L"000";
	if (::GetPrivateProfileString(L"BATCH", L"CouponOrgCode", _T(""), szCode, sizeof(szCode), g_strRegPath))
	{
		m_strCouponOrgCode = szCode;
	}
	
	CString strYesterdayMMDD = getYesterDayMMDD();

	// CN01
	TCHAR szBatchFilePath[MAX_PATH];
	if (::GetPrivateProfileString(L"BATCH", L"CN01Path", _T(""), szBatchFilePath, sizeof(szBatchFilePath), g_strRegPath))
	{
		CreateDirectory(szBatchFilePath, 0);	
	} else {
		lstrcpy(szBatchFilePath, g_strRegPath);
	}
	
	// create file
	CString strBatchFile;
	strBatchFile.Format(L"%s\\CN01%s", szBatchFilePath, strYesterdayMMDD);
	HANDLE hFile = _CreateBatchFile(strBatchFile);

	_WriteBatchRefundLog(hFile);

	::CloseHandle(hFile);

	// CN02
	if (::GetPrivateProfileString(L"BATCH", L"CN02Path", _T(""), szBatchFilePath, sizeof(szBatchFilePath), g_strRegPath))
	{
		CreateDirectory(szBatchFilePath, 0);	
	} else {
		lstrcpy(szBatchFilePath, g_strRegPath);
	}	

	// create file
	strBatchFile.Format(L"%s\\CN02%s", szBatchFilePath, strYesterdayMMDD);
	hFile = _CreateBatchFile(strBatchFile);

	_WriteBatchRefundNoReplyLog(hFile);

	::CloseHandle(hFile);

	// CN04
	if (::GetPrivateProfileString(L"BATCH", L"CN04Path", _T(""), szBatchFilePath, sizeof(szBatchFilePath), g_strRegPath))
	{
		CreateDirectory(szBatchFilePath, 0);	
	} else {
		lstrcpy(szBatchFilePath, g_strRegPath);
	}

	// create file
	strBatchFile.Format(L"%s\\%s%s001", szBatchFilePath, L"sw1122", strYesterdayMMDD);
	hFile = _CreateBatchFile(strBatchFile);

	_WriteBatchCouponNotReplyLog(hFile);

	::CloseHandle(hFile);

	m_bDoneBatchProcess = TRUE;
}


void CDailyBatchJob::_WriteBatchRefundLog(HANDLE hFile)
{
	USES_CONVERSION;

	ST_REFUND_LOG_HEADER stRefundLogHeader;
	memcpy(stRefundLogHeader.RecordType, "HH", 2);
	memcpy(stRefundLogHeader.No, "0000000", 7);
	memcpy(stRefundLogHeader.Date, T2A(getYesterDay()), 8);
	memcpy(stRefundLogHeader.Code, T2A(m_strOrgCode), 7);
	memcpy(stRefundLogHeader.ServiceType, "CN01", 4);
	char space[172];
	for (int i=0; i<172; i++) space[i] = ' ';
	memcpy(stRefundLogHeader.Filler, space, 172);

	DWORD dwWritten;
	::WriteFile(hFile, &stRefundLogHeader, (DWORD)sizeof(stRefundLogHeader), &dwWritten, 0);
	//::WriteFile(hFile, "\r\n", 2, &dwWritten, 0);

	VEC_REFUND_LIST vecRefundList;
	theApp.GetDBManager()->GetSysRefundList(&vecRefundList);

	int iRecCount = vecRefundList.size();

	for (int i=0; i<iRecCount; i++) {
		::WriteFile(hFile, T2A(vecRefundList[i].strType), 2, &dwWritten, 0);
		::WriteFile(hFile, T2A(vecRefundList[i].strSerialNum), 7, &dwWritten, 0);
		::WriteFile(hFile, T2A(vecRefundList[i].strTradeDate), 8, &dwWritten, 0);
		::WriteFile(hFile, T2A(vecRefundList[i].strFranchiseNum), 15, &dwWritten, 0);
		::WriteFile(hFile, T2A(vecRefundList[i].strBarcodeNum), 20, &dwWritten, 0);
		::WriteFile(hFile, T2A(vecRefundList[i].strTradeAmt), 12, &dwWritten, 0);
		::WriteFile(hFile, T2A(vecRefundList[i].strTid), 40, &dwWritten, 0);

		char szFiller[96];
		for (int iFiller=0; iFiller<96; iFiller++) szFiller[iFiller] = ' ';
		CString strFiller = vecRefundList[i].strFiller;
		if (strFiller.GetLength()) {
			memcpy(szFiller, T2A(strFiller), 96);
		}

		::WriteFile(hFile, szFiller, 96, &dwWritten, 0);
		//::WriteFile(hFile, "\r\n", 2, &dwWritten, 0);
	}

	ST_REFUND_LOG_FOOTER stRefundLogFooter;
	memcpy(stRefundLogFooter.RecordType, "TT", 2);
	memcpy(stRefundLogFooter.No, "9999999", 7);

	char recCount[8];
	ZeroMemory(recCount, 8);
	_snprintf_s(recCount, sizeof(recCount), "%07d", iRecCount);
	memcpy(stRefundLogFooter.Count, recCount, 7);

	char spaceFiller[184];
	for (int i=0; i<184; i++) spaceFiller[i] = ' ';
	memcpy(stRefundLogFooter.Filler, spaceFiller, 184);

	::WriteFile(hFile, &stRefundLogFooter, (DWORD)sizeof(stRefundLogFooter), &dwWritten, 0);
}

void CDailyBatchJob::_WriteBatchRefundNoReplyLog(HANDLE hFile)
{
	USES_CONVERSION;

	ST_REFUND_LOG_HEADER stRefundLogHeader;
	memcpy(stRefundLogHeader.RecordType, "HH", 2);
	memcpy(stRefundLogHeader.No, "0000000", 7);
	memcpy(stRefundLogHeader.Date, T2A(getYesterDay()), 8);
	memcpy(stRefundLogHeader.Code, T2A(m_strOrgCode), 7);
	memcpy(stRefundLogHeader.ServiceType, "CN02", 4);
	char space[172];
	for (int i=0; i<172; i++) space[i] = ' ';
	memcpy(stRefundLogHeader.Filler, space, 172);

	DWORD dwWritten;
	::WriteFile(hFile, &stRefundLogHeader, (DWORD)sizeof(stRefundLogHeader), &dwWritten, 0);
	//::WriteFile(hFile, "\r\n", 2, &dwWritten, 0);

	VEC_REFUND_NO_REPLY_LIST vecRefundList;
	theApp.GetDBManager()->GetSysRefundNoReplyList(&vecRefundList);

	int iRecCount = vecRefundList.size();
	for (int i=0; i<iRecCount; i++) {
		::WriteFile(hFile, T2A(vecRefundList[i].strType), 2, &dwWritten, 0);
		::WriteFile(hFile, T2A(vecRefundList[i].strSerialNum), 7, &dwWritten, 0);
		::WriteFile(hFile, T2A(vecRefundList[i].strTradeDate), 8, &dwWritten, 0);
		::WriteFile(hFile, T2A(vecRefundList[i].strTid), 40, &dwWritten, 0);
		::WriteFile(hFile, T2A(vecRefundList[i].strFranchiseNum), 15, &dwWritten, 0);
		::WriteFile(hFile, T2A(vecRefundList[i].strBarcodeNum), 20, &dwWritten, 0);
		::WriteFile(hFile, T2A(vecRefundList[i].strTradeAmt), 12, &dwWritten, 0);
		::WriteFile(hFile, T2A(vecRefundList[i].strOriTradeDate), 8, &dwWritten, 0);
		::WriteFile(hFile, T2A(vecRefundList[i].strOriTid), 40, &dwWritten, 0);

		char szFiller[48];
		for (int iFiller=0; iFiller<48; iFiller++) szFiller[iFiller] = ' ';
		CString strFiller = vecRefundList[i].strFiller;
		if (strFiller.GetLength()) {
			memcpy(szFiller, T2A(strFiller), 48);
		}

		::WriteFile(hFile, szFiller, 96, &dwWritten, 0);
		//::WriteFile(hFile, "\r\n", 2, &dwWritten, 0);
	}

	ST_REFUND_LOG_FOOTER stRefundLogFooter;
	memcpy(stRefundLogFooter.RecordType, "TT", 2);
	memcpy(stRefundLogFooter.No, "9999999", 7);

	char recCount[8];
	ZeroMemory(recCount, 8);
	_snprintf_s(recCount, sizeof(recCount), "%07d", iRecCount);
	memcpy(stRefundLogFooter.Count, recCount, 7);

	char spaceFiller[184];
	for (int i=0; i<184; i++) spaceFiller[i] = ' ';
	memcpy(stRefundLogFooter.Filler, spaceFiller, 184);

	::WriteFile(hFile, &stRefundLogFooter, (DWORD)sizeof(stRefundLogFooter), &dwWritten, 0);
}

void CDailyBatchJob::_WriteBatchCouponNotReplyLog(HANDLE hFile)
{
	USES_CONVERSION;

	VEC_COUPON_NOT_REPLY_LIST vecCouponList;
	theApp.GetDBManager()->GetCouponNotReplyList(&vecCouponList);

	int iRecCount = vecCouponList.size();
	char recCount[8];
	ZeroMemory(recCount, 8);
	_snprintf_s(recCount, sizeof(recCount), "%07d", iRecCount);

	ST_COUPON_LOG_HEADER stCouponLogHeader;
	memcpy(stCouponLogHeader.JobType, "SW1122", 6);
	memcpy(stCouponLogHeader.DataType, "11", 2);
	memcpy(stCouponLogHeader.SerialNo, "0000000", 7);
	memcpy(stCouponLogHeader.OrgCode, T2A(m_strCouponOrgCode), 3);
	memcpy(stCouponLogHeader.Count, recCount, 7);
	memcpy(stCouponLogHeader.Date, T2A(getYesterDay()), 8);
	char space[267];
	for (int i=0; i<267; i++) space[i] = ' ';
	memcpy(stCouponLogHeader.Filler, space, 267);

	DWORD dwWritten;
	::WriteFile(hFile, &stCouponLogHeader, (DWORD)sizeof(stCouponLogHeader), &dwWritten, 0);
	//::WriteFile(hFile, "\r\n", 2, &dwWritten, 0);

	for (int i=0; i<iRecCount; i++) {
		::WriteFile(hFile, T2A(vecCouponList[i].strJobType), 6, &dwWritten, 0);
		::WriteFile(hFile, T2A(vecCouponList[i].strDataType), 2, &dwWritten, 0);
		::WriteFile(hFile, T2A(vecCouponList[i].strSerialNum), 7, &dwWritten, 0);
		::WriteFile(hFile, T2A(vecCouponList[i].strTradeType), 6, &dwWritten, 0);
		::WriteFile(hFile, T2A(vecCouponList[i].strReturnCode), 3, &dwWritten, 0);
		::WriteFile(hFile, T2A(vecCouponList[i].strTradeTime), 6, &dwWritten, 0);
		::WriteFile(hFile, T2A(vecCouponList[i].strTradeUniqueNum), 13, &dwWritten, 0);
		::WriteFile(hFile, T2A(vecCouponList[i].strBarcodeNum), 20, &dwWritten, 0);
		::WriteFile(hFile, T2A(vecCouponList[i].strTid), 40, &dwWritten, 0);

		char szFranchiseName[50];
		ZeroMemory(szFranchiseName, 50);
		memcpy(szFranchiseName, T2A(vecCouponList[i].strFranchiseName), vecCouponList[i].strFranchiseName.GetLength());
		::WriteFile(hFile, szFranchiseName, 50, &dwWritten, 0);
		::WriteFile(hFile, T2A(vecCouponList[i].strTradeAmt), 12, &dwWritten, 0);
		::WriteFile(hFile, T2A(vecCouponList[i].strApproveNum), 13, &dwWritten, 0);
		::WriteFile(hFile, T2A(vecCouponList[i].strProductCode), 20, &dwWritten, 0);

		char szFiller[94];
		for (int iFiller=0; iFiller<94; iFiller++) szFiller[iFiller] = ' ';
		
		CString strFiller = vecCouponList[i].strFiller;
		if (strFiller.GetLength()) {
			memcpy(szFiller, T2A(strFiller), strFiller.GetLength());
		}

		::WriteFile(hFile, szFiller, 94, &dwWritten, 0);
		//::WriteFile(hFile, "\r\n", 2, &dwWritten, 0);
	}

	ST_COUPON_LOG_FOOTER stCouponLogFooter;
	memcpy(stCouponLogFooter.JobType, "SW1122", 6);
	memcpy(stCouponLogFooter.DataType, "33", 2);
	memcpy(stCouponLogFooter.SerialNo, "9999999", 7);
	memcpy(stCouponLogFooter.OrgCode, T2A(m_strCouponOrgCode), 3);
	memcpy(stCouponLogFooter.Count, recCount, 7);
	char spaceFooter[275];
	for (int i=0; i<275; i++) spaceFooter[i] = ' ';
	memcpy(stCouponLogFooter.Filler, spaceFooter, 275);

	::WriteFile(hFile, &stCouponLogFooter, (DWORD)sizeof(stCouponLogFooter), &dwWritten, 0);
}

void CDailyBatchJob::ProcessGenerateVanBillList()
{
	//////////// 沥魂贸府
	// CN03
	theApp.AddLog("Process GenerateVanBillList------");

	TCHAR szBatchFilePath[MAX_PATH];
	if (::GetPrivateProfileString(L"BATCH", L"CN03Path", _T(""), szBatchFilePath, sizeof(szBatchFilePath), g_strRegPath))
	{
		CreateDirectory(szBatchFilePath, 0);	
	} else {
		lstrcpy(szBatchFilePath, g_strRegPath);
	}

	CString strYesterdayMMDD = getYesterDayMMDD();

	CString strBatchFile;
	strBatchFile.Format(L"%s\\CN03%s", szBatchFilePath, strYesterdayMMDD);
	HANDLE hFile = ::CreateFile(
				strBatchFile,
				GENERIC_READ,
				FILE_SHARE_READ,
				NULL,
				OPEN_EXISTING,
				0,
				NULL);

	VEC_ST_VAN_BILL_DATA vec_van_bill_data;
	
	DWORD dwCurrent = ::SetFilePointer(hFile, 200, 0, FILE_BEGIN);
	DWORD dwRead=0;
	while (1) {
		ST_VAN_BILL_DATA stVanBillData;
		ZeroMemory(&stVanBillData, sizeof(ST_VAN_BILL_DATA));

		if (FALSE == ::ReadFile(hFile, (LPVOID)&stVanBillData, sizeof(ST_VAN_BILL_DATA), &dwRead, 0)) {
			DWORD dwError = GetLastError();
			break;
		}

		if (0 == strncmp(stVanBillData.RecordType, "TT", 2)) break;
		
		if (dwRead > 0) {
			vec_van_bill_data.push_back(stVanBillData);
		} else {
			break;
		}
	}
	::CloseHandle(hFile);
	
	if (vec_van_bill_data.size() > 0) {
		theApp.GetDBManager()->Call_GenerateVanBillList(ATLPath::FindFileName(strBatchFile), m_strOrgCode, &vec_van_bill_data);
	}

	// 贸府等 颇老 颇老疙 函版
	SYSTEMTIME cTime;
	::GetLocalTime(&cTime);

	CString strCurrentTime;
	strCurrentTime.Format(L".%02d%02d%02d", cTime.wHour, cTime.wMinute, cTime.wSecond);

	CString doneBatchFile = strBatchFile;
	doneBatchFile += strCurrentTime;
	::MoveFile(strBatchFile, doneBatchFile);

	m_bDoneBillProcess = TRUE;
}
