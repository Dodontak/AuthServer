#pragma once

#include "Types.h"
#include "Session.h"

class ServerSession : public PacketSession
{
public:
	ServerSession(int clientSocket, struct sockaddr_in addr, ServiceRef service);
	~ServerSession();
private:
	virtual void	OnWrite(int len) override;
	virtual void	OnReadPacket(BYTE* buffer, int len) override;
};