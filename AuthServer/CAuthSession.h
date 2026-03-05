#pragma once

#include "Types.h"
#include "Session.h"

class CAuthSession : public PacketSession
{
public:
	CAuthSession(int clientSocket, struct sockaddr_in addr, ServiceRef service);
	~CAuthSession();
private:
	virtual void	OnWrite(int len) override;
	virtual void	OnReadPacket(BYTE* buffer, int len) override;
};