#pragma once

#include "telpacket.h"

#define MAX_BUFFER 1024

class CPosClient;
class CVANClient;
class CPGServer
{
public:
	CPGServer(void);
	~CPGServer(void);
	BOOL InitServer(void);

	static unsigned int __stdcall AcceptThread(void *param);
	static unsigned int __stdcall ClientWorkerThread(void *param);

	DWORD GetNewPGTraceNO();
	DWORD GetCurrentTraceNO(){
		return m_sPGTraceNO;
	}

private:
	void _SaveProcessToDB(CVANProtocol *pClient, BOOL bRequest);
	void ProcessVAN(RES_CODE errCode, CPosClient *pClient);
	void _StartVANProcess(CPosClient *pClient);
	void _ProcessSystemRefund(CPosClient *pClient);

	DWORD m_dwTimeOut;

	int nServerPort;
	CTime m_tDBTime;

	HANDLE *m_phAcceptThreads;
	HANDLE m_hShutdownEvent;

	SOCKET m_sockListen;

	//Network Event for Accept
	WSAEVENT m_hAcceptEvent;
	HANDLE m_hAcceptThread;

	int m_nMaxThreadCount;

	CRITICAL_SECTION m_csMakePGTrace;
	
	DWORD m_sPGTraceNO;
	CString m_PGTraceDate;
};
