#pragma once

#include "VANProtocol.h"

class CPGServer;
class CPosClient : public CVANProtocol
{
public:
	CPosClient(SOCKET clSock, CPGServer *pServer);
	~CPosClient(void);

	SOCKET m_sock;
	CPGServer *m_pPGServer;

	//void SendResultToPOS(PG_ERROR_CODE errCode);
	void SendResultToPOS(RES_CODE resCode);
	void CloseSocket();

private:
	TR_TYPE _MakeCmdResponsePacket(char *pData);
};
