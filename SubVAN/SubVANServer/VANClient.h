#pragma once

#include "sockcomm.h"
#include "vanprotocol.h"

class CTELPacket;
class CPosClient;
class CVANClient : public CSockComm, public CVANProtocol
{
public:
	CVANClient::CVANClient(void);
	~CVANClient(void);

	HANDLE m_hWaitVANProcess;  //  VAN 통신 작업 완료 이벤트

	PG_ERROR_CODE MakeVANRequestData(CTELPacket *packet, BOOL sysRefund=FALSE);
	BOOL SendRequestToVAN(CPosClient *pClient);

	virtual BOOL OnReceive(int nErrorCode);
	virtual void OnSend(int nErrorCode){}
	virtual void OnClose(int nErrorCode);

private:
	CPosClient *m_pPosClient;

	void _MakeCmdReqPacket(char *pData);
	void _assignFieldValue(int *pos, char* pdata, char *value, int nLen);
};

