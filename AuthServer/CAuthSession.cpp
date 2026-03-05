#include "CAuthSession.h"
#include "ClientPacketHandler.h"
#include "CoreGlobal.h"
#include "ThreadManager.h"
#include "JobQueue.h"
#include <functional>
#include <iostream>

CAuthSession::CAuthSession(int clientSocket, struct sockaddr_in addr, ServiceRef service) : PacketSession(clientSocket, addr, service)
{

}


CAuthSession::~CAuthSession()
{

}


void	CAuthSession::OnWrite(int len)
{
}

void	CAuthSession::OnReadPacket(BYTE* buffer, int len)
{
	PacketHeader	header = *reinterpret_cast<PacketHeader*>(buffer);

	std::function<void()>	callback;
	PacketSessionRef	session = std::static_pointer_cast<PacketSession>(shared_from_this());
	bool result = ClientPacketHandler::GetCallback(callback, session, buffer, len);
	if (result == false)
		std::cout << "error" << std::endl;
	else
	{
		std::cout << "success" << std::endl;
		JobRef			job = std::make_shared<Job>(callback);
		GThreadManager->InsertJob(job);
	}
}