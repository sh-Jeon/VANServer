#include "StdAfx.h"
#include "ADODB.h"
#include "SubVANServer.h"
#include "VANProtocol.h"
#include "PosClient.h"

CADODB::CADODB(void)
{
	m_pConn = NULL;

	m_strErrorMessage = _T("");
}

CADODB::~CADODB(void)
{
	if (IsConnect()) Close();

	if (m_pConn)
	{
		m_pConn.Release();
		m_pConn = NULL;
		::CoUninitialize();
	}
}

int CADODB::IsConnect(void)
{
	if (m_pConn  == NULL || (m_pConn != NULL && m_pConn->State == 0))
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CADODB::Open()
{
	if (IsConnect()) return TRUE;

	TCHAR szProvider[255], szIP[20], szSource[50], szUID[25];
	ZeroMemory(szProvider, 255);
	ZeroMemory(szIP, 20);
	ZeroMemory(szSource, 50);
	ZeroMemory(szUID, 25);

	DWORD dwRet = GetPrivateProfileStringW(L"DB_INFO", L"Provider", _T(""), szProvider, sizeof(szProvider), g_strRegPath);
	dwRet = GetPrivateProfileStringW(L"DB_INFO", L"DataSource", _T(""), szIP, sizeof(szIP), g_strRegPath);
	dwRet = GetPrivateProfileStringW(L"DB_INFO", L"Initial Catalog", _T(""), szSource, sizeof(szSource), g_strRegPath);
	dwRet = GetPrivateProfileStringW(L"DB_INFO", L"User ID", _T(""), szUID, sizeof(szUID), g_strRegPath);

	CString strServer;
	strServer.Format(L"Provider=%s;Persist Security Info=True;Data Source=%s;Initial Catalog=%s;",
		szProvider, szIP, szSource);
	strServer += L"User ID=";
	strServer += szUID;
	strServer += ";Password=tlaxmfk1@#$;";

	_bstr_t strCnn(strServer);

	::CoInitialize(NULL);
	if FAILED(m_pConn.CreateInstance(__uuidof(Connection)))
	{
		m_strErrorMessage="Fail to Create Instance for ADO Connect ptr";
		AddLog(m_strErrorMessage);

		//g_AgentWnd.AddLog(L"%s", m_strErrorMessage);
		return FALSE;
	}

	try
	{
		m_pConn->Open(strCnn, L"", L"", NULL);
	}
	catch(_com_error &e)
	{
		CString s = (LPCTSTR)e.ErrorMessage();
		m_strErrorMessage = L"Open 에러 (" + s + L")\n";
		m_pConn = NULL;
		//g_AgentWnd.AddLog(L"%s", m_strErrorMessage);

		return FALSE;
	}

	return TRUE;
}

void CADODB::Close()
{
	if (m_pConn)
	{
		try {
			m_pConn->Close();
			m_pConn.Release();
			m_pConn = NULL;
		}
		catch(_com_error &e)
		{
			::CoUninitialize();
		}
	}
}

CString CADODB::ErrMsg()
{
	return m_strErrorMessage;
}

void _AddCommandParameter(_CommandPtr pCommand, char *strData, LPCTSTR paramName, int nLen) {
	char *copyData = new char[nLen+1];
	ZeroMemory(copyData, nLen+1);
	memcpy(copyData, strData, nLen);

	_variant_t varParam = _bstr_t(copyData);
	_ParameterPtr pParam = pCommand->CreateParameter(_bstr_t(paramName), adVarChar, adParamInput, nLen, &varParam);
	pCommand->Parameters->Append(pParam);

	delete copyData;
}

void _AddCommandParameter(_CommandPtr pCommand, LPCTSTR strData, LPCTSTR paramName) {
	_bstr_t bstrData(strData);
	_variant_t varParam = bstrData;
	_ParameterPtr pParam = pCommand->CreateParameter(_bstr_t(paramName), adVarChar, adParamInput, bstrData.length(), &varParam);
	pCommand->Parameters->Append(pParam);
}

CString CADODB::Call_BarCodePurchaseRequest(CVANProtocol *pClient, BOOL bRequest)
{
	if(IsConnect() != TRUE) {
		if (FALSE == Open()) {
			return FALSE;
		}
	}

	CString strResult;
	try
	{
		_CommandPtr pCommand;
		pCommand.CreateInstance(__uuidof(Command));

		pCommand->ActiveConnection = m_pConn; 
		pCommand->CommandText = _bstr_t(L"SP_BarCodePurchaseRequest");
		pCommand->CommandType = adCmdStoredProc;
		pCommand->CommandTimeout = 5;

		CTELPacket *pRequest = bRequest ? pClient->m_pRequest : pClient->m_pResponse;

		// Pgdate
		SYSTEMTIME cTime;
		::GetLocalTime(&cTime);

		CString strCurrentDate;
		strCurrentDate.Format(L"%4d%02d%02d", cTime.wYear, cTime.wMonth, cTime.wDay);
		_AddCommandParameter(pCommand, strCurrentDate, L"Pgdate");

		CString strTraceNO;
		strTraceNO.Format(L"BB%06d", pClient->m_PGTraceNO);
		_AddCommandParameter(pCommand, strTraceNO, L"PgTraceNum");

		_AddCommandParameter(pCommand, pRequest->m_pktHeader.lenData, L"Header1", 4);
		_AddCommandParameter(pCommand, pRequest->m_pktHeader.VAN, L"Header2", 3);
		_AddCommandParameter(pCommand, pRequest->mSystemType, L"SysTypeCode", 3);
		_AddCommandParameter(pCommand, pRequest->mVANCode, L"VanCode", 4);
		_AddCommandParameter(pCommand, pRequest->mTelReqType, L"TextType", 4);
		_AddCommandParameter(pCommand, pRequest->mTelCode, L"TextClassCode", 6);
		_AddCommandParameter(pCommand, &(pRequest->mSendFlag), L"SendRecvFlag", 1);
		_AddCommandParameter(pCommand, pRequest->mStatus, L"Status", 3);
		_AddCommandParameter(pCommand, pRequest->mTelRespType, L"ReturnCode", 3);
		_AddCommandParameter(pCommand, pRequest->mSendDate, L"SendDate", 8);
		_AddCommandParameter(pCommand, pRequest->mSendTime, L"SendTime", 6);
		_AddCommandParameter(pCommand, pRequest->mTraceNo, L"TextTraceNum", 8);

		// 공통부 filler 15자리 중 앞 8자리에 PG가 생성한 전문추적 번호를 넣음.
		if (FALSE == bRequest) {
			char PGTranceNO[16];
			ZeroMemory(PGTranceNO, 16);
			_snprintf_s(PGTranceNO, sizeof(PGTranceNO), "BB%06d0000000", pClient->m_PGTraceNO);
			_AddCommandParameter(pCommand, PGTranceNO, L"Filler1", 15);
		} else _AddCommandParameter(pCommand, "               ", L"Filler1", 15);

		_AddCommandParameter(pCommand, pRequest->mPurchaseDate, L"TradeDate", 8);
		_AddCommandParameter(pCommand, pRequest->mPurchaseTime, L"TradeTime", 6);
		_AddCommandParameter(pCommand, pRequest->mPosNo, L"FranchiseCode", 15);
		_AddCommandParameter(pCommand, pRequest->mPosName, L"FranchiseName", 50);

		_AddCommandParameter(pCommand, pRequest->mBarcode, L"BarcodeNum", 20);
		_AddCommandParameter(pCommand, pRequest->mPrice, L"TradeAmt", 12);
		_AddCommandParameter(pCommand, pRequest->mTID, L"Tid", 40);
		_AddCommandParameter(pCommand, pRequest->mAcceptNo, L"ApproveNum", 13);
		_AddCommandParameter(pCommand, pRequest->mResponseMessage, L"ReturnMsg", 256);

		char Filler[70];
		ZeroMemory(Filler, 70);
		for (int i=0; i<70; i++) Filler[i] = ' ';
		_AddCommandParameter(pCommand, Filler, L"Filler2", 70);
		_AddCommandParameter(pCommand, pClient->m_strClientIP, L"RequestIP");
		_AddCommandParameter(pCommand, L"BARCODE PG DAEMON", L"WorkerID");

		// Result
		_ParameterPtr pParamReturn = pCommand->CreateParameter(_bstr_t("Result"), adBSTR, adParamReturnValue, 3);
		pCommand->Parameters->Append(pParamReturn);

		//_RecordsetPtr pRS = 
		pCommand->Execute(NULL, NULL, adCmdStoredProc);

		_variant_t vTempValue = pParamReturn->Value;
		if(vTempValue.vt != VT_NULL)
		{
			strResult = (LPCTSTR)vTempValue.bstrVal;
		}

		pCommand.Release();
	}
	catch(_com_error &e) {
		CString s = (LPCTSTR)e.Description();

		CString strErrorLog;
		strErrorLog.Format(L"Call_BarCodePurchaseRequest DB Error : %s ------", s);
		AddLog(strErrorLog);

	}

	return strResult;
}


CString CADODB::Call_BarCodeRefundRequest(CVANProtocol *pClient, BOOL bRequest)
{
	if(IsConnect() != TRUE) {
		if (FALSE == Open()) {
			return FALSE;
		}
	}

	CString strResult;
	try
	{
		_CommandPtr pCommand;
		pCommand.CreateInstance(__uuidof(Command));

		pCommand->ActiveConnection = m_pConn; 
		pCommand->CommandText = _bstr_t(L"SP_BarCodeRefundRequest");
		pCommand->CommandType = adCmdStoredProc;
		pCommand->CommandTimeout = 5;

		CTELPacket *pRequest = bRequest ? pClient->m_pRequest : pClient->m_pResponse;

		// Pgdate
		SYSTEMTIME cTime;
		::GetLocalTime(&cTime);

		CString strCurrentDate;
		strCurrentDate.Format(L"%4d%02d%02d", cTime.wYear, cTime.wMonth, cTime.wDay);
		_AddCommandParameter(pCommand, strCurrentDate, L"Pgdate");

		CString strTraceNO;
		strTraceNO.Format(L"BB%06d", pClient->m_PGTraceNO);
		_AddCommandParameter(pCommand, strTraceNO, L"PgTraceNum");

		_AddCommandParameter(pCommand, pRequest->m_pktHeader.lenData, L"Header1", 4);
		_AddCommandParameter(pCommand, pRequest->m_pktHeader.VAN, L"Header2", 3);
		_AddCommandParameter(pCommand, pRequest->mSystemType, L"SysTypeCode", 3);
		_AddCommandParameter(pCommand, pRequest->mVANCode, L"VanCode", 4);
		_AddCommandParameter(pCommand, pRequest->mTelReqType, L"TextType", 4);
		_AddCommandParameter(pCommand, pRequest->mTelCode, L"TextClassCode", 6);
		_AddCommandParameter(pCommand, &(pRequest->mSendFlag), L"SendRecvFlag", 1);
		_AddCommandParameter(pCommand, pRequest->mStatus, L"Status", 3);
		_AddCommandParameter(pCommand, pRequest->mTelRespType, L"ReturnCode", 3);
		_AddCommandParameter(pCommand, pRequest->mSendDate, L"SendDate", 8);
		_AddCommandParameter(pCommand, pRequest->mSendTime, L"SendTime", 6);
		_AddCommandParameter(pCommand, pRequest->mTraceNo, L"TextTraceNum", 8);
		// 공통부 filler 15자리 중 앞 8자리에 PG가 생성한 전문추적 번호를 넣음.
		if (FALSE == bRequest) {
			char PGTranceNO[16];
			ZeroMemory(PGTranceNO, 16);
			_snprintf_s(PGTranceNO, sizeof(PGTranceNO), "BB%06d0000000", pClient->m_PGTraceNO);
			_AddCommandParameter(pCommand, PGTranceNO, L"Filler1", 15);
		} else _AddCommandParameter(pCommand, "               ", L"Filler1", 15);

		_AddCommandParameter(pCommand, pRequest->mPurchaseDate, L"TradeDate", 8);
		_AddCommandParameter(pCommand, pRequest->mPurchaseTime, L"TradeTime", 6);
		_AddCommandParameter(pCommand, pRequest->mPosNo, L"FranchiseCode", 15);
		_AddCommandParameter(pCommand, pRequest->mPosName, L"FranchiseName", 50);

		_AddCommandParameter(pCommand, pRequest->mTID, L"Tid", 40);
		_AddCommandParameter(pCommand, pRequest->mOrgPurchaseDate, L"OriTradeDate", 8);
		_AddCommandParameter(pCommand, pRequest->mOrgPurchasePrice, L"OriTradeAmt", 12);
		_AddCommandParameter(pCommand, pRequest->mOrgTID, L"OriTradeTid", 40);
		_AddCommandParameter(pCommand, pRequest->mAcceptNo, L"OriTradeAppNum", 13);
		_AddCommandParameter(pCommand, pRequest->mResponseMessage, L"ReturnMsg", 256);

		char Filler[42];
		for (int i=0; i<42; i++) Filler[i] = ' ';
		_AddCommandParameter(pCommand, Filler, L"Filler2", 42);
		_AddCommandParameter(pCommand, pClient->m_strClientIP, L"RequestIP");
		_AddCommandParameter(pCommand, L"BARCODE PG DAEMON", L"WorkerID");

		// Result
		_ParameterPtr pParamReturn = pCommand->CreateParameter(_bstr_t("Result"), adBSTR, adParamReturnValue, 3);
		pCommand->Parameters->Append(pParamReturn);

		//_RecordsetPtr pRS = 
		pCommand->Execute(NULL, NULL, adCmdStoredProc);

		_variant_t vTempValue = pParamReturn->Value;
		if(vTempValue.vt != VT_NULL)
		{
			strResult = (LPCTSTR)vTempValue.bstrVal;
		}

		pCommand.Release();
	}
	catch(_com_error &e) {
		USES_CONVERSION;
		CString s = (LPCTSTR)e.Description();

		CString strErrorLog;
		strErrorLog.Format(L"Call_BarCodeRefundRequest DB Error : %s ------", s);
		AddLog(strErrorLog);

		return L"";
	}

	return strResult;
}


CString CADODB::Call_CouponApproveRequest(CVANProtocol *pClient, BOOL bRequest)
{
	if(IsConnect() != TRUE) {
		if (FALSE == Open()) {
			return FALSE;
		}
	}

	CString strResult;
	try
	{
		_CommandPtr pCommand;
		pCommand.CreateInstance(__uuidof(Command));

		pCommand->ActiveConnection = m_pConn; 
		pCommand->CommandText = _bstr_t(L"SP_CouponApproveRequest");
		pCommand->CommandType = adCmdStoredProc;
		pCommand->CommandTimeout = 5;

		CTELPacket *pRequest = bRequest ? pClient->m_pRequest : pClient->m_pResponse;

		// Pgdate
		SYSTEMTIME cTime;
		::GetLocalTime(&cTime);

		CString strCurrentDate;
		strCurrentDate.Format(L"%4d%02d%02d", cTime.wYear, cTime.wMonth, cTime.wDay);
		_AddCommandParameter(pCommand, strCurrentDate, L"Pgdate");

		CString strTraceNO;
		strTraceNO.Format(L"BB%06d", pClient->m_PGTraceNO);
		_AddCommandParameter(pCommand, strTraceNO, L"PgTraceNum");

		_AddCommandParameter(pCommand, pRequest->m_pktHeader.lenData, L"Header1", 4);
		_AddCommandParameter(pCommand, pRequest->m_pktHeader.VAN, L"Header2", 3);
		_AddCommandParameter(pCommand, pRequest->mSystemType, L"SysTypeCode", 3);
		_AddCommandParameter(pCommand, pRequest->mVANCode, L"VanCode", 4);
		_AddCommandParameter(pCommand, pRequest->mTelReqType, L"TextType", 4);
		_AddCommandParameter(pCommand, pRequest->mTelCode, L"TextClassCode", 6);
		_AddCommandParameter(pCommand, &(pRequest->mSendFlag), L"SendRecvFlag", 1);
		_AddCommandParameter(pCommand, pRequest->mStatus, L"Status", 3);
		_AddCommandParameter(pCommand, pRequest->mTelRespType, L"ReturnCode", 3);
		_AddCommandParameter(pCommand, pRequest->mSendDate, L"SendDate", 8);
		_AddCommandParameter(pCommand, pRequest->mSendTime, L"SendTime", 6);
		_AddCommandParameter(pCommand, pRequest->mTraceNo, L"TextTraceNum", 8);

		// 공통부 filler 15자리 중 앞 8자리에 PG가 생성한 전문추적 번호를 넣음.
		if (FALSE == bRequest) {
			char PGTranceNO[16];
			ZeroMemory(PGTranceNO, 16);
			_snprintf_s(PGTranceNO, sizeof(PGTranceNO), "BB%06d0000000", pClient->m_PGTraceNO);
			_AddCommandParameter(pCommand, PGTranceNO, L"Filler1", 15);
		} else _AddCommandParameter(pCommand, "               ", L"Filler1", 15);

		_AddCommandParameter(pCommand, pRequest->mPurchaseDate, L"TradeDate", 8);
		_AddCommandParameter(pCommand, pRequest->mPurchaseTime, L"TradeTime", 6);
		_AddCommandParameter(pCommand, pRequest->mPosNo, L"FranchiseCode", 15);
		_AddCommandParameter(pCommand, pRequest->mPosName, L"FranchiseName", 50);

		_AddCommandParameter(pCommand, pRequest->mBarcode, L"BarcodeNum", 20);
		_AddCommandParameter(pCommand, pRequest->mPrice, L"TradeAmt", 12);
		_AddCommandParameter(pCommand, pRequest->mTID, L"Tid", 40);
		_AddCommandParameter(pCommand, pRequest->mAcceptNo, L"ApproveNum", 13);
		_AddCommandParameter(pCommand, pRequest->mResponseMessage, L"ReturnMsg", 256);
		_AddCommandParameter(pCommand, pRequest->mProductCode, L"ProductCode", 20);

		char Filler[50];
		ZeroMemory(Filler, 50);
		for (int i=0; i<50; i++) Filler[i] = ' ';
		_AddCommandParameter(pCommand, Filler, L"Filler2", 50);
		_AddCommandParameter(pCommand, pClient->m_strClientIP, L"RequestIP");
		_AddCommandParameter(pCommand, L"BARCODE PG DAEMON", L"WorkerID");

		// Result
		_ParameterPtr pParamReturn = pCommand->CreateParameter(_bstr_t("Result"), adBSTR, adParamReturnValue, 3);
		pCommand->Parameters->Append(pParamReturn);

		//_RecordsetPtr pRS = 
		pCommand->Execute(NULL, NULL, adCmdStoredProc);

		_variant_t vTempValue = pParamReturn->Value;
		if(vTempValue.vt != VT_NULL)
		{
			strResult = (LPCTSTR)vTempValue.bstrVal;
		}

		pCommand.Release();
	}
	catch(_com_error &e) {
		CString s = (LPCTSTR)e.Description();

		CString strErrorLog;
		strErrorLog.Format(L"Call_CouponApproveRequest DB Error : %s ------", s);
		AddLog(strErrorLog);

		return L"";
	}

	return strResult;
}


CString CADODB::Call_CouponStatusRequest(CVANProtocol *pClient, BOOL bRequest)
{
	if(IsConnect() != TRUE) {
		if (FALSE == Open()) {
			return FALSE;
		}
	}

	CString strResult;
	try
	{
		_CommandPtr pCommand;
		pCommand.CreateInstance(__uuidof(Command));

		pCommand->ActiveConnection = m_pConn; 
		pCommand->CommandText = _bstr_t(L"SP_CouponStatusRequest");
		pCommand->CommandType = adCmdStoredProc;
		pCommand->CommandTimeout = 5;

		CTELPacket *pRequest = bRequest ? pClient->m_pRequest : pClient->m_pResponse;

		// Pgdate
		SYSTEMTIME cTime;
		::GetLocalTime(&cTime);

		CString strCurrentDate;
		strCurrentDate.Format(L"%4d%02d%02d", cTime.wYear, cTime.wMonth, cTime.wDay);
		_AddCommandParameter(pCommand, strCurrentDate, L"Pgdate");

		CString strTraceNO;
		strTraceNO.Format(L"BB%06d", pClient->m_PGTraceNO);
		_AddCommandParameter(pCommand, strTraceNO, L"PgTraceNum");

		_AddCommandParameter(pCommand, pRequest->m_pktHeader.lenData, L"Header1", 4);
		_AddCommandParameter(pCommand, pRequest->m_pktHeader.VAN, L"Header2", 3);
		_AddCommandParameter(pCommand, pRequest->mSystemType, L"SysTypeCode", 3);
		_AddCommandParameter(pCommand, pRequest->mVANCode, L"VanCode", 4);
		_AddCommandParameter(pCommand, pRequest->mTelReqType, L"TextType", 4);
		_AddCommandParameter(pCommand, pRequest->mTelCode, L"TextClassCode", 6);
		_AddCommandParameter(pCommand, &(pRequest->mSendFlag), L"SendRecvFlag", 1);
		_AddCommandParameter(pCommand, pRequest->mStatus, L"Status", 3);
		_AddCommandParameter(pCommand, pRequest->mTelRespType, L"ReturnCode", 3);
		_AddCommandParameter(pCommand, pRequest->mSendDate, L"SendDate", 8);
		_AddCommandParameter(pCommand, pRequest->mSendTime, L"SendTime", 6);
		_AddCommandParameter(pCommand, pRequest->mTraceNo, L"TextTraceNum", 8);
		// 공통부 filler 15자리 중 앞 8자리에 PG가 생성한 전문추적 번호를 넣음.
		if (FALSE == bRequest) {
			char PGTranceNO[16];
			ZeroMemory(PGTranceNO, 16);
			_snprintf_s(PGTranceNO, sizeof(PGTranceNO), "BB%06d0000000", pClient->m_PGTraceNO);
			_AddCommandParameter(pCommand, PGTranceNO, L"Filler1", 15);
		} else _AddCommandParameter(pCommand, "               ", L"Filler1", 15);

		_AddCommandParameter(pCommand, pRequest->mPurchaseDate, L"TradeDate", 8);
		_AddCommandParameter(pCommand, pRequest->mPurchaseTime, L"TradeTime", 6);
		_AddCommandParameter(pCommand, &pRequest->mCouponStatus, L"CouponStatus", 1);
		_AddCommandParameter(pCommand, pRequest->mPosNo, L"FranchiseCode", 15);
		_AddCommandParameter(pCommand, pRequest->mPosName, L"FranchiseName", 50);

		_AddCommandParameter(pCommand, pRequest->mBarcode, L"BarcodeNum", 20);
		_AddCommandParameter(pCommand, pRequest->mPrice, L"TradeAmt", 12);
		_AddCommandParameter(pCommand, pRequest->mTID, L"Tid", 40);
		_AddCommandParameter(pCommand, pRequest->mAcceptNo, L"ApproveNum", 13);
		_AddCommandParameter(pCommand, pRequest->mAcceptDateTime, L"ApproveDateTime", 14);
		_AddCommandParameter(pCommand, pRequest->mResponseMessage, L"ReturnMsg", 256);
		_AddCommandParameter(pCommand, pRequest->mProductCode, L"ProductCode", 20);

		char Filler[35];
		ZeroMemory(Filler, 35);
		for (int i=0; i<35; i++) Filler[i] = ' ';
		_AddCommandParameter(pCommand, Filler, L"Filler2", 70);
		_AddCommandParameter(pCommand, pClient->m_strClientIP, L"RequestIP");
		_AddCommandParameter(pCommand, L"BARCODE PG DAEMON", L"WorkerID");

		// Result
		_ParameterPtr pParamReturn = pCommand->CreateParameter(_bstr_t("Result"), adBSTR, adParamReturnValue, 3);
		pCommand->Parameters->Append(pParamReturn);

		//_RecordsetPtr pRS = 
		pCommand->Execute(NULL, NULL, adCmdStoredProc);

		_variant_t vTempValue = pParamReturn->Value;
		if(vTempValue.vt != VT_NULL)
		{
			strResult = (LPCTSTR)vTempValue.bstrVal;
		}

		pCommand.Release();
	}
	catch(_com_error &e) {
		CString s = (LPCTSTR)e.Description();

		CString strErrorLog;
		strErrorLog.Format(L"Call_CouponStatusRequest DB Error : %s ------", s);
		AddLog(strErrorLog);
	}

	return strResult;
}

CString _FetchColumn(_RecordsetPtr pRS, LPCTSTR columnName)
{
	CString columnValue;
	
	_variant_t vTempValue;		
	vTempValue = pRS->GetCollect(columnName);
	if(vTempValue.vt != VT_NULL)
	{
		columnValue = (LPCTSTR)vTempValue.bstrVal;
	}
	
	return columnValue;
}

void CADODB::GetSysRefundList(VEC_REFUND_LIST *pRefundList)
{
	if(IsConnect() != TRUE) {
		if (FALSE == Open()) {
			return;
		}
	}

	CString strQuery;
	strQuery.Format(L"select * from FN_GetSysRefundList(\'%s\', \'%s\', \'%s\')", getYesterDay(), L"CN01", L"BARDCODE PG DAEMON");
	try
	{
		_bstr_t	bstrQuery(strQuery);
		_variant_t	vRecsAffected2(0L);
		
		m_pRS = m_pConn->Execute(bstrQuery, &vRecsAffected2, adOptionUnspecified);
		while(!(m_pRS->EndOfFile))
		{
			ST_REFUND_ITEM itemRefund;
			itemRefund.strType = _FetchColumn(m_pRS, L"Type");
			itemRefund.strSerialNum = _FetchColumn(m_pRS, L"SerialNum");
			itemRefund.strTradeDate = _FetchColumn(m_pRS, L"TradeDate");
			itemRefund.strFranchiseNum = _FetchColumn(m_pRS, L"FranchiseNum");
			itemRefund.strBarcodeNum = _FetchColumn(m_pRS, L"BarcodeNum");
			itemRefund.strTradeAmt = _FetchColumn(m_pRS, L"TradeAmt");
			itemRefund.strTid = _FetchColumn(m_pRS, L"Tid");
			itemRefund.strFiller = _FetchColumn(m_pRS, L"Filler");

			pRefundList->push_back(itemRefund);

			m_pRS->MoveNext();
		}				
	}
	catch(_com_error &e)
	{
		CString s = (LPCTSTR)e.Description();

		CString strErrorLog;
		strErrorLog.Format(L"GetSysRefundList DB Error : %s ------", s);
		AddLog(strErrorLog);

		if (0 <= s.Find(L"ORA-03114") || 0 <= s.Find(L"ORA-03113"))
		{
			Close();
		}

		m_strErrorMessage = L"GetSysRefundList 에러 (" + s + L")\n";
	}
}


void CADODB::GetSysRefundNoReplyList(VEC_REFUND_NO_REPLY_LIST *pRefundList)
{
	USES_CONVERSION;

	if(IsConnect() != TRUE) {
		if (FALSE == Open()) {
			return;
		}
	}

	CString strQuery;
	strQuery.Format(L"select * from FN_GetSysRefundNoReplyList(\'%s\', \'%s\', \'%s\')", getYesterDay(), L"CN02", L"BARDCODE PG DAEMON");
	try
	{
		_bstr_t	bstrQuery(strQuery);
		_variant_t	vRecsAffected2(0L);
		
		m_pRS = m_pConn->Execute(bstrQuery, &vRecsAffected2, adOptionUnspecified);
		while(!(m_pRS->EndOfFile))
		{
			ST_REFUND_NO_REPLY_ITEM itemRefund;
			itemRefund.strType = _FetchColumn(m_pRS, L"Type");
			itemRefund.strSerialNum = _FetchColumn(m_pRS, L"SerialNum");
			itemRefund.strTradeDate = _FetchColumn(m_pRS, L"TradeDate");
			itemRefund.strTid = _FetchColumn(m_pRS, L"Tid");
			itemRefund.strFranchiseNum = _FetchColumn(m_pRS, L"FranchiseNum");
			itemRefund.strBarcodeNum = _FetchColumn(m_pRS, L"BarcodeNum");
			itemRefund.strTradeAmt = _FetchColumn(m_pRS, L"TradeAmt");
			itemRefund.strOriTradeDate = _FetchColumn(m_pRS, L"OriTradeDate");
			itemRefund.strOriTid = _FetchColumn(m_pRS, L"OriTid");
			itemRefund.strFiller = _FetchColumn(m_pRS, L"Filler");

			pRefundList->push_back(itemRefund);

			m_pRS->MoveNext();
		}				
	}
	catch(_com_error &e)
	{
		CString s = (LPCTSTR)e.Description();

		CString strErrorLog;
		strErrorLog.Format(L"GetSysRefundNoReplyList DB Error : %s ------", s);
		AddLog(strErrorLog);

		if (0 <= s.Find(L"ORA-03114") || 0 <= s.Find(L"ORA-03113"))
		{
			Close();
		}

		m_strErrorMessage = L"GetSysRefundList 에러 (" + s + L")\n";
	}
}

void CADODB::GetCouponNotReplyList(VEC_COUPON_NOT_REPLY_LIST *pCouponList)
{
	USES_CONVERSION;

	if(IsConnect() != TRUE) {
		if (FALSE == Open()) {
			return;
		}
	}

	CString strQuery;
	strQuery.Format(L"select * from FN_GetCouponNotReplyList(\'%s\', \'%s\', \'%s\')", getYesterDay(), L"CN04", L"BARDCODE PG DAEMON");
	try
	{
		_bstr_t	bstrQuery(strQuery);
		_variant_t	vRecsAffected2(0L);
		
		m_pRS = m_pConn->Execute(bstrQuery, &vRecsAffected2, adOptionUnspecified);
		while(!(m_pRS->EndOfFile))
		{
			_variant_t vTempValue;

			ST_COUPON_NOT_REPLY_ITEM stCoupon;
			stCoupon.strJobType = _FetchColumn(m_pRS, L"JobType");
			stCoupon.strDataType = _FetchColumn(m_pRS, L"DataType");
			stCoupon.strSerialNum = _FetchColumn(m_pRS, L"SerialNum");
			stCoupon.strTradeType = _FetchColumn(m_pRS, L"TradeType");
			stCoupon.strReturnCode = _FetchColumn(m_pRS, L"ReturnCode");
			stCoupon.strTradeDate = _FetchColumn(m_pRS, L"TradeDate");
			stCoupon.strTradeTime = _FetchColumn(m_pRS, L"TradeTime");
			stCoupon.strTradeUniqueNum = _FetchColumn(m_pRS, L"TradeUniqueNum");
			stCoupon.strBarcodeNum = _FetchColumn(m_pRS, L"BarcodeNum");
			stCoupon.strTid = _FetchColumn(m_pRS, L"Tid");
			stCoupon.strFranchiseName = _FetchColumn(m_pRS, L"FranchiseName");
			stCoupon.strTradeAmt = _FetchColumn(m_pRS, L"TradeAmt");
			stCoupon.strApproveNum = _FetchColumn(m_pRS, L"ApproveNum");
			stCoupon.strProductCode = _FetchColumn(m_pRS, L"ProductCode");
			stCoupon.strFiller = _FetchColumn(m_pRS, L"Filler");

			pCouponList->push_back(stCoupon);

			m_pRS->MoveNext();
		}				
	}
	catch(_com_error &e)
	{
		CString s = (LPCTSTR)e.Description();

		CString strErrorLog;
		strErrorLog.Format(L"GetCouponNotReplyList DB Error : %s ------", s);
		AddLog(strErrorLog);

		if (0 <= s.Find(L"ORA-03114") || 0 <= s.Find(L"ORA-03113"))
		{
			Close();
		}

		m_strErrorMessage = L"GetSysRefundList 에러 (" + s + L")\n";
	}
}


CString CADODB::Call_GenerateVanBillList(LPCTSTR fileName, LPCTSTR orgCode, VEC_ST_VAN_BILL_DATA *pVecBillData)
{
	USES_CONVERSION;

	if(IsConnect() != TRUE) {
		if (FALSE == Open()) {
			return FALSE;
		}
	}

	CString strResult;
	for (VEC_ST_VAN_BILL_DATA::iterator itr = pVecBillData->begin(); itr != pVecBillData->end(); itr++) {
		try
		{
			_CommandPtr pCommand;
			pCommand.CreateInstance(__uuidof(Command));

			pCommand->ActiveConnection = m_pConn; 
			pCommand->CommandText = _bstr_t(L"SP_GenerateVanBillList");
			pCommand->CommandType = adCmdStoredProc;
			pCommand->CommandTimeout = 5;

			_AddCommandParameter(pCommand, fileName, L"FileName");
			_AddCommandParameter(pCommand, L"CN03", L"JobGubun");
			_AddCommandParameter(pCommand, getYesterDay(), L"FileCreateDate");
			_AddCommandParameter(pCommand, orgCode, L"OrgCode");
			_AddCommandParameter(pCommand, itr->RecordType, L"RecordType", 2);
			_AddCommandParameter(pCommand, itr->SerialNo, L"SerialNum", 7);
			_AddCommandParameter(pCommand, itr->TradeType, L"TradeType", 1);
			_AddCommandParameter(pCommand, itr->Date, L"TradeDate", 8);
			_AddCommandParameter(pCommand, itr->TID, L"TID", 40);
			_AddCommandParameter(pCommand, itr->Barcode, L"BarcodeNum", 20);
			_AddCommandParameter(pCommand, itr->PosNO, L"FranchiseNum", 15);
			_AddCommandParameter(pCommand, itr->Price, L"TradeAmt", 12);
			_AddCommandParameter(pCommand, itr->Filler, L"Filler", 95);
			_AddCommandParameter(pCommand, L"BARCODE PG DAEMON", L"WorkerID");

			// Result
			_ParameterPtr pParamReturn = pCommand->CreateParameter(_bstr_t("Result"), adBSTR, adParamReturnValue, 3);
			pCommand->Parameters->Append(pParamReturn);

			//_RecordsetPtr pRS = 
			pCommand->Execute(NULL, NULL, adCmdStoredProc);

			_variant_t vTempValue = pParamReturn->Value;
			if(vTempValue.vt != VT_NULL)
			{
				strResult = (LPCTSTR)vTempValue.bstrVal;
			}

			pCommand.Release();
		}
		catch(_com_error &e) {
			CString s = (LPCTSTR)e.Description();
			CString strErrorLog;
			strErrorLog.Format(L"Call_GenerateVanBillList DB Error : %s ------", s);
			AddLog(strErrorLog);
		}
	}

	return strResult;
}

void  CADODB::AddLog(LPCTSTR logString)
{
	USES_CONVERSION;
	theApp.AddLog(T2A(logString));
}