#include "StdAfx.h"
#include "PGServer.h"
#include <Mswsock.h>
#include "SubVANServer.h"
#include "VANProtocol.h"
#include "PosClient.h"
#include "VANClient.h"
#include "../define.h"
#include "SubVANServerDlg.h"

CPGServer::CPGServer(void)
{
	nServerPort = DEF_PORT_SVCSVR;
	m_nMaxThreadCount = 5;

	TCHAR szSerialTraceNO[10];
	DWORD dwTraceNO = 1;
	DWORD dwRet = GetPrivateProfileStringW(L"TRACE", L"TraceSerial", _T(""), szSerialTraceNO, sizeof(szSerialTraceNO), g_strRegPath);
	if (dwRet) dwTraceNO = _tstol(szSerialTraceNO);
	else dwTraceNO = 1;
	m_sPGTraceNO = dwTraceNO;

	InitializeCriticalSection(&m_csMakePGTrace);
}

CPGServer::~CPGServer(void)
{
	delete m_phAcceptThreads;
	DeleteCriticalSection(&m_csMakePGTrace);
}

DWORD CPGServer::GetNewPGTraceNO()
{
	EnterCriticalSection(&m_csMakePGTrace);

	SYSTEMTIME cTime;
	::GetLocalTime(&cTime);

	CString strCurrentDate;
	strCurrentDate.Format(L"%4d-%02d-%02d", cTime.wYear, cTime.wMonth, cTime.wDay);

	TCHAR szTraceDate[11];
	DWORD dwRet = GetPrivateProfileStringW(L"TRACE", L"TraceDate", _T(""), szTraceDate, sizeof(szTraceDate), g_strRegPath);
	if (!dwRet || strCurrentDate.Compare(szTraceDate)) {
		m_sPGTraceNO = 1;
		WritePrivateProfileString(L"TRACE", L"TraceSerial", L"1", g_strRegPath); 
		WritePrivateProfileString(L"TRACE", L"TraceDate", strCurrentDate, g_strRegPath); 

		LeaveCriticalSection(&m_csMakePGTrace);

		AfxGetApp()->m_pMainWnd->PostMessage(WM_APP+101, 1, 0);

		return m_sPGTraceNO;
	}
	
	TCHAR szSerialTraceNO[10];
	DWORD dwTraceNO = 1;
	dwRet = GetPrivateProfileStringW(L"TRACE", L"TraceSerial", _T(""), szSerialTraceNO, sizeof(szSerialTraceNO), g_strRegPath);
	if (dwRet) {
		dwTraceNO = _tstol(szSerialTraceNO);
	} else {
		dwTraceNO = 0;
	}

	m_sPGTraceNO = ++dwTraceNO;

	_ltow_s(m_sPGTraceNO, szSerialTraceNO, 10);
	dwRet = WritePrivateProfileString(L"TRACE", L"TraceSerial", szSerialTraceNO, g_strRegPath); 

	LeaveCriticalSection(&m_csMakePGTrace);

	AfxGetApp()->m_pMainWnd->PostMessage(WM_APP+101, m_sPGTraceNO, 0);

	return m_sPGTraceNO;
}

BOOL CPGServer::InitServer(void)
{
	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) 
	{
		//g_AgentWnd.AddSvrLog(L"%s", L"네트워크를 사용할 수 없습니다");
		return FALSE;
	}

	TCHAR szThreadCount[10];
	DWORD dwRet = GetPrivateProfileStringW(L"SERVER", L"ThreadPool", _T(""), szThreadCount, sizeof(szThreadCount), g_strRegPath);
	if (dwRet) m_nMaxThreadCount = _tstol(szThreadCount);

	m_sockListen = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, 0);
	if(m_sockListen == INVALID_SOCKET)
	{
		//g_AgentWnd.AddSvrLog(L"%s", L"네트워크를 사용할 수 없습니다");
		return FALSE;
	}

	sockaddr_in local;
	local.sin_family=AF_INET;
	local.sin_addr.s_addr=INADDR_ANY;

	TCHAR szPort[10];
	dwRet = GetPrivateProfileStringW(L"SERVER", L"Port", _T(""), szPort, sizeof(szPort), g_strRegPath);
	if (dwRet) nServerPort = _tstol(szPort);
	local.sin_port = htons((u_short)nServerPort);
	if (bind(m_sockListen, (sockaddr*)&local, sizeof(local)) !=0 )
	{
		closesocket(m_sockListen);
		//g_AgentWnd.AddSvrLog(L"%s", L"해당 Port를 사용할 수 없습니다");

		return FALSE;
	}

	if (listen(m_sockListen, SOMAXCONN) != ERROR_SUCCESS)
	{
		//g_AgentWnd.AddSvrLog(L"%s", L"서버를 시작할 수 없습니다");
		return FALSE;
	}

	m_dwTimeOut = RESP_TIMEOUT;
	TCHAR szTimeOut[10];
	if (GetPrivateProfileStringW(L"CONFIG", L"ConnTimeout", _T(""), szTimeOut, sizeof(szTimeOut), g_strRegPath))
	{
		m_dwTimeOut = _tstol(szTimeOut);
	}

	unsigned int nThreadID;
	m_phAcceptThreads = new HANDLE[m_nMaxThreadCount];
	for (int i = 0; i < m_nMaxThreadCount; i++)
	{
		m_phAcceptThreads[i] = (HANDLE)_beginthreadex(0, 0, AcceptThread, this, 0, &nThreadID);
	}

	return TRUE;
}

unsigned int __stdcall CPGServer::AcceptThread(void *param)
{
	CPGServer* pPGServer = (CPGServer*)param;

	int ErrCode=0;
	int sockaddr_size = sizeof(SOCKADDR_IN);    

	SOCKET ListenSocket = pPGServer->m_sockListen;
	//Accept thread will be around to look for accept event, until a Shutdown event is not Signaled.
	while(WAIT_OBJECT_0 != WaitForSingleObject(pPGServer->m_hShutdownEvent, 0))
	{
		//struct sockaddr cli_addr;
		SOCKADDR_IN cli_addr;
		SOCKET socketx = accept(ListenSocket, (struct sockaddr*)&cli_addr, &sockaddr_size);
		if (socketx) {
			CPosClient *pClient = new CPosClient(socketx, pPGServer);
			pClient->m_strClientIP = inet_ntoa(cli_addr.sin_addr);

			unsigned int nThreadID;
			HANDLE hClientThread = (HANDLE)_beginthreadex(0, 0, ClientWorkerThread, pClient, 0, &nThreadID);
		}
	}

	return 0;
}

unsigned int __stdcall CPGServer::ClientWorkerThread(void *param) {
	CPosClient *pClient = (CPosClient *)param;
	CPGServer *pPGServer = pClient->m_pPGServer;

	DWORD dwNewPGTraceNO = pPGServer->GetNewPGTraceNO();
	pClient->m_PGTraceNO = dwNewPGTraceNO;

	USES_CONVERSION;
	ST_MONITOR_LIST_INFO *pListInfo = new ST_MONITOR_LIST_INFO;
	pListInfo->strIP = pClient->m_strClientIP;
	pListInfo->index.Format(L"%08u", dwNewPGTraceNO);
	pListInfo->strDirection = L"POS->PG";

	CSubVANServerDlg *pMonitorDlg = (CSubVANServerDlg*)AfxGetApp()->m_pMainWnd;
	pClient->m_dwRequestListRow = pMonitorDlg->m_monList.GetItemCount();
	AfxGetApp()->m_pMainWnd->PostMessage(WM_ADD_REQUEST_ITEM, (WPARAM)-1, (LPARAM)pListInfo);

	BYTE *pRecvBuffer = new BYTE[MAX_BUFFER];
	ZeroMemory(pRecvBuffer, MAX_BUFFER);

	char *pDataBuffer = new char[MAX_BUFFER];
	ZeroMemory(pDataBuffer, MAX_BUFFER);

	int iResult = -1;
	int nReceived = 0;

	PG_ERROR_CODE result = ERROR_WAIT;
	do {
		iResult = 0;
		char recvBuffer[MAX_BUFFER];
		iResult = recv(pClient->m_sock, recvBuffer, MAX_BUFFER, 0);
		if (iResult > 0) {
			if (nReceived + iResult > MAX_BUFFER) {
				iResult = MAX_BUFFER - nReceived;
			}
			memcpy(pDataBuffer + nReceived, recvBuffer, iResult);
			nReceived += iResult;

			result = CVANProtocol::CheckHeader(nReceived, pDataBuffer);
			if (ERROR_OK == result)
			{
				theApp.AddLog("POS->PG:%s", pDataBuffer);
				result = pClient->SetRequestData(pDataBuffer, nReceived);
			
				break;
			}

			if (ERROR_WAIT != iResult) break;
		}
		else if (iResult == 0) {
//			Connection closing...
			break;
		} else {
//			recv failed with error
			break;
		}
	} while (iResult > 0);

	pPGServer->ProcessVAN(result, pClient);

	pClient->CloseSocket();
	delete pClient;

	delete pRecvBuffer;
	delete pDataBuffer;
	
	return 0;
}

void CPGServer::_SaveProcessToDB(CVANProtocol *pClient, BOOL bRequest)
{
	TR_TYPE trType = CVANProtocol::checkTRFromat(pClient->m_pRequest->mTelCode);

	if (CMD_PURCHASE == trType) {
		theApp.GetDBManager()->Call_BarCodePurchaseRequest(pClient, bRequest);
	} else if (CMD_REFUND == trType) {
		theApp.GetDBManager()->Call_BarCodeRefundRequest(pClient, bRequest);
	} else if (CMD_COUPON == trType) {
		theApp.GetDBManager()->Call_CouponApproveRequest(pClient, bRequest);
	} else if (CMD_COUPON_STAUTS == trType) {
		theApp.GetDBManager()->Call_CouponStatusRequest(pClient, bRequest);
	}
}

void CPGServer::_StartVANProcess(CPosClient *pClient) 
{
	_SaveProcessToDB(pClient, TRUE);

	///// POS --> PG 성공 ////
	BOOL bRes = FALSE;
	CVANClient *pVANClient = new CVANClient();

	pClient->m_PGTraceNO = m_sPGTraceNO;
	pVANClient->m_PGTraceNO = m_sPGTraceNO;

	if (ERROR_OK == pVANClient->MakeVANRequestData(pClient->m_pRequest)) {
		_SaveProcessToDB(pVANClient, TRUE);

		if (COMM_ERROR_SUCCESS == pVANClient->ConnectEx()) {
			bRes = pVANClient->SendRequestToVAN(pClient);
		}
	}
	
	//bRes : TRUE - VAN 서버 접속 성공, FALSE - VAN 서버에 접속을 하지 못함
	DWORD dwWait = WaitForSingleObject(pVANClient->m_hWaitVANProcess, VAN_TIMEOUT);
	if (WAIT_TIMEOUT == dwWait) {
		_SaveProcessToDB(pVANClient, FALSE);
		pClient->CopyResponseData(pClient->m_pRequest);

		//  POS로 S11응답
		pClient->SendResultToPOS(RES_TIMEOUT);
		_SaveProcessToDB(pClient, FALSE);

		// TODO - VAN으로 환불요청
		pVANClient->Close();
	} else {
		_SaveProcessToDB(pVANClient, FALSE);
		pClient->CopyResponseData(pVANClient->m_pResponse);

		pClient->SendResultToPOS(RES_SUCCESS);
		_SaveProcessToDB(pClient, FALSE);
	}

	//////////////////////////////////////////////////////////
	// Time out이 난 경우 시스템 환불처리 한번 더 필요.
	//////////////////////////////////////////////////////////
	if (WAIT_TIMEOUT == dwWait) {
		ResetEvent(pVANClient->m_hWaitVANProcess);

		TR_TYPE trType = CVANProtocol::checkTRFromat(pVANClient->m_pRequest->mTelCode);
		// 시스템 환불코드
		if (trType == CMD_PURCHASE || trType == CMD_REFUND) {
			pVANClient->m_PGTraceNO = GetNewPGTraceNO();

			char traceNO[9];
			_snprintf_s(traceNO, 8, "BB%06d", pVANClient->m_PGTraceNO);
			memcpy(pVANClient->m_pRequest->mTraceNo, traceNO, 8);
			memcpy(pVANClient->m_pResponse->mTraceNo, traceNO, 8);

			if (trType == CMD_PURCHASE) {
				memcpy(pVANClient->m_pRequest->mTelReqType, "0420", 4);
			}

			if (trType == CMD_REFUND) {
				memcpy(pVANClient->m_pRequest->mTelReqType, "0500", 4);
				memcpy(pVANClient->m_pRequest->mTelCode, "810030", 6);
			}

			if (COMM_ERROR_SUCCESS == pVANClient->ConnectEx()) {
				bRes = pVANClient->SendRequestToVAN(pClient);
			}

			DWORD dwWait = WaitForSingleObject(pVANClient->m_hWaitVANProcess, VAN_TIMEOUT);

			if (WAIT_TIMEOUT == dwWait) {
				pVANClient->CopyResponseData(pVANClient->m_pRequest);

				char traceNO[9];
				_snprintf_s(traceNO, 8, "BB%06d", pVANClient->m_PGTraceNO);
				memcpy(pVANClient->m_pResponse->mTraceNo, traceNO, 8);

				CVANProtocol::setResponseErrorCode(pVANClient->m_pResponse->mTelRespType, RES_TIMEOUT);
			}

			// VAN 처리에 대한 응답을 DB에 기록
			_SaveProcessToDB(pVANClient, FALSE);
		}
	}

	delete pVANClient;
}

void CPGServer::ProcessVAN(PG_ERROR_CODE errCode, CPosClient *pClient) 
{
	USES_CONVERSION;
	if (ERROR_OK == errCode) {
		if (pClient->m_pRequest) {
			_StartVANProcess(pClient);

			return;
		} 
	} 

	if (pClient->m_pRequest) {
		ST_MONITOR_LIST_INFO *pListInfo = new ST_MONITOR_LIST_INFO;
		pListInfo->strIP = pClient->m_strClientIP;
		pListInfo->index.Format(L"%08u", m_sPGTraceNO);
		pListInfo->strDirection = L"PG->POS";

		CSubVANServerDlg *pMonitorDlg = (CSubVANServerDlg*)AfxGetApp()->m_pMainWnd;
		pClient->m_dwResponseListRow = pMonitorDlg->m_monList.GetItemCount();
		AfxGetApp()->m_pMainWnd->PostMessage(WM_ADD_RESPONSE_ITEM, (WPARAM)-1, (LPARAM)pListInfo);

		pClient->CopyResponseData(pClient->m_pRequest);
		pClient->SendResultToPOS(errCode);	
	}
}
