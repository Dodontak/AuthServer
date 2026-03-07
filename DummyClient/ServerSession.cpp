#include "ServerSession.h"
#include "ServerPacketHandler.h"
#include "CoreGlobal.h"
#include "ThreadManager.h"
#include "JobQueue.h"
#include <functional>
#include <iostream>

ServerSession::ServerSession(int clientSocket, struct sockaddr_in addr, ServiceRef service)
	: PacketSession(clientSocket, addr, service)
{
	_isClient = false;
}


ServerSession::~ServerSession()
{

}


void	ServerSession::OnWrite(int len)
{
}

void	ServerSession::OnReadPacket(BYTE* buffer, int len)
{
	std::function<void()>	callback;
	PacketSessionRef	session = std::static_pointer_cast<PacketSession>(shared_from_this());
	bool	success = ServerPacketHandler::PacketHandler(callback, session, buffer, len);
	if (success)
	{
		JobRef			job = std::make_shared<Job>(callback);
		GThreadManager->InsertJob(job);
	}
	else
	{
		Disconnect();
	}
}